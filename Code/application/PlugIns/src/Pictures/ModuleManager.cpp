/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ModuleManager.h"
#include "BmpDetails.h"
#include "GeoTIFFExporter.h"
#include "GeoTIFFImporter.h"
#include "GeoTiffPager.h"
#include "JpegDetails.h"
#include "OptionQWidgetWrapper.h"
#include "OptionsJpegExporter.h"
#include "OptionsTiffExporter.h"
#include "PicturesPlotWidgetExporter.h"
#include "PicturesViewExporter.h"
#include "PngDetails.h"
#include "PostScriptExporter.h"
#include "TiffDetails.h"

const char* ModuleManager::mspName = "Picture PlugIns";
const char* ModuleManager::mspVersion = "1.0.0";
const char* ModuleManager::mspDescription = "Import/Export of Pictures to files.";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{F250CD0A-CCC1-4d49-918F-B07BBE0545BF}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 14;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;
   switch (plugInNumber)
   {
      // Non-PicturesExporter subclasses
      case 0:
         pPlugIn = new GeoTIFFImporter();
         break;

      case 1:
         pPlugIn = new GeoTiffPager();
         break;

      case 2:
         pPlugIn = new GeoTIFFExporter();
         break;

      case 3:
         pPlugIn = new PostScriptExporter();
         break;

      // PicturesExporter subclasses
      case 4:
         pPlugIn = new PicturesViewExporter(new BmpDetails);
         break;

      case 5:
         pPlugIn = new PicturesViewExporter(new JpegDetails);
         break;

      case 6:
         pPlugIn = new PicturesViewExporter(new PngDetails);
         break;

      case 7:
         pPlugIn = new PicturesViewExporter(new TiffDetails);
         break;

      case 8:
         pPlugIn = new PicturesPlotWidgetExporter(new BmpDetails);
         break;

      case 9:
         pPlugIn = new PicturesPlotWidgetExporter(new JpegDetails);
         break;

      case 10:
         pPlugIn = new PicturesPlotWidgetExporter(new PngDetails);
         break;

      case 11:
         pPlugIn = new PicturesPlotWidgetExporter(new TiffDetails);
         break;

      case 12:
         pPlugIn = new OptionQWidgetWrapper<OptionsJpegExporter>();
         break;

      case 13:
         pPlugIn = new OptionQWidgetWrapper<OptionsTiffExporter>();
         break;

      default:
         break;
   }

   return pPlugIn;
}
