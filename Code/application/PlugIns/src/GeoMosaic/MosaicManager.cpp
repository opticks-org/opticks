/**
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Animation.h"
#include "AnimationFrame.h"
#include "AnimationController.h"
#include "AnimationServices.h"
#include "AnimationToolBar.h"
#include "AppVersion.h"
#include "DateTime.h"
#include "DesktopServices.h"
#include "LayerList.h"
#include "MosaicManager.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "SpecialMetadata.h"
#include "UtilityServices.h"

#include <vector>

REGISTER_PLUGIN_BASIC(OpticksGeoMosaic, MosaicManager);

namespace
{

   class LayerTimeSorter
   {
   public:
      bool operator()(std::pair<Layer*, double> lhs, std::pair<Layer*, double> rhs)
      {
         if (lhs.first == rhs.first)
         {
            return false;
         }
         if (lhs.first == NULL || rhs.first == NULL)
         {
            return true;
         }
         return lhs.second < rhs.second;
      }
   };

   class LayerGeoSorter
   {
   public:
      bool operator()(std::pair<Layer*, double> lhs, std::pair<Layer*, double> rhs)
      {
         if (lhs.first == rhs.first)
         {
            return false;
         }
         if (lhs.first == NULL || rhs.first == NULL)
         {
            return true;
         }
         Layer* pLhs = lhs.first;
         Layer* pRhs = rhs.first;
         if (pLhs->getXOffset() == pRhs->getXOffset())
         {
            return pLhs->getYOffset() < pRhs->getYOffset();
         }
         else
         {
            return pLhs->getXOffset() < pRhs->getXOffset();
         }
      }
   };
};

MosaicManager::MosaicManager()
   : mpView(NULL),
   mpData(NULL),
   mTimeBased(false)
{
   setName("Mosaic Manager");
   setDescription("Singleton plug-in to manage the Mosaic data type.");
   setType("Manager");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescriptorId("{D8F8F7FC-E39E-444B-B8D8-E29BFD3F8357}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   allowMultipleInstances(false);
   executeOnStartup(true);
   destroyAfterExecute(false);
   setWizardSupported(false);
}

MosaicManager::~MosaicManager()
{
   delete mpData;
}

bool MosaicManager::getInputSpecification(PlugInArgList*& pInArgList)
{
   pInArgList = NULL;
   return true;
}

bool MosaicManager::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = NULL;
   return true;
}

bool MosaicManager::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   return true;
}

void MosaicManager::layerDeleted(Subject& subject, const std::string& signal, const boost::any& value)
{
   Layer* pLayer = dynamic_cast<Layer*>(&subject);
   if (pLayer == NULL)
   {
      return;
   }
   for (unsigned int i = 0; i < mLayers.size(); ++i)
   {
      if (mLayers[i].first == pLayer)
      {
         mLayers[i].first = NULL;
         break;
      }
   }
}

void MosaicManager::changeFrame(Subject& subject, const std::string& signalName, const boost::any& data)
{
   if (mpData == NULL || mpView.get() == NULL)
   {
      return;
   }
   Animation* pAnimation = dynamic_cast<Animation*>(&subject);
   AnimationFrame* pFrame = boost::any_cast<AnimationFrame*>(data);
   if (pAnimation == NULL || pFrame == NULL)
   {
      return;
   }
   // show or hide based on direction
   unsigned int frameNumber = 0;
   double frameTime = 0;
   if (mTimeBased)
   {
      frameTime = pFrame->mTime;
   }
   else
   {
      frameNumber = pFrame->mFrameNumber;
   }
   if (frameNumber < 0 || frameNumber >= mLayers.size())
   {
      return;
   }
   for (unsigned int idx = 0; idx < mLayers.size(); ++idx)
   {
      double layerTime = mLayers[idx].second;
      Layer* pLayer = mLayers[idx].first;
      if (pLayer == NULL)
      {
         continue;
      }
      if ((mTimeBased && layerTime <= frameTime) ||
          (!mTimeBased && idx <= frameNumber))
      {
         mpView->showLayer(pLayer);
      }
      else
      {
         mpView->hideLayer(pLayer);
      }
      mpView->refresh();
   }
}

bool MosaicManager::geoStitch(MosaicManager::MosaicData* pData, Progress* pProgress)
{
   Service<DesktopServices> pDesktop;

   if (pData == NULL)
   {
      return false;
   }
   if (mpData != NULL)
   {
      delete mpData;
   }
   mpData = pData;
   if (mpData->mpRasters.empty())
   {
      return false;
   }
   mLayers.clear();
   if (mpView.get() != NULL)
   {
      pDesktop->deleteView(mpView.get());
   }
   SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->createWindow("GeoMosaic Window", SPATIAL_DATA_WINDOW));
   VERIFY(pWindow != NULL);
   VERIFY(pDesktop->setCurrentWorkspaceWindow(pWindow));
   mpView.reset((pWindow == NULL) ? NULL : pWindow->getSpatialDataView());
   if (mpView.get() == NULL)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Unable to create view.", 99, ERRORS);
      }
      return false;
   }
   // Make the primary raster layer reside at offset 0, 0
   RasterElement* pPrimaryElement = mpData->mpRasters[0];
   bool haveTimes = true;
   mTimeBased = false;
   for (unsigned int idx = 0; idx < mpData->mpRasters.size(); ++idx)
   {
      RasterElement* pRaster = mpData->mpRasters.at(idx);
      if (pRaster == NULL)
      {
         continue;
      }
      if (!pRaster->isGeoreferenced())
      {
         ExecutableResource georef("Georeference", "", NULL, false);
         georef->createProgressDialog(true);
         bool bSuccess = false;
         if (georef.get() != NULL)
         {
            georef->getInArgList().setPlugInArgValue(Executable::DataElementArg(), pRaster);
            bSuccess = georef->execute();
         }
         if (!bSuccess || !pRaster->isGeoreferenced())
         {
            if (pProgress != NULL)
            {
               pProgress->updateProgress("Unable to georeference " + pRaster->getName() + ".", 99, ERRORS);
            }
            pDesktop->deleteWindow(pWindow);
            return false;
         }
      }

      DynamicObject* pMetadata = pRaster->getMetadata();
      DateTime* pCollectionDateTime = dv_cast<DateTime>(&pMetadata->getAttributeByPath(COLLECTION_DATE_TIME_METADATA_PATH));
      double time = 0.0;
      if (pCollectionDateTime == NULL)
      {
         haveTimes = false;
      }
      else
      {
         time = static_cast<double>(pCollectionDateTime->getStructured());
      }
      if (idx == 0)
      {
         mpView->setPrimaryRasterElement(pRaster);
         mLayers.push_back(std::make_pair(mpView->createLayer(RASTER, pRaster), time));
      }
      else
      {
         RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
         RasterLayer* pLayer = dynamic_cast<RasterLayer*>(mpView->createLayer(RASTER, pRaster));
         if (pLayer != NULL)
         {
            mLayers.push_back(std::make_pair(pLayer, time));
            // Calculate the pixel offsets for the secondary image by finding each corner's geolocation in it's own space
            // and determining which pixel that woold be in the space of the primary element
            int Sx1(0);
            int Sy1(0);
            LocationType secondaryLlc = pRaster->convertPixelToGeocoord(LocationType(Sx1, Sy1));
            LocationType primarySecondaryLlc = pPrimaryElement->convertGeocoordToPixel(secondaryLlc);
            pLayer->setXOffset(primarySecondaryLlc.mX);
            pLayer->setYOffset(primarySecondaryLlc.mY);
            if (mpData->createAnimation)
            {
               mpView->hideLayer(pLayer);
            }
         }
      }
   }

   std::stable_sort(mLayers.begin(), mLayers.end(), LayerGeoSorter());
   for (unsigned int i = 0; i < mLayers.size(); ++i)
   {
      Layer* pLayer = mLayers[i].first;
      if (pLayer == NULL)
      {
         continue;
      }
      pLayer->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &MosaicManager::layerDeleted));
      mpView->setLayerDisplayIndex(pLayer, mLayers.size() - i - 1);
   }
   if (haveTimes)
   {
      std::stable_sort(mLayers.begin(), mLayers.end(), LayerTimeSorter());
   }
   mTimeBased = haveTimes;

   mpView->setPanLimit(MAX_EXTENTS);
   if (mpData->createAnimation)
   {
      if (!createAnimation(haveTimes, pProgress))
      {
         pProgress->updateProgress("Unable to create animation.", 99, ERRORS);
         return false;
      }
   }
   return true;
}

bool MosaicManager::createAnimation(bool haveTimes, Progress* pProgress)
{
   if (mpData == NULL || mpView.get() == NULL)
   {
      return false;
   }
   FrameType animType = (haveTimes ? FRAME_TIME : FRAME_ID);
   Service<AnimationServices> pAnim;
   pAnim->destroyAnimationController(pAnim->getAnimationController("Mosaic"));
   AnimationController* pController =
      Service<AnimationServices>()->createAnimationController("Mosaic", animType);
   mpView->setAnimationController(pController);
   Animation* pAnimation = pController == NULL ? NULL : pController->createAnimation("Mosaic");
   if (pAnimation == NULL)
   {
      return false;
   }
   pController->setCanDropFrames(false);
   pAnimation->attach(SIGNAL_NAME(Animation, FrameChanged), Slot(this, &MosaicManager::changeFrame));
   
   // Load frame times
   std::vector<AnimationFrame> frames;
   frames.reserve(mLayers.size());

   if (haveTimes)
   {
      unsigned int idx = 0;
      time_t previousTime = 0;
      for (idx = 0; idx < mLayers.size(); ++idx)
      {
         Layer* pLayer = mLayers[idx].first;
         if (pLayer == NULL)
         {
            continue;
         }
         DataElement* pElement = pLayer->getDataElement();
         if (pElement == NULL)
         {
            continue;
         }
         DynamicObject* pMetadata = pElement->getMetadata();
         DateTime* pCollectionDateTime = dv_cast<DateTime>(&pMetadata->getAttributeByPath(COLLECTION_DATE_TIME_METADATA_PATH));
         if (pCollectionDateTime == NULL)
         {
            continue;
         }
         time_t curFrameTime = pCollectionDateTime->getStructured();
         if (abs(curFrameTime - previousTime) > 0)
         {
            frames.push_back(AnimationFrame("", idx, static_cast<double>(curFrameTime)));
         }
         previousTime = curFrameTime;
      }
   }
   else
   {
      for (unsigned int idx = 0; idx < mLayers.size(); ++idx)
      {
         frames.push_back(AnimationFrame("", idx));
      }
   }

   if (frames.size() == 1)
   {
      if (pProgress != NULL)
      {
         if (haveTimes)
         {
            pProgress->updateProgress("All of the animation frames have the same time.", 0, WARNING);
         }
         else
         {
            pProgress->updateProgress("The animation only has 1 frame.", 0, WARNING);
         }
      }
   }

   pAnimation->setFrames(frames);
   AnimationToolBar* pTb = dynamic_cast<AnimationToolBar*>(Service<DesktopServices>()->getWindow("Animation", TOOLBAR));
   if (pTb != NULL)
   {
      pTb->setAnimationController(pController);
      pTb->show();
   }

   return true;
}
