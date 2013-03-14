/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "ContextMenuImp.h"
#include "MessageLog.h"
#include "MessageLogMgr.h"
#include "SessionItem.h"

#include <algorithm>
using namespace std;

ContextMenuImp::ContextMenuImp(const vector<SessionItem*>& sessionItems, const QPoint& mouseLocation,
                               const list<ContextMenuAction>& actions, QWidget* pParent, QObject* pActionParent) :
   mSessionItems(sessionItems),
   mMouseLocation(mouseLocation),
   mActions(QList<ContextMenuAction>::fromStdList(actions)),
   mpMenu(new QMenu(pParent)),
   mpActionParent(pActionParent != NULL ? pActionParent : mpMenu),
   mpStep(NULL)
{
   // Log the default actions
   logActions("Default actions");
}

ContextMenuImp::~ContextMenuImp()
{
   if (mpStep != NULL)
   {
      mpStep->finalize();
   }

   delete mpMenu;
}

const vector<SessionItem*>& ContextMenuImp::getSessionItems() const
{
   return mSessionItems;
}

const QPoint& ContextMenuImp::getMouseLocation() const
{
   return mMouseLocation;
}

QObject* ContextMenuImp::getActionParent() const
{
   return mpActionParent;
}

bool ContextMenuImp::addAction(const ContextMenuAction& menuAction)
{
   if (mActions.contains(menuAction) == true)
   {
      return false;
   }

   mActions.append(menuAction);
   return true;
}

bool ContextMenuImp::addAction(QAction* pAction, const string& id)
{
   if ((pAction == NULL) || (id.empty() == true))
   {
      return false;
   }

   ContextMenuAction menuAction(pAction, id);
   return addAction(menuAction);
}

bool ContextMenuImp::addActionBefore(QAction* pAction, const string& id, const string& beforeId)
{
   if ((pAction == NULL) || (id.empty() == true))
   {
      return false;
   }

   ContextMenuAction menuAction(pAction, id);
   if (beforeId.empty() == false)
   {
      menuAction.mBuddyType = ContextMenuAction::BEFORE;
      menuAction.mBuddyId = beforeId;
   }

   return addAction(menuAction);
}

bool ContextMenuImp::addActionAfter(QAction* pAction, const string& id, const string& afterId)
{
   if ((pAction == NULL) || (id.empty() == true))
   {
      return false;
   }

   ContextMenuAction menuAction(pAction, id);
   if (afterId.empty() == false)
   {
      menuAction.mBuddyType = ContextMenuAction::AFTER;
      menuAction.mBuddyId = afterId;
   }

   return addAction(menuAction);
}

void ContextMenuImp::removeAction(const string& id)
{
   if (id.empty() == true)
   {
      return;
   }

   ContextMenuAction menuAction(NULL, string());
   menuAction.mBuddyType = ContextMenuAction::REMOVE;
   menuAction.mBuddyId = id;

   addAction(menuAction);
}

const QList<ContextMenuAction>& ContextMenuImp::getActions() const
{
   return mActions;
}

QList<ContextMenuAction>& ContextMenuImp::getActions()
{
   return mActions;
}

void ContextMenuImp::clear()
{
   mActions.clear();
}

bool ContextMenuImp::show()
{
   // Log the menu actions
   logActions("Menu actions");

   // Build the list of actions
   QList<QAction*> menuActions;
   for (int i = 0; i < mActions.count(); ++i)
   {
      ContextMenuAction menuAction = mActions[i];
      if (menuAction.mpAction != NULL)
      {
         menuActions.append(menuAction.mpAction);
      }
   }

   // Reorder the actions
   for (int i = 0; i < mActions.count(); ++i)
   {
      ContextMenuAction menuAction = mActions[i];
      if ((menuAction.mBuddyType == ContextMenuAction::BEFORE) || (menuAction.mBuddyType == ContextMenuAction::AFTER))
      {
         QAction* pMoveAction = getAction(menuAction.mId);
         if (pMoveAction != NULL)
         {
            int index = menuActions.indexOf(pMoveAction);
            if (index != -1)
            {
               QAction* pInsertAction = getAction(menuAction.mBuddyId);
               VERIFYNR_MSG(pInsertAction != NULL, "Attempting to move a context menu action next "
                  "to another action that does not exist in the menu.");

               if (pInsertAction != NULL)
               {
                  int insertIndex = menuActions.indexOf(pInsertAction);
                  if (insertIndex != -1)
                  {
                     if (index != insertIndex)
                     {
                        if (index < insertIndex)
                        {
                           insertIndex--;
                        }

                        if (menuAction.mBuddyType == ContextMenuAction::AFTER)
                        {
                           insertIndex++;
                        }

                        menuActions.removeAt(index);
                        menuActions.insert(insertIndex, pMoveAction);
                     }
                  }
               }
            }
         }
      }
   }

   // Remove actions
   for (int i = 0; i < mActions.count(); ++i)
   {
      ContextMenuAction menuAction = mActions[i];
      if (menuAction.mBuddyType == ContextMenuAction::REMOVE)
      {
         QAction* pAction = getAction(menuAction.mBuddyId);
         VERIFYNR_MSG(pAction != NULL, "Attempting to remove a context menu action "
            "that does not exist in the menu.");

         if (pAction != NULL)
         {
            int index = menuActions.indexOf(pAction);
            if (index != -1)
            {
               menuActions.removeAt(index);
            }
         }
      }
   }

   if (menuActions.isEmpty() == true)
   {
      return false;
   }

   // Invoke the menu
   mpMenu->clear();
   mpMenu->addActions(menuActions);
   mpMenu->exec(mMouseLocation);

   return true;
}

QAction* ContextMenuImp::getAction(const string& id) const
{
   if (id.empty() == true)
   {
      return NULL;
   }

   for (int i = 0; i < mActions.count(); ++i)
   {
      ContextMenuAction menuAction = mActions[i];
      if (menuAction.mId == id)
      {
         return menuAction.mpAction;
      }
   }

   return NULL;
}

void ContextMenuImp::logActions(const string& stepName)
{
#ifndef DEBUG
   return;
#endif

   if ((ContextMenu::hasSettingLogActions() == false) || (ContextMenu::getSettingLogActions() == false))
   {
      return;
   }

   if (mpStep == NULL)
   {
      MessageLog* pLog = Service<MessageLogMgr>()->getLog();
      if (pLog != NULL)
      {
         mpStep = pLog->createStep("Context Menu", "app", "D076926C-856C-4423-9F3B-F3B2C5768CDC");
         if (mpStep != NULL)
         {
            // Log the session items
            for (vector<SessionItem*>::iterator iter = mSessionItems.begin(); iter != mSessionItems.end(); ++iter)
            {
               SessionItem* pItem = *iter;
               if (pItem != NULL)
               {
                  const string& itemName = pItem->getName();
                  if (itemName.empty() == false)
                  {
                     mpStep->addProperty("Session item", itemName);
                  }
               }
            }

            // Log the mouse location
            mpStep->addProperty("Mouse location x-coordinate", mMouseLocation.x());
            mpStep->addProperty("Mouse location y-coordinate", mMouseLocation.y());
         }
      }
   }

   if (mpStep == NULL)
   {
      return;
   }

   string name = stepName;
   if (name.empty() == true)
   {
      name = "Context menu actions";
   }

   Step* pActionStep = mpStep->addStep(stepName, "app", "FED3BA8A-AFAD-4387-964D-A282EE45145C");
   if (pActionStep != NULL)
   {
      for (int i = 0; i < mActions.count(); ++i)
      {
         ContextMenuAction action = mActions[i];
         string actionName;

         QAction* pAction = action.mpAction;
         if (pAction != NULL)
         {
            actionName = pAction->text().toStdString();
            if ((actionName.empty() == true) && (pAction->isSeparator() == true))
            {
               actionName = "Separator";
            }
         }

         Message* pActionMessage = pActionStep->addMessage(actionName, "app", "5A4A5625-3C2C-4B83-9BBA-DEBA3F006CA6");
         if (pActionMessage != NULL)
         {
            string typeText;
            if (action.mBuddyType == ContextMenuAction::AFTER)
            {
               typeText = "After";
            }
            else if (action.mBuddyType == ContextMenuAction::BEFORE)
            {
               typeText = "Before";
            }
            else if (action.mBuddyType == ContextMenuAction::REMOVE)
            {
               typeText = "Remove";
            }

            pActionMessage->addProperty("ID", action.mId);
            pActionMessage->addProperty("Buddy Type", typeText);
            pActionMessage->addProperty("Buddy ID", action.mBuddyId);
         }
      }

      pActionStep->finalize();
   }
}
