/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVersion.h"
#include "DesktopServices.h"
#include "AppVerify.h"
#include "DeriveProduct.h"
#include "Filename.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "ProductView.h"
#include "ProductWindow.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "View.h"

REGISTER_PLUGIN_BASIC(OpticksWizardItems, DeriveProduct);

DeriveProduct::DeriveProduct()
{
   setName("Derive Product");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Derive a product from an existing view.");
   setDescriptorId("{E89BE407-5255-4faa-BE57-162E1E9A24C6}");
   allowMultipleInstances(true);
}

DeriveProduct::~DeriveProduct()
{
}

bool DeriveProduct::getInputSpecification(PlugInArgList*& pArgList)
{
   VERIFY(DesktopItems::getInputSpecification(pArgList) && (pArgList != NULL));
   bool success = pArgList->addArg<SpatialDataView>("View");
   success = success && pArgList->addArg<Filename>("Template", NULL);
   return success;
}

bool DeriveProduct::getOutputSpecification(PlugInArgList*& pArgList)
{
   VERIFY((pArgList = Service<PlugInManagerServices>()->getPlugInArgList()) != NULL);
   bool success = pArgList->addArg<ProductWindow>("Window");
   success = success && pArgList->addArg<ProductView>("View");
   return success;
}

bool DeriveProduct::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "58FE5CAB-E941-4E60-BA55-B29D70715FC4");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "5E158F48-6089-4A88-ABD0-55C717BD13E2");
      return false;
   }
   View* pView = pInArgList->getPlugInArgValue<View>("View");
   if (pView == NULL)
   {
      std::vector<Window*> windows;
      Service<DesktopServices>()->getWindows(SPATIAL_DATA_WINDOW, windows);
      if (!windows.empty())
      {
         pView = static_cast<SpatialDataWindow*>(windows.front())->getSpatialDataView();
      }
   }
   if (pView == NULL)
   {
      reportError("No view provided.", "852F585B-D239-4C0A-B993-70EE68EC8DEE");
      return false;
   }

   ProductWindow* pProductWindow = Service<DesktopServices>()->deriveProduct(pView);
   if (pProductWindow == NULL)
   {
      reportError("Unable to derive product", "E24BB5A5-A675-4897-9C48-A4E4109379DF");
      return false;
   }

   // Load a template if one is specified
   Filename* pTemplate = pInArgList->getPlugInArgValue<Filename>("Template");
   if (pTemplate != NULL)
   {
      if (!pProductWindow->getProductView()->loadTemplate(pTemplate->getFullPathAndName()))
      {
         reportError("Could not load the requested template!", "C99CE97E-3F0F-4CBB-8460-28D96D55596B");
         return false;
      }
   }

   // Set the output values
   if (!pOutArgList->setPlugInArgValue("Window", pProductWindow) ||
      !pOutArgList->setPlugInArgValue("View", pProductWindow->getProductView()))
   {
      reportError("Could not set the data set output value!", "3C53EDAE-DC70-4141-9759-ECD3EB9BE186");
      return false;
   }

   reportComplete();
   pStep->finalize(Message::Success);
   return true;
}
