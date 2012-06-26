/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "PlugInManagerServicesImp.h"
#include "ConfigurationSettingsImp.h"
#include "CoreModuleDescriptor.h"
#include "DataVariant.h"
#include "DynamicModuleImp.h"
#include "DynamicObjectAdapter.h"
#include "FileFinderImp.h"
#include "FilenameImp.h"
#include "FileResource.h"
#include "ModuleDescriptor.h"
#include "ObjectResource.h"
#include "PlugIn.h"
#include "PlugInArgImp.h"
#include "PlugInArgListImp.h"
#include "PlugInDescriptorImp.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "StringUtilities.h"

#include <iostream>
#include <vector>
#include <algorithm>

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QString>

using namespace std;

class SettableSessionItem;

PlugInManagerServicesImp* PlugInManagerServicesImp::spInstance = NULL;
bool PlugInManagerServicesImp::mDestroyed = false;

PlugInManagerServicesImp* PlugInManagerServicesImp::instance()
{
   if (spInstance == NULL)
   {
      if (mDestroyed)
      {
         throw std::logic_error("Attempting to use PlugInManagerServices after destroying it.");
      }
      spInstance = new PlugInManagerServicesImp;
   }

   return spInstance;
}

void PlugInManagerServicesImp::destroy()
{
   if (mDestroyed)
   {
      throw std::logic_error("Attempting to destroy PlugInManagerServices after destroying it.");
   }
   delete spInstance;
   spInstance = NULL;
   mDestroyed = true;
}

PlugInManagerServicesImp::PlugInManagerServicesImp()
{
#if defined(WIN_API)
   //Currently the excluded list only includes the Visual Studio 2010 runtime dll's
   //because they are required to be present in the plug-in path and simply by
   //calling LoadLibrary on those dll's it will cause errors to be generated
   //to include Windows system modal dialogs and errors in the message log.
   mExcludedPlugIns.push_back("vcomp100.dll");
   mExcludedPlugIns.push_back("vcomp100d.dll");
   mExcludedPlugIns.push_back("msvcp100.dll");
   mExcludedPlugIns.push_back("msvcp100d.dll");
   mExcludedPlugIns.push_back("msvcr100.dll");
   mExcludedPlugIns.push_back("msvcr100d.dll");
#endif
}

PlugInManagerServicesImp::~PlugInManagerServicesImp()
{
   notify(SIGNAL_NAME(Subject, Deleted));

   clear();
}

const string& PlugInManagerServicesImp::getObjectType() const
{
   static string type("PlugInManagerServicesImp");
   return type;
}

bool PlugInManagerServicesImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PlugInManagerServices"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

DynamicModule* PlugInManagerServicesImp::getDynamicModule(const string& value)
{
   //Only attempt to load a plug-in .dll if it's not in the excluded list.
   QFileInfo modulePath(QString::fromStdString(value));
   QString fileName = modulePath.fileName().toLower();
   if (std::find(mExcludedPlugIns.begin(), mExcludedPlugIns.end(), fileName.toStdString()) != mExcludedPlugIns.end())
   {
      return new DynamicModuleImp();
   }
   return new DynamicModuleImp(value);
}

bool PlugInManagerServicesImp::destroyDynamicModule(DynamicModule* pModule)
{
   if (pModule == NULL)
   {
      return false;
   }

   delete static_cast<DynamicModuleImp*>(pModule);
   return true;
}

ModuleDescriptor* PlugInManagerServicesImp::getModuleDescriptor(const string& moduleFilename)
{
   if (moduleFilename.empty() == true)
   {
      return NULL;
   }

   FilenameImp fileObj(moduleFilename);
   string cleanModuleFilename = fileObj.getFullPathAndName().c_str();
   vector<ModuleDescriptor*>::iterator iter = mModules.begin();
   while (iter != mModules.end())
   {
      ModuleDescriptor* pModule = *iter;
      if (pModule != NULL)
      {
         string filename = pModule->getFileName();
         if (filename == cleanModuleFilename)
         {
            return pModule;
         }
      }

      iter++;
   }

   return NULL;
}

ModuleDescriptor* PlugInManagerServicesImp::getModuleDescriptorByName(const string& moduleName)
{
   if (moduleName.empty() == true)
   {
      return NULL;
   }

   vector<ModuleDescriptor*>::iterator iter;
   for (iter = mModules.begin(); iter != mModules.end(); ++iter)
   {
      ModuleDescriptor* pModule = *iter;
      if (pModule == NULL)
      {
         continue;
      }
      if (pModule->getName() == moduleName)
      {
         return pModule;
      }
   }

   return NULL;
}


const vector<ModuleDescriptor*>& PlugInManagerServicesImp::getModuleDescriptors() const
{
   return mModules;
}

void PlugInManagerServicesImp::listPlugIns(bool showModules, bool showPlugIns, bool fullDetail)
{
   vector<ModuleDescriptor*>::iterator iter = mModules.begin();
   while (iter != mModules.end())
   {
      ModuleDescriptor* pModule = *iter;
      if (pModule != NULL)
      {
         if (showModules)
         {
            string filename = pModule->getFileName();
            if (!fullDetail) 
            {
               filename.erase(0, filename.find_last_of("/") + 1);
            }
            const unsigned int total = pModule->getNumPlugIns();
            const DateTime* pDate = pModule->getFileDate();
            string dateString;
            string format = "%Y-%m-%d, %H:%M:%S";  //"%d%b%Y %H:%M:%S";
            if (pDate != NULL)
            {
               dateString = pDate->getFormattedUtc(format);
            }

            const double size = pModule->getFileSize();
            string version = pModule->getVersion();

            cout << pModule->getName() << ", " << version << ", " << total << ", " 
               << dateString << ", " << size << ", ";
            if (fullDetail)
            {
               cout << endl << "      " ;
            }

            cout << filename << endl;
         } // if (listModules)

         if (showPlugIns)
         {
            vector<PlugInDescriptorImp*> plugins = pModule->getPlugInSet();
            vector<PlugInDescriptorImp*>::iterator plugin = plugins.begin(); 
            string message = "";

            while (plugin != plugins.end() )
            {
               if (showModules)
               {
                  cout << "   ";
               }

               PlugInDescriptorImp* pDescriptor = *plugin;
               message = pDescriptor->getName();
               remove(message.begin(), message.end(), ',');
               cout << message << + ", ";

               message = pDescriptor->getVersion();
               remove(message.begin(), message.end(), ',');
               cout << message << + ", ";

               message = pDescriptor->getType();
               remove(message.begin(), message.end(), ',');
               cout << message ;

               if (fullDetail) 
               {
                  message = pDescriptor->getCreator();
                  remove(message.begin(), message.end(), ',');
                  cout << ", " << message;
               }
               cout << endl;

               plugin++;
            } // while (plugin != plugins.end() )
         } // if (listPlugIns)
      }

      iter++;
   }
}

Progress* PlugInManagerServicesImp::getProgress(PlugIn* pPlugIn)
{
   if (pPlugIn == NULL)
   {
      return NULL;
   }

   PlugInDescriptorImp* pDescriptor = dynamic_cast<PlugInDescriptorImp*>(getPlugInDescriptor(pPlugIn));
   if (pDescriptor != NULL)
   {
      return pDescriptor->getProgress(pPlugIn);
   }

   return NULL;
}

void PlugInManagerServicesImp::executeStartupPlugIns(Progress* pProgress) const
{
   Service<ApplicationServices> pApp;
   bool bBatch = pApp->isBatch();

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Executing the startup plug-ins...", 0, NORMAL);
   }

   map<string, PlugInDescriptorImp*>::const_iterator iter;
   for (iter = mPlugIns.begin(); iter != mPlugIns.end(); ++iter)
   {
      PlugInDescriptorImp* pDescriptor = iter->second;
      if (pDescriptor != NULL)
      {
         if (pDescriptor->isExecutedOnStartup() == true)
         {
            if (((bBatch == true) && (pDescriptor->hasBatchSupport() == true)) ||
               ((bBatch == false) && (pDescriptor->hasInteractiveSupport() == true)))
            {
               ExecutableResource plugIn(pDescriptor->getName(), string(), pProgress, bBatch);
               plugIn->execute();
            }
         }
      }
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Executing the startup plug-ins is complete", 100, NORMAL);
   }
}

void PlugInManagerServicesImp::buildPlugInList(const string& plugInPath)
{
   map<string, string> plugInIds;
   set<string> moduleIds;

   //Add PlugIns located inside application executable
   ModuleDescriptor* pCoreModule = new CoreModuleDescriptor("{E84F08DC-017A-4298-BE0A-E5F02517F40F}", plugInIds);
   if (getModuleDescriptorByName(pCoreModule->getName()) == NULL)
   {
      moduleIds.insert(pCoreModule->getId());
      mModules.push_back(pCoreModule);
      vector<PlugInDescriptorImp*> plugIns = pCoreModule->getPlugInSet();
      vector<PlugInDescriptorImp*>::iterator plugInIter = plugIns.begin();
      while (plugInIter != plugIns.end())
      {
         pair<map<string, PlugInDescriptorImp*>::iterator, bool> insertResult;
         insertResult = mPlugIns.insert(make_pair((*plugInIter)->getName(), (*plugInIter)));
         if (!insertResult.second)
         {
            string msg = string("Multiple plug-ins are attempting to "
               "register with the same name of " + (*plugInIter)->getName());
            VERIFYNR_MSG(false, msg.c_str());
         }
         plugInIter++;
      }
      notify(SIGNAL_NAME(PlugInManagerServices, ModuleCreated), boost::any(pCoreModule));
   }
   else
   {
      delete pCoreModule;
   }

   //Load the plug-ins found in the plug-in path
   if (plugInPath.empty()) // can't do anything more with an empty path
   {
      return;
   }

   FactoryResource<DynamicObject> pPlugInCache;
   if (PlugInManagerServicesImp::getSettingCachePlugInInformation())
   {
      pPlugInCache = loadPlugInListCache();
   }

#if defined(WIN_API)
   string dlExtension = ".dll";
#elif defined(UNIX_API)
   string dlExtension = ".so";
#else
#error "Unsupported platform"
#endif

   FileFinderImp finder;

   // Search plug-in directory for modules
   finder.findFile(plugInPath, "*"+ dlExtension);

   // Remove modules from the list that no longer exist
   vector<ModuleDescriptor*> removedModules;

   vector<ModuleDescriptor*>::iterator iter = mModules.begin();
   while (iter != mModules.end())
   {
      ModuleDescriptor* pModule = *iter;
      if (pModule != NULL)
      {
         string moduleFilename = pModule->getFileName();

         bool bExists = finder.findNextFile();
         while (bExists == true)
         {
            string filename = "";
            finder.getFullPath(filename);
            if (filename == moduleFilename)
            {
               break;
            }

            bExists = finder.findNextFile();
         }

         if (bExists == false)
         {
            removedModules.push_back(pModule);
         }
      }

      iter++;
   }

   vector<ModuleDescriptor*>::iterator removeIter = removedModules.begin();
   while (removeIter != removedModules.end())
   {
      ModuleDescriptor* pModule = *removeIter;
      if (pModule != NULL)
      {
         removeModule(pModule, plugInIds);
      }

      removeIter++;
   }

   // Add new modules and update existing modules
   string autoImporter = "AutoImporter" + dlExtension;
   finder.findFile(plugInPath, "*" + dlExtension);

   bool bSuccess = finder.findNextFile();

   string moduleFilename = "";
   string libraryFilename;
   if (bSuccess)
   {
      finder.getFullPath(moduleFilename);
   }

   while (bSuccess == true)
   {
      libraryFilename = finder.getFileName();
      if (libraryFilename == autoImporter)
      {
         //skip AutoImporter, we will load it later
         //outside this loop
         // Get the next file in the directory
         bSuccess = finder.findNextFile();
         finder.getFullPath(moduleFilename);
         continue;
      }
      double dFileSize = finder.getLength();

      DateTimeImp fileDate;
      finder.getLastModificationTime(fileDate);

      bool bAddModule = true;

      // Check if an existing module should be updated
      ModuleDescriptor* pModule = NULL;
      pModule = getModuleDescriptor(moduleFilename);
      if (pModule != NULL)
      {
         bAddModule = false;

         // File size
         double dCurrentFileSize = pModule->getFileSize();
         if (dCurrentFileSize != dFileSize)
         {
            bAddModule = true;
         }

         // File date
         const DateTimeImp* pDateTime = static_cast<const DateTimeImp*>(pModule->getFileDate());
         if (pDateTime != NULL)
         {
            if (*pDateTime != fileDate)
            {
               bAddModule = true;
            }
         }

         // Remove the existing module if necessary so it can be added
         if (bAddModule == true)
         {
            removeModule(pModule, plugInIds);
         }
      }

      // Add the module if necessary
      if (bAddModule == true)
      {
         pModule = addModule(moduleFilename, pPlugInCache.get(), plugInIds);
         if (pModule != NULL)
         {
            // disallow multiple modules with the same id
            if (moduleIds.find(pModule->getId()) != moduleIds.end())
            {
               VERIFYNR_MSG(false, "Multiple plug-in modules are attempting to register with the same session id");
               removeModule(pModule, plugInIds);
            }
            else
            {
               moduleIds.insert(pModule->getId());
            }
         }
      }

      // Get the next file in the directory
      bSuccess = finder.findNextFile();
      finder.getFullPath(moduleFilename);
   }

   //load AutoImporter as the last plug-in, so that it can
   //properly determine its extensions based upon extensions
   //of all other importers.
   if (finder.findFile(plugInPath, autoImporter) == true)
   {
      finder.findNextFile();
      string autoImporterPath;
      if (finder.getFullPath(autoImporterPath))
      {
         ModuleDescriptor* pModule = NULL;
         pModule = getModuleDescriptor(autoImporterPath);
         if (pModule != NULL)
         {
            removeModule(pModule, plugInIds);
         }
         //can't use cache because AutoImporter determines
         //its extensions by querying all of the other
         //loaded importers
         addModule(autoImporterPath, NULL, plugInIds);
      }
   }

   savePlugInListCache();

   ConfigurationSettingsImp::instance()->updateProductionStatus();
}

void PlugInManagerServicesImp::clear()
{
   vector<ModuleDescriptor*>::iterator ppModule = mModules.begin();
   while (ppModule != mModules.end())
   {
      ModuleDescriptor* pModule = *ppModule;
      if (pModule != NULL)
      {
         vector<PlugInDescriptorImp*> plugIns = pModule->getPlugInSet();

         for (vector<PlugInDescriptorImp*>::iterator iter = plugIns.begin(); iter != plugIns.end(); ++iter)
         {
            PlugInDescriptorImp* pPlugIn = *iter;
            if (pPlugIn != NULL)
            {
               map<string, PlugInDescriptorImp*>::iterator masterIter = mPlugIns.find(pPlugIn->getName());
               if (masterIter!= mPlugIns.end())
               {
                  mPlugIns.erase(masterIter);
               }
            }
         }
         notify(SIGNAL_NAME(PlugInManagerServices, ModuleDestroyed), boost::any(pModule));
         delete pModule;
      }
      mModules.erase(ppModule);
      ppModule = mModules.begin();
   }
}

vector<PlugInDescriptor*> PlugInManagerServicesImp::getPlugInDescriptors(const string& plugInType) const
{
   vector<PlugInDescriptor*> plugIns;   
   for (map<string, PlugInDescriptorImp*>::const_iterator iter = mPlugIns.begin();
        iter != mPlugIns.end(); ++iter)
   {
      PlugInDescriptor* pDescriptor = dynamic_cast<PlugInDescriptor*>(iter->second);
      if (pDescriptor != NULL)
      {
         string currentType = pDescriptor->getType();
         if ((plugInType.empty() == true) || (currentType == plugInType))
         {
            plugIns.push_back(pDescriptor);
         }
      }
   }
   return plugIns;
}

vector<PlugIn*> PlugInManagerServicesImp::getPlugInInstances(const string& plugInName)
{
   vector<PlugIn*> plugins;
   map<string, PlugInDescriptorImp*>::iterator pFirst;
   map<string, PlugInDescriptorImp*>::iterator pLast;
   if (plugInName.empty())
   {
      pFirst = mPlugIns.begin();
      pLast = mPlugIns.end();
   }
   else
   {
      pFirst = pLast = mPlugIns.find(plugInName);
      if (pLast != mPlugIns.end())
      {
         ++pLast;
      }
   }
   map<string, PlugInDescriptorImp*>::iterator pCurrent;
   for (pCurrent = pFirst; pCurrent != pLast; ++pCurrent)
   {
      if (pCurrent->second != NULL)
      {
         vector<PlugIn*> plugInList = pCurrent->second->getPlugIns();
         copy(plugInList.begin(), plugInList.end(), back_inserter(plugins));
      }
   }
   return plugins;
}

PlugIn* PlugInManagerServicesImp::createPlugIn(const string& plugInName)
{
   return createPlugInInstance(plugInName);
}

PlugIn* PlugInManagerServicesImp::createPlugInInstance(const string& plugInName, const string& id)
{
   if (plugInName.empty())
   {
      return NULL;
   }

   map<string, PlugInDescriptorImp*>::iterator plugInIter = mPlugIns.find(plugInName);
   if (plugInIter != mPlugIns.end())
   {
      PlugInDescriptorImp* pPlugInDescriptor = plugInIter->second;
      if (pPlugInDescriptor != NULL)
      {
         int iCreatedPlugIns = pPlugInDescriptor->getNumPlugIns();
         if (iCreatedPlugIns > 0)
         {
            bool bMultipleInstances = pPlugInDescriptor->areMultipleInstancesAllowed();
            if (bMultipleInstances == false)
            {
               return NULL;
            }
         }

         ModuleDescriptor* pModule = getModuleDescriptorByName(pPlugInDescriptor->getModuleName());
         if (pModule != NULL)
         {
            pModule->load();

            PlugIn* pPlugIn = pModule->createInterface(pPlugInDescriptor);
            if (pPlugIn != NULL)
            {
               if (!id.empty())
               {
                  SessionItemId itemId(id);
                  pPlugIn->setId(itemId);
               }

               if (pPlugInDescriptor->addPlugIn(pPlugIn) == false)
               {
                  pPlugInDescriptor->destroyPlugIn(pPlugIn);
                  return NULL;
               }

               notify(SIGNAL_NAME(PlugInManagerServices, PlugInCreated), boost::any(pPlugIn));
            }

            return pPlugIn;
         }
      }
   }

   return NULL;
}

bool PlugInManagerServicesImp::destroyPlugIn(PlugIn* pPlugIn)
{
   if (pPlugIn == NULL)
   {
      return false;
   }

   // Get the descriptor
   PlugInDescriptorImp* pPlugInDescriptor = NULL;

   map<string, PlugInDescriptorImp*>::iterator plugInIter = mPlugIns.begin();
   while (plugInIter != mPlugIns.end())
   {
      pPlugInDescriptor = plugInIter->second;
      if (pPlugInDescriptor != NULL)
      {
         bool bContains = pPlugInDescriptor->containsPlugIn(pPlugIn);
         if (bContains == true)
         {
            break;
         }

         pPlugInDescriptor = NULL;
      }

      plugInIter++;
   }

   if (pPlugInDescriptor == NULL)
   {
      return false;
   }

   // Notify attached objects that the plug-in is about to be destroyed
   notify(SIGNAL_NAME(PlugInManagerServices, PlugInDestroyed), boost::any(pPlugIn));

   // Destroy the plug-in instance
   bool bSuccess = pPlugInDescriptor->destroyPlugIn(pPlugIn);

   // Unload the module if all plug-ins are destroyed
   string moduleName = pPlugInDescriptor->getModuleName();

   ModuleDescriptor* pModule = getModuleDescriptorByName(moduleName);
   if (pModule != NULL)
   {
      const vector<PlugInDescriptorImp*>& plugIns = pModule->getPlugInSet();

      vector<PlugInDescriptorImp*>::const_iterator iter;
      for (iter = plugIns.begin(); iter != plugIns.end(); iter++)
      {
         if ((*iter)->getNumPlugIns() != 0)
         {
            break;
         }
      }

      if (iter == plugIns.end())
      {
         pModule->unload();
      }
   }

   return bSuccess;
}

PlugInDescriptor* PlugInManagerServicesImp::getPlugInDescriptor(const string& plugInName) const
{
   if (plugInName.empty() == false)
   {
      map<string, PlugInDescriptorImp*>::const_iterator iter = mPlugIns.find(plugInName);
      if (iter != mPlugIns.end())
      {
         return iter->second;
      }
   }

   return NULL;
}

PlugInDescriptor* PlugInManagerServicesImp::getPlugInDescriptor(PlugIn* pPlugIn) const
{
   if (pPlugIn == NULL)
   {
      return NULL;
   }

   PlugInDescriptor* pDescriptor = NULL;

   string plugInName = pPlugIn->getName();
   if (plugInName.empty() == false)
   {
      pDescriptor = getPlugInDescriptor(plugInName);
   }

   return pDescriptor;
}

PlugInArgList* PlugInManagerServicesImp::getPlugInArgList()
{
   return new PlugInArgListImp;
}

PlugInArg* PlugInManagerServicesImp::getPlugInArg()
{
   return new PlugInArgImp;
}

bool PlugInManagerServicesImp::destroyPlugInArgList(PlugInArgList* pArgList)
{
   if (pArgList == NULL)
   {
      return false;
   }

   delete static_cast<PlugInArgListImp*>(pArgList);
   return true;
}

const vector<string>& PlugInManagerServicesImp::getArgTypes()
{
   return PlugInArgImp::getArgTypes();
}

ModuleDescriptor* PlugInManagerServicesImp::addModule(const string& moduleFilename,
                                                      DynamicObject* pPlugInCache,
                                                      map<string, string>& plugInIds)
{
   if (moduleFilename.empty() == true)
   {
      return NULL;
   }

   // Do not add the module if is already exists
   ModuleDescriptor* pModule = getModuleDescriptor(moduleFilename);
   if (pModule != NULL)
   {
      return NULL;
   }

   // Read the module information, either from the cache or by loading the shared library
   // Check the cache first
   if (pPlugInCache != NULL)
   {
      const DynamicObject* pModuleSettings = pPlugInCache->getAttribute(
         moduleFilename).getPointerToValue<DynamicObject>();
      if (pModuleSettings != NULL)
      {
         pModule = ModuleDescriptor::fromSettings(*pModuleSettings);
         if (pModule != NULL && pModule->getFileName() != moduleFilename)
         {
            delete pModule;
            pModule = NULL;
         }
      }
   }
   if (pModule == NULL)
   {
      // couldn't find in cache, so load the shared library
      pModule = ModuleDescriptor::getModule(moduleFilename, plugInIds);
   }
   if (pModule == NULL)
   {
      return NULL;
   }

   string moduleId = pModule->getId();
   VERIFYRV_MSG(moduleId.empty() == false, NULL, 
      "A plug-in module is specifying an empty session id. It will not be loaded.");

   // Make sure module hasn't already been added
   if (getModuleDescriptorByName(pModule->getName()) != NULL)
   {
      delete pModule;
      return NULL;
   }

   // Make sure the module is a valid version
   int moduleVersion = pModule->getModuleVersion();
   if (moduleVersion < MOD_ONE || // legacy module
       moduleVersion > MOD_THREE)   // version 3 is the current version
   {
      delete pModule;
      return NULL;
   }
   // Add the module to the list
   mModules.push_back(pModule);

   // Add the module's plug-ins to the map
   vector<PlugInDescriptorImp*> plugIns = pModule->getPlugInSet();

   for (vector<PlugInDescriptorImp*>::iterator iter = plugIns.begin(); iter != plugIns.end(); ++iter)
   {
      PlugInDescriptorImp* pPlugIn = *iter;
      if (pPlugIn != NULL)
      {
         pair<map<string, PlugInDescriptorImp*>::iterator, bool> insertResult;
         insertResult = mPlugIns.insert(make_pair(pPlugIn->getName(), pPlugIn));
         if (!insertResult.second)
         {
            string msg = string("Multiple plug-ins are attempting to "
               "register with the same name of " + pPlugIn->getName());
            VERIFYNR_MSG(false, msg.c_str());
         }
      }
   }
   notify(SIGNAL_NAME(PlugInManagerServices, ModuleCreated), boost::any(pModule));

   return pModule;
}

bool PlugInManagerServicesImp::containsModule(ModuleDescriptor* pModule)
{
   if (pModule == NULL)
   {
      return false;
   }

   vector<ModuleDescriptor*>::iterator iter = mModules.begin();
   while (iter != mModules.end())
   {
      ModuleDescriptor* pCurrentModule = *iter;
      if (pCurrentModule == pModule)
      {
         return true;
      }

      iter++;
   }

   return false;
}

bool PlugInManagerServicesImp::removeModule(ModuleDescriptor* pModule, map<string, string>& plugInIds)
{
   if (pModule == NULL)
   {
      return false;
   }

   bool bExists = false;
   bExists = containsModule(pModule);
   if (bExists == false)
   {
      return false;
   }

   // Do not remove a loaded plug-in
   if (pModule->isLoaded() == true)
   {
      return false;
   }

   // Remove the module's plug-ins from the plug-in map
   vector<PlugInDescriptorImp*> plugIns = pModule->getPlugInSet();

   vector<PlugInDescriptorImp*>::iterator iter = plugIns.begin();
   while (iter != plugIns.end())
   {
      PlugInDescriptorImp* pDescriptor = *iter;
      if (pDescriptor != NULL)
      {
         string plugInName = pDescriptor->getName();
         if (plugInName.empty() == false)
         {
            map<string, PlugInDescriptorImp*>::iterator mapIter;
            mapIter = mPlugIns.find(plugInName);
            if (mapIter != mPlugIns.end())
            {
               if (mapIter->second != NULL)
               {
                  plugInIds.erase(mapIter->second->getId());
               }
               mPlugIns.erase(mapIter);
            }
         }
      }

      iter++;
   }

   // Remove the module from the list
   vector<ModuleDescriptor*>::iterator moduleIter;
   for (moduleIter = mModules.begin(); moduleIter != mModules.end(); ++moduleIter)
   {
      ModuleDescriptor* pCurrentModule = *moduleIter;
      if (pCurrentModule == pModule)
      {
         mModules.erase(moduleIter);
         notify(SIGNAL_NAME(PlugInManagerServices, ModuleDestroyed), boost::any(pModule));
         delete pCurrentModule;
         break;
      }
   }

   return true;
}

FactoryResource<DynamicObject> PlugInManagerServicesImp::loadPlugInListCache()
{
   FactoryResource<DynamicObject> pCache;
   string plugInCacheFile = getPlugInCacheFilePath();
   if (!QFile::exists(QString::fromStdString(plugInCacheFile)))
   {
      return pCache;
   }
   XmlReader xmlReader(Service<MessageLogMgr>()->getLog(), false);
   XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* pDomDoc(NULL);
   pDomDoc = xmlReader.parse(plugInCacheFile);
   if (pDomDoc == NULL)
   {
      return pCache;
   }
   XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList* pConfList(NULL);
   pConfList = pDomDoc->getElementsByTagName(X("DynamicObject"));
   if (pConfList == NULL || pConfList->getLength() != 1)
   {
      return pCache;
   }
   XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* pCacheNode = pConfList->item(0);

   if (!pCache->fromXml(pCacheNode, XmlBase::VERSION))
   {
      pCache->clear();
   }
   return pCache;
}

void PlugInManagerServicesImp::savePlugInListCache() const
{
   string plugInCacheFile = getPlugInCacheFilePath();
   if (plugInCacheFile.empty())
   {
      return;
   }
   vector<ModuleDescriptor*>::const_iterator ppModule;
   DynamicObjectAdapter settingsAdapter;
   DynamicObject& settings = settingsAdapter;
   bool moduleSettingsSaved = true;
   for (ppModule = mModules.begin(); ppModule != mModules.end(); ++ppModule)
   {
      ModuleDescriptor* pModule = *ppModule;
      if (pModule != NULL && pModule->canCache())
      {
         DynamicObjectAdapter moduleSettingsAdapter;
         DynamicObject& moduleSettings = moduleSettingsAdapter;
         moduleSettingsSaved = pModule->updateSettings(moduleSettings);
         if (moduleSettingsSaved)
         {
            settings.setAttribute(pModule->getFileName(), moduleSettings);
         }
      }
   }
   XMLWriter xmlWriter("PlugInCache");
   if (!settings.toXml(&xmlWriter))
   {
      return;
   }
   FileResource pFile(plugInCacheFile.c_str(), "wt");
   if (pFile.get() != NULL)
   {
      xmlWriter.writeToFile(pFile.get());
      if (ferror(pFile.get()))
      {
         pFile.setDeleteOnClose(true);
      }
   }
}

string PlugInManagerServicesImp::getPlugInCacheFilePath()
{
   ConfigurationSettingsImp* pSettings = dynamic_cast<ConfigurationSettingsImp*>(Service<ConfigurationSettings>().get());
   return pSettings->getUserStorageFilePath("PlugInCache", "xml");
}
