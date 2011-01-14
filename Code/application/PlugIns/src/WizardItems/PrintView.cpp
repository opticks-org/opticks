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
#include "LayerList.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PrintView.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

#include <vector>
using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWizardItems, PrintView);

PrintView::PrintView() :
   mpRasterElement(NULL),
   mbPrintDialog(false)
{
   setName("Print View");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Prints a viewer in the main application window in its current display state");
   setDescriptorId("{1FEF3B0F-0F44-411b-9060-68A88E673AEF}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

PrintView::~PrintView()
{
}

bool PrintView::getInputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;

   if (mbInteractive)
   {
      VERIFY(DesktopItems::getInputSpecification(pArgList) && pArgList != NULL);

      Service<PlugInManagerServices> pPlugInManager;
      VERIFY(pPlugInManager.get() != NULL);

      // Add args
      PlugInArg* pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Data set");
      pArg->setType("RasterElement");
      pArg->setDefaultValue(NULL);
      pArg->setDescription("Data set to be printed.");
      pArgList->addArg(*pArg);

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Setup Dialog");
      pArg->setType("bool");
      pArg->setDefaultValue(&mbPrintDialog);
      pArg->setDescription("Whether to show the setup dialog before printing.");
      pArgList->addArg(*pArg);
   }

   return true;
}

bool PrintView::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool PrintView::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "4EA89098-57C8-4b93-B04F-3197C59B0D58");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "9FC540AC-4BCF-4041-9E8E-484A494AF6AD");
      return false;
   }

   // Get the window
   SpatialDataWindow* pWindow = NULL;

   vector<Window*> windows;
   Service<DesktopServices> pDesktop;
   if (pDesktop.get() != NULL)
   {
      pDesktop->getWindows(SPATIAL_DATA_WINDOW, windows);
   }

   for (vector<Window*>::iterator iter = windows.begin(); iter != windows.end(); ++iter)
   {
      SpatialDataWindow* pCurrentWindow = static_cast<SpatialDataWindow*>(*iter);
      if (pCurrentWindow != NULL)
      {
         SpatialDataView* pView = pCurrentWindow->getSpatialDataView();
         if (pView != NULL)
         {
            LayerList* pLayerList = pView->getLayerList();
            if (pLayerList != NULL)
            {
               RasterElement* pRasterElement = pLayerList->getPrimaryRasterElement();
               if (pRasterElement != NULL && pRasterElement == mpRasterElement)
               {
                  pWindow = pCurrentWindow;
                  break;
               }
            }
         }
      }
   }

   if (pWindow == NULL)
   {
      reportError("Could not get the window for the data set!", "28355746-8AE3-44a4-9253-58684E1964C1");
      return false;
   }

   // Print the view
   pWindow->print(mbPrintDialog);

   reportComplete();
   return true;
}

bool PrintView::extractInputArgs(PlugInArgList* pInArgList)
{
   if (!DesktopItems::extractInputArgs(pInArgList))
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // Data set
   if (!pInArgList->getArg("Data set", pArg) || (pArg == NULL))
   {
      reportError("Could not read the data set input value!", "3606840C-EA86-4aa4-9E5A-F74604E5DA3E");
      return false;
   }

   mpRasterElement = pArg->getPlugInArgValue<RasterElement>();
   if (mpRasterElement == NULL)
   {
      reportError("The data set input value is invalid!", "1920C97E-A786-46ca-AA84-11DE889E3A1E");
      return false;
   }

   if (mpStep != NULL)
   {
      mpStep->addProperty("dataSet", mpRasterElement->getName()); 
   }

   // Setup dialog
   if (pInArgList->getArg("Setup Dialog", pArg) && (pArg != NULL))
   {
      bool* pBoolVal = pArg->getPlugInArgValue<bool>();
      if (pBoolVal != NULL)
      {
         mbPrintDialog = *pBoolVal;
      }
   }

   if (mpStep != NULL)
   {
      mpStep->addProperty("printDialog", mbPrintDialog);
   }

   return true;
}
