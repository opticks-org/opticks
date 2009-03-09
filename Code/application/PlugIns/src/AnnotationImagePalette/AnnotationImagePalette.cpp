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

PLUGINFACTORY(AnnotationImagePalette);

AnnotationImagePalette::AnnotationImagePalette() : mpWindowAction(NULL)
{
   AlgorithmShell::setName("Annotation Image Palette");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setDescription("Drag and drop thumbnails into an annotation layer.");
   executeOnStartup(true);
   setDescriptorId("{48828156-5A17-4EA2-8575-2C8E8ED205E1}");
   allowMultipleInstances(false);
   destroyAfterExecute(false);
   setWizardSupported(false);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

AnnotationImagePalette::~AnnotationImagePalette()
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
      pWindow->detach(SIGNAL_NAME(DockWindow, Shown), Slot(this, &AnnotationImagePalette::windowShown));
      pWindow->detach(SIGNAL_NAME(DockWindow, Hidden), Slot(this, &AnnotationImagePalette::windowHidden));
      Service<DesktopServices>()->deleteWindow(pWindow);
   }
}

void AnnotationImagePalette::windowHidden(Subject& subject, const std::string& signal, const boost::any& v)
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

void AnnotationImagePalette::windowShown(Subject& subject, const std::string& signal, const boost::any& v)
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

bool AnnotationImagePalette::execute(PlugInArgList* pInputArgList, PlugInArgList* pOutputArgList)
{
   createMenuItem();
   return createWindow() && mpWindowAction != NULL;
}

void AnnotationImagePalette::createMenuItem()
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
         connect(mpWindowAction, SIGNAL(triggered(bool)), this, SLOT(displayWindow(bool)));
      }
   }
}

bool AnnotationImagePalette::createWindow()
{
   // Create the editor window
   if (mpWindowAction != NULL)
   {
      Service<DesktopServices> pDesktop;
      DockWindow* pWindow = static_cast<DockWindow*>(pDesktop->getWindow(getWindowName(), DOCK_WINDOW));
      if (pWindow == NULL)
      {
         pWindow = static_cast<DockWindow*>(pDesktop->createWindow(getWindowName(), DOCK_WINDOW));
         if (pWindow != NULL)
         {
            pDesktop->setDockWindowArea(pWindow, DOCK_LEFT);
            attachToWindow(pWindow);
            pWindow->hide();
         }
         else
         {
            return false;
         }
      }
   }
   return true;
}

void AnnotationImagePalette::attachToWindow(DockWindow* pDockWindow)
{
   if (pDockWindow)
   {
      pDockWindow->attach(SIGNAL_NAME(DockWindow, Shown), Slot(this, &AnnotationImagePalette::windowShown));
      pDockWindow->attach(SIGNAL_NAME(DockWindow, Hidden), Slot(this, &AnnotationImagePalette::windowHidden));

      QWidget* pWidget = createWidget();
      if (pWidget != NULL)
      {
         pDockWindow->setWidget(pWidget);
      }
   }
}

void AnnotationImagePalette::displayWindow(bool bDisplay)
{
   DockWindow* pWindow = static_cast<DockWindow*>(Service<DesktopServices>()->getWindow(getWindowName(), DOCK_WINDOW));
   if (pWindow != NULL)
   {
      if (bDisplay)
      {
         pWindow->show();
      }
      else
      {
         pWindow->hide();
      }
   }
}

QWidget* AnnotationImagePalette::createWidget() const
{
   return new AnnotationImagePaletteWidget(Service<DesktopServices>()->getMainWidget());
}

bool AnnotationImagePalette::serialize(SessionItemSerializer& serializer) const
{
   if (mpWindowAction != NULL)
   {
      XMLWriter writer("AnnotationImagePalette");
      writer.addAttr("shown", mpWindowAction->isChecked());
      DockWindow* pAipWindow = static_cast<DockWindow*>(
         Service<DesktopServices>()->getWindow(getWindowName(), DOCK_WINDOW));
      if (pAipWindow)
      {
         writer.addAttr("windowId", pAipWindow->getId());
         return serializer.serialize(writer);
      }
   }
   return false;
}

bool AnnotationImagePalette::deserialize(SessionItemDeserializer& deserializer)
{
   createMenuItem();

   if (mpWindowAction != NULL)
   {
      XmlReader reader(NULL, false);
      DOMElement* pRootElement = deserializer.deserialize(reader, "AnnotationImagePalette");
      if (pRootElement)
      {
         std::string windowId = A(pRootElement->getAttribute(X("windowId")));
         DockWindow* pAipWindow = dynamic_cast<DockWindow*>(Service<SessionManager>()->getSessionItem(windowId));
         if (pAipWindow != NULL)
         {
            attachToWindow(pAipWindow);
            bool shown = XmlReader::StringStreamAssigner<bool>()(A(pRootElement->getAttribute(X("shown"))));
            mpWindowAction->setChecked(shown);
            return true;
         }
      }
   }

   return false;
}
