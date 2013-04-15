/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QMenu>

#include "AppVersion.h"
#include "DesktopServices.h"
#include "FlickerControls.h"
#include "ImageAdjustWidget.h"
#include "MenuBar.h"
#include "PlugInRegistration.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SpatialDataWindow.h"

REGISTER_PLUGIN_BASIC(OpticksDataFusion, FlickerControls);

FlickerControls::FlickerControls()
{
   setName("Flicker Controls");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Manipulates the display of the primary raster layer and the topmost raster layer.");
   setDescriptorId("{99CBDAC8-9E7E-4e6d-A3A7-F6936A84E9BF}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

FlickerControls::~FlickerControls()
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
   }
}

QAction* FlickerControls::createAction()
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
   QAction* pAction = pMenuBar->addCommand("&Tools/&Flicker Controls", getName(), pBeforeAction);
   if (pAction != NULL)
   {
      pAction->setToolTip("Flicker Controls Window");
      pAction->setStatusTip("Toggles the display of the Flicker Controls Window");
   }

   return pAction;
}

QWidget* FlickerControls::createWidget()
{
   Service<DesktopServices> pDesktop;

   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getCurrentWorkspaceWindow());
   return new ImageAdjustWidget(pWindow, pDesktop->getMainWidget());
}

bool FlickerControls::serialize(SessionItemSerializer& serializer) const
{
   if (!DockWindowShell::serialize(serializer))
   {
      return false;
   }

   serializer.endBlock();
   if (getWidget() == NULL)
   {
      return false;
   }
   return static_cast<ImageAdjustWidget*>(getWidget())->serialize(serializer);
}

bool FlickerControls::deserialize(SessionItemDeserializer& deserializer)
{
   if (!DockWindowShell::deserialize(deserializer))
   {
      return false;
   }

   deserializer.nextBlock();
   if (getWidget() == NULL)
   {
      return false;
   }
   return static_cast<ImageAdjustWidget*>(getWidget())->deserialize(deserializer);
}