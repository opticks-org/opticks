/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"
#include "GdalImporter.h"
#include "GdalRasterPager.h"

const char *ModuleManager::mspName = "Gdal";
const char *ModuleManager::mspVersion = "1";
const char *ModuleManager::mspDescription = "GDAL based generic I/O";
const char *ModuleManager::mspValidationKey = "none";
const char *ModuleManager::mspUniqueId = "{567B5816-610D-4a64-965F-63F33F0A587D}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 2;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn *pPlugIn = NULL;

   switch(plugInNumber)
   {
      case 0:
         pPlugIn = new GdalImporter;
         break;
      case 1:
         pPlugIn = new GdalRasterPager;
         break;
   }
   return pPlugIn;
}
