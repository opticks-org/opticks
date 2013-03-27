/*
* The information in this file is
* Copyright(c) 2007 Ball Aerospace & Technologies Corporation
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from   
* http://www.gnu.org/licenses/lgpl.html
*/

#include "Animation.h"
#include "AnimationController.h"
#include "AnimationServices.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "AttachmentPtr.h"
#include "DynamicObject.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "SimpleAnimation.h"
#include "SimpleApiErrors.h"

#include <boost/any.hpp>
#include <limits>
#include <QtCore/QDateTime>

class CallbackHandler
{
public:
   CallbackHandler(Animation* pAnimation, animation_callback_t pCallback, void* pUser) :
      mpAnimation(pAnimation, SIGNAL_NAME(Animation, FrameChanged), Slot(this, &CallbackHandler::frameChanged)),
      mpCallback(pCallback),
      mpUser(pUser)
   {
   }

   virtual ~CallbackHandler() {}

   void frameChanged(Subject& subject, const std::string& signal, const boost::any& v)
   {
      Animation& animation = dynamic_cast<Animation&>(subject);
      AnimationFrame* pFrame = boost::any_cast<AnimationFrame*>(v);
      if (pFrame != NULL)
      {
         std::string animName = animation.getName();
         mpCallback(animName.c_str(), pFrame->mName.c_str(),
            pFrame->mFrameNumber, pFrame->mTime, mpUser);
      }
   }

private:
   CallbackHandler(const CallbackHandler& rhs);
   CallbackHandler& operator=(const CallbackHandler& rhs);
   AttachmentPtr<Animation> mpAnimation;
   animation_callback_t mpCallback;
   void* mpUser;
};

namespace
{
   bool createAnimationForRasterLayer(RasterLayer *pRasterLayer, AnimationController *pController)
   {
      if(pRasterLayer == NULL || pController == NULL)
      {
         return false;
      }
      std::vector<double> frameTimes;
      unsigned int numFrames = 0;
      RasterElement *pRasterElement = dynamic_cast<RasterElement*>(pRasterLayer->getDataElement());
      if(pRasterElement != NULL)
      {
         const RasterDataDescriptor *pDescriptor =
            dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
         if(pDescriptor != NULL)
         {
            numFrames = pDescriptor->getBandCount();
            const DynamicObject *pMetadata = pDescriptor->getMetadata();
            try
            {
               frameTimes = dv_cast<std::vector<double> >(pMetadata->getAttributeByPath(FRAME_TIMES_METADATA_PATH));
               if(frameTimes.size() < numFrames)
               {
                  frameTimes.clear();
               }
            }
            catch(const std::bad_cast&)
            {
               // do nothing
            }
         }
      }
      if(frameTimes.empty())
      {
         double frameTime = pController->getStartFrame();
         if(frameTime < 0.0)
         {
            frameTime = QDateTime::currentDateTime().toUTC().toTime_t();
         }
         frameTimes.reserve(numFrames);
         for(unsigned int i = 0; i < numFrames; i++)
         {
            frameTimes.push_back(frameTime);
            frameTime += 1.0;
         }
      }
      Animation *pAnim = pController->createAnimation(pRasterLayer->getName());
      if (pAnim == NULL)
      {
         return false;
      }

      std::vector<AnimationFrame> frames;
      for(unsigned int i = 0; i < numFrames; ++i)
      {
         AnimationFrame frame;
         frame.mFrameNumber = i;
         if(pAnim->getFrameType() == FRAME_TIME)
         {
            frame.mTime = frameTimes[i];
         }
         frames.push_back(frame);
      }
      pAnim->setFrames(frames);
      pRasterLayer->setAnimation(pAnim);
      return true;
   }

   Animation* createAnimationForCallback(AnimationController *pController, const std::string& name,
                                         unsigned int numFrames, std::vector<double> frameTimes)
   {
      if(pController == NULL)
      {
         return NULL;
      }
      if(frameTimes.empty())
      {
         double frameTime = pController->getStartFrame();
         if(frameTime < 0.0)
         {
            frameTime = QDateTime::currentDateTime().toUTC().toTime_t();
         }
         frameTimes.reserve(numFrames);
         for(unsigned int i = 0; i < numFrames; i++)
         {
            frameTimes.push_back(frameTime);
            frameTime += 1.0;
         }
      }
      Animation *pAnim = pController->createAnimation(name);
      if (pAnim == NULL)
      {
         return NULL;
      }

      std::vector<AnimationFrame> frames;
      for(unsigned int i = 0; i < numFrames; ++i)
      {
         AnimationFrame frame;
         frame.mFrameNumber = i;
         if(pAnim->getFrameType() == FRAME_TIME)
         {
            frame.mTime = frameTimes[i];
         }
         frames.push_back(frame);
      }
      pAnim->setFrames(frames);
      return pAnim;
   }
}


extern "C"
{
   AnimationController* getAnimationController(const char* pName)
   {
      AnimationController* pController;
      if (pName == NULL)
      {
         pController = Service<AnimationServices>()->getCurrentAnimationController();
      }
      else
      {
         pController = Service<AnimationServices>()->getAnimationController(std::string(pName));
      }
      if (pController == NULL)
      {
         setLastError(SIMPLE_NOT_FOUND);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pController;
   }

   AnimationController* createAnimationController(const char* pName, int timeBased)
   {
      if (pName == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      AnimationController* pController = Service<AnimationServices>()->createAnimationController(
         std::string(pName), (timeBased == 0) ? FRAME_ID : FRAME_TIME);
      if (pController == NULL)
      {
         setLastError(SIMPLE_EXISTS);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pController;
   }

   void destroyAnimationController(AnimationController* pController)
   {
      if (pController != NULL)
      {
         Service<AnimationServices>()->destroyAnimationController(pController);
      }
   }

   int activateAnimationController(AnimationController* pController)
   {
      if (pController == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      Service<AnimationServices>()->setCurrentAnimationController(pController);
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   uint32_t getAnimationControllerState(AnimationController* pController)
   {
      if (pController == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      uint32_t rval = 0;
      switch (pController->getAnimationState())
      {
      case STOP:
         rval = 0;
         break;
      case PLAY_FORWARD:
         rval = 1;
         break;
      case PLAY_BACKWARD:
         rval = 2;
         break;
      case PAUSE_FORWARD:
         rval = 3;
         break;
      case PAUSE_BACKWARD:
         rval = 4;
         break;
      default:
         setLastError(SIMPLE_OTHER_FAILURE);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return rval;
   }

   int setAnimationControllerState(AnimationController* pController, uint32_t state)
   {
      if (pController == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      switch (state)
      {
      case 0:
         pController->stop();
         break;
      case 1:
         pController->play();
         pController->setAnimationState(PLAY_FORWARD);
         break;
      case 2:
         pController->play();
         pController->setAnimationState(PLAY_BACKWARD);
         break;
      case 3:
         pController->pause();
         pController->setAnimationState(PAUSE_FORWARD);
         break;
      case 4:
         pController->pause();
         pController->setAnimationState(PAUSE_BACKWARD);
         break;
      default:
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int playAnimationController(AnimationController* pController)
   {
      if (pController == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      pController->play();
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int pauseAnimationController(AnimationController* pController)
   {
      if (pController == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      pController->pause();
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int stopAnimationController(AnimationController* pController)
   {
      if (pController == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      pController->stop();
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   uint32_t getAnimationControllerCycle(AnimationController* pController)
   {
      if (pController == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      uint32_t rval = 0;
      switch (pController->getAnimationCycle())
      {
      case PLAY_ONCE:
         rval = 0;
         break;
      case REPEAT:
         rval = 1;
         break;
      case BOUNCE:
         rval = 2;
         break;
      default:
         setLastError(SIMPLE_OTHER_FAILURE);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return rval;
   }

   int setAnimationControllerCycle(AnimationController* pController, uint32_t cycle)
   {
      if (pController == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      switch (cycle)
      {
      case 0:
         pController->setAnimationCycle(PLAY_ONCE);
         break;
      case 1:
         pController->setAnimationCycle(REPEAT);
         break;
      case 2:
         pController->setAnimationCycle(BOUNCE);
         break;
      default:
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int canAnimationControllerDropFrames(AnimationController* pController)
   {
      if (pController == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return static_cast<int>(pController->getCanDropFrames());
   }

   int setAnimationControllerCanDropFrames(AnimationController* pController, int dropFrames)
   {
      if (pController == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      pController->setCanDropFrames(dropFrames != 0);
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   double getAnimationControllerIntervalMultiplier(AnimationController* pController)
   {
      if (pController == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return std::numeric_limits<double>::quiet_NaN();
      }
      setLastError(SIMPLE_NO_ERROR);
      return pController->getIntervalMultiplier();
   }

   int setAnimationControllerIntervalMultiplier(AnimationController* pController, double multiplier)
   {
      if (pController == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      pController->setIntervalMultiplier(multiplier);
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int attachRasterLayerToAnimationController(AnimationController* pController, Layer* pLayer)
   {
      RasterLayer* pRaster = dynamic_cast<RasterLayer*>(pLayer);
      if (pController == NULL || pRaster == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      if (!createAnimationForRasterLayer(pRaster, pController))
      {
         if (pController->hasAnimation(pRaster->getName()))
         {
            setLastError(SIMPLE_EXISTS);
         }
         else
         {
            setLastError(SIMPLE_OTHER_FAILURE);
         }
         return 1;
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   void* attachCallbackToAnimationController(AnimationController* pController,
                                             const char* pName,
                                             animation_callback_t pCallback,
                                             void* pUser,
                                             uint32_t frameCount,
                                             double* pFrameTimes)
   {
      if (pController == NULL || pName == NULL || pCallback == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      std::string name(pName);
      std::vector<double> frameTimes;
      if (pFrameTimes != NULL)
      {
         frameTimes = std::vector<double>(pFrameTimes, pFrameTimes + frameCount);
      }
      Animation* pAnimation = createAnimationForCallback(pController, name, frameCount, frameTimes);
      if (pAnimation == NULL)
      {
         if (pController->hasAnimation(name))
         {
            setLastError(SIMPLE_EXISTS);
         }
         else
         {
            setLastError(SIMPLE_OTHER_FAILURE);
         }
         return NULL;
      }
      CallbackHandler* pHandler = new CallbackHandler(pAnimation, pCallback, pUser);
      setLastError(SIMPLE_NO_ERROR);
      return pHandler;
   }

   void destroyAnimationControllerAttachment(AnimationController* pController, const char* pName, void* pHandler)
   {
      if (pController != NULL && pName != NULL)
      {
         Animation* pAnim = pController->getAnimation(std::string(pName));
         if (pAnim != NULL)
         {
            pController->destroyAnimation(pAnim);
         }
      }
      delete reinterpret_cast<CallbackHandler*>(pHandler);
   }
};