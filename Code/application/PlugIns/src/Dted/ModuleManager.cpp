/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DtedImporter.h"
#include "ModuleManager.h"

const char* ModuleManager::mspName = "DTED";
const char* ModuleManager::mspVersion = "1.00";
const char* ModuleManager::mspDescription = "DTED";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{5210B1CC-A305-46ec-BEF8-2F51EFD06E51}";

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
         pPlugIn = new DtedImporter();
         break;

      default:
         break;
   }

   return pPlugIn;
}
