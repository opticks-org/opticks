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
#include "DataDescriptor.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "GetDataDescriptor.h"
#include "ImportDescriptor.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWizardItems, GetDataDescriptor);

GetDataDescriptor::GetDataDescriptor() :
   mpFilename(NULL)
{
   setName("Get Data Descriptor");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Queries an importer for a data set descriptor");
   setDescriptorId("{A4020F37-2D1B-4005-8D75-2A877C723BC9}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

GetDataDescriptor::~GetDataDescriptor()
{
}

bool GetDataDescriptor::getInputSpecification(PlugInArgList*& pArgList)
{
   bool bSuccess = ModelItems::getInputSpecification(pArgList);
   if (bSuccess == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);

   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   // Add args
   PlugInArg* pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Filename");                // Filename
   pArg->setType("Filename");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Importer Name");           // Importer name
   pArg->setType("string");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Data Set Location");       // Data set location
   pArg->setType("string");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   return true;
}

bool GetDataDescriptor::getOutputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   // Set up list
   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   // Add args
   PlugInArg* pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Data Descriptor");         // Data descriptor
   pArg->setType("DataDescriptor");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   return true;
}

bool GetDataDescriptor::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "E077A09D-D932-46A7-BA1F-C5CC0C101105");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   // Get the data descriptor from the importer
   string filename = mpFilename->getFullPathAndName();
   Progress* pProgress = getProgress();

   if (mImporterName.empty() == true)
   {
      mImporterName = "Auto Importer";
   }

   ImporterResource importer(mImporterName, filename, pProgress);
   vector<ImportDescriptor*> descriptors = importer->getImportDescriptors();

   DataDescriptor* pDescriptor = NULL;
   if (descriptors.empty() == false)
   {
      if (mDatasetLocation.empty() == false)
      {
         vector<ImportDescriptor*>::iterator iter;
         for (iter = descriptors.begin(); iter != descriptors.end(); ++iter)
         {
            ImportDescriptor* pImportDescriptor = *iter;
            if (pImportDescriptor != NULL)
            {
               DataDescriptor* pCurrentDescriptor = pImportDescriptor->getDataDescriptor();
               if (pCurrentDescriptor != NULL)
               {
                  const FileDescriptor* pFileDescriptor = pCurrentDescriptor->getFileDescriptor();
                  if (pFileDescriptor != NULL)
                  {
                     const string& datasetLocation = pFileDescriptor->getDatasetLocation();
                     if (datasetLocation == mDatasetLocation)
                     {
                        pDescriptor = pCurrentDescriptor;
                        break;
                     }
                  }
               }
            }
         }

         if (pDescriptor == NULL)
         {
            reportError("The " + mImporterName + " did not find a valid data set at the '" +
               mDatasetLocation + "' location!", "6BD14A0F-D003-4D8B-AC5F-BBF1483BA228");
            return false;
         }
      }
      else
      {
         ImportDescriptor* pImportDescriptor = descriptors.front();
         if (pImportDescriptor != NULL)
         {
            pDescriptor = pImportDescriptor->getDataDescriptor();
         }

         if (pDescriptor == NULL)
         {
            reportError("The data set in the file is invalid!", "C477287B-2A40-4CD7-8222-F00990DD75C3");
            return false;
         }
      }
   }
   else
   {
      reportError("The " + mImporterName + " did not find any valid data sets in the file!",
         "562285A0-32E1-4500-BBD9-6478B474B1FC");
      return false;
   }

   //make copy of found descriptor, because ImporterResource owns all of the DataDescriptors
   VERIFY(pDescriptor != NULL);
   DataDescriptor* pSelectedDescriptor = pDescriptor->copy();
   if (pSelectedDescriptor == NULL)
   {
      reportError("Could not copy the Data Descriptor.", "DF068245-4FD3-4c6b-A991-1A2397AC30E8");
      return false;
   }

   // Set the output value
   if (pOutArgList != NULL)
   {
      PlugInArg* pArg = NULL;

      // Data element
      if ((pOutArgList->getArg("Data Descriptor", pArg) == true) && (pArg != NULL))
      {
         pArg->setActualValue(pSelectedDescriptor);
      }
      else
      {
         reportError("Could not set the output arg value!", "CEA5EFDD-3C75-457D-875F-77F8096C604B");
         return false;
      }
   }

   reportComplete();
   return true;
}

bool GetDataDescriptor::extractInputArgs(PlugInArgList* pInArgList)
{
   bool bSuccess = ModelItems::extractInputArgs(pInArgList);
   if (bSuccess == false)
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // Filename
   if ((pInArgList->getArg("Filename", pArg) == false) || (pArg == NULL))
   {
      reportError("Could not read the filename input value!", "A759E061-E85B-4DAC-A3AA-CA58ACF2A903");
      return false;
   }

   mpFilename = pArg->getPlugInArgValue<Filename>();
   if (mpFilename == NULL)
   {
      reportError("The filename input value is invalid!", "F89A1350-80D7-4E7B-B5AB-1289A67D699B");
      return false;
   }

   // Importer name
   mImporterName.erase();

   if ((pInArgList->getArg("Importer Name", pArg) == true) && (pArg != NULL))
   {
      string* pImporterName = pArg->getPlugInArgValue<string>();
      if (pImporterName != NULL)
      {
         mImporterName = *pImporterName;
      }
   }

   // Data set location
   mDatasetLocation.erase();

   if ((pInArgList->getArg("Data Set Location", pArg) == true) && (pArg != NULL))
   {
      string* pDatasetLocation = pArg->getPlugInArgValue<string>();
      if (pDatasetLocation != NULL)
      {
         mDatasetLocation = *pDatasetLocation;
      }
   }

   return true;
}
