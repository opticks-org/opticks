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
#include "PowerLine.h"
#include "ProgressTracker.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "Statistics.h"
#include "switchOnEncoding.h"
#include "ThresholdLayer.h"
#include "Undo.h"

#include <pcl/point_types.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>

REGISTER_PLUGIN_BASIC(NGAtda, PowerLine);

PowerLine::PowerLine()
{
   setCreator("Ball");
   setVersion("0.1");
   setCopyright("Copyright (C) 2015, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setName("Power Lines");
   setShortDescription("Curve fit to a power line");
   setDescriptorId("{7752F455-7E2B-4836-9A9C-32F1B5FA8FDD}");
   setMenuLocation("[Point Cloud]/Power Line");
}

PowerLine::~PowerLine()
{
}

bool PowerLine::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   pArgList->addArg<Progress>(Executable::ProgressArg(), "Object for progress reporting.");
   pArgList->addArg<PointCloudElement>(Executable::DataElementArg(), "The point cloud to process.");
   pArgList->addArg<PointCloudView>(Executable::ViewArg(), NULL, "The view displaying the point cloud. If set, the displayed data will be set to classification.");
   return true;
}

bool PowerLine::getOutputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   return true;
}

template<typename T>
void assignClass(T* pData)
{
   *pData = (T)42;
}

bool PowerLine::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }
   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()),
      "Finding power lines", "nga", getDescriptorId());

   PointCloudElement* pElement = pInArgList->getPlugInArgValue<PointCloudElement>(Executable::DataElementArg());
   if (pElement == NULL)
   {
      progress.report("A valid point cloud element must be provided.", 0, ERRORS, true);
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
   double zMin = pDesc->getZMin() * pDesc->getZScale() + pDesc->getZOffset();
   double zMax = pDesc->getZMax() * pDesc->getZScale() + pDesc->getZOffset();

   pcl::PointCloud<pcl::PointXYZ>::Ptr pCloud(new pcl::PointCloud<pcl::PointXYZ>);
   pCloud->width = (uint32_t)std::max(xMax-xMin+1, yMax-yMin+1);
   pCloud->height = (uint32_t)(zMax-zMin+1);
   pCloud->points.reserve(pDesc->getPointCount());
   for (size_t idx = 0; idx < pDesc->getPointCount(); ++idx)
   {
      if (!acc.isValid())
      {
         progress.report("Unable to access data.", 0, ERRORS, true);
         return false;
      }
      if (!acc->isPointValid())
      {
         acc->nextValidPoint();
         continue;
      }
      pCloud->points.push_back(pcl::PointXYZ((float)acc->getXAsDouble(),
                                             (float)acc->getYAsDouble(),
                                             (float)acc->getZAsDouble()));
   }

   pcl::ModelCoefficients::Ptr pCoeff(new pcl::ModelCoefficients);
   pcl::PointIndices::Ptr pInliers(new pcl::PointIndices);
   pcl::SACSegmentation<pcl::PointXYZ> seg;
   seg.setOptimizeCoefficients(true);
   seg.setModelType(pcl::SACMODEL_PLANE);
   seg.setMethodType(pcl::SAC_RANSAC);
   seg.setDistanceThreshold(10);
   seg.setInputCloud(pCloud);
   seg.segment(*pInliers, *pCoeff);
   if (pInliers->indices.empty())
   {
      progress.report("Can't estimate a planar model", 0, ERRORS, true);
      return false;
   }

   for (auto it = pInliers->indices.begin(); it != pInliers->indices.end(); ++it)
   {
      acc->toIndex(*it);
      if (acc->isPointValid())
      {
         switchOnEncoding(pDesc->getClassificationDataType(), assignClass, acc->getRawClassification());
      }
   }
#if 0
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
#endif
#if 0
   Eigen::MatrixXf classified(nDim, mDim);
   if (topo::ClassifyTopography<Eigen::MatrixXf, Eigen::MatrixXf>(dem, classified, post_spacing, kernel_size, curve_tolerance, slope_tolerance, progress) != camp::SUCCESS)
   {
      progress.report("Failure running topography classification.", 0, ERRORS, true);
      return false;
   }
   progress.nextStage();
#endif
#if 0
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
#endif

#if 0
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
#endif
   progress.report("Complete", 100, NORMAL);
   progress.upALevel();
   return true;
}
