/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"
#include "AspamImporter.h"
#include "AspamManager.h"
#include "AspamViewer.h"

const char* ModuleManager::mspName = "Aspam";
const char* ModuleManager::mspVersion = "1.0";
const char* ModuleManager::mspDescription = "Import and view ASPAM/PAR data";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{91F8A690-ED42-4757-BE0B-6350C9F48C6C}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 3;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;
   switch (plugInNumber)
   {
      case 0:
         pPlugIn = new AspamImporter();
         break;

      case 1:
         pPlugIn = new AspamManager();
         break;

      case 2:
         pPlugIn = new AspamViewer();
         break;

      default:
         break;
   }

   return pPlugIn;
}
