/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DesktopServices.h"
#include "DockWindowShell.h"
#include "PlugInManagerServices.h"
#include "SessionItemDeserializer.h"
#include "SessionItemSerializer.h"
#include "StringUtilities.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <string>
using namespace std;
XERCES_CPP_NAMESPACE_USE

DockWindowShell::DockWindowShell() :
   mpWindowAction(NULL),
   mpDockWindow(NULL)
{
   // Initialization
   setType(PlugInManagerServices::DockWindowType());
   allowMultipleInstances(false);
   executeOnStartup(true);
   destroyAfterExecute(false);
   setWizardSupported(false);

   // Connections
   mpDockWindow.addSignal(SIGNAL_NAME(DockWindow, Shown), Slot(this, &DockWindowShell::windowShown));
   mpDockWindow.addSignal(SIGNAL_NAME(DockWindow, Hidden), Slot(this, &DockWindowShell::windowHidden));
}

DockWindowShell::~DockWindowShell()
{
   // Destroy the dock window
   if (mpDockWindow.get() != NULL)
   {
      Service<DesktopServices> pDesktop;
      pDesktop->deleteWindow(mpDockWindow.get());
   }
}

bool DockWindowShell::setBatch()
{
   ExecutableShell::setBatch();
   return false;
}

bool DockWindowShell::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return !isBatch();
}

bool DockWindowShell::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return !isBatch();
}

bool DockWindowShell::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (isBatch() == true)
   {
      return false;
   }

   // Create the menu and/or toolbar action
   mpWindowAction = createAction();
   if (mpWindowAction == NULL)
   {
      return false;
   }

   mpWindowAction->setAutoRepeat(false);
   mpWindowAction->setCheckable(true);
   VERIFYNR(connect(mpWindowAction, SIGNAL(triggered(bool)), this, SLOT(displayDockWindow(bool))));

   // Create the dock window
   Service<DesktopServices> pDesktop;
   const string& windowName = getName();

   mpDockWindow.reset(static_cast<DockWindow*>(pDesktop->getWindow(windowName, DOCK_WINDOW)));
   if (mpDockWindow.get() == NULL)
   {
      mpDockWindow.reset(static_cast<DockWindow*>(pDesktop->createWindow(windowName, DOCK_WINDOW)));
      if (mpDockWindow.get() == NULL)
      {
         return false;
      }

      mpDockWindow->hide();
   }

   // Initialize the window icon from the action
   QIcon icon = mpWindowAction->icon();
   mpDockWindow->setIcon(icon);

   // Create the widget and set it in the dock window
   QWidget* pWidget = createWidget();
   if (pWidget != NULL)
   {
      mpDockWindow->setWidget(pWidget);
   }

   return true;
}

bool DockWindowShell::serialize(SessionItemSerializer& serializer) const
{
   if (mpWindowAction != NULL)
   {
      QString windowName = QString::fromStdString(getName());
      windowName.remove(" ");

      XMLWriter writer(windowName.toUtf8());
      writer.addAttr("batch", isBatch());
      writer.addAttr("shown", mpWindowAction->isChecked());
      return serializer.serialize(writer);
   }

   return false;
}

bool DockWindowShell::deserialize(SessionItemDeserializer& deserializer)
{
   QString windowName = QString::fromStdString(getName());
   windowName.remove(" ");

   bool shown = false;

   XmlReader reader(NULL, false);
   DOMElement* pRootElement = deserializer.deserialize(reader, windowName.toUtf8());
   if (pRootElement != NULL)
   {
      bool batch = StringUtilities::fromXmlString<bool>(A(pRootElement->getAttribute(X("batch"))));
      if (batch == true)
      {
         setBatch();
      }
      else
      {
         setInteractive();
      }

      shown = StringUtilities::fromXmlString<bool>(A(pRootElement->getAttribute(X("shown"))));
   }

   if (execute(NULL, NULL) == false)
   {
      return false;
   }

   if (mpWindowAction != NULL)
   {
      mpWindowAction->setChecked(shown);
      return true;
   }

   return false;
}

QAction* DockWindowShell::getAction()
{
   return mpWindowAction;
}

const QAction* DockWindowShell::getAction() const
{
   return mpWindowAction;
}

QWidget* DockWindowShell::getWidget() const
{
   if (mpDockWindow.get() != NULL)
   {
      return mpDockWindow->getWidget();
   }

   return NULL;
}

DockWindow* DockWindowShell::getDockWindow()
{
   return mpDockWindow.get();
}

const DockWindow* DockWindowShell::getDockWindow() const
{
   return mpDockWindow.get();
}

void DockWindowShell::displayDockWindow(bool bDisplay)
{
   if (mpDockWindow.get() != NULL)
   {
      if (bDisplay == true)
      {
         mpDockWindow->show();
      }
      else
      {
         mpDockWindow->hide();
      }
   }
}

void DockWindowShell::windowShown(Subject& subject, const string& signal, const boost::any& value)
{
   DockWindow* pWindow = dynamic_cast<DockWindow*>(&subject);
   if ((pWindow != NULL) && (pWindow == mpDockWindow.get()) && (mpWindowAction != NULL))
   {
      mpWindowAction->setChecked(true);
   }
}

void DockWindowShell::windowHidden(Subject& subject, const string& signal, const boost::any& value)
{
   DockWindow* pWindow = dynamic_cast<DockWindow*>(&subject);
   if ((pWindow != NULL) && (pWindow == mpDockWindow.get()) && (mpWindowAction != NULL))
   {
      mpWindowAction->setChecked(false);
   }
}
