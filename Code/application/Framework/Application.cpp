/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#if defined(WIN_API)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "AnimationServicesImp.h"
#include "Application.h"
#include "ApplicationServicesImp.h"
#include "AppVersion.h"
#include "ArgumentList.h"
#include "BatchWizard.h"
#include "ConfigurationSettingsImp.h"
#include "ConnectionManager.h"
#include "DataVariantFactoryImp.h"
#include "DesktopServicesImp.h"
#include "FileFinderImp.h"
#include "Filename.h"
#include "MessageLogMgrImp.h"
#include "ModelServicesImp.h"
#include "ModuleManager.h"
#include "ObjectFactoryImp.h"
#include "PlugInManagerServicesImp.h"
#include "SessionManagerImp.h"
#include "UtilityServicesImp.h"
#include "WizardUtilities.h"

#include <QtCore/QCoreApplication>
#include <vector>
#include <iostream>
using namespace std;

Application::Application(QCoreApplication& app) :
   mApplication(app),
   mXmlInitialized(true)
{
   // Initialize Xerces and XQilla
   try
   {
      XQillaPlatformUtils::initialize();
   }
   catch (const XERCES_CPP_NAMESPACE_QUALIFIER XMLException&)
   {
      mXmlInitialized = false;
   }

   ModuleManager::instance()->setService(ConnectionManager::instance());
}

Application::~Application()
{
   PlugInManagerServicesImp::destroy();
   AnimationServicesImp::destroy();
   DesktopServicesImp::destroy();
   ModelServicesImp::destroy();
   ConfigurationSettingsImp::destroy();
   ObjectFactoryImp::destroy();
   DataVariantFactoryImp::destroy();
   ConnectionManager::destroy();
   ApplicationServicesImp::destroy();
   UtilityServicesImp::destroy();
   SessionManagerImp::destroy();
   MessageLogMgrImp::destroy();
}

int Application::run(int argc, char** argv)
{
   // Add the number of processors to the configuration settings
   // determine number of available processors
   unsigned int numberOfProcessors = UtilityServicesImp::instance()->getNumProcessors();
   string processorArg = ArgumentList::instance()->getOption("processors");
   Service<ConfigurationSettings> pSettings;
   if (!processorArg.empty())
   {
      unsigned int procArgValue = atoi(processorArg.c_str());
      if (procArgValue >= 1u && procArgValue <= numberOfProcessors)
      {
         pSettings->setSessionSetting(ConfigurationSettings::getSettingThreadCountKey(), procArgValue);
      }
      else
      {
         reportWarning("The requested number of processors are not available.  Using the actual number of "
            "processors on the system instead.");
         pSettings->setSessionSetting(ConfigurationSettings::getSettingThreadCountKey(), numberOfProcessors);
      }
   }
   else
   {
      if (ConfigurationSettings::hasSettingThreadCount())
      {
         unsigned int currentThreadCount = ConfigurationSettings::getSettingThreadCount();
         if (currentThreadCount > numberOfProcessors)
         {
            ConfigurationSettings::setSettingThreadCount(numberOfProcessors);
         }
      }
      else
      {
         pSettings->setSessionSetting(ConfigurationSettings::getSettingThreadCountKey(), numberOfProcessors);
      }
   }

   // Build the plug-in list from the plug-in directory
   ConfigurationSettingsExt2* pSettings2 = dynamic_cast<ConfigurationSettingsExt2*>(pSettings.get());
   if (pSettings2 == NULL)
   {
      return -1;
   }
   string plugPath = pSettings2->getPlugInPath();

   //check all singletons here for proper creation
   PlugInManagerServicesImp* pManager = NULL;
   pManager = PlugInManagerServicesImp::instance();
   if (pManager == NULL)
   {
      return -1;
   }
   try
   {
      if ( (PlugInManagerServicesImp::instance() == NULL) ||
           (SessionManagerImp::instance() == NULL) ||
           (AnimationServicesImp::instance() == NULL) ||
           (ApplicationServicesImp::instance() == NULL) ||
           (UtilityServicesImp::instance() == NULL) ||
           (ModelServicesImp::instance() == NULL) ||
           (DesktopServicesImp::instance() == NULL) ||
           (ConfigurationSettingsImp::instance() == NULL) ||
           (ObjectFactoryImp::instance() == NULL) ||
           (DataVariantFactoryImp::instance() == NULL) ||
           (MessageLogMgrImp::instance() == NULL) ||
           (ConnectionManager::instance() == NULL))
      {
         return -1;
      }
   }
   catch (...)
   {
      // one of the services cxtr's threw an assertion...treat it as if the instance is NULL
      return -1;
   }

   const Filename* pSupportFilesPath = ConfigurationSettings::getSettingSupportFilesPath();
   UtilityServicesImp* pUtils = UtilityServicesImp::instance();
   if (pSupportFilesPath != NULL && pUtils != NULL)
   {
      pUtils->loadSecurityMarkings(pSupportFilesPath->getFullPathAndName() + SLASH + "SecurityMarkings");
   }

   pManager->buildPlugInList(plugPath);
   return 0;
}

bool Application::isXmlInitialized() const
{
   return mXmlInitialized;
}

bool Application::generateXml()
{
   vector<string> files;

   ArgumentList* pArgumentList = ArgumentList::instance();
   if (pArgumentList != NULL)
   {
      files = pArgumentList->getOptions("generate");
   }

   for (vector<string>::iterator iter = files.begin(); iter != files.end(); ++iter)
   {
      string wizardFilename = *iter;
      BatchWizard* pBatchWizard = WizardUtilities::createBatchWizardFromWizard(wizardFilename);
      if (pBatchWizard != NULL)
      {
         vector<BatchWizard*> batchWizards;
         batchWizards.push_back(pBatchWizard);
         string outputFilename = WizardUtilities::deriveBatchWizardFilename(wizardFilename);
         if (!WizardUtilities::writeBatchWizard(batchWizards, outputFilename))
         {
            reportError("Cannot write out batch wizard file: " + outputFilename);
            delete pBatchWizard;
            return false;
         }
      }
      else
      {
         reportError("Cannot load the wizard file: " + wizardFilename);
         delete pBatchWizard;
         return false;
      }
      delete pBatchWizard;
   }

   return true;
}

bool Application::executeStartupBatchWizards(Progress* pProgress)
{
   vector<string> wizardFiles;

   ArgumentList* pArgumentList = ArgumentList::instance();
   if (pArgumentList != NULL)
   {
      wizardFiles = pArgumentList->getOptions("input");
   }

   if (wizardFiles.empty() == true)
   {
      return true;
   }

   return WizardUtilities::runBatchFiles(wizardFiles, pProgress);
}
