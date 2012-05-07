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
#include "Filename.h"
#include "Layer.h"
#include "LayerList.h"
#include "LoadLayer.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

#include <string>
using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWizardItems, LoadAnnotation);
REGISTER_PLUGIN_BASIC(OpticksWizardItems, LoadAoi);
REGISTER_PLUGIN_BASIC(OpticksWizardItems, LoadGcpList);

///////////////
// LoadLayer //
///////////////

LoadLayer::LoadLayer() :
   mpFilename(NULL),
   mpView(NULL)
{}

LoadLayer::~LoadLayer()
{}

bool LoadLayer::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if (isBatch() == false)
   {
      VERIFY(DesktopItems::getInputSpecification(pArgList) && (pArgList != NULL));
      VERIFY(pArgList->addArg<Filename>("Filename", NULL, "Name of the file to be loaded."));
      VERIFY(pArgList->addArg<SpatialDataView>("View", NULL, "View for the new layer to be added to."));
   }

   return true;
}

bool LoadLayer::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if (isBatch() == false)
   {
      Service<PlugInManagerServices> pPlugInManager;

      // Set up list
      pArgList = pPlugInManager->getPlugInArgList();
      VERIFY(pArgList != NULL);

      LayerType eType = getLayerType();
      string modelType = getModelType(eType);

      // Add args
      VERIFY(pArgList->addArg<string>("Layer Name", NULL, "Name of the created layer."));

      if (!modelType.empty())
      {
         PlugInArg* pArg = pPlugInManager->getPlugInArg();
         VERIFY(pArg != NULL);
         pArg->setName("Layer Element");
         pArg->setType(modelType);
         pArg->setDefaultValue(NULL);
         pArg->setDescription("Element created by the load operation.");
         pArgList->addArg(*pArg);
      }
   }

   return true;
}

bool LoadLayer::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "41744FC3-755C-4014-88B5-36C4D62D3EAD");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "BF8BE1FB-9978-44d6-A849-CE822A224860");
      return false;
   }

   // Check for valid input values
   VERIFY(mpFilename != NULL);

   // Get the layer type
   LayerType eType = getLayerType();

   // Load the layer
   ImporterResource importer("Layer Importer", mpFilename->getFullPathAndName(), getProgress(), isBatch());
   if (mpView != NULL)
   {
      importer->getInArgList().setPlugInArgValue("View", mpView);
   }
   importer->execute();
   Layer* pLayer = importer->getOutArgList().getPlugInArgValue<Layer>("Layer");
   if (pLayer == NULL)
   {
      reportError("Could not load the layer!", "2399B97B-2D00-46ae-8B0E-F83DD91F938D");
      return false;
   }

   // Set the output values
   if (pOutArgList != NULL)
   {
      PlugInArg* pArg = NULL;

      // Layer name
      string layerName = pLayer->getName();

      if (pOutArgList->getArg("Layer Name", pArg) && (pArg != NULL) && !layerName.empty())
      {
         pArg->setActualValue(&layerName);
      }
      else
      {
         reportError("Could not set the layer name output value!", "E7E5FC29-E1E2-446f-955D-51FB30E00BF8");
         return false;
      }

      // Data Element
      if (pOutArgList->getArg("Layer Element", pArg) && (pArg != NULL))
      {
         DataElement* pElement = pLayer->getDataElement();
         if (pElement != NULL)
         {
            pArg->setActualValue(pElement);
         }
         else
         {
            reportError("Could not set the data element output value!", "D06D59A7-29A1-4ca3-878C-1A96DA3901C3");
            return false;
         }
      }
      else
      {
         reportError("Could not set the data element output value!", "D5C33CD4-508D-4366-A24A-4A37D9531E54");
         return false;
      }
   }

   reportComplete();
   return true;
}

bool LoadLayer::extractInputArgs(PlugInArgList* pInArgList)
{
   if (!DesktopItems::extractInputArgs(pInArgList))
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // Filename
   if (!pInArgList->getArg("Filename", pArg) || (pArg == NULL))
   {
      reportError("Could not read the filename input value!", "AACDEFFD-1D64-4e56-8D0B-F748507EBCF8");
      return false;
   }

   mpFilename = pArg->getPlugInArgValue<Filename>();
   if (mpFilename == NULL)
   {
      reportError("The filename input value is NULL!", "AE49DE8D-6AC7-4621-8730-B7BE1E119B6F");
      return false;
   }

   if (mpStep != NULL)
   {
      mpStep->addProperty("filename", mpFilename->getFullPathAndName());
   }

   // Data set
   if (!pInArgList->getArg("View", pArg) || (pArg == NULL))
   {
      reportError("Could not read the view input value!", "5E96A945-49FD-4ca5-8758-AD91063FD53A");
      return false;
   }

   mpView = pArg->getPlugInArgValue<SpatialDataView>();
   if ((mpStep != NULL) && (mpView != NULL))
   {
      mpStep->addProperty("view", mpView->getName());
   }

   return true;
}

/////////////
// LoadAoi //
/////////////

LoadAoi::LoadAoi()
{
   setName("Load AOI");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Loads an AOI from a file and adds a layer to the data set view window");
   setDescriptorId("{42D7196E-C1F9-4120-83F2-78EDE5954F1C}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

LoadAoi::~LoadAoi()
{
}

LayerType LoadAoi::getLayerType() const
{
   return AOI_LAYER;
}

////////////////////
// LoadAnnotation //
////////////////////

LoadAnnotation::LoadAnnotation()
{
   setName("Load Annotation");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Loads an annotation layer from a file and adds it to the data set view window");
   setDescriptorId("{8A35549B-1C4F-4b88-9429-6DD1FD8EB0AF}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

LoadAnnotation::~LoadAnnotation()
{
}

LayerType LoadAnnotation::getLayerType() const
{
   return ANNOTATION;
}

/////////////////
// LoadGcpList //
/////////////////

LoadGcpList::LoadGcpList()
{
   setName("Load GCP List");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Loads an GCP list from a file and adds a layer to the data set view window");
   setDescriptorId("{32548CE1-5C13-40fb-9FF8-2F2B48D22345}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

LoadGcpList::~LoadGcpList()
{
}

LayerType LoadGcpList::getLayerType() const
{
   return GCP_LAYER;
}
