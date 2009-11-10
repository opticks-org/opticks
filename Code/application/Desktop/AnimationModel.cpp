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
#include "AnimationModel.h"
#include "DesktopServices.h"
#include "Slot.h"

using namespace std;

AnimationModel::AnimationModel(QObject* pParent) :
   SessionItemModel(pParent),
   mpAnimationServices(Service<AnimationServices>().get()),
   mpAnimationToolBar(SIGNAL_NAME(AnimationToolBar, ControllerChanged),
      Slot(this, &AnimationModel::setCurrentController))
{
   // Initialization
   const vector<AnimationController*>& controllers = mpAnimationServices->getAnimationControllers();
   for (vector<AnimationController*>::size_type i = 0; i < controllers.size(); ++i)
   {
      AnimationController* pController = controllers[i];
      if (pController != NULL)
      {
         addControllerItem(pController);
      }
   }

   // Connections
   mpAnimationServices.addSignal(SIGNAL_NAME(AnimationServices, ControllerCreated),
      Slot(this, &AnimationModel::addController));
   mpAnimationServices.addSignal(SIGNAL_NAME(AnimationServices, ControllerDestroyed),
      Slot(this, &AnimationModel::removeController));

   Service<DesktopServices> pDesktop;

   AnimationToolBar* pToolBar = static_cast<AnimationToolBar*>(pDesktop->getWindow("Animation", TOOLBAR));
   if (pToolBar != NULL)
   {
      mpAnimationToolBar.reset(pToolBar);
   }
}

AnimationModel::~AnimationModel()
{
   if (mpAnimationServices.get() != NULL)
   {
      // Remove the controller items
      const vector<AnimationController*>& controllers = mpAnimationServices->getAnimationControllers();
      for (vector<AnimationController*>::size_type i = 0; i < controllers.size(); ++i)
      {
         AnimationController* pController = controllers[i];
         if (pController != NULL)
         {
            removeControllerItem(pController);
         }
      }
   }
}

void AnimationModel::addController(Subject& subject, const string& signal, const boost::any& value)
{
   AnimationController* pController = boost::any_cast<AnimationController*>(value);
   if (pController != NULL)
   {
      addControllerItem(pController);
   }
}

void AnimationModel::removeController(Subject& subject, const string& signal, const boost::any& value)
{
   AnimationController* pController = boost::any_cast<AnimationController*>(value);
   if (pController != NULL)
   {
      removeControllerItem(pController);
   }
}

void AnimationModel::setCurrentController(Subject& subject, const string& signal, const boost::any& value)
{
   AnimationController* pController = boost::any_cast<AnimationController*>(value);
   if (pController != NULL)
   {
      activateItem(pController);
   }
}

void AnimationModel::addAnimation(Subject& subject, const string& signal, const boost::any& value)
{
   AnimationController* pController = dynamic_cast<AnimationController*>(&subject);
   if (pController != NULL)
   {
      Animation* pAnimation = boost::any_cast<Animation*>(value);
      if (pAnimation != NULL)
      {
         SessionItemWrapper* pControllerWrapper = getWrapper(pController);
         if (pController != NULL)
         {
            pControllerWrapper->addChild(pAnimation);
         }
      }
   }
}

void AnimationModel::removeAnimation(Subject& subject, const string& signal, const boost::any& value)
{
   AnimationController* pController = dynamic_cast<AnimationController*>(&subject);
   if (pController != NULL)
   {
      Animation* pAnimation = boost::any_cast<Animation*>(value);
      if (pAnimation != NULL)
      {
         SessionItemWrapper* pControllerWrapper = getWrapper(pController);
         if (pController != NULL)
         {
            pControllerWrapper->removeChild(pAnimation);
         }
      }
   }
}

void AnimationModel::addControllerItem(AnimationController* pController)
{
   if (pController == NULL)
   {
      return;
   }

   SessionItemWrapper* pRootWrapper = getRootWrapper();
   if (pRootWrapper == NULL)
   {
      return;
   }

   // Add the controller item
   SessionItemWrapper* pControllerWrapper = pRootWrapper->addChild(pController);
   if (pControllerWrapper != NULL)
   {
      // Add the animation items
      const vector<Animation*>& animations = pController->getAnimations();
      for (vector<Animation*>::size_type i = 0; i < animations.size(); ++i)
      {
         Animation* pAnimation = animations[i];
         if (pAnimation != NULL)
         {
            pControllerWrapper->addChild(pAnimation);
         }
      }

      // Connections
      pController->attach(SIGNAL_NAME(AnimationController, AnimationAdded),
         Slot(this, &AnimationModel::addAnimation));
      pController->attach(SIGNAL_NAME(AnimationController, AnimationRemoved),
         Slot(this, &AnimationModel::removeAnimation));
   }
}

void AnimationModel::removeControllerItem(AnimationController* pController)
{
   if (pController == NULL)
   {
      return;
   }

   // Detach the controller
   pController->detach(SIGNAL_NAME(AnimationController, AnimationAdded),
      Slot(this, &AnimationModel::addAnimation));
   pController->detach(SIGNAL_NAME(AnimationController, AnimationRemoved),
      Slot(this, &AnimationModel::removeAnimation));

   SessionItemWrapper* pControllerWrapper = getWrapper(pController);
   if (pControllerWrapper != NULL)
   {
      // Remove the animation items
      const vector<Animation*>& animations = pController->getAnimations();
      for (vector<Animation*>::size_type i = 0; i < animations.size(); ++i)
      {
         Animation* pAnimation = animations[i];
         if (pAnimation != NULL)
         {
            pControllerWrapper->removeChild(pAnimation);
         }
      }

      // Remove the controller item
      SessionItemWrapper* pRootWrapper = pControllerWrapper->getParent();
      if (pRootWrapper != NULL)
      {
         pRootWrapper->removeChild(pController);
      }
   }
}
