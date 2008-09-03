/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"
#include "ProductionPlugInTester.h"

//
// These static variables are used to describe the Module.  Set
// these according to how you want the Module configured.
//
const char *ModuleManager::mspName = "Production Plug-In Tester";
const char *ModuleManager::mspVersion = "1.0";
const char *ModuleManager::mspDescription = "Lists the Not For Production Plug-Ins.";
const char *ModuleManager::mspValidationKey = "none";
const char *ModuleManager::mspUniqueId = "{EC7AD794-B303-40da-B73B-FC8ED33107FB}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 1;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;

   switch (plugInNumber)
   {
      case 0:
         pPlugIn = new ProductionPlugInTester();
         break;

      default:
         break;
   }

   return pPlugIn;
}
