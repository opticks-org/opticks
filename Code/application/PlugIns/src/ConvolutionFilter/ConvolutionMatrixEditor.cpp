/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "ConvolutionMatrixEditor.h"
#include "ConvolutionMatrixWidget.h"
#include "DesktopServices.h"
#include "DockWindow.h"
#include "MenuBar.h"
#include "PlugInFactory.h"

#include <QtGui/QAction>
#include <QtGui/QMenu>

PLUGINFACTORY(ConvolutionMatrixEditor);

ConvolutionMatrixEditor::ConvolutionMatrixEditor()
{
   setName("Convolution Matrix Editor");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Apply convolution matrix filtering to the active dataset.");
   setDescriptorId("{130E18DB-3974-4EC7-AC18-91BA2F5A4181}");
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

ConvolutionMatrixEditor::~ConvolutionMatrixEditor()
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

bool ConvolutionMatrixEditor::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
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

QAction* ConvolutionMatrixEditor::createAction()
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

      pWindowAction = pMenuBar->addCommand("&Tools/&Convolution Matrix Editor", getName(), pBeforeAction);
      if (pWindowAction != NULL)
      {
         pWindowAction->setToolTip("Convolution Matrix Editor");
         pWindowAction->setStatusTip("Toggles the display of the Convolution Matrix Editor");
      }
   }
   return pWindowAction;
}

QWidget* ConvolutionMatrixEditor::createWidget()
{
   return new ConvolutionMatrixWidget(Service<DesktopServices>()->getMainWidget());
}
