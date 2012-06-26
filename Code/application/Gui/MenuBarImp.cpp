/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QRegExp>

#include "MenuBarImp.h"
#include "AppVerify.h"
#include "DesktopServicesImp.h"

using namespace std;

MenuBarImp::MenuBarImp(const QString& strMenuBarName, QWidget* parent) :
   QMenuBar(parent),
   mName(strMenuBarName)
{
}

MenuBarImp::~MenuBarImp()
{
}

QMenu* MenuBarImp::addMenu(const string& location, QAction* pBefore)
{
   if (location.empty() == true)
   {
      return NULL;
   }

   QString strLocation = QString::fromStdString(location);
   strLocation.replace(QRegExp("/"), "\\");
   if (strLocation.startsWith("\\") == true)
   {
      strLocation = strLocation.mid(1);
   }

   QMenu* pParentMenu = NULL;
   QMenu* pMenu = NULL;

   while (strLocation.isEmpty() == false)
   {
      int iPos = strLocation.indexOf("\\");
      QString strMenuItem = strLocation.left(iPos);

      QAction* pAction = getAction(strMenuItem, pParentMenu);
      if (pAction != NULL)
      {
         if (isMenu(pAction) == true)
         {
            pParentMenu = getMenu(pAction);
         }
         else if (iPos != -1)
         {
            return NULL;
         }
      }
      else
      {
         if (iPos == -1)
         {
            // Add the menu
            if (pParentMenu != NULL)
            {
               if (pBefore != NULL)
               {
                  pMenu = new QMenu(strMenuItem, pParentMenu);
                  VERIFYRV(pMenu != NULL, NULL);

                  pParentMenu->insertMenu(pBefore, pMenu);
               }
               else
               {
                  pMenu = pParentMenu->addMenu(strMenuItem);
               }
            }
            else
            {
               if (pBefore != NULL)
               {
                  pMenu = new QMenu(strMenuItem, this);
                  VERIFYRV(pMenu != NULL, NULL);

                  QMenuBar::insertMenu(pBefore, pMenu);
               }
               else
               {
                  pMenu = QMenuBar::addMenu(strMenuItem);
               }
            }
         }
         else
         {
            // Create a new menu
            QMenu* pNewMenu = new QMenu(strMenuItem, this);
            VERIFYRV(pNewMenu != NULL, NULL);

            // Add the new menu at the end of the bar
            pAction = insertMenu(pNewMenu, pParentMenu);
            if (pAction != NULL)
            {
               pParentMenu = pNewMenu;
            }
         }
      }

      if (iPos != -1)
      {
         int iLength = strLocation.length();
         strLocation = strLocation.right(iLength - iPos - 1);
      }
      else
      {
         strLocation.clear();
      }
   }

   return pMenu;
}

QAction* MenuBarImp::insertMenu(QMenu* pMenu, QMenu* pParentMenu, QAction* pBefore)
{
   if (pMenu == NULL)
   {
      return NULL;
   }

   QAction* pAction = getMenuItem(pMenu);
   if (pAction != NULL)
   {
      return pAction;
   }

   if (pParentMenu == NULL)
   {
      QString strMenuName = pMenu->title();
      if (strMenuName == mName)
      {
         // Add the menu as the first menu of the menu bar
         QList<QAction*> menuActions = actions();
         if (menuActions.empty() == false)
         {
            pBefore = menuActions.front();
         }
      }

      if (pBefore != NULL)
      {
         pAction = QMenuBar::insertMenu(pBefore, pMenu);
      }
      else
      {
         pAction = QMenuBar::addMenu(pMenu);
      }
   }
   else
   {
      if (pBefore != NULL)
      {
         pAction = pParentMenu->insertMenu(pBefore, pMenu);
      }
      else
      {
         pAction = pParentMenu->addMenu(pMenu);
      }
   }

   if (pAction != NULL)
   {
      mMenus.append(pMenu);
   }

   return pAction;
}


QAction* MenuBarImp::addCommand(const string& location, QAction* pBefore)
{
   return addCommand(location, string(), pBefore);
}

QAction* MenuBarImp::addCommand(const string& location, const string& shortcutContext, QAction* pBefore)
{
   if (location.empty() == true)
   {
      return NULL;
   }

   QString strLocation = QString::fromStdString(location);
   strLocation.replace(QRegExp("/"), "\\");
   if (strLocation.startsWith("\\") == true)
   {
      strLocation = strLocation.mid(1);
   }

   QMenu* pParentMenu = NULL;
   QAction* pAction = NULL;

   while (strLocation.isEmpty() == false)
   {
      int iPos = strLocation.indexOf("\\");
      QString strMenuItem = strLocation.left(iPos);

      pAction = getAction(strMenuItem, pParentMenu);
      if (pAction != NULL)
      {
         if (isMenu(pAction) == true)
         {
            pParentMenu = getMenu(pAction);
         }
         else if (iPos != -1)
         {
            return NULL;
         }
      }
      else
      {
         if (iPos == -1)
         {
            // Add the command
            if (pParentMenu != NULL)
            {
               pAction = new QAction(strMenuItem, pParentMenu);
               VERIFYRV(pAction != NULL, NULL);
            }
            else
            {
               // Cannot add a command directly on the menu bar
               return NULL;
            }
         }
         else
         {
            // Create a new menu
            QMenu* pMenu = new QMenu(strMenuItem, this);
            if (pMenu == NULL)
            {
               return NULL;
            }

            // Add the new menu at the end of the bar
            pAction = insertMenu(pMenu, pParentMenu);
            if (pAction != NULL)
            {
               pParentMenu = pMenu;
            }
         }
      }

      if (iPos != -1)
      {
         int iLength = strLocation.length();
         strLocation = strLocation.right(iLength - iPos - 1);
      }
      else
      {
         strLocation.clear();
      }
   }

   if ((pAction != NULL) && (pParentMenu != NULL))
   {
      insertCommand(pAction, pParentMenu, shortcutContext, pBefore);
   }

   return pAction;
}

bool MenuBarImp::insertCommand(QAction* pCommand, QMenu* pMenu, const string& shortcutContext, QAction* pBefore)
{
   if ((pCommand == NULL) || (pMenu == NULL))
   {
      return false;
   }

   // Initialize the keyboard shortcut in the command
   DesktopServicesImp* pDesktop = DesktopServicesImp::instance();
   VERIFY(pDesktop != NULL);

   pDesktop->initializeAction(pCommand, shortcutContext);

   // Add the command to the menu
   pMenu->insertAction(pBefore, pCommand);

   return true;
}

QAction* MenuBarImp::getMenuItem(const string& location) const
{
   if (location.empty() == true)
   {
      return NULL;
   }

   QString strLocation = QString::fromStdString(location);
   strLocation.replace(QRegExp("/"), "\\");
   if (strLocation.startsWith("\\") == true)
   {
      strLocation = strLocation.mid(1);
   }

   QMenu* pParentMenu = NULL;
   QAction* pAction = NULL;

   while (strLocation.isEmpty() == false)
   {
      int iPos = strLocation.indexOf("\\");
      QString strMenuItem = strLocation.left(iPos);

      pAction = getAction(strMenuItem, pParentMenu);
      if (pAction == NULL)
      {
         return NULL;
      }

      if (isMenu(pAction) == true)
      {
         pParentMenu = getMenu(pAction);
      }
      else if (iPos != -1)
      {
         return NULL;
      }
      else
      {
         return pAction;
      }

      if (iPos != -1)
      {
         int iLength = strLocation.length();
         strLocation = strLocation.right(iLength - iPos - 1);
      }
      else
      {
         strLocation.clear();
      }
   }

   return pAction;
}

QAction* MenuBarImp::getMenuItem(QMenu* pMenu) const
{
   if (pMenu == NULL)
   {
      return NULL;
   }

   if (mMenus.contains(pMenu) == true)
   {
      QAction* pAction = pMenu->menuAction();
      return pAction;
   }

   return NULL;
}

QMenu* MenuBarImp::getMenu(QAction* pAction) const
{
   if (pAction == NULL)
   {
      return NULL;
   }

   QList<QMenu*>::const_iterator iter;
   for (iter = mMenus.begin(); iter != mMenus.end(); ++iter)
   {
      QMenu* pMenu = *iter;
      if (pMenu != NULL)
      {
         QAction* pCurrentAction = pMenu->menuAction();
         if (pCurrentAction == pAction)
         {
            return pMenu;
         }
      }
   }

   return NULL;
}

bool MenuBarImp::isMenu(QAction* pAction) const
{
   QMenu* pMenu = getMenu(pAction);
   return (pMenu != NULL);
}

bool MenuBarImp::isCommand(QAction* pAction) const
{
   // Check for a menu
   if (isMenu(pAction) == true)
   {
      return false;
   }

   // Check the menu bar
   QList<QAction*> menuBarActions = actions();
   if (menuBarActions.contains(pAction) == true)
   {
      return true;
   }

   // Check the popup menus
   QList<QMenu*>::const_iterator iter;
   for (iter = mMenus.begin(); iter != mMenus.end(); ++iter)
   {
      // Get the current menu pointer
      QMenu* pMenu = *iter;
      if (pMenu != NULL)
      {
         QList<QAction*> menuActions = pMenu->actions();
         if (menuActions.contains(pAction) == true)
         {
            return true;
         }
      }
   }

   return false;
}

bool MenuBarImp::removeMenu(QMenu* pMenu)
{
   if (pMenu == NULL)
   {
      return false;
   }

   QAction* pAction = pMenu->menuAction();
   return removeMenuItem(pAction);
}

bool MenuBarImp::removeMenuItem(QAction* pAction)
{
   if (pAction == NULL)
   {
      return false;
   }

   // Check if the item is a menu on the menu bar
   QList<QAction*> menuBarActions = actions();
   if (menuBarActions.contains(pAction) == true)
   {
      // Remove the menu from the menu bar
      removeAction(pAction);

      // Delete the menu
      QMenu* pMenu = getMenu(pAction);
      if (pMenu != NULL)
      {
         mMenus.removeAll(pMenu);
         delete pMenu;
      }

      return true;
   }

   // Check all the menus for the action
   QList<QMenu*>::iterator iter;
   for (iter = mMenus.begin(); iter != mMenus.end(); ++iter)
   {
      // Get the current menu pointer
      QMenu* pParentMenu = *iter;
      if (pParentMenu != NULL)
      {
         QList<QAction*> menuActions = pParentMenu->actions();
         if (menuActions.contains(pAction) == true)
         {
            // Remove the action from the menu
            pParentMenu->removeAction(pAction);

            // Delete the menu
            QMenu* pMenu = getMenu(pAction);
            if (pMenu != NULL)
            {
               mMenus.removeAll(pMenu);
               delete pMenu;
            }

            // Remove the menu if all items have been removed
            if (pParentMenu->actions().empty() == true)
            {
               removeMenu(pParentMenu);
            }

            return true;
         }
      }
   }

   return false;
}

QList<QAction*> MenuBarImp::getAllActions() const
{
   // Menu bar
   QList<QAction*> allActions = actions();

   // Popup menus
   QList<QMenu*>::const_iterator iter;
   for (iter = mMenus.begin(); iter != mMenus.end(); ++iter)
   {
      QMenu* pMenu = *iter;
      if (pMenu != NULL)
      {
         allActions += pMenu->actions();
      }
   }

   return allActions;
}

QList<QAction*> MenuBarImp::getMenuActions() const
{
   QList<QAction*> menuActions;

   QList<QMenu*>::const_iterator iter;
   for (iter = mMenus.begin(); iter != mMenus.end(); ++iter)
   {
      QMenu* pMenu = *iter;
      if (pMenu != NULL)
      {
         QAction* pAction = pMenu->menuAction();
         if (pAction != NULL)
         {
            menuActions.push_back(pAction);
         }
      }
   }

   return menuActions;
}

QList<QAction*> MenuBarImp::getCommandActions() const
{
   QList<QAction*> commandActions;
   QList<QAction*> allActions = getAllActions();

   for (int i = 0; i < allActions.count(); i++)
   {
      QAction* pAction = allActions[i];
      if (pAction != NULL)
      {
         bool bCommand = isCommand(pAction);
         if (bCommand == true)
         {
            commandActions.push_back(pAction);
         }
      }
   }

   return commandActions;
}

QAction* MenuBarImp::getAction(const QString& strMenuItem, QMenu* pMenu) const
{
   if (strMenuItem.isEmpty() == true)
   {
      return NULL;
   }

   if (pMenu == NULL)
   {
      QList<QAction*> barActions = actions();
      for (int i = 0; i < barActions.count(); ++i)
      {
         QAction* pAction = barActions[i];
         if (pAction != NULL)
         {
            QString strCurrentName = pAction->text();
            if (strCurrentName == strMenuItem)
            {
               return pAction;
            }
         }
      }
   }
   else
   {
      QList<QAction*> menuActions = pMenu->actions();
      for (int i = 0; i < menuActions.count(); ++i)
      {
         QAction* pAction = menuActions[i];
         if (pAction != NULL)
         {
            QString strCurrentName = pAction->text();
            if (strCurrentName == strMenuItem)
            {
               return pAction;
            }
         }
      }
   }

   return NULL;
}
