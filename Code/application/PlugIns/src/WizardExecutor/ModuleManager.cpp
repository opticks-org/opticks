/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "BatchWizardExecutor.h"
#include "ModuleManager.h"
#include "WizardExecutor.h"

const char* ModuleManager::mspName = "Wizard Executor";
const char* ModuleManager::mspVersion = "1.0.0";
const char* ModuleManager::mspDescription = "Executor for launching wizards";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{13192489-18B5-43e6-8DD4-024405CA95BA}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 2;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;
   switch (plugInNumber)
   {
      case 0:
         pPlugIn = new WizardExecutor();
         break;

      case 1:
         pPlugIn = new BatchWizardExecutor();
         break;

      default:
         break;
   }

   return pPlugIn;
}
