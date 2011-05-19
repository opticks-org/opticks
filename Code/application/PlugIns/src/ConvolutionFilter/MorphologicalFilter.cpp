/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "BitMask.h"
#include "Layer.h"
#include "LayerList.h"
#include "MorphologicalFilter.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "SpatialDataView.h"
#include <opencv2/opencv.hpp>

REGISTER_PLUGIN_BASIC(OpticksConvolutionFilter, Dilation);
REGISTER_PLUGIN_BASIC(OpticksConvolutionFilter, Erosion);
REGISTER_PLUGIN_BASIC(OpticksConvolutionFilter, Open);
REGISTER_PLUGIN_BASIC(OpticksConvolutionFilter, Close);

MorphologicalFilter::MorphologicalFilter(const std::string& opName) : mpProgress(NULL)
{
   setName("Morphological " + opName);
   setSubtype("Morphological Filter");
   setDescription("Perform morphological " + opName + " on an AOI.");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setAbortSupported(true);
   setMenuLocation("[General Algorithms]/AOI Morphology/" + opName);
}

MorphologicalFilter::~MorphologicalFilter()
{
}

bool MorphologicalFilter::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   VERIFY(pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL,
      Executable::ProgressArgDescription()));
   VERIFY(pInArgList->addArg<SpatialDataView>(Executable::ViewArg(), NULL,
      "If \"" + Executable::DataElementArg() + "\" is not specified, this is used to locate the active AOI layer, " \
      "otherwise this argument is ignored."));
   VERIFY(pInArgList->addArg<AoiElement>(Executable::DataElementArg(), NULL, 
      "The AOI to perform the operation on"));
   return true;
}

bool MorphologicalFilter::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

bool MorphologicalFilter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   VERIFY(pInArgList);
   mpProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   SpatialDataView* pView = pInArgList->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
   AoiElement* pAoi = pInArgList->getPlugInArgValue<AoiElement>(Executable::DataElementArg());
   if (pAoi == NULL && pView != NULL)
   {
      Layer* pLayer = pView->getActiveLayer();
      if (pLayer == NULL)
      {
         std::vector<Layer*> layers;
         pView->getLayerList()->getLayers(AOI_LAYER, layers);
         if (!layers.empty())
         {
            pLayer = layers.front();
         }
      }
      pAoi = pLayer == NULL ? NULL : dynamic_cast<AoiElement*>(pLayer->getDataElement());
   }
   if (pAoi == NULL)
   {
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("No AOI specified.", 0, ERRORS);
      }
      return false;
   }
   const BitMask* pData = pAoi->getSelectedPoints();
   if (pData->isOutsideSelected())
   {
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress("Infinite AOIs can not be processed.", 0, ERRORS);
      }
      return false;
   }
   int x1 = 0;
   int x2 = 0;
   int y1 = 0;
   int y2 = 0;
   int x,y;
   pData->getMinimalBoundingBox(x1, y1, x2, y2);
   if (x1 > x2)
   {
      std::swap(x1, x2);
   }
   if (y1 > y2)
   {
      std::swap(y1, y2);
   }
   x1--;
   x2++;
   y1--;
   y2++;
   try
   {
      int matWidth = x2-x1+1;
      int matHeight = y2-y1+1;
      mData = cv::Mat(matHeight, matWidth, CV_8U, cv::Scalar(0));
      for (y = y1; y <= y2; ++y)
      {
         if (isAborted())
         {
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress("User cancelled operation.", 0, ABORT);
            }
            return false;
         }
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress("Copying data", 30*(y-y1) / matHeight, NORMAL);
         }
         for (x = x1; x <= x2; ++x)
         {
            if (pData->getPixel(x, y))
            {
               mData.at<unsigned char>(y - y1, x - x1) = 255;
            }
         }
      }

      if (!process())
      {
         return false;
      }

      FactoryResource<BitMask> pOutData;
      // Set the corners to preallocate memory for the BitMask
      // Then clear them in case those pixels are not on
      pOutData->setPixel(x1 + 1, y1 + 1, true);
      pOutData->setPixel(x2 - 1, y2 - 1, true);
      pOutData->setPixel(x1 + 1, y1 + 1, false);
      pOutData->setPixel(x2 - 1, y2 - 1, false);
      for (y = y1 + 1; y <= y2 - 1; ++y) // ignore the border we added
      {
         if (isAborted())
         {
            if (mpProgress != NULL)
            {
               mpProgress->updateProgress("User cancelled operation.", 0, ABORT);
            }
            return false;
         }
         if (mpProgress != NULL)
         {
            mpProgress->updateProgress("Copying data", 60 + (39 * (y-y1) / matHeight), NORMAL);
         }
         for (x = x1 + 1; x <= x2 - 1; ++x) // ignore the border we added
         {
            if (mData.at<unsigned char>(y - y1, x - x1) > 0)
            {
               pOutData->setPixel(x, y, true);
            }
         }
      }
      pAoi->clearPoints();
      pAoi->addPoints(pOutData.get());
   }
   catch(const cv::Exception& exc)
   {
      if (mpProgress != NULL)
      {
         mpProgress->updateProgress(exc.what(), 0, ERRORS);
      }
      return false;
   }

   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Morphological processing complete", 100, NORMAL);
   }
   return true;
}

Dilation::Dilation() : MorphologicalFilter("Dilation")
{
   setDescriptorId("{4c21305c-1b56-477e-9dcd-50331d80a227}");
}

Dilation::~Dilation()
{
}

bool Dilation::process()
{
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Calculating dilation", 40, NORMAL);
   }
   cv::dilate(mData, mData, cv::Mat());
   return true;
}

Erosion::Erosion() : MorphologicalFilter("Erosion")
{
   setDescriptorId("{74962745-529f-4b1e-ad09-730aab3fb559}");
}

Erosion::~Erosion()
{
}

bool Erosion::process()
{
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Calculating erosion", 40, NORMAL);
   }
   cv::erode(mData, mData, cv::Mat());
   return true;
}

Open::Open() : MorphologicalFilter("Open")
{
   setDescriptorId("{9a7b6c19-b71a-4306-9109-c299a053f12c}");
}

Open::~Open()
{
}

bool Open::process()
{
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Calculating opening", 40, NORMAL);
   }
   cv::erode(mData, mData, cv::Mat());
   cv::dilate(mData, mData, cv::Mat());
   return true;
}

Close::Close() : MorphologicalFilter("Close")
{
   setDescriptorId("{d2ca0869-02f2-4e56-beb1-1b091c57337b}");
}

Close::~Close()
{
}

bool Close::process()
{
   if (mpProgress != NULL)
   {
      mpProgress->updateProgress("Calculating close", 40, NORMAL);
   }
   cv::dilate(mData, mData, cv::Mat());
   cv::erode(mData, mData, cv::Mat());
   return true;
}
