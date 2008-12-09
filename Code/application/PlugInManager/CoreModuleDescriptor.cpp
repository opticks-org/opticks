/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CoreModuleDescriptor.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "ModuleManager.h"
#include "PlugIn.h"
#include "PlugInDescriptorImp.h"

#include <string>
#include <vector>
using namespace std;

CoreModuleDescriptor::CoreModuleDescriptor(const string& id, map<string, string>& plugInIds) :
   ModuleDescriptor(id)
{
   ModuleManager* pCore = ModuleManager::instance();
   setName(pCore->getName());
   mVersion = pCore->getVersion();
   mDescription = pCore->getDescription();
   mPlugInTotal = pCore->getTotalPlugIns();
   mValidationKey = pCore->getValidationKey();
   mFileName = "This module is built into the application and does not have a separate file.";
   mFileDate.set(APP_RELEASE_DATE_YEAR, APP_RELEASE_DATE_MONTH, APP_RELEASE_DATE_DAY);
   initializePlugInInformation(plugInIds);
}

CoreModuleDescriptor::~CoreModuleDescriptor()
{
}

bool CoreModuleDescriptor::load()
{
   return true;
}

void CoreModuleDescriptor::unload()
{
   return;
}

PlugIn* CoreModuleDescriptor::createInterface(unsigned int plugInNumber)
{
   return ModuleManager::instance()->getPlugIn(plugInNumber);
}

PlugIn* CoreModuleDescriptor::createInterface(PlugInDescriptorImp* pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   return ModuleManager::instance()->getPlugIn(pDescriptor->getPlugInNumber());
}

const bool CoreModuleDescriptor::isValidatedModule() const
{
   return true;
}

bool CoreModuleDescriptor::isLoaded() const
{
   return true;
}
