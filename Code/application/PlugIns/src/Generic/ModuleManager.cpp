/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GenericImporter.h"
#include "ModuleManager.h"

const char* ModuleManager::mspName = "Generic File Module";
const char* ModuleManager::mspVersion = "1.0.0";
const char* ModuleManager::mspDescription = "Generic File Support.";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{5D734E60-4738-423f-A499-B397A5520789}";

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
         pPlugIn = new GenericImporter();
         break;

      default:
         break;
   }

   return pPlugIn;
}
