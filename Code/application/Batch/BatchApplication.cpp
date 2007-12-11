/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <iostream>

#include "BatchApplication.h"
#include "ApplicationServicesImp.h"
#include "ArgumentList.h"
#include "MessageLogMgrImp.h"
#include "PlugInManagerServicesImp.h"
#include "ProgressConsole.h"
#include "ProgressBriefConsole.h"
#include "SessionManagerImp.h"

#include <vector>
using namespace std;

BatchApplication::BatchApplication(QCoreApplication &app) : Application(app)
{
   Service<ApplicationServices> pApp;
   attach(SIGNAL_NAME(Subject, Deleted), Signal(pApp.get(), SIGNAL_NAME(ApplicationServices, ApplicationClosed)));
}

BatchApplication::~BatchApplication()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const string& BatchApplication::getObjectType() const
{
   static string type("BatchApplication");
   return type;
}

bool BatchApplication::isKindOf(const string& className) const
{
   if (className == getObjectType())
   {
      return true;
   }

   return SubjectAdapter::isKindOf(className);
}

int BatchApplication::version(int argc, char** argv)
{
   bool bSuccess = false;

   log("Current Plug-In Configuration:", "app", "8071B8A0-8509-4026-A82A-B2E383A03057");

   // Initialize the application
   int iReturn = NULL;
   iReturn = Application::run(argc, argv);
   if (iReturn == -1)
   {
      log("Could not initialize.", "app", "5E1B8A68-F56D-4183-873F-99E36D6C385B");
      return -1;
   }

   // Set the application to run in batch mode
   ApplicationServicesImp* pApp = ApplicationServicesImp::instance();
   if (pApp != NULL)
   {
      pApp->setBatch();
   }

   PlugInManagerServicesImp* pPlugIn = PlugInManagerServicesImp::instance();
   vector<PlugInDescriptor*> plugins = pPlugIn->getPlugInDescriptors();

   string message = "";

   cout << "Total of " << (unsigned int) plugins.size() << " Plug-Ins" << endl;

   pPlugIn->listPlugIns( true, true, false ); //bool showModules, bool showPlugIns, bool fullDetail

   // Close the session to cleanup created objects
   SessionManagerImp::instance()->close();

   if (bSuccess == true)
   {
      return 0;
   }
   return -1;
}  

int BatchApplication::test(int argc, char** argv)
{
   bool bSuccess = false;
   if (bSuccess == true)
   {
      return 0;
   }
   return -1;
}

int BatchApplication::run(int argc, char** argv)
{
   // Generate the XML files
   string errorMessage = "";

   bool bSuccess = false;
   bSuccess = generateXML(errorMessage);
   if (bSuccess == false)
   {
      log(errorMessage, "app", "D0AE97B8-8668-44AB-9EEC-B899A78FD75C");
      return -1;
   }

   // Initialize the application
   int iReturn = NULL;
   iReturn = Application::run(argc, argv);
   if (iReturn == -1)
   {
      return -1;
   }

   // Set the application to run in batch mode
   ApplicationServicesImp* pApp = ApplicationServicesImp::instance();
   if (pApp != NULL)
   {
      pApp->setBatch();
   }

   // Create the progress object
   Progress* pProgress = NULL;
   bool bBrief = false;

   ArgumentList* pArgumentList = NULL;
   pArgumentList = ArgumentList::instance();
   if (pArgumentList != NULL)
   {
      bBrief = pArgumentList->exists("brief");
   }

   if (bBrief == false)
   {
      pProgress = new ProgressConsole();
   }
   else
   {
      pProgress = new ProgressBriefConsole();
   }

   // Perform the batch processing
   log("Executing initial plug-ins...", "app", "8FB13CEB-9993-4376-AA93-38AD9D2479C9");
   executeStartupPlugIns(pProgress);

   log("Executing batch wizards...", "app", "1619818C-AC57-4B0E-A1EA-C55C85822818");
   bSuccess =  executeStartupBatchWizards(pProgress);

   log("Batch processing complete!", "app", "19D2EFE0-2B24-4B02-885A-27EDC20AEABB");

   // Close the session to cleanup created objects
   SessionManagerImp::instance()->close();

   // Cleanup
   if (bBrief == false)
   {
      delete ((ProgressConsole*) pProgress);
   }
   else
   {
      delete ((ProgressBriefConsole*) pProgress);
   }

   if (bSuccess == true)
   {
      return 0;
   }

   return -1;
}

void BatchApplication::log(const string& message,
                      string component,
                      string key)
{
   if (message.empty() == true)
   {
      return;
   }

   cout << endl;
   cout << message.c_str() << endl;

   MessageLogMgr* pLogMgr(MessageLogMgrImp::instance());
   if (pLogMgr != NULL)
   {
      Service<SessionManager> pSessionManager;
      MessageLog *pLog(pLogMgr->getLog(pSessionManager->getName()));
      Message* pMsg = NULL;
      if(pLog != NULL)
      {
         pMsg = pLog->createMessage(message.c_str(), component, key);
         if (pMsg != NULL) pMsg->finalize();
      }
   }
}
