/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "EnviExporter.h"
#include "EnviImporter.h"
#include "EnviLibraryExporter.h"
#include "EnviLibraryImporter.h"
#include "ModuleManager.h"

const char* ModuleManager::mspName = "ENVI";
const char* ModuleManager::mspVersion = "0.1";
const char* ModuleManager::mspDescription = "ENVI I/O";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{6BB87352-49FA-4464-90AB-826D0F0CC74A}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 4;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;
   switch (plugInNumber)
   {
      case 0:
         pPlugIn = new EnviImporter();
         break;

      case 1:
         pPlugIn = new EnviExporter();
         break;

      case 2:
         pPlugIn = new EnviLibraryExporter();
         break;

      case 3:
         pPlugIn = new EnviLibraryImporter();
         break;

      default:
         break;
   }

   return pPlugIn;
}
