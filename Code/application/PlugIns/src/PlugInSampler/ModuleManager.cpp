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

#include "AnyPlugIn.h"
#include "BackgroundTest.h"
#include "CustomElementPlugIn.h"
#include "DummyCustomAlgorithm.h"
#include "DummyCustomImporter.h"
#include "MessageLogTest.h"
#include "ModuleManager.h"
#include "SampleRasterElementImporter.h"
#include "Scriptor.h"

//
// These static variables are used to describe the Module.  Set
// these according to how you want the Module configured.
const char *ModuleManager::mspName = "Plug-In Sampler";
const char *ModuleManager::mspVersion = "1.0";
const char *ModuleManager::mspDescription = "Sample plug-ins that test various components "
   "in the published interfaces.";
const char *ModuleManager::mspValidationKey = "none";
const char *ModuleManager::mspUniqueId = "{76379291-3175-45b7-8752-45FA936F0E22}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 8;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;

   switch (plugInNumber)
   {
      case 0:
         pPlugIn = new Scriptor;
         break;

      case 1:
         pPlugIn = new BackgroundTest;
         break;

      case 2:
         pPlugIn = new CustomElementPlugIn;
         break;

      case 3:
         pPlugIn = new AnyPlugIn;
         break;

      case 4:
         pPlugIn = new DummyCustomAlgorithm;
         break;

      case 5:
         pPlugIn = new DummyCustomImporter;
         break;

      case 6:
         pPlugIn = new MessageLogTestPlugin;
         break;

      case 7:
         pPlugIn = new SampleRasterElementImporter;
         break;

      default:
         break;
   }

   return pPlugIn;
}
