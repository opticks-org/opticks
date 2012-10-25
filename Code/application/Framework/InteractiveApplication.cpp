/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include "Aeb.h"
#include "AebIo.h"
#include "AppConfig.h"
#include "ApplicationServicesImp.h"
#include "ApplicationWindow.h"
#include "AppVersion.h"
#include "ArgumentList.h"
#include "ConfigurationSettingsImp.h"
#include "DataVariant.h"
#include "DesktopServicesImp.h"
#include "FilenameImp.h"
#include "InstallerServicesImp.h"
#include "InteractiveApplication.h"
#include "PlugInManagerServicesImp.h"
#include "PlugInResource.h"
#include "ProgressAdapter.h"
#include "ProgressDlg.h"
#include "RasterLayer.h"
#include "SessionManager.h"
#include "SessionResource.h"
#include "SplashScreen.h"
#include "TypesFile.h"
#include "WizardUtilities.h"

#include <vector>

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
{}

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
   QApplication& qApplication = dynamic_cast<QApplication&>(getQApp());
#if !defined(LINUX)
   qApplication.setFont(QFont("Tahoma", 8));
#endif

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
      if (configSettingsValid)
      {
         pConfigSettings->validateInitialization();
         configSettingsValid = pConfigSettings->isInitialized();
         if (pConfigSettings->getInitializationErrorMsg() != NULL)
         {
            configSettingsErrorMsg = pConfigSettings->getInitializationErrorMsg();
         }
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
      mpProgress = new ProgressAdapter();

      // Splash screen
      Q_INIT_RESOURCE(Application);
      SplashScreen* pSplash = new SplashScreen(mpProgress);
      vector<string> splashPaths = Service<InstallerServices>()->getSplashScreenPaths();
      pSplash->setSplashImages(list<string>(splashPaths.begin(), splashPaths.end()));
      pSplash->show();

      qApplication.processEvents();

      // process pending extension uninstalls
      InstallerServicesImp::instance()->processPending(mpProgress);
      string errMsg;
      if(!ConfigurationSettingsImp::instance()->loadSettings(errMsg))
      {
         if (QMessageBox::warning(pSplash, "Error loading configuration settings",
            QString("Warning: unable to reload application settings.\n%1\nContinue loading Opticks?").arg(QString::fromStdString(errMsg)),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No)
         {
            pSplash->close();
            delete pSplash;
            return -1;
         }
      }

      // Initialization
      int iReturn = Application::run(argc, argv);
      if (iReturn == -1)
      {
         pSplash->close();
         delete pSplash;
         return -1;
      }

      qApplication.processEvents();

      // process auto-installs
      QDirIterator autos(QString::fromStdString(ConfigurationSettingsImp::instance()->getSettingExtensionFilesPath()->getFullPathAndName())
         + "/AutoInstall", QStringList() << "*.aeb", QDir::Files);
      vector<string> pendingInstall;
      while (autos.hasNext())
      {
         pendingInstall.push_back(autos.next().toStdString());
      }
      bool autoInstallOccurred = false;
      InstallerServicesImp::instance()->setPendingInstall(pendingInstall);
      for (vector<string>::iterator autoIter = pendingInstall.begin();
           autoIter != pendingInstall.end();
           ++autoIter)
      {
         bool success = InstallerServicesImp::instance()->installExtension(*autoIter, mpProgress);
         if(!success)
         {
            QFileInfo autoInfo(QString::fromStdString(*autoIter));
            // Attempt to parse the AEB so we can get a better name
            string extName = autoInfo.fileName().toStdString();
            { // scope the AebIo so we don't hold a handle to the aeb file and can delete it below
               Aeb extension;
               AebIo io(extension);
               string errMsg; // ignored
               if (io.fromFile(autoInfo.filePath().toStdString(), errMsg))
               {
                  extName = extension.getName();
               }
            }
            if(DesktopServicesImp::instance()->showMessageBox("Installation error", "Unable to install " + extName
               + "\nWould you like to delete the file?", "Yes", "No") == 0)
            {
               QFile::remove(QString::fromStdString(*autoIter));
            }
         }
         else
         {
            autoInstallOccurred = true;
            QFile::remove(QString::fromStdString(*autoIter));
         }
      }
      InstallerServicesImp::instance()->setPendingInstall();
      if (autoInstallOccurred)
      {
         // rescan the plug-ins
         PlugInManagerServicesImp::instance()->buildPlugInList(Service<ConfigurationSettings>()->getPlugInPath());
      }


      // Create the main GUI window
      mpProgress->updateProgress("Creating the main application window...", 0, NORMAL);

      ApplicationWindow* pAppWindow = new ApplicationWindow(pSplash);
      qApplication.processEvents();

      // Execute startup plug-ins
      PlugInManagerServicesImp* pManager = PlugInManagerServicesImp::instance();
      if (pManager != NULL)
      {
         pManager->executeStartupPlugIns(mpProgress);
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
      mpProgress->attach(SIGNAL_NAME(Subject, Modified), Slot(pProgressDlg, &ProgressDlg::progressUpdated));
      mpProgress->attach(SIGNAL_NAME(Subject, Deleted), Slot(pProgressDlg, &ProgressDlg::progressDeleted));

      // Load files specified on the command line
      ArgumentList* pArgList(ArgumentList::instance());
      if (pArgList != NULL)
      {
         FileType fileType;

         vector<string> filenames(pArgList->getOptions(""));
         for (vector<string>::size_type i = 0; i < filenames.size(); ++i)
         {
            FilenameImp filename(filenames[i]);

            QString strFilename = QString::fromStdString(filename.getFullPathAndName());
            if (strFilename.isEmpty() == false)
            {
               QFileInfo info(strFilename);
               if ((info.suffix() == "wiz") || (info.suffix() == "batchwiz"))
               {
                  if ((fileType.isValid() == true) && (fileType != WIZARD_FILES))
                  {
                     fileType = EnumWrapper<FileTypeEnum>();
                     break;
                  }

                  fileType = WIZARD_FILES;
               }
               else if (info.suffix() == "session")
               {
                  if (fileType.isValid() == true)
                  {
                     fileType = EnumWrapper<FileTypeEnum>();
                     break;
                  }

                  fileType = SESSION_FILE;
               }
               else
               {
                  if ((fileType.isValid() == true) && (fileType != DATASET_FILES))
                  {
                     fileType = EnumWrapper<FileTypeEnum>();
                     break;
                  }

                  fileType = DATASET_FILES;
               }
            }
         }

         if (fileType.isValid() == true)
         {
            // Check for valid filenames
            vector<string> validFilenames;
            for (vector<string>::size_type i = 0; i < filenames.size(); ++i)
            {
               FilenameImp filename(filenames[i]);

               string normalizedFilename = filename.getFullPathAndName();
               if (normalizedFilename.empty() == false)
               {
                  QString strFilename = QString::fromStdString(normalizedFilename);

                  QFileInfo info(strFilename);
                  if ((info.isFile() == true) && (info.exists() == true))
                  {
                     validFilenames.push_back(normalizedFilename);
                  }
                  else
                  {
                     reportWarning("The specified file '" + normalizedFilename +
                        "' does not exist and cannot be loaded.");
                  }
               }
            }

            // Load the files
            switch (fileType)
            {
               case WIZARD_FILES:
               {
                  for (vector<string>::size_type i = 0; i < validFilenames.size(); ++i)
                  {
                     string filename = validFilenames[i];
                     QString strFilename = QString::fromStdString(filename);

                     QFileInfo info(strFilename);
                     if (info.suffix() == "wiz")
                     {
                        pAppWindow->runWizard(strFilename);
                     }
                     else if (info.suffix() == "batchwiz")
                     {
                        vector<string> batchFiles;
                        batchFiles.push_back(filename);
                        WizardUtilities::runBatchFiles(batchFiles, mpProgress);
                     }
                  }

                  break;
               }

               case SESSION_FILE:
               {
                  if (validFilenames.empty() == false)
                  {
                     VERIFYRV(validFilenames.size() == 1, -1);
                     string filename = validFilenames.front();

                     string saveKey = SessionManager::getSettingQueryForSaveKey();
                     SessionSaveType saveType = SESSION_DONT_AUTO_SAVE;
                     DataVariant dvSaveType(saveType);

                     pConfigSettings->adoptTemporarySetting(saveKey, dvSaveType);
                     pAppWindow->openSession(QString::fromStdString(filename));
                     pConfigSettings->deleteTemporarySetting(saveKey);
                  }

                  break;
               }

               case DATASET_FILES:
               {
                  if (validFilenames.empty() == false)
                  {
                     ImporterResource importer("Auto Importer", validFilenames, mpProgress, false);
                     importer->execute();
                  }

                  break;
               }

               default:
                  break;
            }
         }
         else if (filenames.empty() == false)
         {
            string msg = "Unable to import the files specified on the command line.  " + string(APP_NAME) +
               " supports loading one session file, one or more wizard files, or one or more data set files.";
            mpProgress->updateProgress(msg, 0, ERRORS);
         }
      }

      // If there are any wizards, run them
      executeStartupBatchWizards();

      // Destroy the progress object and progress dialog
      delete dynamic_cast<ProgressAdapter*>(mpProgress);

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
         DataVariant dvTempSettingQueryForSave(tempSettingQueryForSave);

         pConfigSettings->adoptTemporarySetting(SessionManager::getSettingQueryForSaveKey(), dvTempSettingQueryForSave);
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
