/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"
#include "SioImporter.h"

const char* ModuleManager::mspName = "Standard Format Module";
const char* ModuleManager::mspVersion = "1.0.0";
const char* ModuleManager::mspDescription = "Import/export of SIO files.";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{5CF90A59-0CB7-4e3e-98BB-186619221BC0}";

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
         pPlugIn = new SioImporter();
         break;

      default:
         break;
   }

   return pPlugIn;
}
