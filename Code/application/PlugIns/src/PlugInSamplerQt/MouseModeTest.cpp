/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QMessageBox>

#include "AppVerify.h"
#include "AppVersion.h"
#include "DesktopServices.h"
#include "MouseModeTest.h"
#include "MouseModeTestGui.h"
#include "SpatialDataWindow.h"

MouseModeTest::MouseModeTest() :
   mpGui(NULL)
{
   ViewerShell::setName("Mouse Mode Test");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(false);
   setDescription("Verifies MPR2 Requirement 215.");
   setShortDescription("Verifies MPR2 Requirement 215.");
   setMenuLocation("[Tests]\\Mouse Mode Test");
   setDescriptorId("{354DD7F9-5CB3-4faf-BA22-7F129AC3CB25}");
}

MouseModeTest::~MouseModeTest()
{
}

bool MouseModeTest::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   Service<DesktopServices> pDesktop;
   SpatialDataView* pView = NULL;

   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getCurrentWorkspaceWindow());
   if (pWindow != NULL)
   {
      pView = pWindow->getSpatialDataView();
   }

   if (pView == NULL)
   {
      QMessageBox::critical(pDesktop->getMainWidget(), "Mouse Mode Test",
         "An active spatial data window is required to run the mouse mode test.");
      return false;
   }

   if (mpGui == NULL)
   {
      mpGui = new MouseModeTestGui(pView, pDesktop->getMainWidget());
      VERIFYNR(connect(mpGui, SIGNAL(finished(int)), this, SLOT(dialogClosed())));
   }

   mpGui->show();
   return true;
}

QWidget* MouseModeTest::getWidget() const
{
   return mpGui;
}

void MouseModeTest::dialogClosed()
{
   abort();
}
