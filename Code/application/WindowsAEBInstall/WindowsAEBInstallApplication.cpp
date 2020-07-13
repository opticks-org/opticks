/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Aeb.h"
#include "AebIo.h"
#include "AppVersion.h"
#include "ApplicationServicesImp.h"
#include "ArgumentList.h"
#include "DesktopServices.h"
#include "DesktopServicesImp.h"
#include "ConfigurationSettingsImp.h"
#include "InstallerServicesImp.h"
#include "PlugInManagerServicesImp.h"
#include "ProgressAdapter.h"
#include "ProgressDlg.h"
#include "SessionResource.h"
#include "WindowsAEBInstallApplication.h"

#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtWidgets/QApplication>
#include <QtGui/QFont>

#include <vector>
using namespace std;

WindowsAEBInstallApplication::WindowsAEBInstallApplication(QCoreApplication& app) :
   InteractiveApplication(app)
{}

WindowsAEBInstallApplication::~WindowsAEBInstallApplication()
{}

int WindowsAEBInstallApplication::run(int argc, char** argv)
{
   // Set the application to run in batch mode
   ApplicationServicesImp* pApp = ApplicationServicesImp::instance();
   if (pApp != NULL)
   {
      pApp->setBatch();
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
      mpProgress->setSettingAutoClose(false);
      // Create a progress dialog
      ProgressDlg* pProgressDlg = new ProgressDlg(APP_NAME, NULL);
      pProgressDlg->enableAbort(false);
      qApplication.setQuitOnLastWindowClosed(true);
      mpProgress->attach(SIGNAL_NAME(Subject, Modified), Slot(pProgressDlg, &ProgressDlg::progressUpdated));
      mpProgress->attach(SIGNAL_NAME(Subject, Deleted), Slot(pProgressDlg, &ProgressDlg::progressDeleted));

      vector<string> extensionList;
      vector<string> removeOptions;
      ArgumentList* pArgList(ArgumentList::instance());
      if (pArgList != NULL)
      {
         removeOptions = pArgList->getOptions("remove");
         extensionList = pArgList->getOptions("");
      }
      
      // Splash screen
      Q_INIT_RESOURCE(Application);

      for (vector<string>::const_iterator it = removeOptions.begin(); it != removeOptions.end(); ++it)
      {
         string errMsg;
         if (!InstallerServicesImp::instance()->uninstallExtension(*it, errMsg))
         {
            mpProgress->updateProgress("Warning: unable to uninstall extension " + *it + "\n" + errMsg, 100, ERRORS);
         }
      }

      // process pending extension uninstalls
      InstallerServicesImp::instance()->processPending(mpProgress);
      string errMsg;
      if(!ConfigurationSettingsImp::instance()->loadSettings(errMsg))
      {
         mpProgress->updateProgress("Warning: unable to reload application settings.\n" + errMsg, DONT_UPDATE, WARNING);
      }

      // Initialization
      int iReturn = Application::run(argc, argv);
      if (iReturn == -1)
      {
         return -1;
      }

      mpProgress->updateProgress("Processing installations.", 1, NORMAL);
      qApplication.processEvents();

      // process auto-installs and command line installs
      QDirIterator autos(QString::fromStdString(ConfigurationSettingsImp::instance()->getSettingExtensionFilesPath()->getFullPathAndName())
         + "/AutoInstall", QStringList() << "*.aeb", QDir::Files);
      vector<string> pendingInstall;
      vector<bool> removeInstallFile;
      while (autos.hasNext())
      {
         pendingInstall.push_back(autos.next().toStdString());
         removeInstallFile.push_back(true);
      }
      for (auto it = extensionList.begin(); it != extensionList.end(); ++it)
      {
         pendingInstall.push_back(*it);
         removeInstallFile.push_back(false);
      }
      bool autoInstallOccurred = false;
      InstallerServicesImp::instance()->setPendingInstall(pendingInstall);
      vector<bool>::const_iterator removeIter = removeInstallFile.begin();
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
               string errMsg2; // ignored
               if (io.fromFile(autoInfo.filePath().toStdString(), errMsg2))
               {
                  extName = extension.getName();
               }
            }
            string cap = string("Unable to install ") + extName;
            if (*removeIter)
            {
               cap += "\nDeleting the file.";
            }
            mpProgress->updateProgress("Installation error: " + *autoIter, DONT_UPDATE, ERRORS);
            qApplication.processEvents();
         }
         else
         {
            autoInstallOccurred = true;

         }
         if (*removeIter)
         {
            QFile::remove(QString::fromStdString(*autoIter));
         }
         ++removeIter;
      }
      InstallerServicesImp::instance()->setPendingInstall();
      InstallerServicesImp::instance()->processPending(mpProgress);
      qApplication.processEvents();

      // Destroy the progress object and progress dialog
   }
   mpProgress->updateProgress("AEB processing complete. Please restart " + std::string(APP_NAME), 100, NORMAL);
   delete dynamic_cast<ProgressAdapter*>(mpProgress);
   return qApplication.exec();
}
