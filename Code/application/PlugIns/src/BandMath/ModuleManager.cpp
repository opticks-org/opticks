/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "BandMath.h"
#include "ModuleManager.h"

const char* ModuleManager::mspName = "Band Math";
const char* ModuleManager::mspVersion = "0.2";
const char* ModuleManager::mspDescription = "Band Math";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{2106A09E-E328-4a91-9F3C-67A7C203287B}";

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
         pPlugIn = new BandMath();
         break;

      default:
         break;
   }

   return pPlugIn;
}
