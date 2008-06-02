/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <ActiveQt/QAxWidget>

#include "AppVersion.h"
#include "DockWindow.h"
#include "SessionItemSerializer.h"
#include "WindowsControl.h"

WindowsControl::WindowsControl()
{
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(false);
   setDescriptorId("{AFFAAB3B-244A-4bbb-93DA-430F08D6FE93}");
   setName("Windows Control");
   setDescription("Provides access to Windows Controls.");
   setMenuLocation("[Demo]\\Windows Control");
   destroyAfterExecute(false);
}

WindowsControl::~WindowsControl()
{
   DockWindow* pWindow = static_cast<DockWindow*>(mpDesktop->getWindow("Microsoft Windows Control", DOCK_WINDOW));
   if (pWindow != NULL)
   {
      QAxWidget* pWidget = dynamic_cast<QAxWidget*>(pWindow->getWidget());
      if (pWidget != NULL)
      {
         delete pWidget;
      }

      mpDesktop->deleteWindow(pWindow);
   }
}

bool WindowsControl::createWidget(DockWindow *pWindow)
{
   if (pWindow != NULL)
   {
      QAxWidget* pControl = new QAxWidget(mpDesktop->getMainWidget());
      if (pControl != NULL)
      {
         //       pControl->setControl("{0002E546-0000-0000-C000-000000000046}");  // Chart
         //       pControl->setControl("{8856F961-340A-11D0-A96B-00C04FD705A2}");  // Web Browser
         pControl->setControl("{0002E541-0000-0000-C000-000000000046}");  // Excel Magic Number
         pControl->setFocusPolicy(Qt::StrongFocus);
         pWindow->setWidget(pControl);
         return true;
      }
   }
   return false;
}

bool WindowsControl::setBatch()
{
   return false;
}

bool WindowsControl::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool WindowsControl::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool WindowsControl::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   DockWindow* pWindow = static_cast<DockWindow*>(mpDesktop->createWindow("Microsoft Windows Control", DOCK_WINDOW));
   return createWidget(pWindow);
}

bool WindowsControl::serialize(SessionItemSerializer &serializer) const
{
   return serializer.serialize(NULL, 0);
}

bool WindowsControl::deserialize(SessionItemDeserializer &deserializer)
{
   DockWindow* pWindow = static_cast<DockWindow*>(mpDesktop->getWindow("Microsoft Windows Control", DOCK_WINDOW));
   return createWidget(pWindow);
}
