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
#include "GetPrimaryRasterLayer.h"
#include "GetView.h"
#include "ImportDataSet.h"
#include "LoadLayer.h"
#include "PrintView.h"
#include "SaveLayer.h"
#include "SelectPlugIn.h"
#include "SetDisplayedBand.h"
#include "SetDisplayMode.h"
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
   return 29;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;
   switch (plugInNumber)
   {
      case 0:
         pPlugIn = new CreateAnimation;
         break;

      case 1:
         pPlugIn = new CreateExportFileDescriptor;
         break;

      case 2:
         pPlugIn = new CreateFileDescriptor;
         break;

      case 3:
         pPlugIn = new CreateRasterFileDescriptor;
         break;

      case 4:
         pPlugIn = new DeriveAoi;
         break;

      case 5:
         pPlugIn = new DeriveProduct;
         break;

      case 6:
         pPlugIn = new EditDataDescriptor;
         break;

      case 7:
         pPlugIn = new ExportDataSet;
         break;

      case 8:
         pPlugIn = new GetDataDescriptor;
         break;

      case 9:
         pPlugIn = new GetDataSet;
         break;

      case 10:
         pPlugIn = new GetExistingFilename;
         break;

      case 11:
         pPlugIn = new GetExistingFilenames;
         break;

      case 12:
         pPlugIn = new GetNewFilename;
         break;

      case 13:
         pPlugIn = new GetPrimaryRasterLayer;
         break;

      case 14:
         pPlugIn = new ImportDataSet;
         break;

      case 15:
         pPlugIn = new LoadAnnotation;
         break;

      case 16:
         pPlugIn = new LoadAoi;
         break;

      case 17:
         pPlugIn = new LoadGcpList;
         break;

      case 18:
         pPlugIn = new LoadTemplate;
         break;

      case 19:
         pPlugIn = new PrintView;
         break;

      case 20:
         pPlugIn = new SaveAnnotation;
         break;

      case 21:
         pPlugIn = new SaveAoi;
         break;

      case 22:
         pPlugIn = new SaveAoiFromDataSet;
         break;

      case 23:
         pPlugIn = new SaveGcpList;
         break;

      case 24:
         pPlugIn = new SaveGcpListFromDataSet;
         break;

      case 25:
         pPlugIn = new SaveTemplate;
         break;

      case 26:
         pPlugIn = new SelectPlugIn;
         break;

      case 27:
         pPlugIn = new SetDisplayedBand;
         break;

      case 28:
         pPlugIn = new SetDisplayMode;
         break;

      default:
         break;
   }

   return pPlugIn;
}
