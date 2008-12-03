/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "KmlExporter.h"
#include "KMLServer.h"
#include "ModuleManager.h"

const char* ModuleManager::mspName = "KML Module";
const char* ModuleManager::mspVersion = "1";
const char* ModuleManager::mspDescription = "Serve and export KML.";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{669E57D5-91EC-4a0b-B9BB-A163B6F57046}";

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
         pPlugIn = new KMLServer();
         break;

      case 1:
         pPlugIn = new KmlExporter();
         break;

      case 2:
         pPlugIn = new KmlLayerExporter();
         break;

      default:
         break;
   }

   return pPlugIn;
}
