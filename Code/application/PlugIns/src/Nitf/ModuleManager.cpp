/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"
#include "NitfExporter.h"
#include "NitfImporter.h"
#include "NitfPager.h"
#include "NitfUnknownTreParser.h"
#include "OssimServices.h"
#include "RpcGeoreference.h"

const char* ModuleManager::mspName = "Nitf";
const char* ModuleManager::mspVersion = "1.0.0";
const char* ModuleManager::mspDescription = "Nitf 2.0/2.1 Capability";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{086FC4DF-7865-4f15-934B-956C0D01807E}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 6;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;
   switch (plugInNumber)
   {
      case 0:
         pPlugIn = new Nitf::NitfImporter();
         break;

      case 1:
         pPlugIn = new Nitf::Pager();
         break;

      case 2:
         pPlugIn = new Nitf::OssimServices();
         break;

      case 3:
         pPlugIn = new Nitf::NitfExporter();
         break;

      case 4:
         pPlugIn = new Nitf::RpcGeoreference();
         break;

      case 5:
         pPlugIn = new Nitf::UnknownTreParser();
         break;

      default:
         break;
   }

   return pPlugIn;
}
