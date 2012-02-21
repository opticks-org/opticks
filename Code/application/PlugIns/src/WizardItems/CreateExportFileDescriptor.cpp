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
#include "CreateExportFileDescriptor.h"
#include "DimensionDescriptor.h"
#include "FileDescriptor.h"
#include "MessageLogResource.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SignatureDataDescriptor.h"
#include "SignatureFileDescriptor.h"
#include "Units.h"

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWizardItems, CreateExportFileDescriptor);

CreateExportFileDescriptor::CreateExportFileDescriptor() :
   mpFilename(NULL),
   mpElement(NULL),
   mpStartRow(NULL),
   mpEndRow(NULL),
   mpRowSkipFactor(NULL),
   mpStartColumn(NULL),
   mpEndColumn(NULL),
   mpColumnSkipFactor(NULL),
   mpStartBand(NULL),
   mpEndBand(NULL),
   mpBandSkipFactor(NULL),
   mpUnitsName(NULL),
   mpUnitsType(NULL),
   mpUnitsScale(NULL),
   mpUnitsRangeMin(NULL),
   mpUnitsRangeMax(NULL),
   mpComponentName(NULL)
{
   setName("Create Export File Descriptor");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Creates a file descriptor from a data set with an optional "
      "subset that is suitable for export");
   setDescriptorId("{111C8EEA-7BE3-4416-BDAA-4A65C8DB968E}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

CreateExportFileDescriptor::~CreateExportFileDescriptor()
{
}

bool CreateExportFileDescriptor::getInputSpecification(PlugInArgList*& pArgList)
{
   if (!ModelItems::getInputSpecification(pArgList))
   {
      return false;
   }

   VERIFY(pArgList != NULL);

   // Add args
   VERIFY(pArgList->addArg<Filename>("Filename", NULL, "Location on-disk for the file data."));
   VERIFY(pArgList->addArg<DataElement>("Data Set", NULL, "Element to create the new descriptor from."));
   VERIFY(pArgList->addArg<unsigned int>("Start Row", NULL, "Beginning row for this raster descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("End Row", NULL, "Ending row for this raster descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("Row Skip Factor", NULL, "Row skip factor for this raster descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("Start Column", NULL, "Beginning column for this raster descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("End Column", NULL, "Ending column for this raster descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("Column Skip Factor", NULL,
      "Column skip factor for this raster descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("Start Band", NULL, "Beginning band for this raster descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("End Band", NULL, "Ending band for this raster descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("Band Skip Factor", NULL, "Band skip factor for this raster descriptor."));
   VERIFY(pArgList->addArg<string>("Units Name", NULL, "Modify the name used for units."));
   VERIFY(pArgList->addArg<UnitType>("Units Type", NULL, "Modify the type of units used."));
   VERIFY(pArgList->addArg<double>("Units Scale Factor", NULL,
      "Modify the scale factor for the units in the descriptor."));
   VERIFY(pArgList->addArg<double>("Units Range Minimum", NULL, "Modify the minimum units range."));
   VERIFY(pArgList->addArg<double>("Units Range Maximum", NULL, "Modify the maximum units range."));
   VERIFY(pArgList->addArg<string>("Component Name", NULL,
      "The signature component name for which to set the units values in a signature file descriptor.\n"
      "This argument is ignored if no units input values are specified."));

   return true;
}

bool CreateExportFileDescriptor::getOutputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   // Set up list
   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   // Add arg
   VERIFY(pArgList->addArg<FileDescriptor>("File Descriptor", NULL, "Resulting descriptor."));

   return true;
}

bool CreateExportFileDescriptor::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "E15CA38D-02CD-464B-8412-C0937D05A1FB");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   VERIFY(mpFilename != NULL);

   // Create a file descriptor with the values in the element's data descriptor
   FileDescriptor* pFileDescriptor = NULL;
   if (mpElement != NULL)
   {
      const RasterDataDescriptor* pRasterDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(mpElement->getDataDescriptor());
      const SignatureDataDescriptor* pSignatureDescriptor =
         dynamic_cast<const SignatureDataDescriptor*>(mpElement->getDataDescriptor());

      if (pRasterDescriptor != NULL)
      {
         // Start row
         DimensionDescriptor startRow;
         if (mpStartRow != NULL)
         {
            startRow = pRasterDescriptor->getOriginalRow(*mpStartRow - 1);
         }

         // End row
         DimensionDescriptor endRow;
         if (mpEndRow != NULL)
         {
            endRow = pRasterDescriptor->getOriginalRow(*mpEndRow - 1);
         }

         // Row skip
         unsigned int rowSkip = 0;
         if (mpRowSkipFactor != NULL)
         {
            rowSkip = *mpRowSkipFactor;
         }

         // Start column
         DimensionDescriptor startColumn;
         if (mpStartColumn != NULL)
         {
            startColumn = pRasterDescriptor->getOriginalColumn(*mpStartColumn - 1);
         }

         // End column
         DimensionDescriptor endColumn;
         if (mpEndColumn != NULL)
         {
            endColumn = pRasterDescriptor->getOriginalColumn(*mpEndColumn - 1);
         }

         // Column skip
         unsigned int columnSkip = 0;
         if (mpColumnSkipFactor != NULL)
         {
            columnSkip = *mpColumnSkipFactor;
         }

         // Start band
         DimensionDescriptor startBand;
         if (mpStartBand != NULL)
         {
            startBand = pRasterDescriptor->getOriginalBand(*mpStartBand - 1);
         }

         // End band
         DimensionDescriptor endBand;
         if (mpEndBand != NULL)
         {
            endBand = pRasterDescriptor->getOriginalBand(*mpEndBand - 1);
         }

         // Band skip
         unsigned int bandSkip = 0;
         if (mpBandSkipFactor != NULL)
         {
            bandSkip = *mpBandSkipFactor;
         }

         pFileDescriptor = RasterUtilities::generateRasterFileDescriptorForExport(pRasterDescriptor,
            mpFilename->getFullPathAndName(), startRow, endRow, rowSkip, startColumn, endColumn, columnSkip,
            startBand, endBand, bandSkip);
      }
      else if (pSignatureDescriptor != NULL)
      {
         FactoryResource<SignatureFileDescriptor> pSigFileDescriptor;
         pFileDescriptor = pSigFileDescriptor.release();
         pFileDescriptor->setFilename(mpFilename->getFullPathAndName());
      }
   }

   // Create a file descriptor with the input filename
   if (pFileDescriptor == NULL)
   {
      FactoryResource<FileDescriptor> pFactoryFileDescriptor;
      pFileDescriptor = pFactoryFileDescriptor.release();

      if (pFileDescriptor != NULL)
      {
         pFileDescriptor->setFilename(mpFilename->getFullPathAndName());
      }
   }
   else
   {
      // set any units value inputs
      populateUnits(pFileDescriptor);
   }

   if (pFileDescriptor == NULL)
   {
      reportError("Could not create the file descriptor!", "219F9340-1681-4BD8-90B4-2D9335E8B751");
      return false;
   }

   // Set the output arg value
   if (pOutArgList != NULL)
   {
      PlugInArg* pArg = NULL;

      // File descriptor
      if (pOutArgList->getArg("File Descriptor", pArg) && (pArg != NULL))
      {
         pArg->setActualValue(pFileDescriptor);
      }
      else
      {
         // Destroy the file descriptor
         Service<ApplicationServices> pApp;
         if (pApp.get() != NULL)
         {
            ObjectFactory* pObjFact = pApp->getObjectFactory();
            if (pObjFact != NULL)
            {
               pObjFact->destroyObject(pFileDescriptor, "FileDescriptor");
            }
         }

         reportError("Could not set the file descriptor output value!", "98FB7EB3-BC2A-4075-9DE2-FD28ABAECE5B");
         return false;
      }
   }

   reportComplete();
   return true;
}

bool CreateExportFileDescriptor::extractInputArgs(PlugInArgList* pInArgList)
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
      reportError("Could not read the filename input value!", "BB123032-1D12-4AEF-B714-2F105FF9C2BD");
      return false;
   }

   mpFilename = pArg->getPlugInArgValue<Filename>();
   if (mpFilename == NULL)
   {
      reportError("The filename input value is invalid!", "B7E73340-B858-4AA8-B996-958D5AC654B7");
      return false;
   }

   // Data element
   if ((pInArgList->getArg("Data Set", pArg) == true) && (pArg != NULL))
   {
      mpElement = pArg->getPlugInArgValue<DataElement>();
   }

   // Start row
   if ((pInArgList->getArg("Start Row", pArg) == true) && (pArg != NULL))
   {
      mpStartRow = pArg->getPlugInArgValue<unsigned int>();
   }

   // End row
   if ((pInArgList->getArg("End Row", pArg) == true) && (pArg != NULL))
   {
      mpEndRow = pArg->getPlugInArgValue<unsigned int>();
   }

   // Row skip factor
   if ((pInArgList->getArg("Row Skip Factor", pArg) == true) && (pArg != NULL))
   {
      mpRowSkipFactor = pArg->getPlugInArgValue<unsigned int>();
   }

   // Start column
   if ((pInArgList->getArg("Start Column", pArg) == true) && (pArg != NULL))
   {
      mpStartColumn = pArg->getPlugInArgValue<unsigned int>();
   }

   // End column
   if ((pInArgList->getArg("End Column", pArg) == true) && (pArg != NULL))
   {
      mpEndColumn = pArg->getPlugInArgValue<unsigned int>();
   }

   // Column skip factor
   if ((pInArgList->getArg("Column Skip Factor", pArg) == true) && (pArg != NULL))
   {
      mpColumnSkipFactor = pArg->getPlugInArgValue<unsigned int>();
   }

   // Start band
   if ((pInArgList->getArg("Start Band", pArg) == true) && (pArg != NULL))
   {
      mpStartBand = pArg->getPlugInArgValue<unsigned int>();
   }

   // End band
   if ((pInArgList->getArg("End Band", pArg) == true) && (pArg != NULL))
   {
      mpEndBand = pArg->getPlugInArgValue<unsigned int>();
   }

   // Band skip factor
   if ((pInArgList->getArg("Band Skip Factor", pArg) == true) && (pArg != NULL))
   {
      mpBandSkipFactor = pArg->getPlugInArgValue<unsigned int>();
   }

   // Units name
   if ((pInArgList->getArg("Units Name", pArg) == true) && (pArg != NULL))
   {
      mpUnitsName = pArg->getPlugInArgValue<string>();
   }

   // Units type
   if ((pInArgList->getArg("Units Type", pArg) == true) && (pArg != NULL))
   {
      mpUnitsType = pArg->getPlugInArgValue<UnitType>();
   }

   // Units scale factor
   if ((pInArgList->getArg("Units Scale Factor", pArg) == true) && (pArg != NULL))
   {
      mpUnitsScale = pArg->getPlugInArgValue<double>();
   }

   // Units range min
   if ((pInArgList->getArg("Units Range Minimum", pArg) == true) && (pArg != NULL))
   {
      mpUnitsRangeMin = pArg->getPlugInArgValue<double>();
   }

   // Units range max
   if ((pInArgList->getArg("Units Range Maximum", pArg) == true) && (pArg != NULL))
   {
      mpUnitsRangeMax = pArg->getPlugInArgValue<double>();
   }

   // Signature component name
   if ((pInArgList->getArg("Component Name", pArg) == true) && (pArg != NULL))
   {
      mpComponentName = pArg->getPlugInArgValue<string>();
   }

   return true;
}

void CreateExportFileDescriptor::populateUnits(FileDescriptor* pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return;
   }

   FactoryResource<Units> pUnits;
   if (mpUnitsName != NULL)
   {
      pUnits->setUnitName(*mpUnitsName);
   }
   if (mpUnitsType != NULL)
   {
      pUnits->setUnitType(*mpUnitsType);
   }
   if (mpUnitsScale != NULL)
   {
      pUnits->setScaleFromStandard(*mpUnitsScale);
   }
   if (mpUnitsRangeMin != NULL)
   {
      pUnits->setRangeMin(*mpUnitsRangeMin);
   }
   if (mpUnitsRangeMax != NULL)
   {
      pUnits->setRangeMax(*mpUnitsRangeMax);
   }

   RasterFileDescriptor* pRasterFd = dynamic_cast<RasterFileDescriptor*>(pDescriptor);
   SignatureFileDescriptor* pSignatureFd = dynamic_cast<SignatureFileDescriptor*>(pDescriptor);

   if (pRasterFd != NULL)
   {
      pRasterFd->setUnits(pUnits.get());
   }
   else if (pSignatureFd != NULL && mpComponentName != NULL && mpComponentName->empty() == false)
   {
      pSignatureFd->setUnits(*mpComponentName, pUnits.get());
   }
}