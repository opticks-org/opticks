/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "FeatureLayerImporter.h"
#include "FeatureManager.h"
#include "ModuleManager.h"
#include "PropertiesFeatureClass.h"
#include "ShapeFileImporter.h"

const char* ModuleManager::mspName = "Geographic Features";
const char* ModuleManager::mspVersion = "1.0";
const char* ModuleManager::mspDescription = "Geographic Features";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{D903D713-C3D3-44ca-A3AD-1B69ED3E92BA}";

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
         pPlugIn = new FeatureLayerImporter();
         break;

      case 1:
         pPlugIn = new ShapeFileImporter();
         break;

      case 2:
         pPlugIn = new FeatureManager();
         break;

      case 3:
         pPlugIn = new PropertiesFeatureClass();
         break;

      default:
         break;
   }

   return pPlugIn;
}
