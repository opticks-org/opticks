/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Animation.h"
#include "AnimationImp.h"
#include "FrameLabelObject.h"
#include "FrameLabelObjectImp.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "RasterLayer.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "TypesFile.h"
#include "ViewImp.h"

#include <algorithm>
#include <vector>
using namespace std;

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Replace the getAutoMode()/setAutoMode() interface with the overloaded setAnimations() methods (dadkins)")

FrameLabelObjectImp::FrameLabelObjectImp(const string& id,
   GraphicObjectType type, GraphicLayer* pLayer, LocationType pixelCoord) :
   TextObjectImp(id, type, pLayer, pixelCoord),
   mpView(SIGNAL_NAME(ViewImp, AnimationControllerChanged),
      Slot(this, &FrameLabelObjectImp::animationControllerChanged)),
   mpLayerList(SIGNAL_NAME(LayerList, LayerAdded),
      Slot(this, &FrameLabelObjectImp::layerAdded))
{
   mpAnimationController.addSignal(SIGNAL_NAME(AnimationController, AnimationAdded),
      Slot(this, &FrameLabelObjectImp::animationAdded));

   mpAnimationController.addSignal(SIGNAL_NAME(AnimationController, AnimationRemoved),
      Slot(this, &FrameLabelObjectImp::animationRemoved));

   mpAnimationController.addSignal(SIGNAL_NAME(Subject, Deleted),
      Slot(this, &FrameLabelObjectImp::controllerDeleted));

   reset();
}

FrameLabelObjectImp::~FrameLabelObjectImp()
{
   reset();
}

void FrameLabelObjectImp::reset()
{
   mpView.reset(NULL);
   mpAnimationController.reset(NULL);
   mpLayerList.reset(NULL);
   clearLayers();
   clearAnimations();
   updateText();
}

void FrameLabelObjectImp::setAutoMode(bool autoMode)
{
   if (autoMode != getAutoMode())
   {
      View* pView = NULL;
      if (autoMode == true)
      {
         GraphicLayer* pLayer = getLayer();
         if (pLayer != NULL)
         {
            pView = pLayer->getView();
         }
      }

      setAnimations(pView);
   }
}

bool FrameLabelObjectImp::getAutoMode() const
{
   return (mpView.get() != NULL);
}

bool FrameLabelObjectImp::processMousePress(LocationType screenCoord,
   Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
   Layer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      setAutoMode(true);
      dynamic_cast<GraphicLayerImp*>(pLayer)->completeInsertion();
   }

   return true;
}

void FrameLabelObjectImp::frameChanged(Subject &subject, const string &signal, const boost::any &value)
{
   updateText();
}

void FrameLabelObjectImp::animationControllerChanged(Subject &subject, const string &signal, const boost::any &value)
{
   setAnimations(mpView.get());
}

void FrameLabelObjectImp::animationAdded(Subject &subject, const std::string &signal, const boost::any &value)
{
   insertAnimation(boost::any_cast<Animation*>(value));
}

void FrameLabelObjectImp::animationRemoved(Subject &subject, const std::string &signal, const boost::any &value)
{
   eraseAnimation(boost::any_cast<Animation*>(value));
}

void FrameLabelObjectImp::controllerDeleted(Subject &subject, const string &signal, const boost::any &value)
{
   setAnimations(mpView.get());
}

void FrameLabelObjectImp::animationDeleted(Subject &subject, const string &signal, const boost::any &value)
{
   eraseAnimation(dynamic_cast<Animation*>(&subject));
}

void FrameLabelObjectImp::layerAdded(Subject &subject, const std::string &signal, const boost::any &value)
{
   setAnimations(mpView.get());
}

void FrameLabelObjectImp::animationChanged(Subject &subject, const std::string &signal, const boost::any &value)
{
   setAnimations(mpView.get());
}

void FrameLabelObjectImp::layerDeleted(Subject &subject, const std::string &signal, const boost::any &value)
{
   eraseLayer(dynamic_cast<RasterLayer*>(&subject));
}

void FrameLabelObjectImp::setAnimations(View* pView)
{
   reset();
   mpView.reset(pView);
   vector<Animation*> pAnimations;
   if (mpView.get() != NULL)
   {
      SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(mpView.get());
      if (pSpatialDataView == NULL)
      {
         mpAnimationController.reset(mpView->getAnimationController());
         if (mpAnimationController.get() != NULL)
         {
            pAnimations = mpAnimationController->getAnimations();
         }
      }
      else
      {
         mpLayerList.reset(pSpatialDataView->getLayerList());
         VERIFYNRV(mpLayerList.get() != NULL);

         vector<Layer*> pLayers;
         mpLayerList->getLayers(RASTER, pLayers);
         for (vector<Layer*>::iterator iter = pLayers.begin(); iter != pLayers.end(); ++iter)
         {
            RasterLayer* pRasterLayer = dynamic_cast<RasterLayer*>(*iter);
            VERIFYNRV(pRasterLayer != NULL);
            pRasterLayer->attach(SIGNAL_NAME(RasterLayer, AnimationChanged), Slot(this, &FrameLabelObjectImp::animationChanged));
            pRasterLayer->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FrameLabelObjectImp::layerDeleted));
            mLayers.push_back(pRasterLayer);
            if (pRasterLayer->getAnimation() != NULL)
            {
               pAnimations.push_back(pRasterLayer->getAnimation());
            }
         }
      }
   }

   insertAnimations(pAnimations);
}

void FrameLabelObjectImp::eraseLayer(RasterLayer* pLayer)
{
   if (pLayer != NULL)
   {
      vector<RasterLayer*>::iterator location = find(mLayers.begin(), mLayers.end(), pLayer);
      if (location != mLayers.end())
      {
         pLayer->detach(SIGNAL_NAME(RasterLayer, AnimationChanged), Slot(this, &FrameLabelObjectImp::animationChanged));
         pLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FrameLabelObjectImp::layerDeleted));
         eraseAnimation(pLayer->getAnimation());
         mLayers.erase(location);
      }
   }
}

void FrameLabelObjectImp::clearLayers()
{
   while (mLayers.empty() == false)
   {
      eraseLayer(mLayers.front());
   }
}

void FrameLabelObjectImp::updateText()
{
   const AnimationFrame* pFrame(NULL);
   FrameType frameType;
   unsigned int maxCount = 0;
   const bool findMinimum = FrameLabelObject::getSettingDisplayMinimumFrame();

   for (vector<Animation*>::const_iterator iter = mAnimations.begin(); iter != mAnimations.end(); iter++)
   {
      const Animation* pCurrentAnimation = *iter;
      if (pCurrentAnimation != NULL)
      {
         const AnimationFrame* pCurrentFrame = pCurrentAnimation->getCurrentFrame();
         if (pCurrentFrame != NULL)
         {
            const FrameType currentFrameType = pCurrentAnimation->getFrameType();
            if (pFrame == NULL ||
                  ((findMinimum == true) && (pCurrentFrame < pFrame)) ||
                  ((findMinimum == false) && (pCurrentFrame > pFrame)))
            {
               pFrame = pCurrentFrame;
               frameType = currentFrameType;
            }

            if (pCurrentAnimation->getFrameType() == FRAME_ID)
            {
               maxCount = max(maxCount, static_cast<unsigned int>(pCurrentAnimation->getStopValue()));
            }
         }
      }
   }

   string text;
   if (pFrame != NULL)
   {
      text = AnimationImp::frameToQString(pFrame, frameType, maxCount + 1).toStdString();
   }
   if (text.empty())
   {
      setText("[Frame Label]");
   }
   else
   {
      setText(text.c_str());
   }
}

void FrameLabelObjectImp::setAnimations(const vector<Animation*> &animations)
{
   reset();
   insertAnimations(animations);
}

void FrameLabelObjectImp::insertAnimations(const vector<Animation*> &animations)
{
   for (vector<Animation*>::const_iterator iter = animations.begin(); iter != animations.end(); iter++)
   {
      insertAnimation(*iter);
   }

   updateText();
}

const vector<Animation*> &FrameLabelObjectImp::getAnimations() const
{
   return mAnimations;
}

void FrameLabelObjectImp::insertAnimation(Animation* pAnimation)
{
   if (pAnimation != NULL)
   {
      if (find(mAnimations.begin(), mAnimations.end(), pAnimation) == mAnimations.end())
      {
         pAnimation->attach(SIGNAL_NAME(Animation, FrameChanged), Slot(this, &FrameLabelObjectImp::frameChanged));
         pAnimation->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FrameLabelObjectImp::animationDeleted));
         mAnimations.push_back(pAnimation);
      }
   }
}

void FrameLabelObjectImp::eraseAnimation(Animation* pAnimation)
{
   if (pAnimation != NULL)
   {
      vector<Animation*>::iterator location = find(mAnimations.begin(), mAnimations.end(), pAnimation);
      if (location != mAnimations.end())
      {
         pAnimation->detach(SIGNAL_NAME(Animation, FrameChanged), Slot(this, &FrameLabelObjectImp::frameChanged));
         pAnimation->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FrameLabelObjectImp::animationDeleted));
         mAnimations.erase(location);
      }
   }
}

void FrameLabelObjectImp::clearAnimations()
{
   while (mAnimations.empty() == false)
   {
      eraseAnimation(mAnimations.front());
   }
}

const string& FrameLabelObjectImp::getObjectType() const
{
   static string type("FrameLabelObjectImp");
   return type;
}

bool FrameLabelObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "FrameLabelObject"))
   {
      return true;
   }

   return TextObjectImp::isKindOf(className);
}

void FrameLabelObjectImp::updateGeo()
{
   // remain fixed in place, so do nothing
}

bool FrameLabelObjectImp::replicateObject(const GraphicObject* pObject)
{
   if (TextObjectImp::replicateObject(pObject) == false)
   {
      return false;
   }

   const FrameLabelObjectImp* pFrameLabelObject = dynamic_cast<const FrameLabelObjectImp*>(pObject);
   VERIFY(pFrameLabelObject != NULL);
   if (pFrameLabelObject->mpView.get() != NULL)
   {
      setAnimations(const_cast<View*>(pFrameLabelObject->mpView.get()));
   }
   else
   {
      setAnimations(pFrameLabelObject->mAnimations);
   }

   return true;
}
