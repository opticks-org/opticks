/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"
#include "ShapeFileExporter.h"

const char* ModuleManager::mspName = "Shape File Exporter";
const char* ModuleManager::mspVersion = "1.0";
const char* ModuleManager::mspDescription = "Shape File Exporter";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{F4923A27-7CFE-45ca-99EC-FFF4E8C74434}";

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
         pPlugIn = new ShapeFileExporter();
         break;

      default:
         break;
   }

   return pPlugIn;
}
