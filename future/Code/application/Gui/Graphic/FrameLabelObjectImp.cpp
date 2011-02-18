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
#include "SessionManager.h"
#include "Slot.h"
#include "SpatialDataView.h"
#include "StringUtilities.h"
#include "TypesFile.h"
#include "ViewImp.h"
#include "XercesIncludes.h"
#include "xmlreader.h"

#include <algorithm>
#include <vector>
using namespace std;
XERCES_CPP_NAMESPACE_USE

FrameLabelObjectImp::FrameLabelObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                                         LocationType pixelCoord) :
   TextObjectImp(id, type, pLayer, pixelCoord),
   mAutoMode(false),
   mLocked(false),
   mpView(SIGNAL_NAME(ViewImp, AnimationControllerChanged), Slot(this, &FrameLabelObjectImp::updateAnimations)),
   mpLayerList(SIGNAL_NAME(LayerList, LayerAdded), Slot(this, &FrameLabelObjectImp::updateAnimations))
{
   mpAnimationController.addSignal(SIGNAL_NAME(AnimationController, AnimationAdded),
      Slot(this, &FrameLabelObjectImp::animationAdded));
   mpAnimationController.addSignal(SIGNAL_NAME(AnimationController, AnimationRemoved),
      Slot(this, &FrameLabelObjectImp::animationRemoved));
   mpAnimationController.addSignal(SIGNAL_NAME(Subject, Deleted), Slot(this, &FrameLabelObjectImp::updateAnimations));
   mpLayer.addSignal(SIGNAL_NAME(LayerImp, ViewModified), Slot(this, &FrameLabelObjectImp::updateAnimations));

   setTextEditable(false);
   reset();
}

FrameLabelObjectImp::~FrameLabelObjectImp()
{
   reset();
}

void FrameLabelObjectImp::reset()
{
   setLocked(false);
   mpView.reset(NULL);
   mpAnimationController.reset(NULL);
   mpLayerList.reset(NULL);
   clearLayers();
   clearAnimations();
   updateText();
}

void FrameLabelObjectImp::setAutoMode(bool autoMode)
{
   if (autoMode != mAutoMode)
   {
      mAutoMode = autoMode;
      reset();
      updateAnimationList();
      setLocked(autoMode);
   }
}

bool FrameLabelObjectImp::getAutoMode() const
{
   return mAutoMode;
}

void FrameLabelObjectImp::setLocked(bool locked)
{
   mLocked = locked;
}

bool FrameLabelObjectImp::getLocked() const
{
   return mLocked;
}

void FrameLabelObjectImp::updateAnimationList(bool force)
{
   if (getAutoMode() == false)
   {
      return;
   }

   const bool wasLocked = getLocked();
   if (force == true)
   {
      setLocked(false);
   }

   if (getLocked() == false)
   {
      View* pView = NULL;
      GraphicLayer* pLayer = getLayer();
      if (pLayer != NULL)
      {
         pView = pLayer->getView();
      }

      setAnimations(pView);
   }

   if (force == true)
   {
      setLocked(wasLocked);
   }
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

void FrameLabelObjectImp::updateAnimations(Subject &subject, const string &signal, const boost::any &value)
{
   updateAnimationList(true);
}

void FrameLabelObjectImp::animationAdded(Subject &subject, const std::string &signal, const boost::any &value)
{
   const bool wasLocked = getLocked();
   setLocked(false);
   insertAnimation(boost::any_cast<Animation*>(value));
   setLocked(wasLocked);
}

void FrameLabelObjectImp::animationRemoved(Subject &subject, const std::string &signal, const boost::any &value)
{
   const bool wasLocked = getLocked();
   setLocked(false);
   eraseAnimation(boost::any_cast<Animation*>(value));
   setLocked(wasLocked);
}

void FrameLabelObjectImp::animationDeleted(Subject &subject, const string &signal, const boost::any &value)
{
   const bool wasLocked = getLocked();
   setLocked(false);
   eraseAnimation(dynamic_cast<Animation*>(&subject));
   setLocked(wasLocked);
}

void FrameLabelObjectImp::layerDeleted(Subject &subject, const std::string &signal, const boost::any &value)
{
   const bool wasLocked = getLocked();
   setLocked(false);
   eraseLayer(dynamic_cast<RasterLayer*>(&subject));
   setLocked(wasLocked);
}

void FrameLabelObjectImp::setAnimations(View* pView)
{
   if (getLocked() == false)
   {
      reset();
      vector<Animation*> pAnimations;
      if (pView != NULL)
      {
         SpatialDataView* pSpatialDataView = dynamic_cast<SpatialDataView*>(pView);
         if (pSpatialDataView == NULL)
         {
            mpView.reset(pView);
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
               pRasterLayer->attach(SIGNAL_NAME(RasterLayer, AnimationChanged),
                  Slot(this, &FrameLabelObjectImp::updateAnimations));
               pRasterLayer->attach(SIGNAL_NAME(Subject, Deleted),
                  Slot(this, &FrameLabelObjectImp::layerDeleted));
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
}

void FrameLabelObjectImp::eraseLayer(RasterLayer* pLayer)
{
   if (getLocked() == false && pLayer != NULL)
   {
      vector<RasterLayer*>::iterator location = find(mLayers.begin(), mLayers.end(), pLayer);
      if (location != mLayers.end())
      {
         pLayer->detach(SIGNAL_NAME(RasterLayer, AnimationChanged), Slot(this, &FrameLabelObjectImp::updateAnimations));
         pLayer->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FrameLabelObjectImp::layerDeleted));
         eraseAnimation(pLayer->getAnimation());
         mLayers.erase(location);
      }
   }
}

void FrameLabelObjectImp::clearLayers()
{
   if (getLocked() == false)
   {
      while (mLayers.empty() == false)
      {
         eraseLayer(mLayers.front());
      }
   }
}

void FrameLabelObjectImp::updateText()
{
   const AnimationFrame* pFrame(NULL);
   FrameType frameType;
   unsigned int maxCount = 0;
   const bool findMinimum = FrameLabelObject::getSettingDisplayMinimumFrame();

   for (vector<Animation*>::const_iterator iter = mAnimations.begin(); iter != mAnimations.end(); ++iter)
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

   string text = "[Frame Label]";
   if (pFrame != NULL)
   {
      text = AnimationImp::frameToQString(pFrame, frameType, maxCount + 1).toStdString();
   }

   setText(text.c_str());
}

void FrameLabelObjectImp::setAnimations(const vector<Animation*> &animations)
{
   if (getLocked() == false)
   {
      reset();
      insertAnimations(animations);
   }
}

void FrameLabelObjectImp::insertAnimations(const vector<Animation*> &animations)
{
   if (getLocked() == false)
   {
      for (vector<Animation*>::const_iterator iter = animations.begin(); iter != animations.end(); ++iter)
      {
         insertAnimation(*iter);
      }
   }
}

const vector<Animation*> &FrameLabelObjectImp::getAnimations() const
{
   return mAnimations;
}

void FrameLabelObjectImp::insertAnimation(Animation* pAnimation)
{
   if (getLocked() == false && pAnimation != NULL)
   {
      if (find(mAnimations.begin(), mAnimations.end(), pAnimation) == mAnimations.end())
      {
         pAnimation->attach(SIGNAL_NAME(Animation, FrameChanged), Slot(this, &FrameLabelObjectImp::frameChanged));
         pAnimation->attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FrameLabelObjectImp::animationDeleted));
         mAnimations.push_back(pAnimation);
         updateText();
      }
   }
}

void FrameLabelObjectImp::eraseAnimation(Animation* pAnimation)
{
   if (getLocked() == false && pAnimation != NULL)
   {
      vector<Animation*>::iterator location = find(mAnimations.begin(), mAnimations.end(), pAnimation);
      if (location != mAnimations.end())
      {
         pAnimation->detach(SIGNAL_NAME(Animation, FrameChanged), Slot(this, &FrameLabelObjectImp::frameChanged));
         pAnimation->detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &FrameLabelObjectImp::animationDeleted));
         mAnimations.erase(location);
         updateText();
      }
   }
}

void FrameLabelObjectImp::clearAnimations()
{
   if (getLocked() == false)
   {
      while (mAnimations.empty() == false)
      {
         eraseAnimation(mAnimations.front());
      }
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

void FrameLabelObjectImp::setLayer(GraphicLayer* pLayer)
{
   TextObjectImp::setLayer(pLayer);
   updateAnimationList(true);
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

   // Copy the autoMode of pFrameLabelObject.
   // If it is in autoMode, all necessary information is copied automatically via setAutoMode(true).
   const FrameLabelObjectImp* pFrameLabelObject = dynamic_cast<const FrameLabelObjectImp*>(pObject);
   VERIFY(pFrameLabelObject != NULL);
   const bool autoMode = pFrameLabelObject->getAutoMode();
   setAutoMode(autoMode);

   // If it is not in autoMode, manually copy necessary information now.
   if (autoMode == false)
   {
      setAnimations(pFrameLabelObject->getAnimations());
      setLocked(pFrameLabelObject->getLocked());
   }

   return true;
}

bool FrameLabelObjectImp::toXml(XMLWriter* pXml) const
{
   if (pXml == NULL || Service<SessionManager>()->isSessionSaving() == false)
   {
      return false;
   }

   if (!TextObjectImp::toXml(pXml))
   {
      return false;
   }

   pXml->addAttr("autoMode", StringUtilities::toXmlString<bool>(getAutoMode()));
   if (mAnimations.empty() == false)
   {
      pXml->pushAddPoint(pXml->addElement("Animations"));
      vector<Animation*>::const_iterator it;
      for (it = mAnimations.begin(); it != mAnimations.end(); ++it)
      {
         Animation* pAnim = *it;
         if (pAnim != NULL)
         {
            pXml->pushAddPoint(pXml->addElement("Animation"));
            pXml->addAttr("id", pAnim->getId());
            pXml->popAddPoint();
         }
      }
      pXml->popAddPoint();
   }

   return true;
}

bool FrameLabelObjectImp::fromXml(DOMNode* pDocument, unsigned int version)
{
   if (pDocument == NULL || Service<SessionManager>()->isSessionLoading() == false)
   {
      return false;
   }

   if (!TextObjectImp::fromXml(pDocument, version))
   {
      return false;
   }

   DOMElement* pElement = static_cast<DOMElement*> (pDocument);
   if (pElement != NULL)
   {
      // Set the autoMode setting (which will also call setLocked)
      setAutoMode(StringUtilities::fromXmlString<bool>(A(pElement->getAttribute(X("autoMode")))));

      // If the object is not currently locked, load all frames
      if (getLocked() == false)
      {
         for (DOMNode* pChild = pDocument->getFirstChild(); 
            pChild != NULL; pChild = pChild->getNextSibling())
         {
            if (XMLString::equals(pChild->getNodeName(), X("Animations")))
            {
               vector<Animation*> animations;
               for (DOMNode* pGrandchild = pChild->getFirstChild();
                  pGrandchild != NULL;
                  pGrandchild = pGrandchild->getNextSibling())
               {
                  if (XMLString::equals(pGrandchild->getNodeName(), X("Animation")))
                  {
                     pElement = dynamic_cast<DOMElement*>(pGrandchild);
                     if (pElement != NULL)
                     {
                        string id(A(pElement->getAttribute(X("id"))));
                        if (id.empty() != true)
                        {
                           Animation* pAnim = dynamic_cast<Animation*>(Service<SessionManager>()->getSessionItem(id));
                           if (pAnim != NULL)
                           {
                              animations.push_back(pAnim);
                           }
                        }
                     }
                  }
               }

               setAnimations(animations);
            }
         }
      }
   }

   return true;
}
