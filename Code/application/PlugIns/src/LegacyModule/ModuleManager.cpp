/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Passthrough.h"
#include "ModuleManager.h"

const char* ModuleManager::mspName = "Legacy Module";
const char* ModuleManager::mspVersion = "1.0.0";
const char* ModuleManager::mspDescription = "Legacy Module";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "OpticksLegacyModule";

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
         pPlugIn = new Passthrough();
         break;
      default:
         break;
   }

   return pPlugIn;
}

