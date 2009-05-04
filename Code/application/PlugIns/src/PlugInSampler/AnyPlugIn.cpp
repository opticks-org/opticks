/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Any.h"
#include "AnyPlugIn.h"
#include "AppVerify.h"
#include "CustomElementData.h"
#include "DataVariant.h"
#include "DataVariantAnyData.h"
#include "DateTime.h"
#include "DesktopServices.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"

#include <string>
using namespace std;

REGISTER_PLUGIN_BASIC(OpticksPlugInSampler, AnyPlugIn);

AnyPlugIn::AnyPlugIn() :
   mInteractive(false)
{
   setName("Any Plug-In");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setDescription("Demonstrates creation and storage of the Any data element containing default and custom data.");
   setShortDescription("Demonstrates usage of the Any data element.");
   setMenuLocation("[Demo]/Any Plug-In");
   setDescriptorId("{455F29DB-F910-44e2-9FC1-A9477DC13358}");
   setProductionStatus(false);
}

AnyPlugIn::~AnyPlugIn()
{
}

bool AnyPlugIn::setBatch()
{
   mInteractive = false;
   return false;
}

bool AnyPlugIn::setInteractive()
{
   mInteractive = true;
   return true;
}

bool AnyPlugIn::getInputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pManager;

   pArgList = pManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   PlugInArg* pArg = pManager->getPlugInArg();
   VERIFY(pArg != NULL);

   // Register the CustomElement type with ModelServices here to ensure
   // that the custom arg type is registered before setting the arg type
   Service<ModelServices> pModel;
   pModel->addElementType("CustomElement");

   pArg->setName("Custom Element");
   pArg->setType("CustomElement");
   pArg->setDefaultValue(pModel->getElement("AnyPlugInCount", "CustomElement", NULL));
   pArgList->addArg(*pArg);

   return true;
}

bool AnyPlugIn::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool AnyPlugIn::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (mInteractive == false)
   {
      return false;
   }

   // Get the number of times the plug-in has previously been executed from an Any element stored in the model
   int executionCount = 0;
   Service<ModelServices> pModel;

   CustomElementData* pCountData = NULL;
   if (pInArgList != NULL)
   {
      pCountData = model_cast<CustomElementData*>
         (pInArgList->getPlugInArgValueUnsafe<Any>("Custom Element"));
   }

   if (pCountData == NULL)
   {
      pCountData = model_cast<CustomElementData*>(pModel->getElement("AnyPlugInCount", "CustomElement", NULL));
   }

   if (pCountData != NULL)
   {
      executionCount = pCountData->getValue();
   }

   // Get the date and time when the plug-in was previously executed from another Any element stored in the model
   DateTime* pPreviousDateTime = NULL;

   DataVariantAnyData* pDateTimeData = model_cast<DataVariantAnyData*>(pModel->getElement("AnyPlugInDateTime",
      "Any", NULL));
   if (pDateTimeData != NULL)
   {
      pPreviousDateTime = pDateTimeData->getAttribute().getPointerToValue<DateTime>();
   }

   // Create a string message indicating the number of times the plug-in
   // has been executed and the date and time of the previous execution
   string message;
   if (++executionCount == 1)
   {
      message = "This is the first time the plug-in has been executed.\n";
   }
   else
   {
      char count[16];
      sprintf(count, "%d", executionCount);
      message = "The plug-in has now been executed " + string(count) + " times.\n";
   }

   if (pPreviousDateTime != NULL)
   {
      string dateString;

      dateString = pPreviousDateTime->getFormattedLocal("%d %b %Y");
      message += "The plug-in was last executed on " + dateString + " at ";
      dateString = pPreviousDateTime->getFormattedLocal("%H:%M:%S");
      message += dateString + ".\n";
   }

   // Get the current date and time
   FactoryResource<DateTime> pDateTime;
   pDateTime->setToCurrentTime();

   // Add the current date and time to the string message
   string dateString;

   dateString = pDateTime->getFormattedLocal("%d %b %Y");
   message += "This plug-in instance was executed on " + dateString + " at ";
   dateString = pDateTime->getFormattedLocal("%H:%M:%S");
   message += dateString + ".";

   // Display the message to the user
   Service<DesktopServices> pDesktop;
   pDesktop->showMessageBox("Any Plug-In", message);

   // Create the date and time Any element and data object if necessary
   if (pDateTimeData == NULL)
   {
      Any* pDateTimeAny = static_cast<Any*>(pModel->createElement("AnyPlugInDateTime", "Any", NULL));
      VERIFY(pDateTimeAny != NULL);

      FactoryResource<DataVariantAnyData> pAnyData;
      pDateTimeData = pAnyData.release();

      pDateTimeAny->setData(pDateTimeData);
   }

   // Set the current execution date and time into the Any element's data object
   VERIFY(pDateTimeData != NULL);
   pDateTimeData->setAttribute(DataVariant(*pDateTime.get()));

   // Create the count Any element and data object if necessary
   if (pCountData == NULL)
   {
      Any* pCountAny = static_cast<Any*>(pModel->createElement("AnyPlugInCount", "CustomElement", NULL));
      VERIFY(pCountAny != NULL);

      pCountData = new CustomElementData();
      VERIFY(pCountData != NULL);

      pCountAny->setData(pCountData);
   }

   // Set the current execution count into the Any element's data object
   VERIFY(pCountData != NULL);
   pCountData->setValue(executionCount);

   return true;
}
