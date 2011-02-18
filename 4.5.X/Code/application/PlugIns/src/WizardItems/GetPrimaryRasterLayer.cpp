/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "GetPrimaryRasterLayer.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "SpatialDataView.h"

#include <string>

REGISTER_PLUGIN_BASIC(OpticksWizardItems, GetPrimaryRasterLayer);

GetPrimaryRasterLayer::GetPrimaryRasterLayer() :
   mpSpatialDataView(NULL)
{
   setName("Get Primary Raster Layer");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Gets the Primary Raster Layer for the specified Spatial Data View");
   setDescriptorId("{023A593B-DD28-4ba8-975F-E6DED903A728}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GetPrimaryRasterLayer::~GetPrimaryRasterLayer()
{
}

bool GetPrimaryRasterLayer::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(DesktopItems::getInputSpecification(pArgList) == true);
   if (pArgList == NULL)
   {
      pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
      VERIFY(pArgList != NULL);
   }

   VERIFY(pArgList->addArg<SpatialDataView>(Executable::ViewArg()) == true);
   return true;
}

bool GetPrimaryRasterLayer::getOutputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(DesktopItems::getOutputSpecification(pArgList) == true);
   if (pArgList == NULL)
   {
      pArgList = Service<PlugInManagerServices>()->getPlugInArgList();
      VERIFY(pArgList != NULL);
   }

   VERIFY(pArgList->addArg<RasterLayer>(Executable::LayerArg()) == true);
   return true;
}

bool GetPrimaryRasterLayer::extractInputArgs(PlugInArgList* pInArgList)
{
   if (pInArgList == NULL || DesktopItems::extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   mpSpatialDataView = pInArgList->getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
   if (mpSpatialDataView == NULL)
   {
      reportError("Invalid " + Executable::ViewArg() + " specified.", "46BF9CF0-5428-4d09-9776-C97BF24EEF63");
      return false;
   }

   return true;
}

bool GetPrimaryRasterLayer::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "5D20DF72-1C71-4a5d-9B1A-398A91154EBD");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (pInArgList == NULL || pOutArgList == NULL)
   {
      reportError("Invalid Plug-In Arg List specified.", "8161F92F-124F-4118-8E24-532187A442AA");
      return false;
   }

   if (extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   LayerList* pLayerList = mpSpatialDataView->getLayerList();
   VERIFY(pLayerList != NULL);

   RasterElement* pElement = pLayerList->getPrimaryRasterElement();
   RasterLayer* pLayer = dynamic_cast<RasterLayer*>(pLayerList->getLayer(RASTER, pElement));
   if (pOutArgList->setPlugInArgValue(Executable::LayerArg(), pLayer) == false)
   {
      reportError("Unable to set output argument.", "B4DE1827-3B96-4a96-89BB-62431B6E81CF");
      return false;
   }

   reportComplete();
   return true;
}
