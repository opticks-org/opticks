/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QBitmap>
#include <QtGui/QMenu>

#include "DesktopServices.h"
#include "DockWindowWidget.h"
#include "MenuBar.h"
#include "PlotManager.h"
#include "ToolBar.h"

static const char* const PlotManagerIcon[] =
{
   "16 17 6 1",
   "# c #000000",
   "a c #000080",
   "d c #0000ff",
   "c c #808080",
   "b c #c0c0c0",
   ". c #ffffff",
   "................",
   ".......######aaa",
   "......#b....baaa",
   "#....#.#......aa",
   "c...#.#.#....caa",
   "#..#.#.#.#bbb#aa",
   "c.#.#.#.#.###.aa",
   "#.c#...#.#.....a",
   "c.......#c......",
   "#.dd.......dd...",
   "c.dd.dd....dd...",
   "#.dd.dd....dd...",
   "c.dd.dd.dd.dd...",
   "#.dd.dd.dd.dd...",
   "c.dd.dd.dd.dd...",
   "################",
   "................"
};

PlotManager::PlotManager()
{
   setName(PLOT_MANAGER_NAME);
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Provides access to plot windows and their plots.");
   setDescriptorId("{2204D4DE-7F7E-455f-B4AB-CD730A9AA0A2}");
   setProductionStatus(false);
}

PlotManager::~PlotManager()
{
   QAction* pAction = getAction();
   if (pAction != NULL)
   {
      Service<DesktopServices> pDesktop;

      // Remove the menu command
      MenuBar* pMenuBar = pDesktop->getMainMenuBar();
      if (pMenuBar != NULL)
      {
         pMenuBar->removeMenuItem(pAction);
      }

      // Remove the toolbar button
      ToolBar* pToolBar = static_cast<ToolBar*>(pDesktop->getWindow("Demo", TOOLBAR));
      if (pToolBar != NULL)
      {
         pToolBar->removeItem(pAction);
      }

      // Delete the action
      if (pDesktop->getMainWidget() != NULL)
      {
         delete pAction;
      }
   }
}

QAction* PlotManager::createAction()
{
   Service<DesktopServices> pDesktop;

   MenuBar* pMenuBar = pDesktop->getMainMenuBar();
   if (pMenuBar == NULL)
   {
      return NULL;
   }

   QAction* pBeforeAction = NULL;

   QAction* pToolsAction = pMenuBar->getMenuItem("&Tools");
   if (pToolsAction != NULL)
   {
      QMenu* pMenu = pToolsAction->menu();
      if (pMenu != NULL)
      {
         QList<QAction*> actions = pMenu->actions();
         for (int i = 0; i < actions.count(); ++i)
         {
            QAction* pAction = actions[i];
            if (pAction != NULL)
            {
               if ((pAction->text() == "S&cripting Window") && (pAction != actions.back()))
               {
                  pBeforeAction = actions[i + 1];
                  break;
               }
            }
         }
      }
   }

   // Create an action on the menu bar
   QAction* pAction = pMenuBar->addCommand("&Tools/&Plot Manager", getName(), pBeforeAction);
   if (pAction != NULL)
   {
      QPixmap windowPix = QPixmap(PlotManagerIcon);
      windowPix.setMask(windowPix.createHeuristicMask());
      QIcon windowIcon(windowPix);

      pAction->setIcon(windowIcon);
      pAction->setToolTip(PLOT_MANAGER_NAME);
      pAction->setStatusTip("Toggles the display of the Plot Manager window");

      // Add the action to the toolbar
      ToolBar* pToolBar = static_cast<ToolBar*>(pDesktop->getWindow("Demo", TOOLBAR));
      if (pToolBar != NULL)
      {
         pToolBar->addSeparator();
         pToolBar->addButton(pAction);
      }
   }

   return pAction;
}

QWidget* PlotManager::createWidget()
{
   Service<DesktopServices> pDesktop;
   return new DockWindowWidget(pDesktop->getMainWidget());
}
