/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "BadValues.h"
#include "DataDescriptor.h"
#include "DimensionDescriptor.h"
#include "EditDataDescriptor.h"
#include "MessageLogResource.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInArg.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "SignatureDataDescriptor.h"
#include "SignatureFileDescriptor.h"
#include "Units.h"

#include <set>
#include <stdio.h>

using namespace std;

REGISTER_PLUGIN_BASIC(OpticksWizardItems, EditDataDescriptor);

EditDataDescriptor::EditDataDescriptor() :
   mpDescriptor(NULL),
   mpFileDescriptor(NULL),
   mpProcessingLocation(NULL),
   mpDataType(NULL),
   mpInterleave(NULL),
   mpBadValuesStr(NULL),
   mpStartRow(NULL),
   mpEndRow(NULL),
   mpRowSkipFactor(NULL),
   mpStartColumn(NULL),
   mpEndColumn(NULL),
   mpColumnSkipFactor(NULL),
   mpStartBand(NULL),
   mpEndBand(NULL),
   mpBandSkipFactor(NULL),
   mpBadBandsFile(NULL),
   mpPixelSizeX(NULL),
   mpPixelSizeY(NULL),
   mpUnitsName(NULL),
   mpUnitsType(NULL),
   mpUnitsScale(NULL),
   mpUnitsRangeMin(NULL),
   mpUnitsRangeMax(NULL),
   mpDisplayMode(NULL),
   mpGrayBand(NULL),
   mpRedBand(NULL),
   mpGreenBand(NULL),
   mpBlueBand(NULL),
   mpComponentName(NULL)
{
   setName("Edit Data Descriptor");
   setVersion(APP_VERSION_NUMBER);
   setCreator("Ball Aerospace & Technologies, Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Edits the values of an existing data set descriptor object");
   setDescriptorId("{8F43A393-14EB-4ad1-934C-7F75B30A53E7}");
   allowMultipleInstances(true);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

EditDataDescriptor::~EditDataDescriptor()
{
}

bool EditDataDescriptor::getInputSpecification(PlugInArgList*& pArgList)
{
   bool bSuccess = ModelItems::getInputSpecification(pArgList);
   if (bSuccess == false)
   {
      return false;
   }

   VERIFY(pArgList != NULL);

   // Add input args
   VERIFY(pArgList->addArg<DataDescriptor>("Data Descriptor", NULL, "Data descriptor to be modified."));
   VERIFY((pArgList->addArg<FileDescriptor>("File Descriptor", NULL, "New file descriptor.")));
   VERIFY(pArgList->addArg<ProcessingLocation>("Processing Location", NULL, "New processing location."));
   VERIFY(pArgList->addArg<EncodingType>("Data Type", NULL, "New encoding type."));
   VERIFY(pArgList->addArg<InterleaveFormatType>("Interleave Format", NULL, "New interleave format."));
   VERIFY(pArgList->addArg<string>("Bad Values", NULL, "Modify the bad values in the data descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("Start Row", NULL, "Modify the start row for the data descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("End Row", NULL, "Modify the end row for the data descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("Row Skip Factor", NULL,
      "Modify the row skip factor for the data descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("Start Column", NULL, "Modify the start column for the data descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("End Column", NULL, "Modify the end column for the data descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("Column Skip Factor", NULL,
      "Modify the column skip factor for the data descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("Start Band", NULL, "Modify the start band for the data descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("End Band", NULL, "Modify the end band for the data descriptor."));
   VERIFY(pArgList->addArg<unsigned int>("Band Skip Factor", NULL,
      "Modify the band skip factor for the data descriptor."));
   VERIFY(pArgList->addArg<Filename>("Bad Bands File", NULL,
      "Modify the file containing bad bands for the data descriptor."));
   VERIFY(pArgList->addArg<double>("X Pixel Size", NULL, "Modify the column size in pixels."));
   VERIFY(pArgList->addArg<double>("Y Pixel Size", NULL, "Modify the row size in pixels."));
   VERIFY(pArgList->addArg<string>("Units Name", NULL, "Modify the name used for units."));
   VERIFY(pArgList->addArg<UnitType>("Units Type", NULL, "Modify the type of units used."));
   VERIFY(pArgList->addArg<double>("Units Scale Factor", NULL,
      "Modify the scale factor for the units in the descriptor."));
   VERIFY(pArgList->addArg<double>("Units Range Minimum", NULL, "Modify the minimum units range."));
   VERIFY(pArgList->addArg<double>("Units Range Maximum", NULL, "Modify the maximum units range."));
   VERIFY(pArgList->addArg<DisplayMode>("Display Mode", NULL, "Modify the display mode to either RGB or grayscale."));
   VERIFY(pArgList->addArg<unsigned int>("Gray Display Band", NULL, "Modify which band is used as the gray channel."));
   VERIFY(pArgList->addArg<unsigned int>("Red Display Band", NULL, "Modify which band is used as the red channel."));
   VERIFY(pArgList->addArg<unsigned int>("Green Display Band", NULL,
      "Modify which band is used as the green channel."));
   VERIFY(pArgList->addArg<unsigned int>("Blue Display Band", NULL, "Modify which band is used as the blue channel."));
   VERIFY(pArgList->addArg<string>("Component Name", NULL,
      "Component name for the units in the signature descriptor."));

   return true;
}

bool EditDataDescriptor::getOutputSpecification(PlugInArgList*& pArgList)
{
   pArgList = NULL;
   return true;
}

bool EditDataDescriptor::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Execute Wizard Item", "app", "055486F4-A9DB-4FDA-9AA7-75D1917E2C87");
   pStep->addProperty("Item", getName());
   mpStep = pStep.get();

   if (extractInputArgs(pInArgList) == false)
   {
      return false;
   }

   // Set the values in the data descriptor
   VERIFY(mpDescriptor != NULL);

   // File descriptor
   if (mpFileDescriptor != NULL)
   {
      mpDescriptor->setFileDescriptor(mpFileDescriptor);
   }

   // Processing location
   if (mpProcessingLocation != NULL)
   {
      mpDescriptor->setProcessingLocation(*mpProcessingLocation);
   }

   RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(mpDescriptor);
   RasterFileDescriptor* pRasterFileDescriptor = dynamic_cast<RasterFileDescriptor*>(mpFileDescriptor);
   SignatureDataDescriptor* pSignatureDescriptor = dynamic_cast<SignatureDataDescriptor*>(mpDescriptor);
   SignatureFileDescriptor* pSignatureFileDescriptor = dynamic_cast<SignatureFileDescriptor*>(mpFileDescriptor);

   if (pRasterDescriptor != NULL)
   {
      if (pRasterFileDescriptor != NULL)
      {
         // Set the rows and columns to match the rows and columns in the file descriptor before creating the subset
         const vector<DimensionDescriptor>& rows = pRasterFileDescriptor->getRows();
         pRasterDescriptor->setRows(rows);

         const vector<DimensionDescriptor>& columns = pRasterFileDescriptor->getColumns();
         pRasterDescriptor->setColumns(columns);

         const vector<DimensionDescriptor>& bands = pRasterFileDescriptor->getBands();
         pRasterDescriptor->setBands(bands);
      }

      // Data type
      if (mpDataType != NULL)
      {
         pRasterDescriptor->setDataType(*mpDataType);
      }

      // InterleaveFormat
      if (mpInterleave != NULL)
      {
         pRasterDescriptor->setInterleaveFormat(*mpInterleave);
      }

      // Bad values
      if (mpBadValuesStr != NULL)
      {
         FactoryResource<BadValues> pBadValues;
         if (mpBadValuesStr->empty() == false)
         {
            pBadValues->setBadValues(*mpBadValuesStr);
         }
         pRasterDescriptor->setBadValues(pBadValues.get());
      }

      // Rows
      if ((mpStartRow != NULL) || (mpEndRow != NULL) || (mpRowSkipFactor != NULL))
      {
         // We need to obtain this origRows from the FileDescriptor if present since an importer
         // may generate a subset by default in which case the DataDescriptor will not contain all
         // the rows and subsetting will not work correctly. We
         // can't just set mpFileDescriptor = pRasterDescriptor->getFileDescriptor() since we only
         // want to replace the DataDescriptor's row list if one of the subset options is specified
         const RasterFileDescriptor* pFileDesc(pRasterFileDescriptor);
         if (pFileDesc == NULL)
         {
            pFileDesc = dynamic_cast<const RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
         }
         const vector<DimensionDescriptor>& origRows = (pFileDesc != NULL) ?
            pFileDesc->getRows() : pRasterDescriptor->getRows();
         unsigned int startRow = 0;
         if (mpStartRow != NULL)
         {
            startRow = *mpStartRow;
         }
         else if (origRows.empty() == false)
         {
            startRow = origRows.front().getOriginalNumber() + 1;
         }

         unsigned int endRow = 0;
         if (mpEndRow != NULL)
         {
            endRow = *mpEndRow;
         }
         else if (origRows.empty() == false)
         {
            endRow = origRows.back().getOriginalNumber() + 1;
         }

         unsigned int rowSkip = 0;
         if (mpRowSkipFactor != NULL)
         {
            rowSkip = *mpRowSkipFactor;
         }

         vector<DimensionDescriptor> rows;
         for (unsigned int i = 0; i < origRows.size(); ++i)
         {
            DimensionDescriptor rowDim = origRows[i];
            unsigned int originalNumber = rowDim.getOriginalNumber() + 1;
            if ((originalNumber >= startRow) && (originalNumber <= endRow))
            {
               rows.push_back(rowDim);
               i += rowSkip;
            }
         }

         pRasterDescriptor->setRows(rows);
      }

      // Columns
      if ((mpStartColumn != NULL) || (mpEndColumn != NULL) || (mpColumnSkipFactor != NULL))
      {
         // We need to obtain this origColumns from the FileDescriptor if present since an importer
         // may generate a subset by default in which case the DataDescriptor will not contain all
         // the columns and subsetting will not work correctly. We
         // can't just set mpFileDescriptor = pRasterDescriptor->getFileDescriptor() since we only
         // want to replace the DataDescriptor's column list if one of the subset options is specified
         const RasterFileDescriptor* pFileDesc(pRasterFileDescriptor);
         if (pFileDesc == NULL)
         {
            pFileDesc = dynamic_cast<const RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
         }
         const vector<DimensionDescriptor>& origColumns = (pFileDesc != NULL) ?
            pFileDesc->getColumns() : pRasterDescriptor->getColumns();

         unsigned int startColumn = 0;
         if (mpStartColumn != NULL)
         {
            startColumn = *mpStartColumn;
         }
         else if (origColumns.empty() == false)
         {
            startColumn = origColumns.front().getOriginalNumber() + 1;
         }

         unsigned int endColumn = 0;
         if (mpEndColumn != NULL)
         {
            endColumn = *mpEndColumn;
         }
         else if (origColumns.empty() == false)
         {
            endColumn = origColumns.back().getOriginalNumber() + 1;
         }

         unsigned int columnSkip = 0;
         if (mpColumnSkipFactor != NULL)
         {
            columnSkip = *mpColumnSkipFactor;
         }

         vector<DimensionDescriptor> columns;
         for (unsigned int i = 0; i < origColumns.size(); ++i)
         {
            DimensionDescriptor columnDim = origColumns[i];
            unsigned int originalNumber = columnDim.getOriginalNumber() + 1;
            if ((originalNumber >= startColumn) && (originalNumber <= endColumn))
            {
               columns.push_back(columnDim);
               i += columnSkip;
            }
         }

         pRasterDescriptor->setColumns(columns);
      }

      // Bands
      if ((mpStartBand != NULL) || (mpEndBand != NULL) || (mpBandSkipFactor != NULL) || (mpBadBandsFile != NULL))
      {
         // We need to obtain this origBands from the FileDescriptor if present since an importer
         // may generate a subset by default in which case the DataDescriptor will not contain all
         // the bands and subsetting (especially by bad band file) will not work correctly. We
         // can't just set mpFileDescriptor = pRasterDescriptor->getFileDescriptor() since we only
         // want to replace the DataDescriptor's band list if one of the subset options is specified
         const RasterFileDescriptor* pFileDesc(pRasterFileDescriptor);
         if (pFileDesc == NULL)
         {
            pFileDesc = dynamic_cast<const RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
         }
         const vector<DimensionDescriptor>& origBands = (pFileDesc != NULL) ?
            pFileDesc->getBands() : pRasterDescriptor->getBands();

         unsigned int startBand = 0;
         if (mpStartBand != NULL)
         {
            startBand = *mpStartBand;
         }
         else if (origBands.empty() == false)
         {
            startBand = origBands.front().getOriginalNumber() + 1;
         }

         unsigned int endBand = 0;
         if (mpEndBand != NULL)
         {
            endBand = *mpEndBand;
         }
         else if (origBands.empty() == false)
         {
            endBand = origBands.back().getOriginalNumber() + 1;
         }

         unsigned int bandSkip = 0;
         if (mpBandSkipFactor != NULL)
         {
            bandSkip = *mpBandSkipFactor;
         }

         // Get the bad bands from the file
         vector<unsigned int> badBands;
         if (mpBadBandsFile != NULL)
         {
            string filename = *mpBadBandsFile;
            if (filename.empty() == false)
            {
               FILE* pFile = fopen(filename.c_str(), "rb");
               if (pFile != NULL)
               {
                  char line[1024];
                  while (fgets(line, 1024, pFile) != NULL)
                  {
                     unsigned int bandNumber = 0;

                     int iValues = sscanf(line, "%u", &bandNumber);
                     if (iValues == 1)
                     {
                        badBands.push_back(bandNumber);
                     }
                  }

                  fclose(pFile);
               }
            }
         }

         vector<DimensionDescriptor> bands;
         for (unsigned int i = 0; i < origBands.size(); ++i)
         {
            DimensionDescriptor bandDim = origBands[i];
            unsigned int originalNumber = bandDim.getOriginalNumber() + 1;
            if ((originalNumber >= startBand) && (originalNumber <= endBand))
            {
               bool bBad = false;
               for (unsigned int j = 0; j < badBands.size(); ++j)
               {
                  unsigned int badBandNumber = badBands[j];
                  if (originalNumber == badBandNumber)
                  {
                     bBad = true;
                     break;
                  }
               }

               if (bBad == false)
               {
                  bands.push_back(bandDim);
                  i += bandSkip;
               }
            }
         }

         pRasterDescriptor->setBands(bands);
      }

      // X pixel size
      if (mpPixelSizeX != NULL)
      {
         pRasterDescriptor->setXPixelSize(*mpPixelSizeX);
      }

      // Y pixel size
      if (mpPixelSizeY != NULL)
      {
         pRasterDescriptor->setYPixelSize(*mpPixelSizeY);
      }

      // Units
      if ((mpUnitsName != NULL) || (mpUnitsType != NULL) ||
         (mpUnitsScale != NULL) || (mpUnitsRangeMin != NULL) || (mpUnitsRangeMax != NULL))
      {
         const Units* pOrigUnits = pRasterDescriptor->getUnits();

         FactoryResource<Units> pUnits;
         VERIFY(pUnits.get() != NULL);

         // Name
         if (mpUnitsName != NULL)
         {
            pUnits->setUnitName(*mpUnitsName);
         }
         else if (pOrigUnits != NULL)
         {
            pUnits->setUnitName(pOrigUnits->getUnitName());
         }

         // Type
         if (mpUnitsType != NULL)
         {
            pUnits->setUnitType(*mpUnitsType);
         }
         else if (pOrigUnits != NULL)
         {
            pUnits->setUnitType(pOrigUnits->getUnitType());
         }

         // Scale
         if (mpUnitsScale != NULL)
         {
            pUnits->setScaleFromStandard(*mpUnitsScale);
         }
         else if (pOrigUnits != NULL)
         {
            pUnits->setScaleFromStandard(pOrigUnits->getScaleFromStandard());
         }

         // Range minimum
         if (mpUnitsRangeMin != NULL)
         {
            pUnits->setRangeMin(*mpUnitsRangeMin);
         }
         else if (pOrigUnits != NULL)
         {
            pUnits->setRangeMin(pOrigUnits->getRangeMin());
         }

         // Range maximum
         if (mpUnitsRangeMax != NULL)
         {
            pUnits->setRangeMax(*mpUnitsRangeMax);
         }
         else if (pOrigUnits != NULL)
         {
            pUnits->setRangeMax(pOrigUnits->getRangeMax());
         }

         pRasterDescriptor->setUnits(pUnits.get());
      }

      // Display mode
      if (mpDisplayMode != NULL)
      {
         pRasterDescriptor->setDisplayMode(*mpDisplayMode);
      }

      // Display bands
      // Gray
      if (mpGrayBand != NULL)
      {
         DimensionDescriptor band = pRasterDescriptor->getOriginalBand(*mpGrayBand - 1);
         pRasterDescriptor->setDisplayBand(GRAY, band);
      }

      // Red
      if (mpRedBand != NULL)
      {
         DimensionDescriptor band = pRasterDescriptor->getOriginalBand(*mpRedBand - 1);
         pRasterDescriptor->setDisplayBand(RED, band);
      }

      // Green
      if (mpGreenBand != NULL)
      {
         DimensionDescriptor band = pRasterDescriptor->getOriginalBand(*mpGreenBand - 1);
         pRasterDescriptor->setDisplayBand(GREEN, band);
      }

      // Blue
      if (mpBlueBand != NULL)
      {
         DimensionDescriptor band = pRasterDescriptor->getOriginalBand(*mpBlueBand - 1);
         pRasterDescriptor->setDisplayBand(BLUE, band);
      }
   }
   else if (pSignatureDescriptor != NULL)
   {
      if (mpComponentName != NULL)
      {
         const Units* pOrigUnits = pSignatureDescriptor->getUnits(*mpComponentName);
         FactoryResource<Units> pUnits;
         if (pOrigUnits != NULL)
         {
            *pUnits = *pOrigUnits;
         }
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
         pSignatureDescriptor->setUnits(*mpComponentName, pUnits.get());
      }
   }

   reportComplete();
   return true;
}

bool EditDataDescriptor::extractInputArgs(PlugInArgList* pInArgList)
{
   bool bSuccess = ModelItems::extractInputArgs(pInArgList);
   if (bSuccess == false)
   {
      return false;
   }

   PlugInArg* pArg = NULL;

   // Data descriptor
   if ((pInArgList->getArg("Data Descriptor", pArg) == false) || (pArg == NULL))
   {
      reportError("Could not read the data descriptor input value!", "88D295E2-F58A-47AD-B5C7-5DC23B2D3EAE");
      return false;
   }

   mpDescriptor = pArg->getPlugInArgValue<DataDescriptor>();
   if (mpDescriptor == NULL)
   {
      reportError("The data descriptor input value is invalid!", "5D2C14DD-503B-4F2F-82ED-24BDE3AC7F0D");
      return false;
   }

   // File descriptor
   if ((pInArgList->getArg("File Descriptor", pArg) == true) && (pArg != NULL))
   {
      mpFileDescriptor = pArg->getPlugInArgValue<FileDescriptor>();
   }

   // Processing location
   if ((pInArgList->getArg("Processing Location", pArg) == true) && (pArg != NULL))
   {
      mpProcessingLocation = pArg->getPlugInArgValue<ProcessingLocation>();
   }

   // Data type
   if ((pInArgList->getArg("Data Type", pArg) == true) && (pArg != NULL))
   {
      mpDataType = pArg->getPlugInArgValue<EncodingType>();
   }

   // Interleave format
   if ((pInArgList->getArg("Interleave Format", pArg) == true) && (pArg != NULL))
   {
      mpInterleave = pArg->getPlugInArgValue<InterleaveFormatType>();
   }

   // Bad values
   if ((pInArgList->getArg("Bad Values", pArg) == true) && (pArg != NULL))
   {
      mpBadValuesStr = pArg->getPlugInArgValue<string>();
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

   // Bad bands file
   if ((pInArgList->getArg("Bad Bands File", pArg) == true) && (pArg != NULL))
   {
      mpBadBandsFile = pArg->getPlugInArgValue<Filename>();
   }

   // X pixel size
   if ((pInArgList->getArg("X Pixel Size", pArg) == true) && (pArg != NULL))
   {
      mpPixelSizeX = pArg->getPlugInArgValue<double>();
   }

   // Y pixel size
   if ((pInArgList->getArg("Y Pixel Size", pArg) == true) && (pArg != NULL))
   {
      mpPixelSizeY = pArg->getPlugInArgValue<double>();
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

   // Display mode
   if ((pInArgList->getArg("Display Mode", pArg) == true) && (pArg != NULL))
   {
      mpDisplayMode = pArg->getPlugInArgValue<DisplayMode>();
   }

   // Gray band
   if ((pInArgList->getArg("Gray Display Band", pArg) == true) && (pArg != NULL))
   {
      mpGrayBand = pArg->getPlugInArgValue<unsigned int>();
   }

   // Red band
   if ((pInArgList->getArg("Red Display Band", pArg) == true) && (pArg != NULL))
   {
      mpRedBand = pArg->getPlugInArgValue<unsigned int>();
   }

   // Green band
   if ((pInArgList->getArg("Green Display Band", pArg) == true) && (pArg != NULL))
   {
      mpGreenBand = pArg->getPlugInArgValue<unsigned int>();
   }

   // Blue band
   if ((pInArgList->getArg("Blue Display Band", pArg) == true) && (pArg != NULL))
   {
      mpBlueBand = pArg->getPlugInArgValue<unsigned int>();
   }

   // signature component name for units
   if ((pInArgList->getArg("Component Name", pArg) == true) && (pArg != NULL))
   {
      mpComponentName = pArg->getPlugInArgValue<string>();
   }

   return true;
}
