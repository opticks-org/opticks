/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"
#include "ResultsExporter.h"

const char* ModuleManager::mspName = "Results";
const char* ModuleManager::mspVersion = "1.0.0";
const char* ModuleManager::mspDescription = "Utilities for results raster elements";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{3120B8DB-56BB-4c55-B450-829C41564C2A}";

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
         pPlugIn = new ResultsExporter();
         break;

      default:
         break;
   }

   return pPlugIn;
}
