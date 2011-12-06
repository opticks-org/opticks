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
#include "FileDescriptor.h"
#include "Filename.h"
#include "ImportDescriptor.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"

#include <vector>
using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWizardItems, ImportDataSet);

ImportDataSet::ImportDataSet() :
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
{}

bool ImportDataSet::setBatch()
{
   DesktopItems::setBatch();
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
   VERIFY(pArgList->addArg<Filename>("Filename", NULL, "Single filename to be imported."));
   VERIFY(pArgList->addArg<vector<Filename*> >("Filenames", NULL, "List of multiple filenames to be imported, if necessary."));
   VERIFY(pArgList->addArg<DataDescriptor>("Data Descriptor", NULL, "Data descriptor to load data from."));
   VERIFY(pArgList->addArg<string>("Importer Name", NULL, "Name of importer to be used."));

   if (isBatch() == false)
   {
      VERIFY(pArgList->addArg<bool>("Show Options Dialog", mShowDialog, "Whether to show the options dialog or not."));
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
   VERIFY(pArgList->addArg<DataElement>("Data Set", "Data set resulting from the import operation."));

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

   // Set the data sets to load
   ImporterResource importer(mImporterName, pProgress, isBatch());
   if (mpDescriptor != NULL)
   {
      // Copy the data descriptor because the importer resource owns all data descriptors
      DataDescriptor* pTempDescriptor = mpDescriptor->copy();

      ImportDescriptorResource importDescriptor(pTempDescriptor);
      if (importDescriptor.get() != NULL)
      {
         // Filename
         string filename;

         const FileDescriptor* pFileDescriptor = pTempDescriptor->getFileDescriptor();
         if (pFileDescriptor != NULL)
         {
            filename = pFileDescriptor->getFilename().getFullPathAndName();
         }

         if (filename.empty() == true)
         {
            reportError("The filename could not be obtained from the data descriptor!",
               "67D177B8-345D-4102-A677-81BDA38B6D63");
            return false;
         }

         // Datasets
         vector<ImportDescriptor*> descriptors;
         descriptors.push_back(importDescriptor.release());

         map<string, vector<ImportDescriptor*> > resourceDescriptors;
         resourceDescriptors[filename] = descriptors;

         importer->setDatasets(resourceDescriptors);
      }
   }
   else if (mFilenames.empty() == false)
   {
      importer->setFilenames(mFilenames);
   }

   importer->setEditType(mShowDialog ? ImportAgent::ALWAYS_EDIT : ImportAgent::AS_NEEDED_EDIT);
   importer->updateMruFileList((mpDescriptor == NULL) && (mFilenames.empty() == true));

   // Import the data sets
   if (importer->execute() == false)
   {
      reportError("Unable to import any data set!", "260757E4-31FA-49ba-850D-965AA6F1EB54");
      return false;
   }

   // If specific data sets were given check to make sure that all data sets were successfully imported
   if ((mpDescriptor != NULL) || (mFilenames.empty() == false))
   {
      unsigned int numDatasetsToImport = 0;

      vector<ImportDescriptor*> importDescriptors = importer->getImportDescriptors();
      for (vector<ImportDescriptor*>::iterator iter = importDescriptors.begin();
         iter != importDescriptors.end();
         ++iter)
      {
         ImportDescriptor* pImportDescriptor = *iter;
         if ((pImportDescriptor != NULL) && (pImportDescriptor->isImported() == true))
         {
            ++numDatasetsToImport;
         }
      }

      vector<DataElement*> importedElements = importer->getImportedElements();
      if (importedElements.size() < numDatasetsToImport)
      {
         reportError("Unable to import at least one of the specified data sets.",
            "E5FBD561-2F34-4CFD-A400-B559463114D6");
         return false;
      }
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

   // Filenames
   mFilenames.clear();

   Filename* pFilename = pInArgList->getPlugInArgValue<Filename>("Filename");
   if (pFilename != NULL)
   {
      string filename = pFilename->getFullPathAndName();
      if (filename.empty() == false)
      {
         mFilenames.push_back(filename);
      }
   }

   vector<Filename*>* pFilenames = pInArgList->getPlugInArgValue<vector<Filename*> >("Filenames");
   if (pFilenames != NULL)
   {
      for (vector<Filename*>::iterator iter = pFilenames->begin(); iter != pFilenames->end(); ++iter)
      {
         Filename* pCurrentFilename = *iter;
         if (pCurrentFilename != NULL)
         {
            string filename = pCurrentFilename->getFullPathAndName();
            if (filename.empty() == false)
            {
               mFilenames.push_back(filename);
            }
         }
      }
   }

   // Data descriptor
   mpDescriptor = pInArgList->getPlugInArgValue<DataDescriptor>("Data Descriptor");

   // Importer name
   mImporterName.erase();
   pInArgList->getPlugInArgValue<string>("Importer Name", mImporterName);
   if (mImporterName.empty() == true)
   {
      mImporterName = "Auto Importer";
   }

   // Show options dialog
   if (isBatch() == false)
   {
      pInArgList->getPlugInArgValue<bool>("Show Options Dialog", mShowDialog);
   }

   // Error checking
   if (isBatch() == true)
   {
      if ((mFilenames.empty() == true) && (mpDescriptor == NULL))
      {
         reportError("Both filename and data descriptor input values are invalid!",
            "35588FCD-F8BC-4383-BFFA-35A64297DF23");
         return false;
      }

      if ((mFilenames.empty() == false) && (mpDescriptor != NULL))
      {
         reportWarning("Both filename and data descriptor input values are valid.  "
            "The data descriptor will be used and the filename value(s) will be ignored.",
            "8B77531E-3D22-4EBC-8D6C-D4F4AB8B4026");
      }
   }

   return true;
}
