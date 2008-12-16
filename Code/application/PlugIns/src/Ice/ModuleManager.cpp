/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "IcePseudocolorLayerExporter.h"
#include "IcePseudocolorLayerImporter.h"
#include "IceRasterElementExporter.h"
#include "IceRasterElementImporter.h"
#include "ModuleManager.h"
#include "OptionQWidgetWrapper.h"
#include "OptionsIceExporter.h"

const char* ModuleManager::mspName = "Ice Format Module";
const char* ModuleManager::mspVersion = "1.0";
const char* ModuleManager::mspDescription = "Import/export of Ice files.";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{2FAC8CEC-0140-4eb0-B587-D2959BDDDF08}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 5;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;
   switch (plugInNumber)
   {
      case 0:
         pPlugIn = new IceRasterElementExporter();
         break;

      case 1:
         pPlugIn = new IceRasterElementImporter();
         break;

      case 2:
         pPlugIn = new IcePseudocolorLayerExporter();
         break;

      case 3:
         pPlugIn = new IcePseudocolorLayerImporter();
         break;

      case 4:
         pPlugIn = new OptionQWidgetWrapper<OptionsIceExporter>();
         break;

      default:
         break;
   }

   return pPlugIn;
}
