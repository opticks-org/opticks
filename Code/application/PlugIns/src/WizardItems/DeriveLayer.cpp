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
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StringUtilities.h"

using namespace std;

///////////////
// DeriveAoi //
///////////////

DeriveAoi::DeriveAoi() :
   mpResults(NULL)
{
   setName("Derive AOI");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Derives an AOI layer from an existing threshold or pseudocolor layer");
   setDescriptorId("{BD3E7E6A-D070-4ab8-8D5B-1406BA6461D3}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

DeriveAoi::~DeriveAoi()
{
}

bool DeriveAoi::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if (mbInteractive)
   {
      VERIFY(DesktopItems::getInputSpecification(pArgList) && (pArgList != NULL));

      Service<PlugInManagerServices> pPlugInManager;
      VERIFY(pPlugInManager.get() != NULL);

      // Add args
      PlugInArg* pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Existing Layer Name");
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Existing Layer Type");
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Results Layer");
      pArg->setType("RasterElement");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool DeriveAoi::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if (mbInteractive)
   {
      Service<PlugInManagerServices> pPlugInManager;
      VERIFY(pPlugInManager.get() != NULL);

      // Set up list
      pArgList = pPlugInManager->getPlugInArgList();
      VERIFY(pArgList != NULL);

      // Add args
      PlugInArg* pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("AOI");
      pArg->setType("AoiElement");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool DeriveAoi::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "56D70072-0716-4506-B70D-9E6BD10C96A3");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "4DB42DD5-2198-4d1e-BBB5-9745D4242C9D");
      return false;
   }

   // Get the view
   if (mpResults == NULL)
   {
      reportError("The results layer is invalid!  An AOI cannot be created.", "25F7A0C0-EBCF-4813-BABE-1AA5D2A2EF0F");
      return false;
   }

   DataElement* pParent = mpResults->getParent();

   if (mpStep != NULL)
   {
      mpStep->addProperty("rasterElement", mpResults->getName());

      if (pParent != NULL)
      {
         mpStep->addProperty("rasterDataSet", pParent->getName());
      }
   }

   Service<DesktopServices> pDesktop;
   VERIFY(pDesktop.get() != NULL);

   vector<Window*> windows;
   pDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);

   SpatialDataView* pView = NULL;

   for (vector<Window*>::iterator iter = windows.begin(); iter != windows.end(); ++iter)
   {
      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(*iter);
      if (pWindow != NULL)
      {
         SpatialDataView* pCurrentView = pWindow->getSpatialDataView();
         if (pCurrentView != NULL)
         {
            LayerList* pLayerList = pCurrentView->getLayerList();
            if (pLayerList != NULL)
            {
               RasterElement* pRasterElement = pLayerList->getPrimaryRasterElement();
               if (pRasterElement != NULL)
               {
                  if ((pRasterElement == mpResults) || (pRasterElement == pParent))
                  {
                     pView = pCurrentView;
                     break;
                  }
               }
            }
         }
      }
   }

   if (pView == NULL)
   {
      reportError("Cannot get the view in which to derive the new layer!", "F23AB8FF-3080-4d2c-846D-64801F446B2E");
      return false;
   }

   // Get the existing layer
   Layer* pLayer = NULL;
   if (mLayerType.empty())
   {
      vector<Layer*> layers;

      LayerList* pLayerList = pView->getLayerList();
      if (pLayerList != NULL)
      {
         pLayerList->getLayers(layers);
      }

      for (unsigned int i = 0; i < layers.size(); i++)
      {
         Layer* pCurrentLayer = layers[i];
         if (pCurrentLayer != NULL)
         {
            DataElement* pElement = pCurrentLayer->getDataElement();
            if (pElement == mpResults)
            {
               pLayer = pCurrentLayer;
               break;
            }
         }
      }
   }
   else
   {
      if (mpStep != NULL)
      {
         mpStep->addProperty("layerName", mLayerName);
         mpStep->addProperty("layerType", mLayerType);
      }

      LayerType eType = StringUtilities::fromDisplayString<LayerType>(mLayerType);
      LayerList* pLayerList = pView->getLayerList();
      if (pLayerList != NULL)
      {
         pLayer = pLayerList->getLayer(eType, mpResults);
      }
   }

   if (pLayer == NULL)
   {
      reportError("Could not get the existing layer!", "E576A2D0-E3F9-49d8-8BD3-6DAD74449B17");
      return false;
   }

   Layer* pAoiLayer = pView->deriveLayer(pLayer, AOI_LAYER);
   if (pAoiLayer == NULL)
   {
      reportError("Could not derive the new AOI layer!", "2FD947ED-93AC-4a8c-A83C-A5B8C958331D");
      return false;
   }

   AoiElement* pAoi = static_cast<AoiElement*>(pAoiLayer->getDataElement());

   // Set the output value
   if (pOutArgList != NULL)
   {
      PlugInArg* pArg = NULL;

      if (pOutArgList->getArg("AOI", pArg) && (pArg != NULL))
      {
         pArg->setActualValue(pAoi);
      }
      else
      {
         reportError("Could not set the output value!", "3E2591B0-F41B-4de1-9D76-45245F9EF343");
         return false;
      }
   }

   reportComplete();
   return true;
}

bool DeriveAoi::extractInputArgs(PlugInArgList* pInArgList)
{
   if (!DesktopItems::extractInputArgs(pInArgList))
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // Layer name
   if (!pInArgList->getArg("Existing Layer Name", pArg) || (pArg == NULL))
   {
      reportError("Could not read the layer name input value!", "AE89DA90-9159-4595-9808-DADBAAD37E2E");
      return false;
   }
   string* pLayerName = pArg->getPlugInArgValue<string>();
   if (pLayerName != NULL)
   {
      mLayerName = *pLayerName;
   }

   // Layer type
   if (!pInArgList->getArg("Existing Layer Type", pArg) || (pArg == NULL))
   {
      reportError("Could not read the layer type input value!", "321AB1C6-3946-44d7-BC11-96B3F1FA1458");
      return false;
   }

   string* pLayerType = pArg->getPlugInArgValue<string>();
   if (pLayerType != NULL)
   {
      mLayerType = *pLayerType;
   }

   // Results matrix
   if (!pInArgList->getArg("Results Layer", pArg) || (pArg == NULL))
   {
      reportError("Could not read the results matrix input value!", "D9EE049B-4216-429d-884E-1EF3EE059032");
      return false;
   }
   mpResults = pArg->getPlugInArgValue<RasterElement>();

   return true;
}
