/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ExportDataSet.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "DataElement.h"
#include "Layer.h"
#include "MessageLogResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "View.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWizardItems, ExportDataSet);

ExportDataSet::ExportDataSet() :
   mpView(NULL),
   mpLayer(NULL),
   mpElement(NULL),
   mOutputWidth(0),
   mOutputHeight(0),
   mpFilename(NULL),
   mpFileDescriptor(NULL)
{
   setName("Export Data Set");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Saves a data set to a file");
   setDescriptorId("{03FC2F68-AC01-4f9d-96CB-9AD7913D8ECE}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

ExportDataSet::~ExportDataSet()
{
}

bool ExportDataSet::setBatch()
{
   mbInteractive = false;
   return true;
}

bool ExportDataSet::getInputSpecification(PlugInArgList*& pArgList)
{
   bool bSuccess = DesktopItems::getInputSpecification(pArgList);
   if (bSuccess == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);

   // Add args
   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   PlugInArg* pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("View");                          // View
   pArg->setType("View");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Layer");                         // Layer
   pArg->setType("Layer");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Data Element");                  // Data element
   pArg->setType("DataElement");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   if (mbInteractive == false)
   {
      pArgList->addArg<unsigned int>("Output Width");
      pArgList->addArg<unsigned int>("Output Height");

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Filename");                   // Filename
      pArg->setType("Filename");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("File Descriptor");            // File descriptor
      pArg->setType("FileDescriptor");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);

      pArg = pPlugInManager->getPlugInArg();
      VERIFY(pArg != NULL);
      pArg->setName("Exporter Name");              // Exporter name
      pArg->setType("string");
      pArg->setDefaultValue(NULL);
      pArgList->addArg(*pArg);
   }

   return true;
}

bool ExportDataSet::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool ExportDataSet::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "8CFA7129-13F8-43dc-B829-EBF008E6A47C");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   Progress* pProgress = getProgress();

   // Export the data set
   bool bSuccess = false;
   if (mbInteractive == true)
   {
      if (mpView != NULL)
      {
         bSuccess = mpDesktop->exportSessionItem(mpView, NULL, pProgress);
      }
      else if (mpLayer != NULL)
      {
         bSuccess = mpDesktop->exportSessionItem(mpLayer, NULL, pProgress);
      }
      else if (mpElement != NULL)
      {
         bSuccess = mpDesktop->exportSessionItem(mpElement, NULL, pProgress);
      }
   }
   else
   {
      // Create the exporter
      ExporterResource exporter(mExporterName, pProgress);

      // Set the data to export
      if (mpView != NULL)
      {
         exporter->setItem(mpView);
      }
      else if (mpLayer != NULL)
      {
         exporter->setItem(mpLayer);
      }
      else if (mpElement != NULL)
      {
         exporter->setItem(mpElement);
      }
      if (mOutputWidth != 0 && mOutputHeight != 0)
      {
         exporter->getInArgList().setPlugInArgValue("Output Width", &mOutputWidth);
         exporter->getInArgList().setPlugInArgValue("Output Height", &mOutputHeight);
      }

      // Create a file descriptor from the filename if necessary
      if (mpFileDescriptor == NULL)
      {
         ExecutableResource plugIn("Create Export File Descriptor", "", pProgress, !mbInteractive);
         plugIn->getInArgList().setPlugInArgValue("Data Set", mpElement);
         plugIn->getInArgList().setPlugInArgValue("Filename", mpFilename);

         plugIn->execute();
         mpFileDescriptor = plugIn->getOutArgList().getPlugInArgValue<FileDescriptor>("File Descriptor");
         if (mpFileDescriptor == NULL)
         {
            reportError("Could not create a file descriptor from the filename!",
               "8BF90FB1-F99A-4084-957D-D1DB298D0DF2");
            return false;
         }
      }

      // Set the file descriptor
      exporter->setFileDescriptor(mpFileDescriptor);

      // Export the data
      bSuccess = exporter->execute();
   }

   if (bSuccess == false)
   {
      reportError("Unable to export the data set!", "C9BB1B27-0F3D-4BCF-9A0E-354A37254118");
      return false;
   }

   reportComplete();
   return true;
}

bool ExportDataSet::extractInputArgs(PlugInArgList* pInArgList)
{
   bool bSuccess = DesktopItems::extractInputArgs(pInArgList);
   if (bSuccess == false)
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // View
   if ((pInArgList->getArg("View", pArg) == true) && (pArg != NULL))
   {
      mpView = pArg->getPlugInArgValue<View>();
   }

   // Layer
   if ((pInArgList->getArg("Layer", pArg) == true) && (pArg != NULL))
   {
      mpLayer = pArg->getPlugInArgValue<Layer>();
   }

   // Data element
   if ((pInArgList->getArg("Data Element", pArg) == true) && (pArg != NULL))
   {
      mpElement = pArg->getPlugInArgValue<DataElement>();
   }

   if (mbInteractive == false)
   {
      // Output size
      pInArgList->getPlugInArgValue("Output Width", mOutputWidth);
      pInArgList->getPlugInArgValue("Output Height", mOutputHeight);

      unsigned int itemsToExport = 0;
      if (mpView != NULL)
      {
         itemsToExport++;
      }

      if (mpLayer != NULL)
      {
         itemsToExport++;
      }

      if (mpElement != NULL)
      {
         itemsToExport++;
      }

      if (itemsToExport == 0)
      {
         reportError("All of the view, layer, and data element input values are invalid, "
            "so no data set can be exported!", "F3CC537B-1352-4D8C-A77B-7B2614160F76");
         return false;
      }

      if (itemsToExport > 1)
      {
         reportError("More than one of the view, layer, and data element input values are valid!  "
            "Only one can be specified as the data set to export.", "C4A45917-FF66-42E9-95AB-948215ACB145");
         return false;
      }

      // Filename
      if ((pInArgList->getArg("Filename", pArg) == true) && (pArg != NULL))
      {
         mpFilename = pArg->getPlugInArgValue<Filename>();
      }

      // Data descriptor
      if ((pInArgList->getArg("File Descriptor", pArg) == true) && (pArg != NULL))
      {
         mpFileDescriptor = pArg->getPlugInArgValue<FileDescriptor>();
      }

      if ((mpFilename == NULL) && (mpFileDescriptor == NULL))
      {
         reportError("Both of the filename and the file descriptor input values are invalid!",
            "2DE88B09-C48E-451A-8487-AE39F5872498");
         return false;
      }

      if ((mpFilename != NULL) && (mpFileDescriptor != NULL))
      {
         reportWarning("Both of the filename and the file descriptor input values are valid.  "
            "The file descriptor will be used and the filename value will be ignored.",
            "5D86147B-C447-41CE-BD76-A6B9CD44D65E");
      }

      // Exporter name
      mExporterName.erase();

      if ((pInArgList->getArg("Exporter Name", pArg) == false) || (pArg == NULL))
      {
         reportError("Could not read the exporter name input value!", "F4A03879-5DD7-4DCA-8C9E-1D9B2A8B226F");
         return false;
      }

      string* pExporterName = pArg->getPlugInArgValue<string>();
      if (pExporterName != NULL)
      {
         mExporterName = *pExporterName;
      }

      if (mExporterName.empty() == true)
      {
         reportError("The exporter name input value is invalid!", "F54865E1-5349-4E7A-B333-3D50BF2632C4");
         return false;
      }
   }

   return true;
}
