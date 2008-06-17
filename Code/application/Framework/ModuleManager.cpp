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
 * This ModuleManager is provided so that the main app, Batch and BatchEditor
 * as well as any components soley belong to the Core application
 * and not plugins can have a ModuleManager class available.  This
 * was done so that the implementation and usage of Resource.h
 * in PlugInUtilities could be made simpler.
 */

#include "ModuleManager.h"

#include "AppVersion.h"
#include "CopyrightInformation.h"
#include "InMemoryPager.h"
#include "MemoryMappedPager.h"
#include "OptionsAnnotationLayer.h"
#include "OptionsAoiLayer.h"
#include "OptionsFileLocations.h"
#include "OptionsGcpLayer.h"
#include "OptionsGeneral.h"
#include "OptionsLatLonLayer.h"
#include "OptionsMeasurementLayer.h"
#include "OptionsOverviewWindow.h"
#include "OptionsPseudocolorLayer.h"
#include "OptionsRasterLayer.h"
#include "OptionsSession.h"
#include "OptionsShortcuts.h"
#include "OptionsSpatialDataView.h"
#include "OptionsStatusBar.h"
#include "OptionsThresholdLayer.h"
#include "OptionsTiePointLayer.h"
#include "OptionsView.h"
#include "OptionQWidgetWrapper.h"
#include "PropertiesAnnotationLayer.h"
#include "PropertiesAoiLayer.h"
#include "PropertiesDataDescriptor.h"
#include "PropertiesDataElement.h"
#include "PropertiesFileDescriptor.h"
#include "PropertiesGcpLayer.h"
#include "PropertiesGraphicObject.h"
#include "PropertiesHistogramPlot.h"
#include "PropertiesLatLonLayer.h"
#include "PropertiesMeasurementLayer.h"
#include "PropertiesMeasurementObject.h"
#include "PropertiesModuleDescriptor.h"
#include "PropertiesPlotView.h"
#include "PropertiesPlugInDescriptor.h"
#include "PropertiesProductView.h"
#include "PropertiesPseudocolorLayer.h"
#include "PropertiesQWidgetWrapper.h"
#include "PropertiesRasterLayer.h"
#include "PropertiesScriptingWindow.h"
#include "PropertiesSpatialDataView.h"
#include "PropertiesThresholdLayer.h"
#include "PropertiesTiePointLayer.h"
#include "PropertiesView.h"

#include <string>
using namespace std;

const char *ModuleManager::mspName = "Application";
const char *ModuleManager::mspVersion = APP_VERSION_NUMBER;
const char *ModuleManager::mspDescription = "Plug-ins built into the application";
const char *ModuleManager::mspValidationKey = "none";

unsigned int ModuleManager::getTotalPlugIns()
{
   return 42;
}

PlugIn* ModuleManager::getPlugIn(unsigned int plugInNumber)
{
   PlugIn* pPlugIn = NULL;

   switch (plugInNumber)
   {
      case 0:
         pPlugIn = new CopyrightInformation();
         break;
      case 1:
         pPlugIn = new InMemoryPager();
         break;
      case 2:
         pPlugIn = new MemoryMappedPager();
         break;
      case 3:
         pPlugIn = new OptionQWidgetWrapper<OptionsAnnotationLayer>();
         break;
      case 4:
         pPlugIn = new OptionQWidgetWrapper<OptionsAoiLayer>();
         break;
      case 5:
         pPlugIn = new OptionQWidgetWrapper<OptionsFileLocations>();
         break;
      case 6:
         pPlugIn = new OptionQWidgetWrapper<OptionsGcpLayer>();
         break;
      case 7:
         pPlugIn = new OptionQWidgetWrapper<OptionsGeneral>();
         break;
      case 8:
         pPlugIn = new OptionQWidgetWrapper<OptionsLatLonLayer>();
         break;
      case 9:
         pPlugIn = new OptionQWidgetWrapper<OptionsMeasurementLayer>();
         break;
      case 10:
         pPlugIn = new OptionQWidgetWrapper<OptionsOverviewWindow>();
         break;
      case 11:
         pPlugIn = new OptionQWidgetWrapper<OptionsPseudocolorLayer>();
         break;
      case 12:
         pPlugIn = new OptionQWidgetWrapper<OptionsRasterLayer>();
         break;
      case 13:
         pPlugIn = new OptionQWidgetWrapper<OptionsSession>();
         break;
      case 14:
         pPlugIn = new OptionQWidgetWrapper<OptionsShortcuts>();
         break;
      case 15:
         pPlugIn = new OptionQWidgetWrapper<OptionsSpatialDataView>();
         break;
      case 16:
         pPlugIn = new OptionQWidgetWrapper<OptionsStatusBar>();
         break;
      case 17:
         pPlugIn = new OptionQWidgetWrapper<OptionsThresholdLayer>();
         break;
      case 18:
         pPlugIn = new OptionQWidgetWrapper<OptionsTiePointLayer>();
         break;
      case 19:
         pPlugIn = new OptionQWidgetWrapper<OptionsView>();
         break;
      case 20:
         pPlugIn = new PropertiesDataDescriptor();
         break;
      case 21:
         pPlugIn = new PropertiesDataElement();
         break;
      case 22:
         pPlugIn = new PropertiesFileDescriptor();
         break;
      case 23:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesAnnotationLayer>();
         break;
      case 24:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesAoiLayer>();
         break;
      case 25:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesGcpLayer>();
         break;
      case 26:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesGraphicObject>();
         break;
      case 27:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesHistogramPlot>();
         break;
      case 28:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesLatLonLayer>();
         break;
      case 29:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesMeasurementLayer>();
         break;
      case 30:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesMeasurementObject>();
         break;
      case 31:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesModuleDescriptor>();
         break;
      case 32:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesPlotView>();
         break;
      case 33:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesPlugInDescriptor>();
         break;
      case 34:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesProductView>();
         break;
      case 35:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesPseudocolorLayer>();
         break;
      case 36:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesRasterLayer>();
         break;
      case 37:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesScriptingWindow>();
         break;
      case 38:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesSpatialDataView>();
         break;
      case 39:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesThresholdLayer>();
         break;
      case 40:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesTiePointLayer>();
         break;
      case 41:
         pPlugIn = new PropertiesQWidgetWrapper<PropertiesView>();
         break;
      default:
         break;
   }

   return pPlugIn;
}
