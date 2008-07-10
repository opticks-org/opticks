/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
#include "MessageLogTest.h"
#include "DesktopServices.h"
#include "DockWindow.h"
#include "MessageLog.h"
#include "MessageLogMgr.h"
#include "Service.h"
#include "UtilityServices.h"

MessageLogTestPlugin::MessageLogTestPlugin()
{
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setName("Message Log Test");
   setShortDescription( "MessageLogTest" );
   setDescription("MessageLogTest");
   setMenuLocation("[Tests]\\Message Log Test");
   setDescriptorId("{4E1D4266-D816-49a8-AAC5-ECCCF8FF2A50}");
}

MessageLogTestPlugin::~MessageLogTestPlugin()
{
}

bool MessageLogTestPlugin::getInputSpecification( PlugInArgList * &pArgList )
{
   pArgList = NULL;
   return true;
}

bool MessageLogTestPlugin::getOutputSpecification( PlugInArgList * &pArgList )
{
   pArgList = NULL;
   return true;
}

bool MessageLogTestPlugin::execute( PlugInArgList* inputArgList, PlugInArgList* outputArgList )
{
   Service<DesktopServices> pDesktop;
   Service<UtilityServices> pUtility;

   DockWindow *pWindow = dynamic_cast<DockWindow*>( pDesktop->getWindow( "Message Log Window", DOCK_WINDOW ) );
   pWindow->show();

   MessageLogMgr *pMgr = pUtility->getMessageLog();
   if( pMgr != NULL )
   {
      MessageLog *pLog = pMgr->getLog( "Message Log Test Plugin" );
      if( pLog != NULL )
      {
         pLog->createMessage( "Added new test Message. Key and Component provided.", "Message Log Test Plugin", "message1", true, false );
         pLog->createMessage( "This should be yellow. No Key or Component provided.", "", "", true, false );
         pLog->createStep( "Added new test Step. Key and Component provided.", "Message Log Test Plugin", "step1", false );
         pLog->createStep( "This should be yellow. No Key or Component provided.", "", "", false );
      }
   }

   return true;
}
