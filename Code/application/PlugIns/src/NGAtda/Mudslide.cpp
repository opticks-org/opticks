/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DesktopServices.h"
#include "Mudslide.h"
#include "ObjectFactory.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PointCloudAccessor.h"
#include "PointCloudAccessorImpl.h"
#include "PointCloudDataDescriptor.h"
#include "PointCloudDataRequest.h"
#include "PointCloudElement.h"
#include "PointCloudView.h"
#include "ProgressTracker.h"
#include "PseudocolorLayer.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Statistics.h"
#include "switchOnEncoding.h"
#include "Undo.h"
#include "mudslide.hpp"

#include <Eigen/Core>
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/gdal_alg.h>

#include "mudslide.hpp"
#include "topographic_attributes.hpp"

REGISTER_PLUGIN(NGAtda, NGA_Mudslide, NGA::Mudslide);

namespace
{
template<typename T>
void assign(T* pData, float val)
{
   *pData = static_cast<T>(val);
}
}

NGA::Mudslide::Mudslide()
{
   setCreator("NGA");
   setVersion("0.1");
   setCopyright("Copyright (C) 2014, Ball Aerospace & Technologies Corp. & Copyright (C) 2011, ITT Geospatial Systems and Booz Allen Hamilton");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setName("NGA Mudslide");
   setShortDescription("Calculate mudslide risk from a LIDAR point cloud.");
   setDescription("This plugin generates a DEM from a LIDAR point cloud and then calculates mudslide risk areas and classifies that data based on risk.");
   setDescriptorId("{A4E70D52-EE18-49AB-84F0-3EE623EB9DE0}");
   setMenuLocation("[Point Cloud]/NGA TDAs/Mudslide");
}

NGA::Mudslide::~Mudslide()
{
}

bool NGA::Mudslide::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   pArgList->addArg<Progress>(Executable::ProgressArg(), "Object for progress reporting.");
   pArgList->addArg<PointCloudElement>(Executable::DataElementArg(), "The point cloud to process.");
   pArgList->addArg<PointCloudView>(Executable::ViewArg(), NULL, "The view displaying the point cloud. If set, the displayed data will be set to classification.");
   static const float default_post_spacing = 1.f;
   pArgList->addArg<float>("Post Spacing", &default_post_spacing, "The resolution of the flood output data.");
   return true;
}

bool NGA::Mudslide::getOutputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   return true;
}

bool NGA::Mudslide::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }
   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()),
      "Calculating mudslide risk", "nga", getDescriptorId());
   std::vector<ProgressTracker::Stage> stages;
   stages.push_back(ProgressTracker::Stage("Generating DEM", "nga", "mudslide1", 1));
   stages.push_back(ProgressTracker::Stage("Classifying DEM", "nga", "mudslide2", 2));
   stages.push_back(ProgressTracker::Stage("Generating result", "nga", "mudslide3", 1));
   progress.subdivideCurrentStage(stages);

   PointCloudElement* pElement = pInArgList->getPlugInArgValue<PointCloudElement>(Executable::DataElementArg());
   if (pElement == NULL)
   {
      progress.report("A valid point cloud element must be provided.", 0, ERRORS, true);
      return false;
   }

   float post_spacing;
   if (!pInArgList->getPlugInArgValue("Post Spacing", post_spacing))
   {
      progress.report("Invalid or unspecified algorithm parameters.", 0, ERRORS, true);
      return false;
   }

   /* Main processing loop */
   FactoryResource<PointCloudDataRequest> req;
   req->setWritable(true);
   PointCloudAccessor acc(pElement->getPointCloudAccessor(req.release()));
   if (!acc.isValid())
   {
      progress.report("Unable to write to point cloud.", 0, ERRORS, true);
      return false;
   }
   const PointCloudDataDescriptor* pDesc = static_cast<const PointCloudDataDescriptor*>(pElement->getDataDescriptor());
   double xMin = pDesc->getXMin() * pDesc->getXScale() + pDesc->getXOffset();
   double xMax = pDesc->getXMax() * pDesc->getXScale() + pDesc->getXOffset();
   double yMin = pDesc->getYMin() * pDesc->getYScale() + pDesc->getYOffset();
   double yMax = pDesc->getYMax() * pDesc->getYScale() + pDesc->getYOffset();

   int mDim = static_cast<int>(std::ceil((xMax - xMin) / post_spacing));
   int nDim = static_cast<int>(std::ceil((yMax - yMin) / post_spacing));
   xMax = xMin + mDim * post_spacing;
   yMin = yMax - nDim * post_spacing;

   // create an output raster for the DEM
   Eigen::MatrixXf dem;
   const float badVal = -9999.f;
   dem.setConstant(nDim, mDim, badVal);

   int prog = 0;
   uint32_t adv = pDesc->getPointCount() / 100;
   for (size_t idx = 0; idx < pDesc->getPointCount(); ++idx)
   {
      if (!acc.isValid())
      {
         progress.report("Unable to access data.", 0, ERRORS, true);
         return false;
      }
      if (idx % adv == 0)
      {
         progress.report("Generating DEM", ++prog, NORMAL);
      }
      if (!acc->isPointValid())
      {
         acc->nextValidPoint();
         continue;
      }
      double x = acc->getXAsDouble(true);
      double y = acc->getYAsDouble(true);
      float z = static_cast<float>(acc->getZAsDouble(true));
      // calculate nearest DEM point
      int xIndex = std::max(0, static_cast<int>(std::floor((x - xMin) / post_spacing)));
      int yIndex = std::max(0, static_cast<int>(std::floor((yMax - y) / post_spacing)));
      float demVal = dem(yIndex, xIndex);
      if (demVal == badVal || demVal < z)
      {
         dem(yIndex, xIndex) = z;
      }

      acc->nextValidPoint();
   }
   progress.nextStage();

   Eigen::MatrixXf slope_raster(nDim, mDim);
   Eigen::MatrixXf curve_raster(nDim, mDim);

   progress.report("Computing slope", 5, NORMAL);
   topo::ComputeSlope<Eigen::MatrixXf>(dem, slope_raster, post_spacing);
   progress.report("Computing curvature", 40, NORMAL);
   topo::ComputeCurvature<Eigen::MatrixXf>(dem, curve_raster, post_spacing, camp::PROFILE, 5);
   progress.report("Computing risk", 80, NORMAL);
   Eigen::MatrixXf risk(nDim, mDim);
   mudslide::Mudslide<Eigen::MatrixXf> msalg;
   msalg.computeRiskRaster(slope_raster, curve_raster, dem, nDim, mDim, risk);
   progress.nextStage();

   progress.report("Filtering result", 10, NORMAL);
   GDALAllRegister();
   GDALDriverH drv = GDALGetDriverByName("MEM");
   GDALDatasetH ds = GDALCreate(drv, "", mDim, nDim, 1, GDT_Float32, NULL);
   GDALRasterBandH band = GDALGetRasterBand(ds, 1);
   GDALSetRasterNoDataValue(band, badVal);
   Eigen::MatrixXf riskT = risk.transpose();
   GDALRasterIO(band, GF_Write, 0, 0, mDim, nDim, riskT.data(), mDim, nDim, GDT_Float32, 0, 0);
   GDALSieveFilter(band, NULL, band, 20, 8, NULL, NULL, NULL);

   RasterElement* pRiskOut = RasterUtilities::createRasterElement("Risk", nDim, mDim, FLT4BYTES, true, pElement);
   if (pRiskOut == NULL)
   {
      progress.report("Unable to create risk raster.", 0, ERRORS, true);
      return false;
   }
   pRiskOut->getStatistics()->setBadValues(std::vector<int>(1, (int)badVal));
   GDALRasterIO(band, GF_Read, 0, 0, mDim, nDim, pRiskOut->getRawData(), mDim, nDim, GDT_Float32, 0, 0);
   GDALClose(ds);

   /// push back to point cloud
   prog = 10;
   adv = pDesc->getPointCount() / 90;
   acc->toIndex(0);
   EncodingType typ = pDesc->getClassificationDataType();
   for (size_t idx = 0; idx < pDesc->getPointCount(); ++idx)
   {
      if (!acc.isValid())
      {
         progress.report("Unable to access data.", 0, ERRORS, true);
         return false;
      }
      if (idx % adv == 0)
      {
         progress.report("Generating DEM", ++prog, NORMAL);
      }
      if (!acc->isPointValid())
      {
         acc->nextValidPoint();
         continue;
      }
      double x = acc->getXAsDouble(true);
      double y = acc->getYAsDouble(true);
      float z = static_cast<float>(acc->getZAsDouble(true));
      // calculate nearest DEM point
      int xIndex = std::max(0, static_cast<int>(std::floor((x - xMin) / post_spacing)));
      int yIndex = std::max(0, static_cast<int>(std::floor((yMax - y) / post_spacing)));
      float riskVal = risk(yIndex, xIndex);
      switchOnEncoding(typ, assign, acc->getRawClassification(), riskVal);
      acc->nextValidPoint();
   }
   pElement->updateData(PointCloudElement::UPDATE_CLASSIFICATION);

   // Display rasters
   if (!isBatch())
   {
      progress.report("Displaying result", 95, NORMAL);
      RasterElement* pDemOut = RasterUtilities::createRasterElement("DEM", nDim, mDim, FLT4BYTES, true, pElement);
      if (pDemOut == NULL)
      {
         progress.report("Unable to create DEM raster.", 0, ERRORS, true);
         return false;
      }
      pDemOut->getStatistics()->setBadValues(std::vector<int>(1, (int)badVal));
      FactoryResource<DataRequest> pReq;
      pReq->setWritable(true);
      DataAccessor racc(pDemOut->getDataAccessor(pReq.release()));
      for (int row = 0; row < nDim; row++)
      {
         for (int col = 0; col < mDim; col++)
         {
            if (!racc.isValid())
            {
               progress.report("Error writing output raster.", 0, ERRORS, true);
               return false;
            }
            *reinterpret_cast<float*>(racc->getColumn()) = dem(row, col);
            racc->nextColumn();
         }
         racc->nextRow();
      }
      pDemOut->updateData();

      SpatialDataView* pView = ((SpatialDataWindow*)Service<DesktopServices>()->createWindow("Risk", SPATIAL_DATA_WINDOW))->getSpatialDataView();
      pView->setPrimaryRasterElement(pDemOut);
      {
         UndoLock lock(pView);
         pView->createLayer(RASTER, pDemOut);
         PseudocolorLayer* pRiskLayer = static_cast<PseudocolorLayer*>(pView->createLayer(PSEUDOCOLOR, pRiskOut));
         pRiskLayer->addInitializedClass("High Risk", 3, ColorType(255, 0, 0));
         pRiskLayer->addInitializedClass("Medium Risk", 2, ColorType(255, 0, 255));
         pRiskLayer->addInitializedClass("Low Risk", 1, ColorType(0, 255, 0), false);
      }
   }
   progress.report("Complete", 100, NORMAL);
   progress.upALevel();
   return true;
}
