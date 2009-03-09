/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Descriptors.h"
#include "ModuleManager.h"
#include "ScriptPlugIn.h"

const char* ModuleManager::mspName = "Scripts";
const char* ModuleManager::mspVersion = "0.1";
const char* ModuleManager::mspDescription = "Script PlugIns";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{5CAECFEA-8427-47b4-978F-9A2C1E292FD1}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return gDescriptors.size() + 1;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = new ScriptPlugIn(plugInNumber);
   return pPlugIn;
}
