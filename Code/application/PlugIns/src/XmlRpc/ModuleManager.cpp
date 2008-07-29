/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"
#include "XmlRpcServer.h"

//
// These static variables are used to describe the Module.  Set 
// these according to how you want the Module configured.  
//
const char *ModuleManager::mspName = "XmlRpc";
const char *ModuleManager::mspVersion = "1.0";
const char *ModuleManager::mspDescription = "Plug-ins related to the XML-RPC server and client.";
const char *ModuleManager::mspValidationKey = "none";
const char *ModuleManager::mspUniqueId = "{10AF1F8F-FFD4-4773-B820-E1D2BA266274}";

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
         pPlugIn = new XmlRpcServer();
         break;
      default:
         break;
   }

   return pPlugIn;
}
