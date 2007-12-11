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
 * To create (1) Add the include directories "..\interfaces, ..\pluginlib"
 *           (2) Add the library directories "..\pluginlib"
 *           (3) Add pluginlib.lib to the link directives
 *           (4) Change the name of the DLL to "spXXXX.dll" with XXXX being
 *               the name of the Module.
 */

#include "ModuleManager.h"
#include "CreateAnimation.h"
#include "CreateExportFileDescriptor.h"
#include "CreateFileDescriptor.h"
#include "DeriveLayer.h"
#include "DeriveProduct.h"
#include "EditDataDescriptor.h"
#include "ExportDataSet.h"
#include "GetDataDescriptor.h"
#include "GetFilename.h"
#include "GetView.h"
#include "ImportDataSet.h"
#include "LoadLayer.h"
#include "PrintView.h"
#include "SaveLayer.h"
#include "SelectPlugIn.h"
#include "TemplateUtilities.h"

//
// These static variables are used to describe the Module.  Set 
// these according to how you want the Module configured.  
//
const char* ModuleManager::mspName = "Wizard Items";
const char* ModuleManager::mspVersion = "4.0.0";
const char* ModuleManager::mspDescription = "Contains additional items to add to a wizard";
const char* ModuleManager::mspValidationKey = "none";
const char *ModuleManager::mspUniqueId = "{BA831023-B042-4c89-9B2D-E1ACE238CB04}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 26;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;
   switch (plugInNumber)
   {
      case 0:
         pPlugIn = static_cast<PlugIn*>(new CreateAnimation);
         break;

      case 1:
         pPlugIn = static_cast<PlugIn*>(new CreateExportFileDescriptor);
         break;

      case 2:
         pPlugIn = static_cast<PlugIn*>(new CreateFileDescriptor);
         break;

      case 3:
         pPlugIn = static_cast<PlugIn*>(new CreateRasterFileDescriptor);
         break;

      case 4:
         pPlugIn = static_cast<PlugIn*>(new DeriveAoi);
         break;

      case 5:
         pPlugIn = static_cast<PlugIn*>(new EditDataDescriptor);
         break;

      case 6:
         pPlugIn = static_cast<PlugIn*>(new ExportDataSet);
         break;

      case 7:
         pPlugIn = static_cast<PlugIn*> (new GetDataSet);
         break;

      case 8:
         pPlugIn = static_cast<PlugIn*>(new GetDataDescriptor);
         break;

      case 9:
         pPlugIn = static_cast<PlugIn*>(new GetExistingFilename);
         break;

      case 10:
         pPlugIn = static_cast<PlugIn*>(new GetExistingFilenames);
         break;

      case 11:
         pPlugIn = static_cast<PlugIn*>(new GetNewFilename);
         break;

      case 12:
         pPlugIn = static_cast<PlugIn*>(new ImportDataSet);
         break;

      case 13:
         pPlugIn = static_cast<PlugIn*>(new LoadAoi);
         break;

      case 14:
         pPlugIn = static_cast<PlugIn*>(new LoadAnnotation);
         break;

      case 15:
         pPlugIn = static_cast<PlugIn*>(new LoadGcpList);
         break;

      case 16:
         pPlugIn = static_cast<PlugIn*>(new LoadTemplate);
         break;

      case 17:
         pPlugIn = static_cast<PlugIn*>(new PrintView);
         break;

      case 18:
         pPlugIn = static_cast<PlugIn*>(new SaveAoi);
         break;

      case 19:
         pPlugIn = static_cast<PlugIn*>(new SaveAoiFromDataSet);
         break;

      case 20:
         pPlugIn = static_cast<PlugIn*>(new SaveAnnotation);
         break;

      case 21:
         pPlugIn = static_cast<PlugIn*>(new SaveGcpList);
         break;

      case 22:
         pPlugIn = static_cast<PlugIn*>(new SaveGcpListFromDataSet);
         break;

      case 23:
         pPlugIn = static_cast<PlugIn*>(new SaveTemplate);
         break;

      case 24:
         pPlugIn = static_cast<PlugIn*>(new SelectPlugIn);
         break;

      case 25:
         pPlugIn = static_cast<PlugIn*> (new DeriveProduct);
         break;

      default:
         break;
   }

   return pPlugIn;
}
