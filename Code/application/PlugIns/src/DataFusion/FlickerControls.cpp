/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QMenu>
#include <QtGui/QWidget>

#include "FlickerControls.h"
#include "AppVersion.h"
#include "DockWindow.h"
#include "ImageAdjustWidget.h"
#include "MenuBar.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "Slot.h"
#include "SpatialDataWindow.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <sstream>

XERCES_CPP_NAMESPACE_USE

FlickerControls::FlickerControls() :
   mpWindowAction(NULL)
{
   AlgorithmShell::setName("Flicker Controls");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Manipulates the display of the primary raster layer and "
                  "the topmost raster layer.");
   executeOnStartup(true);
   setDescriptorId("{99CBDAC8-9E7E-4e6d-A3A7-F6936A84E9BF}");
   allowMultipleInstances(false);
   destroyAfterExecute(false);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

FlickerControls::~FlickerControls()
{
   if (mpWindowAction != NULL)
   {
      MenuBar* pMenuBar = mpDesktop->getMainMenuBar();
      if (pMenuBar != NULL)
      {
         pMenuBar->removeMenuItem(mpWindowAction);
      }

      if (mpDesktop->getMainWidget() != NULL)
      {
         delete mpWindowAction;
      }
   }

   Window* pWindow = mpDesktop->getWindow("Flicker Window", DOCK_WINDOW);
   if (pWindow != NULL)
   {
      pWindow->detach(SIGNAL_NAME(DockWindow, Shown), Slot(this, &FlickerControls::windowShown));
      pWindow->detach(SIGNAL_NAME(DockWindow, Hidden), Slot(this, &FlickerControls::windowHidden));
      mpDesktop->deleteWindow(pWindow);
   }
}

void FlickerControls::windowHidden(Subject& subject, const std::string &signal, const boost::any& v)
{
   DockWindow* pWindow = static_cast<DockWindow*>(mpDesktop->getWindow("Flicker Window", DOCK_WINDOW));
   if (pWindow != NULL)
   {
      if ((dynamic_cast<DockWindow*>(&subject) == pWindow) && (mpWindowAction != NULL))
      {
         mpWindowAction->setChecked(false);
      }
   }
}

void FlickerControls::windowShown(Subject& subject, const std::string &signal, const boost::any& v)
{
   DockWindow* pWindow = static_cast<DockWindow*>(mpDesktop->getWindow("Flicker Window", DOCK_WINDOW));
   if (pWindow != NULL)
   {
      if ((dynamic_cast<DockWindow*>(&subject) == pWindow) && (mpWindowAction != NULL))
      {
         mpWindowAction->setChecked(true);
      }
   }
}

bool FlickerControls::execute(PlugInArgList *pInputArgList, PlugInArgList *pOutputArgList)
{
   createMenuItem();

   return createFlickerWindow() && mpWindowAction != NULL;
}

void FlickerControls::createMenuItem()
{
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

      mpWindowAction = pMenuBar->addCommand("&Tools/&Flicker Window", getName(), pBeforeAction);
      if (mpWindowAction != NULL)
      {
         mpWindowAction->setAutoRepeat(false);
         mpWindowAction->setCheckable(true);
         mpWindowAction->setToolTip("Flicker Window");
         mpWindowAction->setStatusTip("Toggles the display of the Flicker Window");
         connect(mpWindowAction, SIGNAL(triggered(bool)), this, SLOT(displayFlickerWindow(bool)));
      }
   }
}

bool FlickerControls::createFlickerWindow()
{
   // Create the flicker window
   if (mpWindowAction != NULL)
   {
      DockWindow* pFlickerWindow = static_cast<DockWindow*>(mpDesktop->getWindow("Flicker Window", DOCK_WINDOW));
      if (pFlickerWindow == NULL)
      {
         pFlickerWindow = static_cast<DockWindow*>(mpDesktop->createWindow("Flicker Window", DOCK_WINDOW));
         if (pFlickerWindow != NULL)
         {
            attachToFlickerWindow(pFlickerWindow);

            pFlickerWindow->hide();
         }
         else
         {
            return false;
         }
      }
   }
   return true;
}

void FlickerControls::attachToFlickerWindow(DockWindow *pFlickerWindow)
{
   if (pFlickerWindow)
   {
      pFlickerWindow->attach(SIGNAL_NAME(DockWindow, Shown), Slot(this, &FlickerControls::windowShown));
      pFlickerWindow->attach(SIGNAL_NAME(DockWindow, Hidden), Slot(this, &FlickerControls::windowHidden));

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(mpDesktop->getCurrentWorkspaceWindow());

      QWidget* pWidget = new ImageAdjustWidget(pWindow, mpDesktop->getMainWidget());
      if (pWidget != NULL)
      {
         pFlickerWindow->setWidget(pWidget);
      }
   }
}

void FlickerControls::displayFlickerWindow(bool bDisplay)
{
   DockWindow* pFlickerWindow = static_cast<DockWindow*>(mpDesktop->getWindow("Flicker Window", DOCK_WINDOW));
   if (pFlickerWindow != NULL)
   {
      if (bDisplay == true)
      {
         pFlickerWindow->show();
      }
      else
      {
         pFlickerWindow->hide();
      }
   }
}

bool FlickerControls::serialize(SessionItemSerializer &serializer) const
{
   if (mpWindowAction != NULL)
   {
      XMLWriter writer("FlickerControls");
      writer.addAttr("shown", mpWindowAction->isChecked());
      DockWindow* pFlickerWindow = static_cast<DockWindow*>(mpDesktop->getWindow("Flicker Window", DOCK_WINDOW));
      if (pFlickerWindow)
      {
         writer.addAttr("windowId", pFlickerWindow->getId());
         return serializer.serialize(writer);
      }
   }
   return false;
}

bool FlickerControls::deserialize(SessionItemDeserializer &deserializer)
{
   createMenuItem();

   if (mpWindowAction != NULL)
   {
      XmlReader reader(NULL, false);
      DOMElement* pRootElement = deserializer.deserialize(reader, "FlickerControls");
      if (pRootElement)
      {
         std::string windowId = A(pRootElement->getAttribute(X("windowId")));
         DockWindow *pFlickerWindow = dynamic_cast<DockWindow*>(Service<SessionManager>()->getSessionItem(windowId));
         if (pFlickerWindow != NULL)
         {
            attachToFlickerWindow(pFlickerWindow);
            bool shown = XmlReader::StringStreamAssigner<bool>()(A(pRootElement->getAttribute(X("shown"))));
            mpWindowAction->setChecked(shown);
            return true;
         }
      }
   }
   return false;
}
