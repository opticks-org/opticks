/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

/**
 * Module Manager
 *
 * The Module Manager is used to inform the main application about the
 * Plug-Ins within the Module.  It is also used to create, destroy, and provide
 * access to the Plug-Ins.  Plug-In developers edit this class to build
 * a Plug-In Module composed of thier Plug-Ins. This is a singleton class.
 * Only one instance of this class exists at a given time.  Use the instance()
 * method to get a reference to the class.
 *
 * To create (1) Add the include directories "..\interfaces, ..\libplugin"
 *           (2) Add the library directories "..\libplugin"
 *           (3) Add libplugin.lib to the link directives
 *           (4) Change the name of the DLL to "spXXXX.dll" with XXXX being
 *               the name of the Module.
 */

#include "AppConfig.h"
#include "ModuleManager.h"

#if defined(HDF4_SUPPORT)
#include "SampleHdf4Importer.h"
#endif

const char *ModuleManager::mspName = "Plug-In Sampler Hdf";
const char *ModuleManager::mspVersion = "1.0";
const char *ModuleManager::mspDescription = "Sample plug-ins that test various components "
   "in the published interfaces.";
const char *ModuleManager::mspValidationKey = "none";
const char *ModuleManager::mspUniqueId = "{D1F56217-05AA-4554-93B8-6950B8E24E93}";

unsigned int ModuleManager::getTotalPlugIns()
{
#if defined(HDF4_SUPPORT)
   return 1;
#else
   return 0;
#endif
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;

   switch (plugInNumber)
   {
#if defined(HDF4_SUPPORT)
      case 0:
         pPlugIn = static_cast<PlugIn*>(new SampleHdf4Importer);
         break;
#endif

      default:
         break;
   }

   return pPlugIn;
}
