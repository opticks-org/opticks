/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include "ApplicationServicesImp.h"
#include "ApplicationWindow.h"
#include "AppVersion.h"
#include "ArgumentList.h"
#include "AppConfig.h"
#include "InteractiveApplication.h"
#include "ConfigurationSettingsImp.h"
#include "DataVariant.h"
#include "FilenameImp.h"
#include "PlugInManagerServices.h"
#include "PlugInBranding.h"
#include "PlugInResource.h"
#include "ProgressAdapter.h"
#include "ProgressDlg.h"
#include "RasterLayer.h"
#include "SplashScreen.h"

#include <string>
using namespace std;

int InteractiveApplication::run(int argc, char** argv)
{
   // Generate the XML files
   string errorMessage = "";

   bool bSuccess = generateXML(errorMessage);
   if (bSuccess == false)
   {
      QString strError = "Not all XML files could be generated!";
      if (errorMessage.empty() == false)
      {
         strError = QString::fromStdString(errorMessage);
      }

      QMessageBox::critical(NULL, APP_NAME, strError);
      return -1;
   }

   // Set the application to run in interactive mode
   ApplicationServicesImp* pApp = ApplicationServicesImp::instance();
   if (pApp != NULL)
   {
      pApp->setInteractive();
   }

   // Initialize the Qt application
   QApplication &qApplication = dynamic_cast<QApplication&>(getQApp());
   qApplication.setFont(QFont("Tahoma", 8));

   bool configSettingsValid = false;
   string configSettingsErrorMsg = "";

   ConfigurationSettingsImp* pConfigSettings = ConfigurationSettingsImp::instance();
   if (pConfigSettings != NULL)
   {
      configSettingsValid = pConfigSettings->isInitialized();
      if (pConfigSettings->getInitializationErrorMsg() != NULL)
      {
         configSettingsErrorMsg = pConfigSettings->getInitializationErrorMsg();
      }
   }

   if (!configSettingsValid)
   {
      if (configSettingsErrorMsg.empty())
      {
         configSettingsErrorMsg = "Unable to locate configuration settings";
      }
      QMessageBox::critical(NULL, QString(APP_NAME) + " Start-up Error", configSettingsErrorMsg.c_str());
      return -1;
   }
   else
   {
      if (!configSettingsErrorMsg.empty())
      {
         QMessageBox::warning(NULL, QString(APP_NAME) + " Start-up Error", configSettingsErrorMsg.c_str());
      }
   }

   // Create a progress object
   ProgressAdapter* pProgress = new ProgressAdapter();

   // Splash screen
   Q_INIT_RESOURCE(Application);
   const vector<PlugInBranding>& branding = PlugInBranding::getBrandings();
   list<string> splashImages;
   for (vector<PlugInBranding>::const_iterator brandingIter = branding.begin();
        brandingIter != branding.end();
        ++brandingIter)
   {
      const Filename* pFilename = brandingIter->getSplashScreenImage();
      if (pFilename != NULL)
      {
         string fullPathAndName = pFilename->getFullPathAndName();
         if (!fullPathAndName.empty())
         {
            splashImages.push_back(fullPathAndName);
         }
      }
   }
   SplashScreen* pSplash = new SplashScreen(pProgress);
   pSplash->setSplashImages(splashImages);
   pSplash->show();

   qApplication.processEvents();

   // Initialization
   pProgress->updateProgress("Building the plug-in list...", 0, NORMAL);

   int iReturn = Application::run(argc, argv);
   if (iReturn == -1)
   {
      pSplash->close();
      delete pSplash;
      return -1;
   }

   qApplication.processEvents();

   // Create the main GUI window
   pProgress->updateProgress("Creating the main application window...", 0, NORMAL);

   ApplicationWindow* pAppWindow = new ApplicationWindow(pSplash);
   qApplication.processEvents();

   // Execute startup plug-ins
   executeStartupPlugIns(pProgress);
   qApplication.processEvents();

   // Restore the previous position and visibility state of the toolbars and dock windows
   pAppWindow->restoreConfiguration();

   // Keep the splash screen up until all images have been shown to the user.
   while (!pSplash->canClose()) {}

   // Display the main application window
   pAppWindow->show();

   // Destroy the splash screen
   pSplash->close();
   delete pSplash;

   // Create a progress dialog
   ProgressDlg* pProgressDlg = new ProgressDlg(APP_NAME, pAppWindow);
   pProgress->attach(SIGNAL_NAME(Subject, Modified), Slot(pProgressDlg, &ProgressDlg::progressUpdated));
   pProgress->attach(SIGNAL_NAME(Subject, Deleted), Slot(pProgressDlg, &ProgressDlg::progressDeleted));

   // Load files specified on the command line
   ArgumentList *pArgList(ArgumentList::instance());
   if(pArgList != NULL)
   {
      ImporterResource importer("Auto Importer", string(), pProgress, false);

      vector<string> filenames(pArgList->getOptions(""));
      for(vector<string>::const_iterator filenamesIt = filenames.begin();
                                         filenamesIt != filenames.end();
                                         ++filenamesIt)
      {
         FilenameImp filename(filenamesIt->c_str());

         // try and import the file
         string normalizedFilename(filename.getFullPathAndName());
         importer->setFilename(normalizedFilename);
         importer->execute();
      }
   }

   // If there are any execute and exit wizards, run them
   //
   executeStartupBatchWizards(pProgress);
   if(!pArgList->getOptions("exitAfterWizards").empty())
   {
      pAppWindow->close();
   }

   // Destroy the progress object and progress dialog
   pProgress->detach(SIGNAL_NAME(Subject, Modified), Slot(pProgressDlg, &ProgressDlg::progressUpdated));
   pProgress->detach(SIGNAL_NAME(Subject, Deleted), Slot(pProgressDlg, &ProgressDlg::progressDeleted));
   delete pProgress;

   // Set the application window to auto-generate textures
   QTimer textureGenerationTimer;
   bool generation = RasterLayer::getSettingBackgroundTileGeneration();
   if (generation == true)
   {
      textureGenerationTimer.setInterval(100);
      pAppWindow->connect(&textureGenerationTimer, SIGNAL(timeout()), SLOT(pregenerateTexture()));
   }

   // Initiate the GUI event loop, which returns when the user exits the application
   iReturn = qApplication.exec();

   if (pAppWindow->isVisible())
   {
      pAppWindow->close();
      qApplication.processEvents();
   }

   return iReturn;
}
