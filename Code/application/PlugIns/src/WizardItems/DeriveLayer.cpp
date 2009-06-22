/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "AoiElement.h"
#include "AppVerify.h"
#include "DeriveLayer.h"
#include "Layer.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWizardItems, DeriveLayer);

DeriveLayer::DeriveLayer() :
   mpInputLayer(NULL)
{
   setName("Derive Layer");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Converts a layer to a different layer type and creates a new copy of an existing layer.");
   setDescriptorId("{BD3E7E6A-D070-4ab8-8D5B-1406BA6461D3}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

DeriveLayer::~DeriveLayer()
{
}

bool DeriveLayer::setBatch()
{
   DesktopItems::setBatch();
   return true;
}

bool DeriveLayer::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(DesktopItems::getInputSpecification(pArgList) && (pArgList != NULL));
   VERIFY(pArgList->addArg<Layer>(LayerArg(), "The layer which will be converted or copied."));
   VERIFY(pArgList->addArg<LayerType>("New Layer Type", "The new type for the layer."));
   VERIFY(pArgList->addArg<std::string>("New Layer Name", std::string(),
      "The name of the new layer. If this is an empty string, the existing layer will be replaced."));

   return true;
}

bool DeriveLayer::getOutputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(pArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   VERIFY(pArgList->addArg<Layer>("Layer", "The new layer."));
   VERIFY(pArgList->addArg<DataElement>("Element", "The data element for the new layer."));

   return true;
}

bool DeriveLayer::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   VERIFY(pInArgList != NULL);
   StepResource pStep("Execute Wizard Item", "app", "56D70072-0716-4506-B70D-9E6BD10C96A3");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "4DB42DD5-2198-4d1e-BBB5-9745D4242C9D");
      return false;
   }

   // Get the view
   SpatialDataView* pView = dynamic_cast<SpatialDataView*>(mpInputLayer->getView());
   if (pView == NULL)
   {
      reportError("Only layers in a spatial data view can be converted.", "{039ac767-e6d5-441b-9b27-bf59cbb9ee9d}");
      return false;
   }

   Layer* pNewLayer = NULL;
   if (mNewLayerName.empty())
   {
      pNewLayer = pView->convertLayer(mpInputLayer, mNewLayerType);
   }
   else
   {
      pNewLayer = pView->deriveLayer(mpInputLayer, mNewLayerType);
      if (pNewLayer != NULL)
      {
         if (!pView->getLayerList()->renameLayer(pNewLayer, mNewLayerName))
         {
            pView->deleteLayer(pNewLayer);
            reportError("Unable to derive a layer with the given name, because another layer "
               "with the same name already exists.", "{1e81e201-624f-4e96-83b5-8ceae01d7c1d}");
            return false;
         }
      }
   }
   if (pNewLayer == NULL)
   {
      reportError("Unable to convert the layer.", "{f09810c2-f2a2-4887-a0f0-6848d74ffd28}");
      return false;
   }

   // Set the output value
   if (pOutArgList != NULL)
   {
      if (!pOutArgList->setPlugInArgValue("Layer", pNewLayer) ||
          !pOutArgList->setPlugInArgValue("Element", pNewLayer->getDataElement()))
      {
         reportError("Could not set the output value!", "3E2591B0-F41B-4de1-9D76-45245F9EF343");
         return false;
      }
   }

   reportComplete();
   return true;
}

bool DeriveLayer::extractInputArgs(PlugInArgList* pInArgList)
{
   if (!DesktopItems::extractInputArgs(pInArgList))
   {
      return false;
   }
   if ((mpInputLayer = pInArgList->getPlugInArgValue<Layer>(LayerArg())) == NULL)
   {
      reportError("Invalid input layer.", "{9089066f-08ea-492e-a555-43dd2f53ecbc}");
      return false;
   }
   if (!pInArgList->getPlugInArgValue("New Layer Type", mNewLayerType) || !mNewLayerType.isValid())
   {
      reportError("Invalid layer type.", "{36b999de-8c09-4760-b98a-9700d8828605}");
      return false;
   }
   if (!pInArgList->getPlugInArgValue("New Layer Name", mNewLayerName))
   {
      reportError("Invalid layer name.", "{63cd1f11-ec28-4547-91cd-118a51b73b17}");
      return false;
   }

   return true;
}
