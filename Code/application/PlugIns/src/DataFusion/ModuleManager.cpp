/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataFusion.h"
#include "FlickerControls.h"
#include "ModuleManager.h"

const char* ModuleManager::mspName = "Data Fusion";
const char* ModuleManager::mspVersion = "1.00";
const char* ModuleManager::mspDescription = "Fuse 2 images together & manipulate them";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{4B3BECDA-6C42-45ea-AE4D-ECF80CEF18EA}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 2;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;
   switch (plugInNumber)
   {
      case 0:
         pPlugIn = new DataFusion();
         break;

      case 1:
         pPlugIn = new FlickerControls();
         break;

      default:
         break;
   }

   return pPlugIn;
}
