/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ImportDataSet.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "DataDescriptor.h"
#include "Filename.h"
#include "ImportDescriptor.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"

#include <vector>
using namespace std;

ImportDataSet::ImportDataSet() :
   mpFilename(NULL),
   mpDescriptor(NULL),
   mShowDialog(false)
{
   setName("Import Data Set");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Loads a data set");
   setDescriptorId("{6376FA28-005A-43c6-9A97-8C2D77C97014}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

ImportDataSet::~ImportDataSet()
{
}

bool ImportDataSet::setBatch()
{
   mbInteractive = false;
   return true;
}

bool ImportDataSet::getInputSpecification(PlugInArgList*& pArgList)
{
   bool bSuccess = DesktopItems::getInputSpecification(pArgList);
   if (bSuccess == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);
   VERIFY(pArgList->addArg<Filename>("Filename", NULL));
   VERIFY(pArgList->addArg<DataDescriptor>("Data Descriptor", NULL));
   VERIFY(pArgList->addArg<string>("Importer Name", NULL));

   if (mbInteractive == true)
   {
      VERIFY(pArgList->addArg<bool>("Show Options Dialog", mShowDialog));
   }

   return true;
}

bool ImportDataSet::getOutputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   // Set up list
   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   // Add args
   VERIFY(pArgList->addArg<DataElement>("Data Set"));

   return true;
}

bool ImportDataSet::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "2D301009-130B-4c05-983A-AAA9E01819D1");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   Progress* pProgress = getProgress();

   // Load the data set
   ImporterResource importer(mImporterName, pProgress, !mbInteractive);
   if (mpDescriptor != NULL)
   {
      // Copy the data descriptor because the importer resource owns all data descriptors
      DataDescriptor* pTempDescriptor = mpDescriptor->copy();

      Service<ModelServices> pModel;

      ImportDescriptor* pImportDescriptor = pModel->createImportDescriptor(pTempDescriptor);
      if (pImportDescriptor != NULL)
      {
         vector<ImportDescriptor*> descriptors;
         descriptors.push_back(pImportDescriptor);

         importer->setImportDescriptors(descriptors);
      }
   }
   else if (mpFilename != NULL)
   {
      const string& filename = mpFilename->getFullPathAndName();
      if (filename.empty() == false)
      {
         importer->setFilename(filename);
      }
   }

   importer->setEditType(mShowDialog ? ImportAgent::ALWAYS_EDIT : ImportAgent::AS_NEEDED_EDIT);
   importer->updateMruFileList((mpDescriptor == NULL) && (mpFilename == NULL));

   if (importer->execute() == false)
   {
      reportError("Unable to import the data set!", "260757E4-31FA-49ba-850D-965AA6F1EB54");
      return false;
   }

   // Set the output arg value
   if (pOutArgList != NULL)
   {
      PlugInArg* pArg = NULL;

      // Data set
      if ((pOutArgList->getArg("Data Set", pArg) == true) && (pArg != NULL))
      {
         vector<DataElement*> importedElements = importer->getImportedElements();
         if (importedElements.empty() == false)
         {
            pArg->setActualValue(importedElements.front());
         }
      }
      else
      {
         reportError("Could not set the data set output arg value!", "64FBFB84-1A50-4446-B796-62D839BB492D");
         return false;
      }
   }

   reportComplete();
   return true;
}

bool ImportDataSet::extractInputArgs(PlugInArgList* pInArgList)
{
   bool bSuccess = DesktopItems::extractInputArgs(pInArgList);
   if (bSuccess == false)
   {
      return false;
   }

   // Filename
   mpFilename = pInArgList->getPlugInArgValue<Filename>("Filename");

   // Data descriptor
   mpDescriptor = pInArgList->getPlugInArgValue<DataDescriptor>("Data Descriptor");

   // Importer name
   mImporterName.erase();
   pInArgList->getPlugInArgValue<string>("Importer Name", mImporterName);

   // Show options dialog
   if (mbInteractive == true)
   {
      pInArgList->getPlugInArgValue<bool>("Show Options Dialog", mShowDialog);
   }

   // Error checking
   if (mbInteractive == false)
   {
      if ((mpFilename == NULL) && (mpDescriptor == NULL))
      {
         reportError("Both of the filename and the data descriptor input values are invalid!",
            "35588FCD-F8BC-4383-BFFA-35A64297DF23");
         return false;
      }

      if ((mpFilename != NULL) && (mpDescriptor != NULL))
      {
         reportWarning("Both of the filename and the data descriptor input values are valid.  "
            "The data descriptor will be used and the filename value will be ignored.",
            "8B77531E-3D22-4EBC-8D6C-D4F4AB8B4026");
      }
   }

   return true;
}
