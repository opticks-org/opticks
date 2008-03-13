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

#include "AppVersion.h"
#include "PlotManager.h"
#include "DesktopServices.h"
#include "DockWindow.h"
#include "DockWindowWidget.h"
#include "MenuBar.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "Slot.h"
#include "ToolBar.h"
#include "xmlreader.h"
#include "xmlwriter.h"

XERCES_CPP_NAMESPACE_USE

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

PlotManager::PlotManager() :
   mpWindowAction(NULL)
{
   AlgorithmShell::setName("Plot Manager");
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Provides access to plot windows and their plots.");
   setProductionStatus(false);
   setDescriptorId("{2204D4DE-7F7E-455f-B4AB-CD730A9AA0A2}");
   allowMultipleInstances(false);
   executeOnStartup(true);
   destroyAfterExecute(false);
   setInteractive();
}

PlotManager::~PlotManager()
{
   if (mpWindowAction != NULL)
   {
      MenuBar* pMenuBar = mpDesktop->getMainMenuBar();
      if (pMenuBar != NULL)
      {
         pMenuBar->removeMenuItem(mpWindowAction);
      }

      ToolBar* pToolBar = static_cast<ToolBar*>(mpDesktop->getWindow("Demo", TOOLBAR));
      if (pToolBar != NULL)
      {
         pToolBar->removeItem(mpWindowAction);
      }

      if (mpDesktop->getMainWidget() != NULL)
      {
         delete mpWindowAction;
      }
   }

   Window* pWindow = mpDesktop->getWindow("Plot Manager", DOCK_WINDOW);
   if (pWindow != NULL)
   {
      pWindow->detach(SIGNAL_NAME(DockWindow, Shown), Slot(this, &PlotManager::windowShown));
      pWindow->detach(SIGNAL_NAME(DockWindow, Hidden), Slot(this, &PlotManager::windowHidden));
      mpDesktop->deleteWindow(pWindow);
   }
}

void PlotManager::windowHidden(Subject& subject, const std::string &signal, const boost::any& v)
{
   DockWindow* pWindow = static_cast<DockWindow*>(mpDesktop->getWindow("Plot Manager", DOCK_WINDOW));
   if (pWindow != NULL)
   {
      if ((dynamic_cast<DockWindow*>(&subject) == pWindow) && (mpWindowAction != NULL))
      {
         mpWindowAction->setChecked(false);
      }
   }
}

void PlotManager::windowShown(Subject& subject, const std::string &signal, const boost::any& v)
{
   DockWindow* pWindow = static_cast<DockWindow*>(mpDesktop->getWindow("Plot Manager", DOCK_WINDOW));
   if (pWindow != NULL)
   {
      if ((dynamic_cast<DockWindow*>(&subject) == pWindow) && (mpWindowAction != NULL))
      {
         mpWindowAction->setChecked(true);
      }
   }
}

bool PlotManager::setBatch()
{
   AlgorithmShell::setBatch();
   return false;
}

bool PlotManager::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool PlotManager::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool PlotManager::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   QPixmap windowPix = QPixmap(PlotManagerIcon);
   windowPix.setMask(windowPix.createHeuristicMask());
   QIcon windowIcon(windowPix);

   // Add a menu command to invoke the window
   MenuBar* pMenuBar = mpDesktop->getMainMenuBar();
   if (pMenuBar != NULL)
   {
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

      mpWindowAction = pMenuBar->addCommand("&Tools/&Plot Manager", getName(), pBeforeAction);
      if (mpWindowAction != NULL)
      {
         mpWindowAction->setAutoRepeat(false);
         mpWindowAction->setIcon(windowIcon);
         mpWindowAction->setCheckable(true);
         mpWindowAction->setToolTip("Plot Manager");
         mpWindowAction->setStatusTip("Toggles the display of the Plot Manager window");
         connect(mpWindowAction, SIGNAL(triggered(bool)), this, SLOT(displayPlotManager(bool)));

         // Add the action to the toolbar
         ToolBar* pToolBar = static_cast<ToolBar*>(mpDesktop->getWindow("Demo", TOOLBAR));
         if (pToolBar != NULL)
         {
            pToolBar->addSeparator();
            pToolBar->addButton(mpWindowAction);
         }
      }
   }

   // Create the plot manager dock window
   if (mpWindowAction != NULL)
   {
      bool created = true;
      DockWindow* pWindow  = static_cast<DockWindow*>(mpDesktop->createWindow("Plot Manager", DOCK_WINDOW));
      if (pWindow == NULL)
      {
         pWindow  = static_cast<DockWindow*>(mpDesktop->getWindow("Plot Manager", DOCK_WINDOW));
         created = false;
      }
      if (pWindow != NULL)
      {
         DockWindowWidget* pWidget = new DockWindowWidget(mpDesktop->getMainWidget());
         if (pWidget != NULL)
         {
            pWindow->setWidget(pWidget);
         }

         pWindow->setIcon(windowIcon);
         pWindow->attach(SIGNAL_NAME(DockWindow, Shown), Slot(this, &PlotManager::windowShown));
         pWindow->attach(SIGNAL_NAME(DockWindow, Hidden), Slot(this, &PlotManager::windowHidden));
         if (created)
         {
            pWindow->hide();
         }
      }
      else
      {
         return false;
      }
   }

   return (mpWindowAction != NULL);
}

void PlotManager::displayPlotManager(bool bDisplay)
{
   DockWindow* pWindow = static_cast<DockWindow*>(mpDesktop->getWindow("Plot Manager", DOCK_WINDOW));
   if (pWindow != NULL)
   {
      if (bDisplay == true)
      {
         pWindow->show();
      }
      else
      {
         pWindow->hide();
      }
   }
}

bool PlotManager::serialize(SessionItemSerializer &serializer) const
{
   if (mpWindowAction != NULL)
   {
      XMLWriter writer("PlotManager");
      writer.addAttr("shown", mpWindowAction->isChecked());
      return serializer.serialize(writer);
   }
   else
   {
      return false;
   }
}

bool PlotManager::deserialize(SessionItemDeserializer &deserializer)
{
   execute(NULL, NULL);
   if (mpWindowAction != NULL)
   {
      XmlReader reader(NULL, false);
      DOMElement* pRootElement = deserializer.deserialize(reader, "PlotManager");
      if (pRootElement)
      {
         bool shown = XmlReader::StringStreamAssigner<bool>()(A(pRootElement->getAttribute(X("shown"))));
         mpWindowAction->setChecked(shown);
         return true;
      }
   }
   return false;
}
