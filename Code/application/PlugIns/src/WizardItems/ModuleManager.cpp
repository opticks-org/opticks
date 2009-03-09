/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

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
#include "ModuleManager.h"
#include "PrintView.h"
#include "SaveLayer.h"
#include "SelectPlugIn.h"
#include "SetDisplayedBand.h"
#include "SetDisplayMode.h"
#include "TemplateUtilities.h"

const char* ModuleManager::mspName = "Wizard Items";
const char* ModuleManager::mspVersion = "4.0.0";
const char* ModuleManager::mspDescription = "Contains additional items to add to a wizard";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{BA831023-B042-4c89-9B2D-E1ACE238CB04}";

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
         pPlugIn = new CreateAnimation();
         break;

      case 1:
         pPlugIn = new CreateExportFileDescriptor();
         break;

      case 2:
         pPlugIn = new CreateFileDescriptor();
         break;

      case 3:
         pPlugIn = new CreateRasterFileDescriptor();
         break;

      case 4:
         pPlugIn = new DeriveAoi();
         break;

      case 5:
         pPlugIn = new DeriveProduct();
         break;

      case 6:
         pPlugIn = new EditDataDescriptor();
         break;

      case 7:
         pPlugIn = new ExportDataSet();
         break;

      case 8:
         pPlugIn = new GetDataDescriptor();
         break;

      case 9:
         pPlugIn = new GetDataSet();
         break;

      case 10:
         pPlugIn = new GetExistingFilename();
         break;

      case 11:
         pPlugIn = new GetExistingFilenames();
         break;

      case 12:
         pPlugIn = new GetNewFilename();
         break;

      case 13:
         pPlugIn = new GetPrimaryRasterLayer();
         break;

      case 14:
         pPlugIn = new ImportDataSet();
         break;

      case 15:
         pPlugIn = new LoadAnnotation();
         break;

      case 16:
         pPlugIn = new LoadAoi();
         break;

      case 17:
         pPlugIn = new LoadGcpList();
         break;

      case 18:
         pPlugIn = new LoadTemplate();
         break;

      case 19:
         pPlugIn = new PrintView();
         break;

      case 20:
         pPlugIn = new SaveAnnotation();
         break;

      case 21:
         pPlugIn = new SaveAoi();
         break;

      case 22:
         pPlugIn = new SaveAoiFromDataSet();
         break;

      case 23:
         pPlugIn = new SaveGcpList();
         break;

      case 24:
         pPlugIn = new SaveGcpListFromDataSet();
         break;

      case 25:
         pPlugIn = new SaveTemplate();
         break;

      case 26:
         pPlugIn = new SelectPlugIn();
         break;

      case 27:
         pPlugIn = new SetDisplayedBand();
         break;

      case 28:
         pPlugIn = new SetDisplayMode();
         break;

      default:
         break;
   }

   return pPlugIn;
}
