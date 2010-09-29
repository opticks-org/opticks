/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AppVerify.h"
#include "CopyrightInformation.h"
#include "CoreModuleDescriptor.h"
#include "InMemoryPager.h"
#include "MemoryMappedPager.h"
#include "OptionsAnimation.h"
#include "OptionsAnnotationLayer.h"
#include "OptionsAoiLayer.h"
#include "OptionsFileLocations.h"
#include "OptionsGcpLayer.h"
#include "OptionsGeneral.h"
#include "OptionsLatLonLayer.h"
#include "OptionsMeasurementLayer.h"
#include "OptionsOverviewWindow.h"
#include "OptionsPseudocolorLayer.h"
#include "OptionsRasterElementImporter.h"
#include "OptionsRasterLayer.h"
#include "OptionsSession.h"
#include "OptionsShortcuts.h"
#include "OptionsSpatialDataView.h"
#include "OptionsStatusBar.h"
#include "OptionsSuppressibleMsg.h"
#include "OptionsThresholdLayer.h"
#include "OptionsTiePointLayer.h"
#include "OptionsView.h"
#include "OptionQWidgetWrapper.h"
#include "PlugIn.h"
#include "PlugInDescriptorImp.h"
#include "PlugInRegistration.h"
#include "PropertiesAnnotationLayer.h"
#include "PropertiesAoiLayer.h"
#include "PropertiesDataDescriptor.h"
#include "PropertiesFileDescriptor.h"
#include "PropertiesGcpLayer.h"
#include "PropertiesGraphicObject.h"
#include "PropertiesHistogramPlot.h"
#include "PropertiesLatLonLayer.h"
#include "PropertiesMeasurementLayer.h"
#include "PropertiesMeasurementObject.h"
#include "PropertiesMetadata.h"
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
#include "PropertiesWavelengths.h"

#include <string>
#include <vector>
using namespace std;

CoreModuleDescriptor::CoreModuleDescriptor(const string& id, map<string, string>& plugInIds) :
   ModuleDescriptor(id)
{
   ModuleManager* pCore = ModuleManager::instance();
   setName("Application");
   mVersion = APP_VERSION_NUMBER;
   mDescription = "Plug-ins built into the application";
   mModuleVersion = MOD_THREE;
   mFileName = "This module is built into the application and does not have a separate file.";
   mFileDate.set(APP_RELEASE_DATE_YEAR, APP_RELEASE_DATE_MONTH, APP_RELEASE_DATE_DAY);
   initializePlugInInformation(plugInIds);
}

CoreModuleDescriptor::~CoreModuleDescriptor()
{
}

bool CoreModuleDescriptor::load()
{
   return true;
}

void CoreModuleDescriptor::unload()
{
   return;
}

GENERATE_FACTORY(OpticksCore);

REGISTER_PLUGIN_BASIC(OpticksCore, CopyrightInformation);
REGISTER_PLUGIN_BASIC(OpticksCore, InMemoryPager);
REGISTER_PLUGIN_BASIC(OpticksCore, MemoryMappedPager);
REGISTER_PLUGIN(OpticksCore, OptionsAnimation, OptionQWidgetWrapper<OptionsAnimation>());
REGISTER_PLUGIN(OpticksCore, OptionsAnnotationLayer, OptionQWidgetWrapper<OptionsAnnotationLayer>());
REGISTER_PLUGIN(OpticksCore, OptionsAoiLayer, OptionQWidgetWrapper<OptionsAoiLayer>());
REGISTER_PLUGIN(OpticksCore, OptionsFileLocations, OptionQWidgetWrapper<OptionsFileLocations>());
REGISTER_PLUGIN(OpticksCore, OptionsGcpLayer, OptionQWidgetWrapper<OptionsGcpLayer>());
REGISTER_PLUGIN(OpticksCore, OptionsGeneral, OptionQWidgetWrapper<OptionsGeneral>());
REGISTER_PLUGIN(OpticksCore, OptionsLatLonLayer, OptionQWidgetWrapper<OptionsLatLonLayer>());
REGISTER_PLUGIN(OpticksCore, OptionsMeasurementLayer, OptionQWidgetWrapper<OptionsMeasurementLayer>());
REGISTER_PLUGIN(OpticksCore, OptionsOverviewWindow, OptionQWidgetWrapper<OptionsOverviewWindow>());
REGISTER_PLUGIN(OpticksCore, OptionsPseudocolorLayer, OptionQWidgetWrapper<OptionsPseudocolorLayer>());
REGISTER_PLUGIN(OpticksCore, OptionsRasterElementImporter, OptionQWidgetWrapper<OptionsRasterElementImporter>());
REGISTER_PLUGIN(OpticksCore, OptionsRasterLayer, OptionQWidgetWrapper<OptionsRasterLayer>());
REGISTER_PLUGIN(OpticksCore, OptionsSession, OptionQWidgetWrapper<OptionsSession>());
REGISTER_PLUGIN(OpticksCore, OptionsShortcuts, OptionQWidgetWrapper<OptionsShortcuts>());
REGISTER_PLUGIN(OpticksCore, OptionsSpatialDataView, OptionQWidgetWrapper<OptionsSpatialDataView>());
REGISTER_PLUGIN(OpticksCore, OptionsStatusBar, OptionQWidgetWrapper<OptionsStatusBar>());
REGISTER_PLUGIN(OpticksCore, OptionsSuppressibleMsg, OptionQWidgetWrapper<OptionsSuppressibleMsg>());
REGISTER_PLUGIN(OpticksCore, OptionsThresholdLayer, OptionQWidgetWrapper<OptionsThresholdLayer>());
REGISTER_PLUGIN(OpticksCore, OptionsTiePointLayer, OptionQWidgetWrapper<OptionsTiePointLayer>());
REGISTER_PLUGIN(OpticksCore, OptionsView, OptionQWidgetWrapper<OptionsView>());
REGISTER_PLUGIN_BASIC(OpticksCore, PropertiesDataDescriptor);
REGISTER_PLUGIN_BASIC(OpticksCore, PropertiesFileDescriptor);
REGISTER_PLUGIN_BASIC(OpticksCore, PropertiesMetadata);
REGISTER_PLUGIN_BASIC(OpticksCore, PropertiesWavelengths);
REGISTER_PLUGIN(OpticksCore, PropertiesAnnotationLayer, PropertiesQWidgetWrapper<PropertiesAnnotationLayer>());
REGISTER_PLUGIN(OpticksCore, PropertiesAoiLayer, PropertiesQWidgetWrapper<PropertiesAoiLayer>());
REGISTER_PLUGIN(OpticksCore, PropertiesGcpLayer, PropertiesQWidgetWrapper<PropertiesGcpLayer>());
REGISTER_PLUGIN(OpticksCore, PropertiesGraphicObject, PropertiesQWidgetWrapper<PropertiesGraphicObject>());
REGISTER_PLUGIN(OpticksCore, PropertiesHistogramPlot, PropertiesQWidgetWrapper<PropertiesHistogramPlot>());
REGISTER_PLUGIN(OpticksCore, PropertiesLatLonLayer, PropertiesQWidgetWrapper<PropertiesLatLonLayer>());
REGISTER_PLUGIN(OpticksCore, PropertiesMeasurementLayer, PropertiesQWidgetWrapper<PropertiesMeasurementLayer>());
REGISTER_PLUGIN(OpticksCore, PropertiesMeasurementObject, PropertiesQWidgetWrapper<PropertiesMeasurementObject>());
REGISTER_PLUGIN(OpticksCore, PropertiesModuleDescriptor, PropertiesQWidgetWrapper<PropertiesModuleDescriptor>());
REGISTER_PLUGIN(OpticksCore, PropertiesPlotView, PropertiesQWidgetWrapper<PropertiesPlotView>());
REGISTER_PLUGIN(OpticksCore, PropertiesPlugInDescriptor, PropertiesQWidgetWrapper<PropertiesPlugInDescriptor>());
REGISTER_PLUGIN(OpticksCore, PropertiesProductView, PropertiesQWidgetWrapper<PropertiesProductView>());
REGISTER_PLUGIN(OpticksCore, PropertiesPseudocolorLayer, PropertiesQWidgetWrapper<PropertiesPseudocolorLayer>());
REGISTER_PLUGIN(OpticksCore, PropertiesRasterLayer, PropertiesQWidgetWrapper<PropertiesRasterLayer>());
REGISTER_PLUGIN(OpticksCore, PropertiesScriptingWindow, PropertiesQWidgetWrapper<PropertiesScriptingWindow>());
REGISTER_PLUGIN(OpticksCore, PropertiesSpatialDataView, PropertiesQWidgetWrapper<PropertiesSpatialDataView>());
REGISTER_PLUGIN(OpticksCore, PropertiesThresholdLayer, PropertiesQWidgetWrapper<PropertiesThresholdLayer>());
REGISTER_PLUGIN(OpticksCore, PropertiesTiePointLayer, PropertiesQWidgetWrapper<PropertiesTiePointLayer>());
REGISTER_PLUGIN(OpticksCore, PropertiesView, PropertiesQWidgetWrapper<PropertiesView>());

PlugIn* CoreModuleDescriptor::createInterface(unsigned int plugInNumber)
{
   return OpticksCore::getPlugIn(plugInNumber);
}

PlugIn* CoreModuleDescriptor::createInterface(PlugInDescriptorImp* pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   return createInterface(pDescriptor->getPlugInNumber());
}

const bool CoreModuleDescriptor::isValidatedModule() const
{
   return true;
}

bool CoreModuleDescriptor::isLoaded() const
{
   return true;
}
