/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

/**
 *  Module Shell
 *
 *  The Module Shell class implements the Module interface.
 *  This class represents the "C" entry points for a Module.  Modules are
 *  made up of Plug-Ins.  The Main Application uses this interface to
 *  query the Module and determine what Plug-Ins are available.  The Module
 *  Manager singleton class keeps track of all Plug-In instances.
 *
 *  PLUG-IN DEVELOPERS SHOULD NOT EDIT ANYTHING IN THIS FILE...
 *
 *  Windows NOTE : This library must be changed from the standard 
 *               static library single threaded /ML to multi-threaded /MT.  
 *               Also, the "-D _LIB" must be changed to "-D _USRDLL" since 
 *               it will be linked into a DLL.
 */

#include "AppConfig.h"
#include "Module.h"
#include "External.h"
#include "PlugIn.h"
#include "ModuleManager.h"

extern "C" int LINKAGE opticks_get_module_interface_version()
{
   return 1;
}

extern "C" bool LINKAGE get_name( char **pName, 
                                  char **pVersion, 
                                  char **pDescription,
                                  unsigned int *pTotalPlugIns,
                                  char **pValidationKey,
                                  char **pModuleId)
{
   if (pName == NULL || pVersion == NULL || pDescription == NULL || pTotalPlugIns == NULL ||
      pValidationKey == NULL || pModuleId == NULL)
   {
      return false;
   }
   *pName = ModuleManager::getName();
   *pVersion = ModuleManager::getVersion();
   *pDescription = ModuleManager::getDescription();
   *pTotalPlugIns = ModuleManager::getTotalPlugIns();
   *pValidationKey = ModuleManager::getValidationKey();
   *pModuleId = ModuleManager::getUniqueId();
   return (*pName != NULL && *pVersion != NULL && *pDescription != NULL && *pValidationKey != NULL &&
      *pModuleId != NULL);
}

extern "C" bool LINKAGE initialize( External *services )
{
    ModuleManager *module = ModuleManager::instance();

    if (module) {

       //
       // Initialize Reference to various Services
       //

       module->setService( services ); 

       return true;
    }

    return false;
}

extern "C" bool LINKAGE instantiate_interface(unsigned int plugInNumber, PlugIn** interfaceAddress)
{
   *interfaceAddress = NULL;

   ModuleManager* pModule = ModuleManager::instance();
   if (pModule != NULL)
   {
      *interfaceAddress = pModule->getPlugIn(plugInNumber);
   }

   return (*interfaceAddress != NULL);
}

extern "C" bool LINKAGE destroy()
{
    return true;
}
