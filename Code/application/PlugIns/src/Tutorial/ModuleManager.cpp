/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"
#include "Tutorial1.h"
#include "Tutorial2.h"
#include "Tutorial3.h"
#include "Tutorial4.h"
#include "Tutorial5.h"

// Information about the module
const char *ModuleManager::mspName = "Tutorial";                                    // module name
const char *ModuleManager::mspVersion = "1.0";                                      // module version
const char *ModuleManager::mspDescription = "Plug-ins described in the tutorial.";  // module description
const char *ModuleManager::mspValidationKey = "none";                               // ignored by Opticks
const char *ModuleManager::mspUniqueId = "{D0879A4B-AD8F-4C48-8911-2F175FD8A104}";  // unique module descriptor

// Number of plug-ins in the module
unsigned int ModuleManager::getTotalPlugIns()
{
   return 5;
}

// Factory for all the plug-ins in the module
PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;
   switch (plugInNumber)
   {
      case 0:
         pPlugIn = new Tutorial1();
         break;

      case 1:
         pPlugIn = new Tutorial2();
         break;

      case 2:
         pPlugIn = new Tutorial3();
         break;

      case 3:
         pPlugIn = new Tutorial4();
         break;

      case 4:
         pPlugIn = new Tutorial5();
         break;

      default:
         break;
   }
   return pPlugIn;
}
