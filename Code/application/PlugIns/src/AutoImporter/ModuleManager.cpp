/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AutoImporter.h"
#include "ModuleManager.h"

const char* ModuleManager::mspName = "AutoImporter";
const char* ModuleManager::mspVersion = "1.0";
const char* ModuleManager::mspDescription = "Automatic Importer Selector";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{E9D97C74-E7C4-42c9-B49D-AC90D94C6D47}";

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
         pPlugIn = new AutoImporter();
         break;

      default:
         break;
   }

   return pPlugIn;
}
