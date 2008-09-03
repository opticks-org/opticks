/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVerify.h"
#include "ArgumentList.h"
#include "InteractiveApplication.h"
#include "SystemServicesImp.h"

#include <QtGui/QApplication>

int main(int argc, char** argv)
{
   // The QApplication will strip its args from argc/argv, so we want to do 
   // this before putting the command-line args into our ArgumentList, or we
   // will try to open the Qt args as data files.
   QApplication app(argc, argv);

   SystemServicesImp::instance()->WriteLogInfo(QString("%1 Startup").arg("Application").toStdString());
   // Register the command line options
   ArgumentList* pArgumentList = NULL;
   pArgumentList = ArgumentList::instance();
   if (pArgumentList == NULL)
   {
      SystemServicesImp::instance()->WriteLogInfo(QString("%1 Shutdown").arg("Application").toStdString());
      return -1;
   }

   pArgumentList->registerOption("input");
   pArgumentList->registerOption("exitAfterWizards");
   pArgumentList->registerOption("defaultDir");
   pArgumentList->registerOption("generate");
   pArgumentList->registerOption("processors");
   pArgumentList->registerOption("configDir");
   pArgumentList->registerOption("");
   pArgumentList->set(argc, argv);

   // Run the application
   int iSuccess = -1;

   InteractiveApplication* pApp = NULL;
   pApp = new InteractiveApplication(app);
   if (pApp != NULL)
   {
      iSuccess = pApp->run(argc, argv);
      delete pApp;
   }

   SystemServicesImp::instance()->WriteLogInfo(QString("%1 Shutdown").arg("Application").toStdString());
   return iSuccess;
}
