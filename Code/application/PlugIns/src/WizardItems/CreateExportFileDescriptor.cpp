/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CreateExportFileDescriptor.h"
#include "AppVersion.h"
#include "AppVerify.h"
#include "DimensionDescriptor.h"
#include "FileDescriptor.h"
#include "MessageLogResource.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"

using namespace std;

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
   mpBandSkipFactor(NULL)
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
   pArg->setName("Filename");                         // Filename
   pArg->setType("Filename");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Data Set");                         // Data element
   pArg->setType("DataElement");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Start Row");                        // Start row
   pArg->setType("unsigned int");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("End Row");                          // End row
   pArg->setType("unsigned int");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Row Skip Factor");                  // Row skip factor
   pArg->setType("unsigned int");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Start Column");                     // Start column
   pArg->setType("unsigned int");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("End Column");                       // End column
   pArg->setType("unsigned int");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Column Skip Factor");               // Column skip factor
   pArg->setType("unsigned int");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Start Band");                       // Start band
   pArg->setType("unsigned int");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("End Band");                         // End band
   pArg->setType("unsigned int");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("Band Skip Factor");                 // Band skip factor
   pArg->setType("unsigned int");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

   return true;
}

bool CreateExportFileDescriptor::getOutputSpecification(PlugInArgList*& pArgList)
{
   Service<PlugInManagerServices> pPlugInManager;
   VERIFY(pPlugInManager.get() != NULL);

   // Set up list
   pArgList = pPlugInManager->getPlugInArgList();
   VERIFY(pArgList != NULL);

   // Add args
   PlugInArg* pArg = pPlugInManager->getPlugInArg();
   VERIFY(pArg != NULL);
   pArg->setName("File Descriptor");                  // File descriptor
   pArg->setType("FileDescriptor");
   pArg->setDefaultValue(NULL);
   pArgList->addArg(*pArg);

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

         pFileDescriptor = RasterUtilities::generateFileDescriptorForExport(pRasterDescriptor,
            mpFilename->getFullPathAndName(), startRow, endRow, rowSkip, startColumn, endColumn, columnSkip,
            startBand, endBand, bandSkip);
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

   return true;
}
