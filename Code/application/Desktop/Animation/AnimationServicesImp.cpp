/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QAction>
#include <QtGui/QMenu>

#include "AnimationControllerAdapter.h"
#include "AnimationServicesImp.h"
#include "AnimationToolBar.h"
#include "ContextMenu.h"
#include "ContextMenuActions.h"
#include "DesktopServices.h"
#include "Icons.h"

#include <boost/bind.hpp>

using namespace std;

AnimationServicesImp* AnimationServicesImp::spInstance = NULL;
bool AnimationServicesImp::mDestroyed = false;

AnimationServicesImp* AnimationServicesImp::instance()
{
   if (spInstance == NULL)
   {
      if (mDestroyed == true)
      {
         throw std::logic_error("Attempting to use AnimationServices after destroying it.");
      }

      spInstance = new AnimationServicesImp();
   }

   return spInstance;
}

void AnimationServicesImp::destroy()
{
   if (mDestroyed == true)
   {
      throw std::logic_error("Attempting to destroy AnimationServices after destroying it.");
   }

   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

AnimationServicesImp::AnimationServicesImp() :
   mpExplorer(SIGNAL_NAME(SessionExplorer, AboutToShowSessionItemContextMenu),
      Slot(this, &AnimationServicesImp::updateContextMenu))
{}

AnimationServicesImp::~AnimationServicesImp()
{
   // Notify of destruction
   notify(SIGNAL_NAME(Subject, Deleted));

   // Destroy all animation controllers
   clear();
}

void AnimationServicesImp::clear()
{
   vector<AnimationController*> controllers(mControllers);
   for_each(controllers.begin(), controllers.end(),
      boost::bind(&AnimationServicesImp::destroyAnimationController, this, _1));
   mControllers.clear();
}

const string& AnimationServicesImp::getObjectType() const
{
   static string sType("AnimationServicesImp");
   return sType;
}

bool AnimationServicesImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "AnimationServices"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

AnimationController* AnimationServicesImp::createAnimationController(const string& name, FrameType frameType,
                                                                     const string& id)
{
   if (name.empty() == true)
   {
      return NULL;
   }

   if (hasAnimationController(name) == true)
   {
      return NULL;
   }

   // Connect to the session explorer now that the window has been created
   if (mpExplorer.get() == NULL)
   {
      Service<SessionExplorer> pExplorer;
      mpExplorer.reset(pExplorer.get());
   }

   // Create the controller
   AnimationController* pController = new AnimationControllerAdapter(frameType,
      (id.empty()) ? SessionItemImp::generateUniqueId() : id);
   if (pController != NULL)
   {
      AnimationControllerImp* pControllerImp = dynamic_cast<AnimationControllerImp*>(pController);
      if (pControllerImp != NULL)
      {
         pControllerImp->setName(name);
      }

      mControllers.push_back(pController);
      notify(SIGNAL_NAME(AnimationServices, ControllerCreated), boost::any(pController));
   }

   return pController;
}

bool AnimationServicesImp::hasAnimationController(const string& name) const
{
   AnimationController* pController = getAnimationController(name);
   return (pController != NULL);
}

AnimationController* AnimationServicesImp::getAnimationController(const string& name) const
{
   if (name.empty() == true)
   {
      return NULL;
   }

   vector<AnimationController*>::const_iterator iter;
   for (iter = mControllers.begin(); iter != mControllers.end(); ++iter)
   {
      AnimationController* pController = *iter;
      if (pController != NULL)
      {
         const string& currentName = pController->getName();
         if (currentName == name)
         {
            return pController;
         }
      }
   }

   return NULL;
}

const vector<AnimationController*>& AnimationServicesImp::getAnimationControllers() const
{
   return mControllers;
}

unsigned int AnimationServicesImp::getNumAnimationControllers() const
{
   return mControllers.size();
}

void AnimationServicesImp::setCurrentAnimationController(AnimationController* pController)
{
   Service<DesktopServices> pDesktop;

   AnimationToolBar* pToolBar = static_cast<AnimationToolBar*>(pDesktop->getWindow("Animation", TOOLBAR));
   if (pToolBar != NULL)
   {
      pToolBar->setAnimationController(pController);
   }
}

AnimationController* AnimationServicesImp::getCurrentAnimationController() const
{
   Service<DesktopServices> pDesktop;

   AnimationToolBar* pToolBar = static_cast<AnimationToolBar*>(pDesktop->getWindow("Animation", TOOLBAR));
   if (pToolBar != NULL)
   {
      return pToolBar->getAnimationController();
   }

   return NULL;
}

bool AnimationServicesImp::renameAnimationController(AnimationController* pController, const string& newName)
{
   if ((pController == NULL) || (newName.empty() == true))
   {
      return false;
   }

   vector<AnimationController*>::iterator iter;
   for (iter = mControllers.begin(); iter != mControllers.end(); ++iter)
   {
      AnimationController* pCurrentController = *iter;
      if (pCurrentController != NULL)
      {
         if (pCurrentController->getName() == newName)
         {
            return false;
         }
      }
   }

   AnimationControllerImp* pControllerImp = dynamic_cast<AnimationControllerImp*>(pController);
   if (pControllerImp != NULL)
   {
      pControllerImp->setName(newName);
   }

   return false;
}

void AnimationServicesImp::destroyAnimationController(AnimationController* pController)
{
   if (pController == NULL)
   {
      return;
   }

   vector<AnimationController*>::iterator iter;
   for (iter = mControllers.begin(); iter != mControllers.end(); ++iter)
   {
      AnimationController* pCurrentController = *iter;
      if (pCurrentController == pController)
      {
         // Reset the animation controller on the toolbar if destroying the current controller
         Service<DesktopServices> pDesktop;

         AnimationToolBar* pToolBar = static_cast<AnimationToolBar*>(pDesktop->getWindow("Animation", TOOLBAR));
         if (pToolBar != NULL)
         {
            if (pToolBar->getAnimationController() == pController)
            {
               pToolBar->setAnimationController(NULL);
            }
         }

         // Destroy the controller
         mControllers.erase(iter);
         notify(SIGNAL_NAME(AnimationServices, ControllerDestroyed), boost::any(pController));
         delete (dynamic_cast<AnimationControllerImp*>(pController));
         break;
      }
   }
}

void AnimationServicesImp::updateContextMenu(Subject& subject, const string& signal, const boost::any& value)
{
   ContextMenu* pMenu = boost::any_cast<ContextMenu*>(value);
   if (pMenu == NULL)
   {
      return;
   }

   // Get the selected controllers
   vector<AnimationController*> selectedControllers = pMenu->getSessionItems<AnimationController>();
   if (selectedControllers.empty() == false)
   {
      QObject* pParent = pMenu->getActionParent();

      // Separator
      QAction* pSeparatorAction = new QAction(pParent);
      pSeparatorAction->setSeparator(true);
      pMenu->addAction(pSeparatorAction, APP_ANIMATIONSERVICES_SEPARATOR_ACTION);

      // If only one controller is selected, add an activate action if it is not the current controller
      if (selectedControllers.size() == 1)
      {
         AnimationController* pController = selectedControllers.front();
         if (pController != NULL)
         {
            Service<DesktopServices> pDesktop;

            AnimationToolBar* pToolBar = static_cast<AnimationToolBar*>(pDesktop->getWindow("Animation", TOOLBAR));
            if (pToolBar != NULL)
            {
               if (pController != pToolBar->getAnimationController())
               {
                  QAction* pActivateAction = new QAction("&Activate", pParent);
                  pActivateAction->setAutoRepeat(false);
                  pActivateAction->setStatusTip("Activates the selected animation controller on the "
                     "Animation toolbar");
                  connect(pActivateAction, SIGNAL(triggered()), this, SLOT(activateSelectedController()));
                  pMenu->addAction(pActivateAction, APP_ANIMATIONSERVICES_ACTIVATE_ACTION);
               }
            }
         }
      }

      // Add a delete action
      QAction* pDeleteAction = new QAction("&Delete", pParent);
      Icons* pIcons = Icons::instance();
      if (pIcons != NULL)
      {
         pDeleteAction->setIcon(QIcon(pIcons->mDelete));
      }
      pDeleteAction->setAutoRepeat(false);
      pDeleteAction->setStatusTip("Destroys the selected animation controller(s)");
      connect(pDeleteAction, SIGNAL(triggered()), this, SLOT(destroySelectedControllers()));
      pMenu->addAction(pDeleteAction, APP_ANIMATIONSERVICES_DELETE_ACTION);
   }
}

void AnimationServicesImp::activateSelectedController()
{
   Service<DesktopServices> pDesktop;

   AnimationToolBar* pToolBar = static_cast<AnimationToolBar*>(pDesktop->getWindow("Animation", TOOLBAR));
   if (pToolBar == NULL)
   {
      return;
   }

   Service<SessionExplorer> pExplorer;

   vector<SessionItem*> selectedItems = pExplorer->getSelectedSessionItems();
   if (selectedItems.size() == 1)
   {
      AnimationController* pController = dynamic_cast<AnimationController*>(selectedItems.front());
      if (pController != NULL)
      {
         pToolBar->setAnimationController(pController);
      }
   }
}

void AnimationServicesImp::destroySelectedControllers()
{
   // Get the selected controllers
   Service<SessionExplorer> pExplorer;
   vector<AnimationController*> selectedControllers = pExplorer->getSelectedSessionItems<AnimationController>();

   // Destroy the selected controllers
   vector<AnimationController*>::iterator iter;
   for (iter = selectedControllers.begin(); iter != selectedControllers.end(); ++iter)
   {
      AnimationController* pController = *iter;
      if (pController != NULL)
      {
         destroyAnimationController(pController);
      }
   }
}
