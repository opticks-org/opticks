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
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "ProductView.h"
#include "ProductWindow.h"
#include "TemplateUtilities.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWizardItems, LoadTemplate);
REGISTER_PLUGIN_BASIC(OpticksWizardItems, SaveTemplate);

///////////////////////
// TemplateUtilities //
///////////////////////

TemplateUtilities::TemplateUtilities() :
   mpFilename(NULL)
{
}

TemplateUtilities::~TemplateUtilities()
{
}

bool TemplateUtilities::getInputSpecification(PlugInArgList*& pArgList)
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
      pArg->setName("Product Name");
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArg->setDescription("String name for the product.");
      pArgList->addArg(*pArg);

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Template File");
      pArg->setType("Filename");
      pArg->setDefaultValue(NULL);
      pArg->setDescription("On-disk filename for the product template.");
      pArgList->addArg(*pArg);
   }

   return true;
}

bool TemplateUtilities::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool TemplateUtilities::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "6276E1E1-526F-4c63-8195-69298E55FBD1");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "DD2764C8-17B2-4dd6-A67D-6B2B5C41A281");
      return false;
   }

   string plugInName = getName();

   // Check for valid input values
   if (mProductName.empty() == true)
   {
      reportError("The product name input value is invalid!", "238D52A1-5BD6-4b66-93D1-A5C736A32ECE");
      return false;
   }

   string filename;
   if (mpFilename != NULL)
   {
      filename = mpFilename->getFullPathAndName();
   }

   if (filename.empty() == true)
   {
      reportError("The template file input value is invalid!", "434A0119-3D35-49cb-BBC6-D25059825FF7");
      return false;
   }

   // Get the view
   ProductView* pView = NULL;

   Service<DesktopServices> pDesktop;
   VERIFY(pDesktop.get() != NULL);

   vector<Window*> windows;
   mpDesktop->getWindows(PRODUCT_WINDOW, windows);

   for (vector<Window*>::iterator iter = windows.begin(); iter != windows.end(); ++iter)
   {
      ProductWindow* pWindow = static_cast<ProductWindow*>(*iter);
      if (pWindow != NULL)
      {
         ProductView* pCurrentView = pWindow->getProductView();
         if (pCurrentView != NULL)
         {
            string viewName = pCurrentView->getName();
            if (viewName == mProductName)
            {
               pView = pCurrentView;
               break;
            }
         }
      }
   }

   if (pView == NULL)
   {
      reportError("Could not get the view!", "F44F65D0-DBCD-400a-8862-F560E889335D");
      return false;
   }

   // Perform the action
   if (!executeUtility(pView, filename))
   {
      return false;
   }

   reportComplete();
   return true;
}

bool TemplateUtilities::extractInputArgs(PlugInArgList* pInArgList)
{
   if (!DesktopItems::extractInputArgs(pInArgList))
   {
      return false;
   }

   string plugInName = getName();
   PlugInArg* pArg = NULL;

   // Product name
   if ((pInArgList->getArg("Product Name", pArg) == false) || (pArg == NULL))
   {
      reportError("Could not read the product name input value!", "24ECC2C2-8683-4ce2-BAB7-B173FB6D24EA");
      return false;
   }

   mProductName.erase();

   string* pName = pArg->getPlugInArgValue<string>();
   if (pName != NULL)
   {
      mProductName = *pName;
   }

   // Template file
   if ((pInArgList->getArg("Template File", pArg) == false) || (pArg == NULL))
   {
      reportError("Could not read the template file input value!", "74EBD7DB-8379-4454-B88F-046947501D03");
      return false;
   }

   mpFilename = pArg->getPlugInArgValue<Filename>();
   return true;
}

//////////////////
// LoadTemplate //
//////////////////

LoadTemplate::LoadTemplate()
{
   setName("Load Template");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Loads a product template from a file");
   setDescriptorId("{FF5745D6-F08B-4a9e-A862-281DA67EBCCA}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

LoadTemplate::~LoadTemplate()
{
}

bool LoadTemplate::executeUtility(ProductView* pView, const string& templateFile)
{
   VERIFY((pView != NULL) && !templateFile.empty());

   reportProgress("Loading the template from the file...", 0, "74EC20B4-6B98-4314-B9F0-FE0C4302E619");

   if (!pView->loadTemplate(templateFile))
   {
      reportError("Could not load the template from the '" + templateFile + "' file!",
         "88CCA25F-F675-478e-BFB4-2DED36100809");
      return false;
   }

   return true;
}

//////////////////
// SaveTemplate //
//////////////////

SaveTemplate::SaveTemplate()
{
   setName("Save Template");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Saves a product template to a file");
   setDescriptorId("{0AAAA873-71D4-40be-A5A9-6233587D62BB}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

SaveTemplate::~SaveTemplate()
{
}

bool SaveTemplate::executeUtility(ProductView* pView, const string& templateFile)
{
   VERIFY((pView != NULL) && !templateFile.empty());

   reportProgress("Saving the template to the file...", 0, "E795F35F-82DD-476b-B8D5-D44C8D1630DF");

   if (!pView->saveTemplate(templateFile))
   {
      reportError("Could not save the template to the '" + templateFile + "' file!",
         "08AD8B83-6139-467e-9F99-6411388AA941");
      return false;
   }

   return true;
}
