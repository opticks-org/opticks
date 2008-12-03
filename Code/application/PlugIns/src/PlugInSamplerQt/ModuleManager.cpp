/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnimationTest.h"
#include "AnimationTimingTest.h"
#include "AppConfig.h"
#include "CloseNotificationTest.h"
#include "DataPlotterPlugIn.h"
#include "Demo.h"
#include "DesktopAPITest.h"
#include "DesktopAPITestProperties.h"
#include "LogContextMenuActions.h"
#include "MenuAndToolBarTest.h"
#include "Modeless.h"
#include "ModuleManager.h"
#include "MouseModePlugIn.h"
#include "MouseModeTest.h"
#include "MultiLayerMovie.h"
#include "MultiMovie.h"
#include "OptionQWidgetWrapper.h"
#include "OptionsSample.h"
#include "PixelAspectRatio.h"
#include "PlugInTester.h"
#include "PlotManager.h"
#include "PropertiesQWidgetWrapper.h"
#include "RasterTimingTest.h"
#include "SampleGeoref.h"
#include "TiePointTester.h"

const char* ModuleManager::mspName = "Plug-In Sampler Qt";
const char* ModuleManager::mspVersion = "1.0";
const char* ModuleManager::mspDescription = "Sample plug-ins that test various components in the published interfaces.";
const char* ModuleManager::mspValidationKey = "none";
const char* ModuleManager::mspUniqueId = "{5A464B36-8FAB-4168-9234-B1C341CE2999}";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 21;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;
   switch (plugInNumber)
   {
      case 0:
         pPlugIn = new PlotManager();
         break;

      case 1:
         pPlugIn = new SampleGeoref();
         break;

      case 2:
         pPlugIn = new Demo();
         break;

      case 3:
         pPlugIn = new TiePointTester();
         break;

      case 4:
         pPlugIn = new ModelessPlugIn();
         break;

      case 5:
         pPlugIn = new PlugInTester();
         break;

      case 6:
         pPlugIn = new DataPlotterPlugIn();
         break;

      case 7:
         pPlugIn = new OptionQWidgetWrapper<OptionsSample>();
         break;

      case 8:
         pPlugIn = new CloseNotificationTest();
         break;

      case 9:
         pPlugIn = new DesktopAPITest();
         break;

      case 10:
         pPlugIn = new PropertiesQWidgetWrapper<DesktopAPITestProperties>();
         break;

      case 11:
         pPlugIn = new MenuAndToolBarTest();
         break;

      case 12:
         pPlugIn = new MouseModeTest();
         break;

      case 13:
         pPlugIn = new PixelAspectRatio();
         break;

      case 14:
         pPlugIn = new LogContextMenuActions();
         break;

      case 15:
         pPlugIn = new RasterTimingTest();
         break;

      case 16:
         pPlugIn = new MultiMovie();
         break;

      case 17:
         pPlugIn = new MultiLayerMovie();
         break;

      case 18:
         pPlugIn = new AnimationTestPlugIn();
         break;

      case 19:
         pPlugIn = new MouseModePlugIn();
         break;

      case 20:
         pPlugIn = new AnimationTimingTestPlugIn();
         break;

      default:
         break;
   }

   return pPlugIn;
}
