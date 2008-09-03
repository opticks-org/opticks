/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include "AppConfig.h"
#include "ApplicationServicesImp.h"
#include "ApplicationWindow.h"
#include "AppVersion.h"
#include "ArgumentList.h"
#include "ConfigurationSettingsImp.h"
#include "DataVariant.h"
#include "FilenameImp.h"
#include "InteractiveApplication.h"
#include "PlugInBranding.h"
#include "PlugInManagerServicesImp.h"
#include "PlugInResource.h"
#include "ProgressAdapter.h"
#include "ProgressDlg.h"
#include "RasterLayer.h"
#include "SessionManager.h"
#include "SessionResource.h"
#include "SplashScreen.h"
#include "WizardUtilities.h"

using namespace std;

InteractiveApplication::InteractiveApplication(QCoreApplication& app) :
   Application(app)
{
   if (isXmlInitialized() == false)
   {
      reportError("Unable to initialize Xerces/XQilla.");
      exit(-1);
   }
}

InteractiveApplication::~InteractiveApplication()
{
}

int InteractiveApplication::run(int argc, char** argv)
{
   // Generate the XML files
   if (generateXml() == false)
   {
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

      reportError(configSettingsErrorMsg);
      return -1;
   }
   else
   {
      if (!configSettingsErrorMsg.empty())
      {
         reportWarning(configSettingsErrorMsg);
      }
   }

   { // scope the lifetime of the lock
      SessionSaveLock lock;

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
      PlugInManagerServicesImp* pManager = PlugInManagerServicesImp::instance();
      if (pManager != NULL)
      {
         pManager->executeStartupPlugIns(pProgress);
         qApplication.processEvents();
      }

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
         bool validImport = true;

         vector<string> filenames(pArgList->getOptions(""));
         if (filenames.size() > 1)
         {
            bool wizardFiles = false;
            bool datasetFiles = false;

            for (vector<string>::size_type i = 0; i < filenames.size(); ++i)
            {
               FilenameImp filename(filenames[i]);

               QString strFilename = QString::fromStdString(filename.getFullPathAndName());
               if (strFilename.isEmpty() == false)
               {
                  QFileInfo info(strFilename);
                  if ((info.suffix() == "wiz") || (info.suffix() == "batchwiz"))
                  {
                     if (datasetFiles == true)
                     {
                        validImport = false;
                        break;
                     }

                     wizardFiles = true;
                  }
                  else if (info.suffix() == "session")
                  {
                     validImport = false;
                     break;
                  }
                  else
                  {
                     if (wizardFiles == true)
                     {
                        validImport = false;
                        break;
                     }

                     datasetFiles = true;
                  }
               }
            }
         }

         if (validImport == true)
         {
            for (vector<string>::size_type i = 0; i < filenames.size(); ++i)
            {
               FilenameImp filename(filenames[i]);
               string normalizedFilename = filename.getFullPathAndName();

               QString strFilename = QString::fromStdString(normalizedFilename);
               if (strFilename.isEmpty() == false)
               {
                  QFileInfo info(strFilename);
                  if ((info.isFile() == true) && (info.exists() == true))
                  {
                     if (info.suffix() == "wiz")
                     {
                        pAppWindow->runWizard(strFilename);
                     }
                     else if (info.suffix() == "batchwiz")
                     {
                        vector<string> batchFiles;
                        batchFiles.push_back(normalizedFilename);
                        WizardUtilities::runBatchFiles(batchFiles, pProgress);
                     }
                     else if (info.suffix() == "session")
                     {
                        string saveKey = SessionManager::getSettingQueryForSaveKey();
                        SessionSaveType saveType = SESSION_DONT_AUTO_SAVE;

                        pConfigSettings->setSessionSetting(saveKey, saveType);
                        pAppWindow->openSession(strFilename);
                        pConfigSettings->deleteSessionSetting(saveKey);
                     }
                     else
                     {
                        ImporterResource importer("Auto Importer", normalizedFilename, pProgress, false);
                        importer->execute();
                     }
                  }
                  else
                  {
                     reportWarning("The specified file '" + normalizedFilename +
                        "' does not exist and cannot be loaded.");
                  }
               }
            }
         }
         else if (pProgress != NULL)
         {
            string msg = "Unable to import the files specified on the command line.  " + string(APP_NAME) +
               " supports loading one session file, one or more wizard files, or one or more data set files.";
            pProgress->updateProgress(msg, 0, ERRORS);
         }
      }

      // If there are any wizards, run them
      executeStartupBatchWizards(pProgress);

      // Destroy the progress object and progress dialog
      pProgress->detach(SIGNAL_NAME(Subject, Modified), Slot(pProgressDlg, &ProgressDlg::progressUpdated));
      pProgress->detach(SIGNAL_NAME(Subject, Deleted), Slot(pProgressDlg, &ProgressDlg::progressDeleted));
      delete pProgress;

      vector<string> autoExitOptions = pArgList->getOptions("autoExit");
      if (autoExitOptions.empty() == false)
      {
         SessionSaveType tempSettingQueryForSave = SESSION_DONT_AUTO_SAVE;
         const string sessionFilename = autoExitOptions.front();
         if (sessionFilename.empty() == false)
         {
            tempSettingQueryForSave = SESSION_AUTO_SAVE;
            pAppWindow->setSessionFilename(FilenameImp(sessionFilename).getFullPathAndName());
         }

         pConfigSettings->setSessionSetting(SessionManager::getSettingQueryForSaveKey(), tempSettingQueryForSave);
         pAppWindow->close();
         return iReturn;
      }

      // Set the application window to auto-generate textures
      QTimer textureGenerationTimer;
      bool generation = RasterLayer::getSettingBackgroundTileGeneration();
      if (generation == true)
      {
         textureGenerationTimer.setInterval(100);
         pAppWindow->connect(&textureGenerationTimer, SIGNAL(timeout()), SLOT(pregenerateTexture()));
      }

   }
   // Initiate the GUI event loop, which returns when the user exits the application
   return qApplication.exec();
}

void InteractiveApplication::reportWarning(const string& warningMessage) const
{
   if (warningMessage.empty() == false)
   {
      QMessageBox::warning(NULL, QString::fromStdString(APP_NAME), QString::fromStdString(warningMessage));
   }
}

void InteractiveApplication::reportError(const string& errorMessage) const
{
   string message = errorMessage;
   if (message.empty() == true)
   {
      message = "Unknown error";
   }

   QMessageBox::critical(NULL, QString::fromStdString(APP_NAME), QString::fromStdString(message));
}
