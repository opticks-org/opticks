/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModelExporter.h"
#include "ModelImporter.h"
#include "ModuleManager.h"
#include "CgmExporter.h"
#include "CgmImporter.h"
#include "LayerExporter.h"
#include "LayerImporter.h"

class AnnotationElement;
class AoiElement;
class GcpList;
class TiePointList;

const char* ModuleManager::mspName = "CoreIo";
const char* ModuleManager::mspVersion = "1.00";
const char* ModuleManager::mspDescription =
   "Plug-ins related to I/O for layers, model elements, and other \"core\" types";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{D83A0C6B-4125-4d53-91E1-18F0C85A966D}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 12;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;
   switch (plugInNumber)
   {
      case 0:
         pPlugIn = new CgmImporter();
         break;
      case 1:
         pPlugIn = new LayerExporter(ANNOTATION);
         break;
      case 2:
         pPlugIn = new LayerExporter(AOI_LAYER);
         break;
      case 3:
         pPlugIn = new LayerExporter(GCP_LAYER);
         break;
      case 4:
         pPlugIn = new LayerExporter(TIEPOINT_LAYER);
         break;
      case 5:
         pPlugIn = new LayerImporter();
         break;
      case 6:
         pPlugIn = new ModelExporter(TypeConverter::toString<AoiElement>());
         break;
      case 7:
         pPlugIn = new ModelExporter(TypeConverter::toString<AnnotationElement>());
         break;
      case 8:
         pPlugIn = new ModelExporter(TypeConverter::toString<GcpList>());
         break;
      case 9:
         pPlugIn = new ModelExporter(TypeConverter::toString<TiePointList>());
         break;
      case 10:
         pPlugIn = new ModelImporter();
         break;
      case 11:
         pPlugIn = new CgmExporter();
         break;
      default:
         break;
   }

   return pPlugIn;
}
