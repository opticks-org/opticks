/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ApplicationServices.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "Filename.h"
#include "GetView.h"
#include "LayerList.h"
#include "MessageLogResource.h"
#include "ObjectFactory.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

#include <string>
using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWizardItems, GetDataSet);

////////////////
// GetDataSet //
////////////////

GetDataSet::GetDataSet()
{
   setName("Get Data Set");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Gets the sensor data object from the currently active data set in the view");
   setDescriptorId("{5A1854C8-BFE3-4edf-A7F2-69EF18CFB569}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GetDataSet::~GetDataSet()
{}

bool GetDataSet::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if (isBatch() == false)
   {
      VERIFY(DesktopItems::getInputSpecification(pArgList) && (pArgList != NULL));
      if (!pArgList->addArg<string>("Name"))
      {
         return false;
      }
   }

   return true;
}

bool GetDataSet::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if (isBatch() == false)
   {
      Service<PlugInManagerServices> pPlugInManager;
      VERIFY(pPlugInManager.get() != NULL);

      // Set up list
      pArgList = pPlugInManager->getPlugInArgList();
      VERIFY(pArgList != NULL);

      // Add args
      VERIFY(pArgList->addArg<SpatialDataView>("View", NULL));
      VERIFY(pArgList->addArg<RasterElement>("Data Set", NULL));
      VERIFY(pArgList->addArg<RasterLayer>("Layer", NULL));
      VERIFY(pArgList->addArg<Filename>("Filename", NULL));
   }

   return true;
}

bool GetDataSet::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "92CA6529-2662-4aed-B406-FBD3F5E0B40E");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "7ED6F0D1-09B0-40b7-BDE3-6294387218B0");
      return false;
   }

   // Did the user specify a spatial data view?
   Service<DesktopServices> pDesktop;
   WorkspaceWindow* pWindow = NULL;
   string windowName;
   if (pInArgList->getPlugInArgValue("Name", windowName) && !windowName.empty())
   {
      pWindow = static_cast<WorkspaceWindow*>(pDesktop->getWindow(windowName, SPATIAL_DATA_WINDOW));
   }
   else
   {
      // Get the current spatial data view
      pWindow = mpDesktop->getCurrentWorkspaceWindow();
   }

   SpatialDataView* pView = NULL;
   if (pWindow != NULL)
   {
      pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
   }

   if (pView == NULL)
   {
      reportError("Could not get the data set view!", "88010049-E294-404e-83EC-B558EBD22F3D");
      return false;
   }

   // Get the raster element
   RasterElement* pRasterElement = NULL;
   RasterLayer* pRasterLayer = NULL;

   LayerList* pLayerList = pView->getLayerList();
   if (pLayerList != NULL)
   {
      pRasterElement = pLayerList->getPrimaryRasterElement();
      pRasterLayer = static_cast<RasterLayer*>(pLayerList->getLayer(RASTER, pRasterElement));
   }

   if (pRasterElement == NULL)
   {
      string viewName = pView->getName();
      reportError("Could not get the data set from the current view - " + viewName,
                           "4309102B-AF9B-46b2-A686-EBE9DF089FD4");
      return false;
   }

   // Set the output values
   if (pOutArgList != NULL)
   {
      PlugInArg* pArg = NULL;

      // View
      if (pOutArgList->getArg("View", pArg) && (pArg != NULL))
      {
         pArg->setActualValue(pView);
      }
      else
      {
         reportError("Could not set the data set output value!", "E6F513E6-8844-4936-B7FA-DBB67AF388ED");
         return false;
      }

      // Data set
      if (pOutArgList->getArg("Data Set", pArg) && (pArg != NULL))
      {
         pArg->setActualValue(pRasterElement);
      }
      else
      {
         reportError("Could not set the data set output value!", "1397E975-C843-469a-9D62-AE704A973079");
         return false;
      }

      // Layer
      if (pOutArgList->getArg("Layer", pArg) && (pArg != NULL))
      {
         pArg->setActualValue(pRasterLayer);
      }
      else
      {
         reportError("Could not set the layer output value!", "{ae0aabcf-996b-4312-80de-bff862c47ae9}");
         return false;
      }

      // Filename
      if (pOutArgList->getArg("Filename", pArg) && (pArg != NULL))
      {
         string filename = pRasterElement->getFilename();

         Filename* pFilename = NULL;
         Service<ApplicationServices> pApplication;
         VERIFY(pApplication.get() != NULL);
         ObjectFactory* pObjFact = pApplication->getObjectFactory();
         if (pObjFact != NULL)
         {
            pFilename = reinterpret_cast<Filename*>(pObjFact->createObject("Filename"));
         }

         if (pFilename != NULL)
         {
            pFilename->setFullPathAndName(filename);
            pArg->setActualValue(pFilename);
         }
         else
         {
            reportError("Could not set the filename output value!", "48C1C995-79A1-4e51-A9EC-82B2CBE9B825");
            return false;
         }
      }
      else
      {
         reportError("Could not set the filename output value!", "0C0C952B-94BE-4b63-8B7A-D4237FAF19F8");
         return false;
      }
   }

   reportComplete();
   return true;
}
