/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DesktopServices.h"
#include "Hydrology.h"
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
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Statistics.h"
#include "ThresholdLayer.h"
#include "Undo.h"
#include "topographic_attributes.hpp"

#include <Eigen/Core>
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/gdal_alg.h>

REGISTER_PLUGIN(NGAtda, NGA_Hydrology, NGA::Hydrology);

NGA::Hydrology::Hydrology()
{
   setCreator("NGA");
   setVersion("0.1");
   setCopyright("Copyright (C) 2014, Ball Aerospace & Technologies Corp. & Copyright (C) 2011, ITT Geospatial Systems and Booz Allen Hamilton");
   setProductionStatus(false);
   setName("NGA Hydrology");
   setShortDescription("Calculate flood risk from a LIDAR point cloud.");
   setDescription("This plugin generates a DEM from a LIDAR point cloud and then calculates flood risk areas and classifies that data based on risk.");
   setDescriptorId("{385D3402-FF61-4858-8208-5215AB1C697C}");
   setMenuLocation("[Point Cloud]/NGA TDAs/Hydrology");
}

NGA::Hydrology::~Hydrology()
{
}

bool NGA::Hydrology::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   pArgList->addArg<Progress>(Executable::ProgressArg(), "Object for progress reporting.");
   pArgList->addArg<PointCloudElement>(Executable::DataElementArg(), "The point cloud to process.");
   pArgList->addArg<PointCloudView>(Executable::ViewArg(), NULL, "The view displaying the point cloud. If set, the displayed data will be set to classification.");
   static const float default_curve_tolerance = 0.5f;
   pArgList->addArg<float>("Curve Tolerance", &default_curve_tolerance, "The tolerance for curvature measurements.");
   static const float default_slope_tolerance = 0.175f;
   pArgList->addArg<float>("Slope Tolerance", &default_slope_tolerance, "The tolerance for slope measurements.");
   static const float default_post_spacing = 1.f;
   pArgList->addArg<float>("Post Spacing", &default_post_spacing, "The resolution of the flood output data.");
   static const int default_kernel_size = 3;
   pArgList->addArg<int>("Kernel Size", &default_kernel_size, "The window size used to generate the topography, in square pixels.");
   static const float default_buffer_size = 1.f;
   pArgList->addArg<float>("Buffer Size", &default_buffer_size, "The buffer zone to place around hydro risk areas.");
   return true;
}

bool NGA::Hydrology::getOutputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   return true;
}

bool NGA::Hydrology::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }
   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()),
      "Calculating flood risk", "nga", getDescriptorId());
   std::vector<ProgressTracker::Stage> stages;
   stages.push_back(ProgressTracker::Stage("Generating DEM", "nga", "hydrology1", 2));
   stages.push_back(ProgressTracker::Stage("Classifying DEM", "nga", "hydrology2", 4));
   stages.push_back(ProgressTracker::Stage("Calclating flood areas", "nga", "hydrology3", 1));
   progress.subdivideCurrentStage(stages);

   PointCloudElement* pElement = pInArgList->getPlugInArgValue<PointCloudElement>(Executable::DataElementArg());
   if (pElement == NULL)
   {
      progress.report("A valid point cloud element must be provided.", 0, ERRORS, true);
      return false;
   }

   float curve_tolerance, slope_tolerance, post_spacing, buffer_size;
   int kernel_size;
   if (!pInArgList->getPlugInArgValue("Curve Tolerance", curve_tolerance) ||
       !pInArgList->getPlugInArgValue("Slope Tolerance", slope_tolerance) ||
       !pInArgList->getPlugInArgValue("Post Spacing", post_spacing) ||
       !pInArgList->getPlugInArgValue("Kernel Size", kernel_size) ||
       !pInArgList->getPlugInArgValue("Buffer Size", buffer_size))
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
   const uint32_t adv = pDesc->getPointCount() / 100;
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
   //Eigen::MatrixXf classified(nDim, mDim);
   Eigen::MatrixXf classified(nDim, mDim);
   if (topo::ClassifyTopography<Eigen::MatrixXf, Eigen::MatrixXf>(dem, classified, post_spacing, kernel_size, curve_tolerance, slope_tolerance, progress) != camp::SUCCESS)
   {
      progress.report("Failure running topography classification.", 0, ERRORS, true);
      return false;
   }
   progress.nextStage();
   for (int row = 0; row < classified.rows(); row++)
   {
      progress.report("Locating flood pixels", row * 80 / 100, NORMAL);
      for (int col = 0; col < classified.cols(); col++)
      {
         if (dem(row, col) == badVal)
         {
            classified(row, col) = badVal;
         }
         else if (classified(row, col) == 5 || classified(row, col) == 6)
         {
            classified(row, col) = 1;
         }
         else
         {
            classified(row, col) = 0;
         }
      }
   }
   progress.report("Filtering result", 90, NORMAL);
   GDALAllRegister();
   GDALDriverH drv = GDALGetDriverByName("MEM");
   GDALDatasetH ds = GDALCreate(drv, "", mDim, nDim, 1, GDT_Float32, NULL);
   GDALRasterBandH band = GDALGetRasterBand(ds, 1);
   GDALSetRasterNoDataValue(band, badVal);
   Eigen::MatrixXf classT = classified.transpose();
   GDALRasterIO(band, GF_Write, 0, 0, mDim, nDim, classT.data(), mDim, nDim, GDT_Float32, 0, 0);
   GDALSieveFilter(band, NULL, band, 20, 8, NULL, NULL, NULL);
   RasterElement* pClassOut = RasterUtilities::createRasterElement("Class", nDim, mDim, FLT4BYTES, true, pElement);
   if (pClassOut == NULL)
   {
      progress.report("Unable to create DEM raster.", 0, ERRORS, true);
      return false;
   }
   pClassOut->getStatistics()->setBadValues(std::vector<int>(1, (int)badVal));
   GDALRasterIO(band, GF_Read, 0, 0, mDim, nDim, pClassOut->getRawData(), mDim, nDim, GDT_Float32, 0, 0);
   GDALClose(ds);

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

      SpatialDataView* pView = ((SpatialDataWindow*)Service<DesktopServices>()->createWindow("Class", SPATIAL_DATA_WINDOW))->getSpatialDataView();
      pView->setPrimaryRasterElement(pDemOut);
      {
         UndoLock lock(pView);
         pView->createLayer(RASTER, pDemOut);
         ThresholdLayer* pThresh = static_cast<ThresholdLayer*>(pView->createLayer(THRESHOLD, pClassOut));
         pThresh->setRegionUnits(RAW_VALUE);
         pThresh->setFirstThreshold(0.1);
         pThresh->setPassArea(UPPER);
      }
   }
   progress.report("Complete", 100, NORMAL);
   progress.upALevel();
   return true;
}
