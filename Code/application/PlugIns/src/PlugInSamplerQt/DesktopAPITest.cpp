/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "DesktopServices.h"
#include "DesktopAPITest.h"
#include "DesktopAPITestGui.h"

DesktopAPITest::DesktopAPITest() :
   mpGui(NULL)
{
   ViewerShell::setName("Desktop API Test");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(false);
   setDescription("Verifies Requirement 324.");
   setShortDescription("Verifies Requirement 324.");
   setMenuLocation("[Tests]\\Desktop API Test");
   setDescriptorId("{DF7B5AEF-B156-40ae-8DF6-6B3D5BC964E1}");
}

DesktopAPITest::~DesktopAPITest()
{
}

bool DesktopAPITest::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (mpGui == NULL)
   {
      Service<DesktopServices> pDesktop;
      mpGui = new DesktopAPITestGui(pDesktop->getMainWidget());
      VERIFYNR(connect(mpGui, SIGNAL(finished(int)), this, SLOT(dialogClosed())));
   }

   mpGui->show();
   return true;
}

QWidget* DesktopAPITest::getWidget() const
{
   return mpGui;
}

void DesktopAPITest::dialogClosed()
{
   abort();
}
