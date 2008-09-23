/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <iostream>

#include "AppVersion.h"
#include "ApplicationServicesImp.h"
#include "ArgumentList.h"
#include "BatchApplication.h"
#include "PlugInManagerServicesImp.h"
#include "ProgressBriefConsole.h"
#include "ProgressConsole.h"
#include "SessionManagerImp.h"

#include <vector>
using namespace std;

BatchApplication::BatchApplication(QCoreApplication& app) :
   Application(app)
{
   if (isXmlInitialized() == false)
   {
      reportError("Unable to initialize Xerces/XQilla.");
      exit(-1);
   }

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
   // Initialize the application
   int iReturn = Application::run(argc, argv);
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

   PlugInManagerServicesImp* pPlugIn = PlugInManagerServicesImp::instance();
   if (pPlugIn == NULL)
   {
      return -1;
   }

   vector<PlugInDescriptor*> plugins = pPlugIn->getPlugInDescriptors();

   cout << endl;
   cout << "Current Plug-In Configuration:" << endl;
   cout << "Total of " << static_cast<unsigned int>(plugins.size()) << " Plug-Ins" << endl;

   pPlugIn->listPlugIns(true, true, false);  // Show modules, show plug-ins, do not show full detail

   // Close the session to cleanup created objects
   SessionManagerImp::instance()->close();

   return 0;
}

int BatchApplication::test(int argc, char** argv)
{
   return -1;
}

int BatchApplication::run(int argc, char** argv)
{
   // Generate the XML files
   if (generateXml() == false)
   {
      return -1;
   }

   // Initialize the application
   int iReturn = Application::run(argc, argv);
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

   ArgumentList* pArgumentList = ArgumentList::instance();
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
   PlugInManagerServicesImp* pManager = PlugInManagerServicesImp::instance();
   if (pManager != NULL)
   {
      pManager->executeStartupPlugIns(pProgress);
   }

   bool bSuccess =  executeStartupBatchWizards(pProgress);

   // Close the session to cleanup created objects
   SessionManagerImp::instance()->close();

   // Cleanup
   if (bBrief == false)
   {
      delete dynamic_cast<ProgressConsole*>(pProgress);
   }
   else
   {
      delete dynamic_cast<ProgressBriefConsole*>(pProgress);
   }

   if (bSuccess == true)
   {
      return 0;
   }

   return -1;
}

void BatchApplication::reportWarning(const string& warningMessage) const
{
   if (warningMessage.empty() == false)
   {
      cerr << endl;
      cerr << APP_NAME << " WARNING: " << warningMessage << endl;
   }
}

void BatchApplication::reportError(const string& errorMessage) const
{
   string message = errorMessage;
   if (message.empty() == true)
   {
      message = "Unknown error";
   }

   cerr << endl;
   cerr << APP_NAME << " ERROR: " << message << endl;
}
