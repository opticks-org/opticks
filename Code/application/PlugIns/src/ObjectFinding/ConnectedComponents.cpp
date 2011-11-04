/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "AppVerify.h"
#include "AppVersion.h"
#include "BitMask.h"
#include "ConnectedComponents.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "LayerList.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PseudocolorLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"

#include <opencv2/imgproc/imgproc.hpp>

REGISTER_PLUGIN_BASIC(OpticksObjectFinding, ConnectedComponents);

namespace
{
   void fillContours(cv::Mat labels,
                     const std::vector<std::vector<cv::Point> >& contours,
                     const std::vector<cv::Vec4i>& hierarchy,
                     unsigned short& label,
                     unsigned short parentLabel, // or 0 for an external
                     int contourIdx)
   {
      if (static_cast<int>(hierarchy.size()) <= contourIdx)
      {
         return;
      }
      // Loop all contours at this level of the tree
      while (contourIdx != -1)
      {
         if (parentLabel > 0)
         {
            // Internal contours include the edge of object, not the edge of the hole
            // So we blank out the internal contour then redraw the contour line
            cv::drawContours(labels, contours, contourIdx, cv::Scalar(0), CV_FILLED, 8, hierarchy, 0);
            cv::drawContours(labels, contours, contourIdx, cv::Scalar(parentLabel), 1, 8, hierarchy, 0);
            if (hierarchy[contourIdx][2] != -1)
            {
               // If there are nested contours, draw them as well
               fillContours(labels, contours, hierarchy, label, 0, hierarchy[contourIdx][2]);
            }
         }
         else
         {
            // Fill the entire contour poly
            cv::drawContours(labels, contours, contourIdx, cv::Scalar(++label), CV_FILLED, 8, hierarchy, 0);
            if (hierarchy[contourIdx][2] != -1)
            {
               // If there are nested contours, draw them as well
               fillContours(labels, contours, hierarchy, label, label, hierarchy[contourIdx][2]);
            }
         }
         contourIdx = hierarchy[contourIdx][0];
      }
   }
}

ConnectedComponents::ConnectedComponents() : mpView(NULL), mpLabels(NULL), mXOffset(0), mYOffset(0)
{
   setName("Connected Components");
   setDescription("Label connected components in an AOI.");
   setDescriptorId("{0535c0ef-6d3f-413e-a2f4-563aa2fe39d9}");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setAbortSupported(true);
   setMenuLocation("[General Algorithms]/Connected Components");
}

ConnectedComponents::~ConnectedComponents()
{
}

bool ConnectedComponents::getInputSpecification(PlugInArgList*& pInArgList)
{
   pInArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pInArgList != NULL);
   VERIFY(pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, Executable::ProgressArgDescription()));
   VERIFY(pInArgList->addArg<AoiElement>("AOI", "AOI where connected components will be labeled."));
   VERIFY(pInArgList->addArg<SpatialDataView>(Executable::ViewArg(), NULL,
      "View where the results pseudocolor layer will be displayed"));
   return true;
}

bool ConnectedComponents::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pOutArgList != NULL);
   VERIFY(pOutArgList->addArg<unsigned int>("Number of Blobs",
      "The number of blobs found after removing blobs which don't meet the minimum size."));
   VERIFY(pOutArgList->addArg<RasterElement>("Blobs",
      "Labeled blobs with 0 indicating no blob. In interactive mode, a pseudocolor layer will "
      "be created with this element."));
   return true;
}

bool ConnectedComponents::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (pInArgList == NULL)
   {
      return false;
   }
   mProgress = ProgressTracker(pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg()),
      "Labeling connected components", "app", "{aa2169d0-9c0a-4d41-9f1d-9a9e83ecf32b}");
   mpView = pInArgList->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
   AoiElement* pAoi = pInArgList->getPlugInArgValue<AoiElement>("AOI");
   if (pAoi == NULL && mpView != NULL)
   {
      Layer* pLayer = mpView->getActiveLayer();
      if (pLayer == NULL)
      {
         std::vector<Layer*> layers;
         mpView->getLayerList()->getLayers(AOI_LAYER, layers);
         if (!layers.empty())
         {
            pLayer = layers.front();
         }
      }
      pAoi = pLayer == NULL ? NULL : dynamic_cast<AoiElement*>(pLayer->getDataElement());
   }
   const BitMask* mpBitmask = (pAoi == NULL) ? NULL : pAoi->getSelectedPoints();
   if (mpBitmask == NULL)
   {
      mProgress.report("Must specify an AOI.", 0, ERRORS, true);
      return false;
   }
   if (mpBitmask->isOutsideSelected())
   {
      mProgress.report("Infinite AOIs can not be processed.", 0, ERRORS, true);
      return false;
   }

   // Get the extents and create the output element
   int x1 = 0;
   int x2 = 0;
   int y1 = 0;
   int y2 = 0;
   mpBitmask->getMinimalBoundingBox(x1, y1, x2, y2);
   if (x1 > x2)
   {
      std::swap(x1, x2);
   }
   if (y1 > y2)
   {
      std::swap(y1, y2);
   }
   if (x1 < 0 || y1 < 0)
   {
      mProgress.report("Negative pixel locations are not supported and will be ignored.", 1, WARNING, true);
      x1 = std::max(x1, 0);
      y1 = std::max(y1, 0);
      x2 = std::max(x2, 0);
      y2 = std::max(y2, 0);
   }
   // Include a 1 pixel border so we include the edge pixels
   x1--;
   x2++;
   y1--;
   y2++;
   unsigned int width = x2 - x1 + 1;
   unsigned int height = y2 - y1 + 1;

   mXOffset = x1;
   mYOffset = y1;

   mpLabels = static_cast<RasterElement*>(
      Service<ModelServices>()->getElement("Blobs", TypeConverter::toString<RasterElement>(), pAoi));
   if (mpLabels != NULL)
   {
      if (!isBatch())
      {
         Service<DesktopServices>()->showSuppressibleMsgDlg(
            getName(), "The \"Blobs\" element exists and will be deleted.", MESSAGE_INFO,
            "ConnectedComponents::DeleteExisting");
      }
      Service<ModelServices>()->destroyElement(mpLabels);
      mpLabels = NULL;
   }
   mpLabels = RasterUtilities::createRasterElement("Blobs", height, width, INT2UBYTES, true, pAoi);
   if (mpLabels == NULL)
   {
      mProgress.report("Unable to create label element.", 0, ERRORS, true);
      return false;
   }
   ModelResource<RasterElement> pLabels(mpLabels);

   try
   {
      cv::Mat data(height, width, CV_8UC1, cv::Scalar(0));
      for (unsigned int y = 0; y < height; ++y)
      {
         mProgress.report("Reading AOI data", 10 * y / height, NORMAL);
         for (unsigned int x = 0; x < width; ++x)
         {
            if (mpBitmask->getPixel(x + mXOffset, y + mYOffset))
            {
               data.at<unsigned char>(y, x) = 255;
            }
         }
      }
      mProgress.report("Finding contours", 15, NORMAL);
      std::vector<std::vector<cv::Point> > contours;
      std::vector<cv::Vec4i> hierarchy;
      cv::findContours(data, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
      cv::Mat labels(height, width, CV_16UC1, mpLabels->getRawData());
      mProgress.report("Filling blobs", 50, NORMAL);
      unsigned short lastLabel = 0;
      fillContours(labels, contours, hierarchy, lastLabel, 0, 0);

      // create a pseudocolor layer for display
      mProgress.report("Displaying results", 90, NORMAL);
      mpLabels->updateData();
      if (!createPseudocolor(lastLabel))
      {
         mProgress.report("Unable to create blob layer", 0, ERRORS, true);
         return false;
      }

      // add blob count to the metadata
      DynamicObject* pMeta = pLabels->getMetadata();
      VERIFY(pMeta);
      unsigned int numBlobs = static_cast<unsigned int>(lastLabel);
      pMeta->setAttribute("BlobCount", numBlobs);
      if (numBlobs == 0 && !isBatch())
      {
         // Inform the user that there were no blobs so they don't think there was an
         // error running the algorithm. No need to do this in batch since this is
         // represented in the metadata already.
         mProgress.report("No blobs were found.", 95, WARNING);
      }
      // update the output arg list
      if (pOutArgList != NULL)
      {
         pOutArgList->setPlugInArgValue("Blobs", pLabels.get());
         pOutArgList->setPlugInArgValue("Number of Blobs", &numBlobs);
      }
   }
   catch(const cv::Exception& exc)
   {
      mProgress.report(exc.what(), 0, ERRORS, true);
      return false;
   }

   pLabels.release();
   mProgress.report("Labeling connected components", 100, NORMAL);
   mProgress.upALevel();
   return true;
}

bool ConnectedComponents::createPseudocolor(unsigned short maxLabel) const
{
   if (isBatch() || mpView == NULL)
   {
      return true;
   }
   if (maxLabel > 50)
   {
      mProgress.report("More than 50 blobs exist, colors will be repeated.", 100, WARNING, true);
   }
   PseudocolorLayer* pOutLayer = static_cast<PseudocolorLayer*>(
      mpView->createLayer(PSEUDOCOLOR, mpLabels));
   VERIFY(pOutLayer);
   pOutLayer->setXOffset(mXOffset);
   pOutLayer->setYOffset(mYOffset);
   if (pOutLayer != NULL)
   {
      std::vector<ColorType> colors;
      std::vector<ColorType> excluded;
      excluded.push_back(ColorType(0, 0, 0));
      excluded.push_back(ColorType(255, 255, 255));
      ColorType::getUniqueColors(std::min<unsigned short>(maxLabel, 50), colors, excluded);
      for (int cl = 1; cl <= maxLabel; ++cl)
      {
         pOutLayer->addInitializedClass(StringUtilities::toDisplayString(cl), cl, colors[(cl - 1) % 50]);
      }
   }
   return true;
}