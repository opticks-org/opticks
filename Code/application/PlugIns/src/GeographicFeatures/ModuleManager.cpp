/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

/**
 * Module Manager 
 *
 * The Module Manager is used to inform the main application about the
 * Plug-Ins within the Module.  It is also used to create, destroy, and provide
 * access to the Plug-Ins.  Plug-In developers edit this class to build
 * a Plug-In Module composed of thier Plug-Ins. This is a singleton class.  
 * Only one instance of this class exists at a given time.  Use the instance() 
 * method to get a reference to the class.
 *
 * To create (1) Add the include directories "..\interfaces, ..\libplugin"
 *           (2) Add the library directories "..\libplugin"
 *           (3) Add libplugin.lib to the link directives
 *           (4) Change the name of the DLL to "spXXXX.dll" with XXXX being
 *               the name of the Module.
 */

#include "ModuleManager.h"
#include "PlugIn.h"

#include "FeatureLayerImporter.h"
#include "FeatureManager.h"
#include "PropertiesFeatureClass.h"
#include "ShapeFileImporter.h"

//
// These static variables are used to describe the Module.  Set 
// these according to how you want the Module configured.  
//
const char *ModuleManager::mspName = "Geographic Features";
const char *ModuleManager::mspVersion = "1.0";
const char *ModuleManager::mspDescription = "Geographic Features";
const char *ModuleManager::mspValidationKey = "none";
const char *ModuleManager::mspUniqueId = "{D903D713-C3D3-44ca-A3AD-1B69ED3E92BA}";

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
         pPlugIn = static_cast<PlugIn*>(new FeatureLayerImporter);
         break;

      case 1:
         pPlugIn = static_cast<PlugIn*>( new ShapeFileImporter );
         break;

      case 2:
         pPlugIn = static_cast<PlugIn*>( new FeatureManager );
         break;

      case 3:
         pPlugIn = static_cast<PlugIn*>( new PropertiesFeatureClass );
         break;

      default:
         break;
   }

   return pPlugIn;
}
