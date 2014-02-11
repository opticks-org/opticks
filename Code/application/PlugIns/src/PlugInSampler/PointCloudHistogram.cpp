/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "ObjectFactory.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PointCloudAccessor.h"
#include "PointCloudAccessorImpl.h"
#include "PointCloudDataDescriptor.h"
#include "PointCloudDataRequest.h"
#include "PointCloudElement.h"
#include "PointCloudHistogram.h"
#include "PointCloudView.h"
#include "ProgressTracker.h"
#include "switchOnEncoding.h"
#include <limits>

REGISTER_PLUGIN_BASIC(OpticksPlugInSampler, PointCloudHistogram);

namespace
{
template<typename T>
void assign(T* pData, int bin)
{
   *pData = static_cast<T>(bin);
}
}

PointCloudHistogram::PointCloudHistogram()
{
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2013, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setName("Point Cloud Histogram");
   setDescription("Sample plug-in which calculates a histogram of intensity values in a point cloud and sets the resuls in the classification data field.");
   setDescriptorId("{88063BE2-BDBE-4649-9D26-9902236ECC9C}");
   setMenuLocation("[Demo]/Point Cloud/Histogram");
}

PointCloudHistogram::~PointCloudHistogram()
{
}

bool PointCloudHistogram::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   pArgList->addArg<Progress>(Executable::ProgressArg(), "Object for progress reporting.");
   pArgList->addArg<PointCloudElement>(Executable::DataElementArg(), "The point cloud to process.");
   pArgList->addArg<PointCloudView>(Executable::ViewArg(), NULL, "The view displaying the point cloud. If set, the displayed data will be set to aux 1.");
   return true;
}

bool PointCloudHistogram::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool PointCloudHistogram::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }
   ProgressTracker progress(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()),
      "Calculating point cloud histogram", "sample", "point cloud histogram");

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
   double minI = std::numeric_limits<double>::max();
   double maxI = -minI;
   int prog = 0;
   const uint32_t adv = pDesc->getPointCount() / 50;
   for (size_t idx = 0; idx < pDesc->getPointCount(); ++idx)
   {
      if (!acc.isValid())
      {
         progress.report("Unable to access data.", 0, ERRORS, true);
         return false;
      }
      if (idx % adv == 0)
      {
         progress.report("Calculating extents", ++prog, NORMAL);
      }
      if (!acc->isPointValid())
      {
         acc->nextValidPoint();
         continue;
      }
      double i = acc->getIntensityAsDouble();
      minI = std::min(minI, i);
      maxI = std::max(maxI, i);
      acc->nextValidPoint();
   }
   progress.report("Calculating histogram", 50, NORMAL);
   acc->toIndex(0);
   EncodingType typ = pDesc->getClassificationDataType();
   maxI -= minI;
   for (size_t idx = 0; idx < pDesc->getPointCount(); ++idx)
   {
      if (!acc.isValid())
      {
         progress.report("Unable to access data.", 0, ERRORS, true);
         return false;
      }
      if (idx % adv == 0)
      {
         progress.report("Calculating extents", ++prog, NORMAL);
      }
      if (!acc->isPointValid())
      {
         acc->nextValidPoint();
         continue;
      }
      double i = acc->getIntensityAsDouble();
      i -= minI;
      i /= maxI;
      int bin = static_cast<int>(i / (1. / 255.));
      bin = std::min(std::max(bin,0),255); // clamp in case of rounding error
      switchOnEncoding(typ, assign, acc->getRawClassification(), bin);
      acc->nextValidPoint();
   }
   pElement->updateData(PointCloudElement::UPDATE_CLASSIFICATION);
   PointCloudView* pView = pInArgList->getPlugInArgValue<PointCloudView>(Executable::ViewArg());
   if (pView != NULL)
   {
      pView->setPointColorizationType(POINT_CLASSIFICATION);
   }
   progress.report("Complete", 100, NORMAL);
   progress.upALevel();
   return true;
}
