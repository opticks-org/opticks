/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtGui/QIcon>

#include "ModuleDescriptor.h"
#include "ConnectionManager.h"
#include "AppVerify.h"
#include "External.h"
#include "FileFinderImp.h"
#include "FilenameImp.h"
#include "Icons.h"
#include "PlugIn.h"
#include "PlugInDescriptorImp.h"
#include "PlugInManagerServices.h"
#include "PropertiesModuleDescriptor.h"
#include "FilenameImp.h"
#include "SessionItemSerializer.h"

#include <string>
#include <vector>
using namespace std;

ModuleDescriptor::ModuleDescriptor(const string& id) :
   SessionItemImp(id),
   mVersion(""),
   mDescription(""),
   mPlugInTotal(0),
   mValidationKey(""),
   mFileName(""),
   mFileSize(0),
   mFileDate(0),
   mpModule(0)
{
   Icons* pIcons = Icons::instance();
   if (pIcons != NULL)
   {
      setIcon(pIcons->mModule);
   }

   addPropertiesPage(PropertiesModuleDescriptor::getName());
}

ModuleDescriptor::ModuleDescriptor(const string& id, const string& file, map<string, string>& plugInIds) :
   SessionItemImp(id),
   mVersion(""),
   mDescription(""),
   mPlugInTotal(0),
   mValidationKey(""),
   mFileName(""),
   mFileSize(0),
   mFileDate(0),
   mpModule(0)
{
   bool initValid = false;
   char* pModuleName = NULL;
   char* pModuleVersion = NULL;
   char* pModuleDescription = NULL;
   unsigned int totalPlugIns = 0;
   char* pModuleValidationKey = NULL;
   char* pModuleId = NULL;

   FilenameImp fileObj(file);
   mFileName = fileObj.getFullPathAndName();

   FileFinderImp find;
   find.findFile(fileObj.getPath(), fileObj.getFileName());
   find.findNextFile();
   mFileSize = find.getLength();

   find.getLastModificationTime(mFileDate);

   if (!load())
   {
      return;
   }

   // Get the name, description, version, number of plugins
   bool(*moduleNameProcedure)(char**, char**, char**, unsigned int*, char**, char**) =
      reinterpret_cast<bool(*)(char**, char**, char**, unsigned int*, char**, char**)>(
                              mpModule->getProcedureAddress("get_name"));

   if (moduleNameProcedure)
   {
      initValid = moduleNameProcedure(&pModuleName, &pModuleVersion, &pModuleDescription,
         &totalPlugIns, &pModuleValidationKey, &pModuleId);
   }

   Icons* pIcons = Icons::instance();
   if (pIcons != NULL)
   {
      setIcon(pIcons->mModule);
   }

   if (initValid)
   {
      setName(pModuleName);
      setDisplayText(mFileName);
      mVersion = pModuleVersion;
      mDescription = pModuleDescription;
      mPlugInTotal = totalPlugIns;
      mValidationKey = pModuleValidationKey;
      initializePlugInInformation(plugInIds);
   }
   else
   {
      mVersion = "";
      mDescription = "";
      mPlugInTotal = 0;
      mValidationKey = "none";
   }

   unload();
   addPropertiesPage(PropertiesModuleDescriptor::getName());
}

ModuleDescriptor::~ModuleDescriptor()
{
   vector<PlugInDescriptorImp*>::iterator plugIn;
   try
   {
      plugIn = mPlugins.begin();
      while (plugIn != mPlugins.end())
      {
         delete *plugIn;
         plugIn++;
      }

      if (mpModule != NULL)
      {
         mpPluginMgrSvc->destroyDynamicModule(mpModule);
         mpModule = NULL;
      }
   }
   catch (...)
   {
      abort();
      // Don't really care if a delete fails, just continue
   }
}

string ModuleDescriptor::getModuleId(const string& filename)
{
   string id;

   DynamicModule* pModule = Service<PlugInManagerServices>()->getDynamicModule(filename);
   if (pModule == NULL)
   {
      return string();
   }

   bool(*pInitProcedure)(External*) =
      reinterpret_cast<bool(*)(External*)>(pModule->getProcedureAddress("initialize"));

   if (pInitProcedure)
   {
      if (!pInitProcedure(ConnectionManager::instance()))
      {
         Service<PlugInManagerServices>()->destroyDynamicModule(pModule);
         return string();
      }
   }

   bool(*pNameProcedure)(char**, char**, char**, unsigned int*, char**, char**) =
      reinterpret_cast<bool(*)(char**, char**, char**, unsigned int*, char**, char**)>(
                               pModule->getProcedureAddress("get_name"));

   if (pNameProcedure)
   {
      char* pModuleName = NULL;
      char* pModuleVersion = NULL;
      char* pModuleDescription = NULL;
      unsigned int totalPlugIns = 0;
      char* pModuleId = NULL;
      char* pValidationKey = NULL;
      if (!pNameProcedure(&pModuleName, &pModuleVersion, &pModuleDescription,
         &totalPlugIns, &pValidationKey, &pModuleId) || pModuleId == NULL)
      {
         Service<PlugInManagerServices>()->destroyDynamicModule(pModule);
         return string();
      }
      id = pModuleId;
   }
   Service<PlugInManagerServices>()->destroyDynamicModule(pModule);
   return id;
}

bool ModuleDescriptor::load()
{
   if (mpModule != NULL)
   {
      return true; // no load is needed
   }

   bool initValid = false;

   mpModule = mpPluginMgrSvc->getDynamicModule(getFileName());
   if (mpModule == NULL)
   {
      return false;
   }

   //
   // Call Module's initialize routine.
   //

   bool(*moduleProcedure)(External*) =
      reinterpret_cast<bool(*)(External*)>(mpModule->getProcedureAddress("initialize"));

   if (moduleProcedure)
   {
      initValid = moduleProcedure(ConnectionManager::instance());
   }

   return initValid;
}

void ModuleDescriptor::unload()
{
   if (mpModule != NULL)
   {
      mpPluginMgrSvc->destroyDynamicModule(mpModule);
      mpModule = NULL;
   }
}

bool ModuleDescriptor::initializePlugInInformation(map<string, string>& plugInIds)
{
   bool previouslyLoaded = false;
   if (mpModule == NULL)
   {
      VERIFY(load());
      previouslyLoaded = true;
   }

   //
   // Determine what Plug-Ins are available.
   //

   // TODO : Update Plug-Ins currently in the list.

   for (unsigned int plugInNumber = 0; plugInNumber < getNumPlugIns(); plugInNumber++)
   {
      PlugIn* pPlugIn = createInterface(plugInNumber);
      if (pPlugIn != NULL)
      {
         string type = pPlugIn->getType();
         if (type.empty())
         {
            VERIFYNR_MSG(false, string("A plug-in is specifying an empty type. "
               "The plug-in " + pPlugIn->getName() + " will not be loaded.").c_str());
            delete pPlugIn;
            continue;
         }

         string descriptorId = pPlugIn->getDescriptorId();
         if (descriptorId.empty())
         {
            VERIFYNR_MSG(false, string("A plug-in is specifying an empty session id. "
               "The plug-in " + pPlugIn->getName() + " will not be loaded.").c_str());
            delete pPlugIn;
            continue;
         }
         map<string, string>::iterator foundPlugIn = plugInIds.find(descriptorId);
         if (foundPlugIn != plugInIds.end())
         {
            VERIFYNR_MSG(false, string("Both the " + foundPlugIn->second + " plug-in and " +
               pPlugIn->getName() + " plug-in are attempting to register with the same session id. "
               "The " + pPlugIn->getName() + " plug-in will not be loaded.").c_str());
            delete pPlugIn;
            continue;
         }
         plugInIds.insert(pair<string, string>(descriptorId, pPlugIn->getName()));
         PlugInDescriptorImp* pDescriptor = new PlugInDescriptorImp(descriptorId, pPlugIn);
         pDescriptor->setModuleFileName(getFileName());
         pDescriptor->setModuleName(getName());
         pDescriptor->setPlugInNumber(plugInNumber);

         mPlugins.push_back(pDescriptor);
         delete pPlugIn;
      }
   }

   if (!previouslyLoaded)
   {
      unload();
   }

   return true;
}

PlugIn* ModuleDescriptor::createInterface(unsigned int plugInNumber)
{
   if (mpModule == NULL)
   {
      return NULL;
   }

   PlugIn* pPlugIn = NULL;

   bool(*moduleProcedure)(unsigned int, PlugIn**) =
      reinterpret_cast<bool(*)(unsigned int, PlugIn**)>(mpModule->getProcedureAddress("instantiate_interface"));

   if (moduleProcedure)
   {
      moduleProcedure(plugInNumber, &pPlugIn);
   }

   return pPlugIn;
}

PlugIn* ModuleDescriptor::createInterface(PlugInDescriptorImp* pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   PlugIn* pPlugIn = createInterface(pDescriptor->getPlugInNumber());
   return pPlugIn;
}

bool ModuleDescriptor::isModule(const string& file)
{
   bool success = false;

   FilenameImp fileObj(file);
   Service<PlugInManagerServices> pPluginMgrSvc;
   DynamicModule* pModule = pPluginMgrSvc->getDynamicModule(fileObj.getFullPathAndName());

   if (pModule->isLoaded())
   {
      // Check the module interface version
      int(*moduleVersionProcedure)() = reinterpret_cast<int(*)()>
         (pModule->getProcedureAddress( "opticks_get_module_interface_version" ));
      int version = 0; // no module should ever have this version so it's a good "invalid" value
      if (moduleVersionProcedure != NULL)
      {
         version = moduleVersionProcedure();
      }

      if (version == 1) // This is the currently expected version...it hardly changes so it's hardcoded
      {
         vector<const char*> procNames;
         procNames.push_back("get_name");
         procNames.push_back("initialize");
         procNames.push_back("instantiate_interface");
         procNames.push_back("destroy");

         success = true; // will be set back to false in loop in case of failure
         for (vector<const char*>::const_iterator iter = procNames.begin();
            iter != procNames.end(); ++iter)
         {
            DMPROC pProcAddr = pModule->getProcedureAddress(*iter);
            success = success && pProcAddr != NULL;
         }
      }
   }
   pPluginMgrSvc->destroyDynamicModule(pModule);

   return success;
}

const bool ModuleDescriptor::isValidatedModule() const
{
   // TODO : Come up with test that uses name, version, and description 
   // to generate a checksum that is tested against validationKey.

   return (mValidationKey == "YES");
}

bool ModuleDescriptor::serialize(SessionItemSerializer& serializer) const
{
   return serializer.serialize(NULL, 0);
}

bool ModuleDescriptor::deserialize(SessionItemDeserializer& deserializer)
{
   return true;
}
