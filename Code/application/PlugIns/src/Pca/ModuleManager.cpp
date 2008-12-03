/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"
#include "PCA.h"

const char* ModuleManager::mspName = "Principal Component Analysis";
const char* ModuleManager::mspVersion = "1.0.0";
const char* ModuleManager::mspDescription = "Run Principal Component Analysis on Data Cube";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{5BF5553F-EC95-487a-A58A-C94F5E5EF042}";

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
         pPlugIn = new PCA();
         break;

      default:
         break;
   }

   return pPlugIn;
}
