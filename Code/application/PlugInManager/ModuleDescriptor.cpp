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
#include "MessageLogResource.h"
#include "PlugIn.h"
#include "PlugInDescriptorImp.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
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
   mModuleVersion(0),
   mFileName(""),
   mFileSize(0),
   mFileDate(0),
   mpModule(NULL)
{
   Icons* pIcons = Icons::instance();
   if (pIcons != NULL)
   {
      setIcon(pIcons->mModule);
   }

   addPropertiesPage(PropertiesModuleDescriptor::getName());
}

ModuleDescriptor::~ModuleDescriptor()
{
   try
   {
      for (std::vector<PlugInDescriptorImp*>::iterator plugIn = mPlugins.begin(); plugIn != mPlugins.end(); ++plugIn)
      {
         delete *plugIn;
      }

      if (mpModule != NULL)
      {
         Service<PlugInManagerServices>()->destroyDynamicModule(mpModule);
         mpModule = NULL;
      }
   }
   catch (...)
   {
      abort();
      // Don't really care if a delete fails, just continue
   }
}

ModuleDescriptor* ModuleDescriptor::getModule(const std::string& filename, map<string, string>& plugInIds)
{
   DynamicModule* pDynMod = Service<PlugInManagerServices>()->getDynamicModule(filename);
   if (pDynMod == NULL || !pDynMod->isLoaded())
   {
      return NULL;
   }

   // try a new style module first
   OpticksGetModuleDescriptorType pProcAddr =
      reinterpret_cast<OpticksGetModuleDescriptorType>(pDynMod->getProcedureAddress("opticks_get_module_descriptor"));
   if (pProcAddr != NULL)
   {
      struct OpticksModuleDescriptor* pDescriptor = pProcAddr(ConnectionManager::instance());
      if (pDescriptor != NULL)
      {
         VERIFY(pDescriptor->version == 2);
#if defined(WIN_API)
         if (pDescriptor->debug == DEBUG_BOOL)
         {
#endif
            ModuleDescriptor* pModule = new ModuleDescriptor(pDescriptor->pModuleId);
            FilenameImp fileObj(filename);
            pModule->mFileName = fileObj.getFullPathAndName();

            pModule->setName(pModule->mFileName);
            pModule->setDisplayText(filename);

            FileFinderImp find;
            find.findFile(fileObj.getPath(), fileObj.getFileName());
            find.findNextFile();
            pModule->mFileSize = find.getLength();

            find.getLastModificationTime(pModule->mFileDate);
            pModule->mDescription = "";
            pModule->mpModule = pDynMod;
            pModule->mModuleVersion = pDescriptor->version;
            pModule->mInstantiateSymbol = pDescriptor->pInstantiateSymbol;
            pModule->initializePlugInInformation(plugInIds);

            return pModule;
#if defined(WIN_API)
         }
         else
         {
            std::string errorMessage("Error loading plug-in from PlugIns folder, the plug-in was ");
            if (DEBUG_BOOL)
            {
               errorMessage += "built in release mode, but this a debug mode version of the application.";
            }
            else
            {
               errorMessage += "built in debug mode, but this a release mode version of the application.";
            }
            MessageResource message("PlugInError", "app", "8B5F1ADB-1210-406f-B961-72C2189CE84D");
            message->addProperty("message", errorMessage);
            message->addProperty("filename", filename);
            message->finalize();

            Service<PlugInManagerServices>()->destroyDynamicModule(pDynMod);
            return NULL;
         }
#endif
      }
      else
      {
         std::string errorMessage("Error loading plug-in from PlugIns folder, the plug-in did not return the proper struct.");
         MessageResource message("PlugInError", "app", "B63EB55A-2881-4A2A-AE8B-C8FE7E9C4080");
         message->addProperty("message", errorMessage);
         message->addProperty("filename", filename);
         message->finalize();

         Service<PlugInManagerServices>()->destroyDynamicModule(pDynMod);
         return NULL;
      }
   }

   // old style modules only supported on Solaris and Windows
#if defined(SOLARIS) || defined(WIN_API)
   // try an old style module
   // Check the module interface version
   int version = 0; // no module should ever have this version so it's a good "invalid" value
   int(*moduleVersionProcedure)() = reinterpret_cast<int(*)()>
      (pDynMod->getProcedureAddress( "opticks_get_module_interface_version" ));
   if (moduleVersionProcedure != NULL)
   {
      version = moduleVersionProcedure();
   }

   if (version != 1) // This is the currently expected version...it hardly changes so it's hardcoded
   {
      Service<PlugInManagerServices>()->destroyDynamicModule(pDynMod);
      return NULL;
   }

   // below are the functions required for a version 1 module
   bool(*pInitProcedure)(External*) =
      reinterpret_cast<bool(*)(External*)>(pDynMod->getProcedureAddress("initialize"));
   bool(*pNameProcedure)(char**, char**, char**, unsigned int*, char**, char**) =
      reinterpret_cast<bool(*)(char**, char**, char**, unsigned int*, char**, char**)>(
                               pDynMod->getProcedureAddress("get_name"));
   DMPROC pInterfaceProc = pDynMod->getProcedureAddress("instantiate_interface");
   DMPROC pDestroyProc = pDynMod->getProcedureAddress("destroy");

   if (pInitProcedure == NULL || pNameProcedure == NULL || pInterfaceProc == NULL || pDestroyProc == NULL)
   {
      Service<PlugInManagerServices>()->destroyDynamicModule(pDynMod);
      return NULL;
   }

   if (!pInitProcedure(ConnectionManager::instance()))
   {
      Service<PlugInManagerServices>()->destroyDynamicModule(pDynMod);
      return NULL;
   }

   char* pModuleName = NULL;
   char* pModuleVersion = NULL;
   char* pModuleDescription = NULL;
   unsigned int totalPlugIns = 0;
   char* pModuleId = NULL;
   char* pValidationKey = NULL;
   if (!pNameProcedure(&pModuleName, &pModuleVersion, &pModuleDescription,
      &totalPlugIns, &pValidationKey, &pModuleId) || pModuleId == NULL)
   {
      Service<PlugInManagerServices>()->destroyDynamicModule(pDynMod);
      return NULL;
   }

   ModuleDescriptor* pModule = new ModuleDescriptor(string(pModuleId));
   FilenameImp fileObj(filename);
   pModule->mFileName = fileObj.getFullPathAndName();
   pModule->setName(pModule->mFileName);
   pModule->setDisplayText(filename);

   FileFinderImp find;
   find.findFile(fileObj.getPath(), fileObj.getFileName());
   find.findNextFile();
   pModule->mFileSize = find.getLength();

   find.getLastModificationTime(pModule->mFileDate);
   pModule->mVersion = pModuleVersion;
   pModule->mDescription = pModuleDescription;
   pModule->mPlugInTotal = totalPlugIns;
   pModule->mValidationKey = pValidationKey;
   pModule->mpModule = pDynMod;
   pModule->mModuleVersion = version;
   pModule->initializePlugInInformation(plugInIds);
  
   return pModule;
#else
   return NULL;
#endif
}

bool ModuleDescriptor::load()
{
   VERIFY(mModuleVersion > 0);
   if (isLoaded())
   {
      return true;
   }

   mpModule = Service<PlugInManagerServices>()->getDynamicModule(getFileName());
   if (mpModule == NULL)
   {
      return false;
   }

   //
   // Attempt to call Module's initialize routine if it is a legacy module.
   //
   if (mModuleVersion == 1)
   {
      bool(*moduleProcedure)(External*) =
         reinterpret_cast<bool(*)(External*)>(mpModule->getProcedureAddress("initialize"));
      if (moduleProcedure != NULL && !moduleProcedure(ConnectionManager::instance()))
      {
         return false;
      }
   }

   return true;
}

void ModuleDescriptor::unload()
{
   if (mpModule != NULL)
   {
      Service<PlugInManagerServices>()->destroyDynamicModule(mpModule);
      mpModule = NULL;
   }
}

bool ModuleDescriptor::initializePlugInInformation(map<string, string>& plugInIds)
{
   VERIFY(load());

   //
   // Determine what Plug-Ins are available.
   //

   // TODO : Update Plug-Ins currently in the list.

   for (unsigned int plugInNumber = 0; mModuleVersion > 1 || plugInNumber < getNumPlugIns(); plugInNumber++)
   {
      PlugIn* pPlugIn = createInterface(plugInNumber);
      if (mModuleVersion > 1 && pPlugIn == NULL)
      {
         mPlugInTotal = plugInNumber + 1;
         break;
      }
      else if (pPlugIn == NULL)
      {
         continue;
      }
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

   unload();

   return true;
}

PlugIn* ModuleDescriptor::createInterface(unsigned int plugInNumber)
{
   if (mpModule == NULL)
   {
      return NULL;
   }

   PlugIn* pPlugIn = NULL;

   if (mModuleVersion == 1)
   {
      bool(*moduleProcedure)(unsigned int, PlugIn**) =
         reinterpret_cast<bool(*)(unsigned int, PlugIn**)>(mpModule->getProcedureAddress("instantiate_interface"));

      if (moduleProcedure)
      {
         moduleProcedure(plugInNumber, &pPlugIn);
      }
   }
   else if (mModuleVersion > 1)
   {
      OpticksInstantiateType moduleProcedure =
         reinterpret_cast<OpticksInstantiateType>(mpModule->getProcedureAddress(mInstantiateSymbol));

      if (moduleProcedure != NULL)
      {
         External* pExternal = ConnectionManager::instance();
         moduleProcedure(pExternal, plugInNumber, &pPlugIn);
      }
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

const bool ModuleDescriptor::isValidatedModule() const
{
   // TODO : Come up with test that uses name, version, and description 
   // to generate a checksum that is tested against validationKey.

   return (mModuleVersion != 1) || (mValidationKey == "YES");
}

bool ModuleDescriptor::serialize(SessionItemSerializer& serializer) const
{
   return serializer.serialize(NULL, 0);
}

bool ModuleDescriptor::deserialize(SessionItemDeserializer& deserializer)
{
   return true;
}
