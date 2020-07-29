/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtWidgets/QApplication>

#include "Aeb.h"
#include "AppConfig.h"
#include "AppVersion.h"
#include "ArgumentList.h"
#include "WindowsAEBInstallApplication.h"
#include "ConfigurationSettingsImp.h"
#include "DateTime.h"
#include "InstallerServicesImp.h"
#include "StringUtilities.h"
#include "SystemServicesImp.h"

#include <string>
#include <iostream>

using namespace std;

// Defines the entry point for the WindowsAEBInstall Processor
int main(int argc, char** argv)
{
   SystemServicesImp::instance()->WriteLogInfo(string(APP_NAME) + " AEB Install Startup");

   //Need to initialize qApplication so that ConfigSettingsImp
   //can use qApplication::applicationDirPath()
   QApplication qApplication(argc, argv);

   // Register the command line options
   ArgumentList* pArgumentList = NULL;
   pArgumentList = ArgumentList::instance();
   if (pArgumentList == NULL)
   {
      SystemServicesImp::instance()->WriteLogInfo(string(APP_NAME) + " AEB Install shutdown");
      return -1;
   }
   pArgumentList->registerOption("remove");
   pArgumentList->set(argc, argv);

   WindowsAEBInstallApplication installApp(qApplication);

   // Run the application
   int iSuccess;
   iSuccess = installApp.run(argc, argv);

   SystemServicesImp::instance()->WriteLogInfo(string(APP_NAME) + " AEB Install Shutdown");
   return iSuccess;
}
