/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"
#include "SecondMoment.h"

const char* ModuleManager::mspName = "SecondMoment";
const char* ModuleManager::mspVersion = "0.1";
const char* ModuleManager::mspDescription = "Second Moment Matrix";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{6AB11EC1-578A-4870-9661-4D3C00FD84EB}";

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
         pPlugIn = new SecondMoment();
         break;

      default:
         break;
   }

   return pPlugIn;
}
