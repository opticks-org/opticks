/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QString>

#include "SelectPlugIn.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "Filename.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInSelectDlg.h"
#include "StringUtilities.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWizardItems, SelectPlugIn);

SelectPlugIn::SelectPlugIn()
{
   setName("Select Plug-In");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Displays a dialog to allow the user to choose a plug-in");
   setDescriptorId("{8C14F4E8-B289-4e1d-9C42-58A77EE2EC73}");
   allowMultipleInstances(true);
   executeOnStartup(false);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

SelectPlugIn::~SelectPlugIn()
{
}

bool SelectPlugIn::getInputSpecification(PlugInArgList*& pArgList)
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
      pArg->setName("Plug-In Type");
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Plug-In Subtype");
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Dialog Caption");
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool SelectPlugIn::getOutputSpecification(PlugInArgList*& pArgList)
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
      pArg->setName("Plug-In Name");
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool SelectPlugIn::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "2D301009-130B-4c05-983A-AAA9E01819D1");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (!extractInputArgs(pInArgList))
   {
      reportError("Unable to extract input arguments.", "6C0440A3-61D8-49e5-8255-7C8EF31A39D4");
      return false;
   }

   // Invoke the dialog
   PlugInSelectDlg dlg(mpDesktop->getMainWidget());
   dlg.setDisplayedPlugInType(mType, mSubtype);

   if (!mCaption.empty())
   {
      dlg.setWindowTitle(QString::fromStdString(mCaption));
   }

   if (dlg.exec() == QDialog::Rejected)
   {
      reportError("The Select Plug-In dialog was cancelled!", "B82E40EB-8309-41cd-887A-8DA75F7B1F05");
      return false;
   }

   // Set the output value
   if (pOutArgList != NULL)
   {
      PlugInArg* pArg = NULL;

      // Plug-in name
      if (pOutArgList->getArg("Plug-In Name", pArg) && (pArg != NULL))
      {
         string plugInName = dlg.getSelectedPlugInName();
         if (!plugInName.empty())
         {
            pArg->setActualValue(&plugInName);
         }
         else
         {
            reportError("Could not set the plug-in name output value!", "38359D0E-0AD3-4058-9FB7-59AD61EE22F5");
            return false;
         }
      }
      else
      {
         reportError("Could not set the plug-in name output value!", "2AA704E4-DA40-4202-8816-0ADAF13F5505");
         return false;
      }
   }

   reportComplete();
   return true;
}

bool SelectPlugIn::extractInputArgs(PlugInArgList* pInArgList)
{
   if (!DesktopItems::extractInputArgs(pInArgList))
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // Plug-in type
   mType = PlugInManagerServices::AlgorithmType();
   pInArgList->getPlugInArgValue<string>("Plug-In Type", mType);

   // Plug-in subtype
   mSubtype.clear();
   pInArgList->getPlugInArgValue<string>("Plug-In Subtype", mSubtype);

   // Dialog caption
   mCaption.clear();
   pInArgList->getPlugInArgValue<string>("Dialog Caption", mCaption);

   if (mpStep != NULL)
   {
      mpStep->addProperty("plugInType", mType);
      mpStep->addProperty("plugInSubtype", mSubtype);
      mpStep->addProperty("caption", mCaption);
   }

   return true;
}
