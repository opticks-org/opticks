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
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "SessionManager.h"
#include "Slot.h"
#include "XercesIncludes.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <QtGui/QAction>
#include <QtGui/QMenu>

XERCES_CPP_NAMESPACE_USE

PLUGINFACTORY(ConvolutionMatrixEditor);

ConvolutionMatrixEditor::ConvolutionMatrixEditor() : mpWindowAction(NULL)
{
   AlgorithmShell::setName("Convolution Matrix Editor");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Apply convolution matrix filtering to the active dataset.");
   executeOnStartup(true);
   setDescriptorId("{130E18DB-3974-4EC7-AC18-91BA2F5A4181}");
   allowMultipleInstances(false);
   destroyAfterExecute(false);
   setWizardSupported(false);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

ConvolutionMatrixEditor::~ConvolutionMatrixEditor()
{
   if (mpWindowAction != NULL)
   {
      MenuBar* pMenuBar = Service<DesktopServices>()->getMainMenuBar();
      if (pMenuBar != NULL)
      {
         pMenuBar->removeMenuItem(mpWindowAction);
      }

      if (Service<DesktopServices>()->getMainWidget() != NULL)
      {
         delete mpWindowAction;
      }
   }
   Window* pWindow = Service<DesktopServices>()->getWindow(getWindowName(), DOCK_WINDOW);
   if (pWindow != NULL)
   {
      pWindow->detach(SIGNAL_NAME(DockWindow, Shown), Slot(this, &ConvolutionMatrixEditor::windowShown));
      pWindow->detach(SIGNAL_NAME(DockWindow, Hidden), Slot(this, &ConvolutionMatrixEditor::windowHidden));
      Service<DesktopServices>()->deleteWindow(pWindow);
   }
}

void ConvolutionMatrixEditor::windowHidden(Subject& subject, const std::string& signal, const boost::any& v)
{
   DockWindow* pWindow = static_cast<DockWindow*>(Service<DesktopServices>()->getWindow(getWindowName(), DOCK_WINDOW));
   if (pWindow != NULL)
   {
      if ((dynamic_cast<DockWindow*>(&subject) == pWindow) && (mpWindowAction != NULL))
      {
         mpWindowAction->setChecked(false);
      }
   }
}

void ConvolutionMatrixEditor::windowShown(Subject& subject, const std::string& signal, const boost::any& v)
{
   DockWindow* pWindow = static_cast<DockWindow*>(Service<DesktopServices>()->getWindow(getWindowName(), DOCK_WINDOW));
   if (pWindow != NULL)
   {
      if ((dynamic_cast<DockWindow*>(&subject) == pWindow) && (mpWindowAction != NULL))
      {
         mpWindowAction->setChecked(true);
      }
   }
}

bool ConvolutionMatrixEditor::execute(PlugInArgList* pInputArgList, PlugInArgList* pOutputArgList)
{
   createMenuItem();
   return createEditorWindow() && mpWindowAction != NULL;
}

void ConvolutionMatrixEditor::createMenuItem()
{
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

      mpWindowAction = pMenuBar->addCommand("&Tools/&" + getWindowName(), getName(), pBeforeAction);
      if (mpWindowAction != NULL)
      {
         mpWindowAction->setAutoRepeat(false);
         mpWindowAction->setCheckable(true);
         mpWindowAction->setToolTip(QString::fromStdString(getWindowName()));
         mpWindowAction->setStatusTip("Toggles the display of the " + QString::fromStdString(getWindowName()));
         connect(mpWindowAction, SIGNAL(triggered(bool)), this, SLOT(displayEditorWindow(bool)));
      }
   }
}

bool ConvolutionMatrixEditor::createEditorWindow()
{
   // Create the editor window
   if (mpWindowAction != NULL)
   {
      Service<DesktopServices> pDesktop;
      DockWindow* pEditorWindow = static_cast<DockWindow*>(
         pDesktop->getWindow(getWindowName(), DOCK_WINDOW));
      if (pEditorWindow == NULL)
      {
         pEditorWindow = static_cast<DockWindow*>(pDesktop->createWindow(getWindowName(), DOCK_WINDOW));
         if (pEditorWindow != NULL)
         {
            pDesktop->setDockWindowArea(pEditorWindow, DOCK_LEFT);
            attachToEditorWindow(pEditorWindow);
            pEditorWindow->hide();
         }
         else
         {
            return false;
         }
      }
   }
   return true;
}

void ConvolutionMatrixEditor::attachToEditorWindow(DockWindow* pDockWindow)
{
   if (pDockWindow)
   {
      pDockWindow->attach(SIGNAL_NAME(DockWindow, Shown), Slot(this, &ConvolutionMatrixEditor::windowShown));
      pDockWindow->attach(SIGNAL_NAME(DockWindow, Hidden), Slot(this, &ConvolutionMatrixEditor::windowHidden));

      QWidget* pWidget = new ConvolutionMatrixWidget(Service<DesktopServices>()->getMainWidget());
      if (pWidget != NULL)
      {
         pDockWindow->setWidget(pWidget);
      }
   }
}

void ConvolutionMatrixEditor::displayEditorWindow(bool bDisplay)
{
   DockWindow* pEditorWindow = static_cast<DockWindow*>(
      Service<DesktopServices>()->getWindow(getWindowName(), DOCK_WINDOW));
   if (pEditorWindow != NULL)
   {
      if (bDisplay == true)
      {
         pEditorWindow->show();
      }
      else
      {
         pEditorWindow->hide();
      }
   }
}

bool ConvolutionMatrixEditor::serialize(SessionItemSerializer& serializer) const
{
   if (mpWindowAction != NULL)
   {
      XMLWriter writer("ConvolutionMatrixEditor");
      writer.addAttr("shown", mpWindowAction->isChecked());
      DockWindow* pCmeWindow = static_cast<DockWindow*>(
         Service<DesktopServices>()->getWindow(getWindowName(), DOCK_WINDOW));
      if (pCmeWindow)
      {
         writer.addAttr("windowId", pCmeWindow->getId());
         return serializer.serialize(writer);
      }
   }
   return false;
}

bool ConvolutionMatrixEditor::deserialize(SessionItemDeserializer& deserializer)
{
   createMenuItem();

   if (mpWindowAction != NULL)
   {
      XmlReader reader(NULL, false);
      DOMElement* pRootElement = deserializer.deserialize(reader, "ConvolutionMatrixEditor");
      if (pRootElement)
      {
         std::string windowId = A(pRootElement->getAttribute(X("windowId")));
         DockWindow* pCmeWindow = dynamic_cast<DockWindow*>(Service<SessionManager>()->getSessionItem(windowId));
         if (pCmeWindow != NULL)
         {
            attachToEditorWindow(pCmeWindow);
            bool shown = XmlReader::StringStreamAssigner<bool>()(A(pRootElement->getAttribute(X("shown"))));
            mpWindowAction->setChecked(shown);
            return true;
         }
      }
   }

   return false;
}
