/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AviExporter.h"
#include "ModuleManager.h"
#include "Mpeg1Exporter.h"
#include "OptionQWidgetWrapper.h"
#include "OptionsMovieExporter.h"

const char* ModuleManager::mspName = "MovieExporter";
const char* ModuleManager::mspVersion = "1.00";
const char* ModuleManager::mspDescription = "Plug-in for saving animation sequences to movie files.";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{0D8254C0-F4DC-4f12-B24A-7E7ED20C1402}";

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
         pPlugIn = new AviExporter();
         break;

      case 1:
         pPlugIn = new Mpeg1Exporter();
         break;

      case 2:
         pPlugIn = new OptionQWidgetWrapper<OptionsMovieExporter>();
         break;

      default:
         break;
   }

   return pPlugIn;
}
