/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationImagePalette.h"
#include "AnnotationImagePaletteWidget.h"
#include "AppVersion.h"
#include "DesktopServices.h"
#include "DockWindow.h"
#include "MenuBar.h"
#include "PlugInRegistration.h"

#include <QtGui/QAction>
#include <QtGui/QMenu>

REGISTER_PLUGIN_BASIC(OpticksAnnotationImagePalette, AnnotationImagePalette);

AnnotationImagePalette::AnnotationImagePalette()
{
   setName("Annotation Image Palette");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Drag and drop thumbnails into an annotation layer.");
   setDescriptorId("{48828156-5A17-4EA2-8575-2C8E8ED205E1}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

AnnotationImagePalette::~AnnotationImagePalette()
{
   QAction* pWindowAction = getAction();
   if (pWindowAction != NULL)
   {
      MenuBar* pMenuBar = Service<DesktopServices>()->getMainMenuBar();
      if (pMenuBar != NULL)
      {
         pMenuBar->removeMenuItem(pWindowAction);
      }
      if (Service<DesktopServices>()->getMainWidget() != NULL)
      {
         delete pWindowAction;
      }
   }
}

bool AnnotationImagePalette::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (!DockWindowShell::execute(pInArgList, pOutArgList))
   {
      return false;
   }
   DockWindow* pWindow = getDockWindow();
   if (pWindow != NULL)
   {
      Service<DesktopServices>()->setDockWindowArea(pWindow, DOCK_LEFT);
   }
   return true;
}

QAction* AnnotationImagePalette::createAction()
{
   QAction* pWindowAction = NULL;

   // Add a menu command to invoke the window
   MenuBar* pMenuBar = Service<DesktopServices>()->getMainMenuBar();
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

      pWindowAction = pMenuBar->addCommand("&Tools/&Annotation Image Palette", getName(), pBeforeAction);
      if (pWindowAction != NULL)
      {
         pWindowAction->setToolTip("Annotation Image Palette");
         pWindowAction->setStatusTip("Toggles the display of the Annotation Image Palette");
      }
   }
   return pWindowAction;
}

QWidget* AnnotationImagePalette::createWidget()
{
   return new AnnotationImagePaletteWidget(Service<DesktopServices>()->getMainWidget());
}