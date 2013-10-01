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
#include "AnimationServices.h"
#include "DesktopServices.h"
#include "SessionItemImp.h"
#include "SessionManager.h"
#include "Slot.h"

#include <QtCore/QMimeData>

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

   setSupportedDragActions(Qt::MoveAction);

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

Qt::ItemFlags AnimationModel::flags(const QModelIndex& index) const
{
   Qt::ItemFlags itemFlags = SessionItemModel::flags(index);
   if (index.isValid() == true)
   {
      // Allow both animation controllers and animations to be dragged
      itemFlags |= Qt::ItemIsDragEnabled;

      // Allow animations to be dropped only on animation controllers
      if (index.parent().isValid() == false)
      {
         itemFlags |= Qt::ItemIsDropEnabled;
      }
   }

   return itemFlags;
}

Qt::DropActions AnimationModel::supportedDropActions() const
{
   return Qt::MoveAction;
}

bool AnimationModel::dropMimeData(const QMimeData* pData, Qt::DropAction action, int row, int column,
                                  const QModelIndex& parentIndex)
{
   if ((pData == NULL) || (action != Qt::MoveAction) || (parentIndex.isValid() == false))
   {
      return false;
   }

   if (pData->hasFormat("application/x-sessionitem-id") == false)
   {
      return false;
   }

   // Get the animation controller in which to insert the dragged animations
   AnimationController* pNewController = dynamic_cast<AnimationController*>(
      parentIndex.data(SessionItemModel::SessionItemRole).value<SessionItem*>());
   if (pNewController == NULL)
   {
      return false;
   }

   // Get the dragged animations
   vector<Animation*> animations;

   QByteArray itemIdArray = pData->data("application/x-sessionitem-id");
   QDataStream itemIdStream(&itemIdArray, QIODevice::ReadOnly);

   while (itemIdStream.atEnd() == false)
   {
      QString itemId;
      itemIdStream >> itemId;
      VERIFY(itemId.isEmpty() == false);

      Animation* pAnimation = dynamic_cast<Animation*>(Service<SessionManager>()->getSessionItem(itemId.toStdString()));
      if (pAnimation != NULL)
      {
         // Check if the animation can be added to the destination controller
         if ((pNewController->hasAnimation(pAnimation) == false) &&
            (pNewController->hasAnimation(pAnimation->getName()) == false) &&
            (pAnimation->getFrameType() == pNewController->getFrameType()))
         {
            animations.push_back(pAnimation);
         }
      }
   }

   if (animations.empty() == true)    // No new animations for the controller were included in the drag items
   {
      return false;
   }

   // Move each of the dragged animations
   for (vector<Animation*>::iterator iter = animations.begin(); iter != animations.end(); ++iter)
   {
      Animation* pAnimation = *iter;
      if (pAnimation != NULL)
      {
         // Remove the animation from its current controller
         const vector<AnimationController*>& controllers = Service<AnimationServices>()->getAnimationControllers();
         for (vector<AnimationController*>::const_iterator controllerIter = controllers.begin();
            controllerIter != controllers.end();
            ++controllerIter)
         {
            AnimationController* pCurrentController = *controllerIter;
            if ((pCurrentController != NULL) && (pCurrentController->hasAnimation(pAnimation) == true))
            {
               pCurrentController->removeAnimation(pAnimation);
               break;
            }
         }

         // Add the animation to the destination controller
         if (pNewController->insertAnimation(pAnimation) == false)
         {
            return false;
         }
      }
   }

   return true;
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
