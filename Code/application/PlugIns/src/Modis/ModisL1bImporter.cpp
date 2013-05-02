/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "AppVersion.h"
#include "BadValues.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "Filename.h"
#include "GeoreferenceDescriptor.h"
#include "Hdf4Attribute.h"
#include "Hdf4Dataset.h"
#include "Hdf4File.h"
#include "Hdf4Group.h"
#include "Hdf4Utilities.h"
#include "HdfEosMetadataParser.h"
#include "ImportDescriptor.h"
#include "ModisGeoreference.h"
#include "ModisL1bImporter.h"
#include "ModisL1bImportOptionsWidget.h"
#include "ModisPager.h"
#include "ModisUtilities.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialDataWindow.h"
#include "SpecialMetadata.h"
#include "StringUtilities.h"
#include "TypeConverter.h"
#include "Units.h"

#include <mfhdf.h>

#include <limits>
#include <vector>

#define MODIS_GEOREFERENCE_NAME "MODIS Georeference"
#define LATITUDE_ELEMENT_NAME "Latitude"
#define LONGITUDE_ELEMENT_NAME "Longitude"

REGISTER_PLUGIN_BASIC(OpticksModis, ModisL1bImporter);

ModisL1bImporter::ModisL1bImporter() :
   mpOptionsWidget(NULL)
{
   setName("MODIS L1B Importer");
   setExtensions("MODIS L1B Files (*.hdf)");
   setCreator("Ball Aerospace & Technologies Corp.");
   setCopyright(APP_COPYRIGHT);
   setDescription("Imports MODIS L1B HDF-EOS files.");
   setShortDescription(std::string());
   setDescriptorId("{D057223C-D883-4934-AEA5-001BDC1C30E7}");
   setVersion(APP_VERSION_NUMBER);
   setProductionStatus(APP_IS_PRODUCTION_RELEASE);
}

ModisL1bImporter::~ModisL1bImporter()
{}

unsigned char ModisL1bImporter::getFileAffinity(const std::string& filename)
{
   // Determine if this is a MODIS L1B file by checking the MODIS product name value in the metadata
   Hdf4File hdfFile(filename);
   if (getFileData(hdfFile) == true)
   {
      FactoryResource<DynamicObject> pMetadata;
      if (pMetadata.get() != NULL)
      {
         populateAttributes(hdfFile.getAttributes(), pMetadata.get());

         std::string modisName = ModisUtilities::getModisProductName(pMetadata.get());
         if ((modisName == "MOD02QKM") ||    // MODIS/Terra 250m resolution
            (modisName == "MOD02HKM") ||     // MODIS/Terra 500m resolution
            (modisName == "MOD021KM") ||     // MODIS/Terra 1km resolution
            (modisName == "MYD02QKM") ||     // MODIS/Aqua 250m resolution
            (modisName == "MYD02HKM") ||     // MODIS/Aqua 500m resolution
            (modisName == "MYD021KM"))       // MODIS/Aqua 1km resolution
         {
            return Importer::CAN_LOAD;
         }
      }
   }

   return Importer::CAN_NOT_LOAD;
}

std::vector<ImportDescriptor*> ModisL1bImporter::getImportDescriptors(const std::string& filename)
{
   Hdf4File hdfFile(filename);
   if (getFileData(hdfFile) == false)
   {
      return std::vector<ImportDescriptor*>();
   }

   HdfUtilities::Hdf4FileResource pFile(filename.c_str());
   if (pFile.get() == NULL || *pFile == FAIL)
   {
      return std::vector<ImportDescriptor*>();
   }

   Hdf4Group* pRootGroup = hdfFile.getRootGroup();
   if (pRootGroup == NULL)
   {
      return std::vector<ImportDescriptor*>();
   }

   std::vector<ImportDescriptor*> importDescriptors;
   DynamicObject* pMetadata = NULL;

   // Create an import descriptor for the raster data set
   ImportDescriptorResource pRasterImportDescriptor(filename, TypeConverter::toString<RasterElement>(), NULL);
   if (pRasterImportDescriptor.get() != NULL)
   {
      RasterDataDescriptor* pDescriptor =
         dynamic_cast<RasterDataDescriptor*>(pRasterImportDescriptor->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         pMetadata = pDescriptor->getMetadata();
         if (pMetadata != NULL)
         {
            // Metadata
            populateAttributes(hdfFile.getAttributes(), pMetadata);

            // Get various descriptor values from the metadata attributes
            unsigned int bandCount = 0;
            unsigned int rowCount = 0;
            unsigned int columnCount = 0;
            EncodingType dataType;
            std::vector<std::string> bandNames;
            std::vector<double> startWavelengths;
            std::vector<double> endWavelengths;
            FactoryResource<BadValues> pBadValues;
            double rangeMin = std::numeric_limits<double>::max();
            double rangeMax = -std::numeric_limits<double>::max();

            std::vector<ModisUtilities::HdfDatasetInfo> bandDatasets = ModisUtilities::getBandDatasets(pMetadata);
            for (std::vector<ModisUtilities::HdfDatasetInfo>::const_iterator iter = bandDatasets.begin();
               iter != bandDatasets.end();
               ++iter)
            {
               HdfUtilities::Hdf4DatasetResource pHdfDataset(*pFile, iter->first);
               if ((pHdfDataset.get() != NULL) && (*pHdfDataset != FAIL))
               {
                  int32 numDims = 0;
                  int32 dimensions[MAX_VAR_DIMS];
                  int32 hdfDataType = 0;
                  int32 numAttr = 0;

                  if (SDgetinfo(*pHdfDataset, NULL, &numDims, dimensions, &hdfDataType, &numAttr) == SUCCEED)
                  {
                     if (numDims >= 3)
                     {
                        std::vector<std::string> currentBandNames;
                        const Hdf4Element* pElement = pRootGroup->getElement(iter->first);
                        if (pElement != NULL)
                        {
                           // Add the band dataset metadata to the parent element metadata
                           FactoryResource<DynamicObject> pBandMetadata;
                           populateAttributes(pElement->getAttributes(), pBandMetadata.get());

                           pMetadata->setAttributeByPath(iter->first, *pBandMetadata.get());

                           // Band names
                           Hdf4Attribute* pBandNamesAttribute = pElement->getAttribute(BAND_NAMES);
                           if (pBandNamesAttribute != NULL)
                           {
                              std::string bandNamesText;
                              if (pBandNamesAttribute->getValueAs(bandNamesText) == true)
                              {
                                 currentBandNames = StringUtilities::split(bandNamesText, ',');
                              }
                           }

                           // Bad values
                           Hdf4Attribute* pBadValueAttribute = pElement->getAttribute(FILL_VALUE);
                           if (pBadValueAttribute != NULL)
                           {
                              const DataVariant& badValue = pBadValueAttribute->getVariant();
                              std::string badValueText = badValue.toDisplayString();
                              if (badValueText.empty() == false)
                              {
                                 pBadValues->addBadValue(badValueText);
                              }
                           }

                           // Units ranges
                           Hdf4Attribute* pRangeAttribute = pElement->getAttribute(VALID_RANGE);
                           if (pRangeAttribute != NULL)
                           {
                              const DataVariant& rangeValue = pRangeAttribute->getVariant();
                              if (rangeValue.isValid() == true)
                              {
                                 std::string rangeText = rangeValue.toDisplayString();
                                 if (rangeText.empty() == false)
                                 {
                                    std::vector<double> range =
                                       StringUtilities::fromDisplayString<std::vector<double> >(rangeText);
                                    if (range.size() == 2)
                                    {
                                       rangeMin = std::min(rangeMin, range[0]);
                                       rangeMax = std::max(rangeMax, range[1]);
                                    }
                                 }
                              }
                           }
                        }

                        unsigned int currentBandCount = static_cast<unsigned int>(dimensions[0]);
                        for (unsigned int i = 0; i < static_cast<unsigned int>(dimensions[0]); ++i)
                        {
                           // Band names
                           unsigned int bandIndex = bandCount + i;
                           unsigned int bandNumber = bandIndex;

                           std::string bandName = "Band " + StringUtilities::toDisplayString(bandIndex + 1);
                           if (currentBandNames.size() == currentBandCount)
                           {
                              bandName = "Band " + currentBandNames[i];
                              bandNumber = StringUtilities::fromDisplayString<unsigned int>(currentBandNames[i]) - 1;
                           }

                           bandNames.push_back(bandName);

                           // Wavelengths
                           std::pair<double, double> bandWavelengths = ModisUtilities::getBandWavelengths(bandNumber);
                           startWavelengths.push_back(bandWavelengths.first);
                           endWavelengths.push_back(bandWavelengths.second);
                        }

                        bandCount += currentBandCount;

                        // Check for a mismatch in the number of rows between the band datasets
                        if ((rowCount != 0) && (rowCount != static_cast<unsigned int>(dimensions[1])))
                        {
                           return std::vector<ImportDescriptor*>();
                        }

                        if (rowCount == 0)
                        {
                           rowCount = static_cast<unsigned int>(dimensions[1]);
                        }

                        // Check for a mismatch in the number of columns between the band datasets
                        if ((columnCount != 0) && (columnCount != static_cast<unsigned int>(dimensions[2])))
                        {
                           return std::vector<ImportDescriptor*>();
                        }

                        if (columnCount == 0)
                        {
                           columnCount = static_cast<unsigned int>(dimensions[2]);
                        }
                     }

                     EncodingType currentDataType = HdfUtilities::hdf4TypeToEncodingType(hdfDataType);
                     if (currentDataType != dataType)
                     {
                        if (dataType.isValid() == true)
                        {
                           return std::vector<ImportDescriptor*>();
                        }

                        dataType = currentDataType;
                     }
                  }
               }
            }

            // Size
            pDescriptor->setRows(RasterUtilities::generateDimensionVector(rowCount));
            pDescriptor->setColumns(RasterUtilities::generateDimensionVector(columnCount));
            pDescriptor->setBands(RasterUtilities::generateDimensionVector(bandCount));

            // Data type
            pDescriptor->setDataType(dataType);
            pDescriptor->setValidDataTypes(std::vector<EncodingType>(1, dataType));

            // Interleave format
            pDescriptor->setInterleaveFormat(BSQ);

            // Bad values
            if (pBadValues->empty() == false)
            {
               pDescriptor->setBadValues(pBadValues.get());
            }

            // Band names
            pMetadata->setAttributeByPath(BAND_NAMES_METADATA_PATH, bandNames);

            // Wavelengths
            pMetadata->setAttributeByPath(START_WAVELENGTHS_METADATA_PATH, startWavelengths);
            pMetadata->setAttributeByPath(END_WAVELENGTHS_METADATA_PATH, endWavelengths);

            // Georeference
            GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
            if (pGeorefDescriptor != NULL)
            {
               // Set the MODIS Georeference plug-in as the default Georeference plug-in
               pGeorefDescriptor->setGeoreferencePlugInName(MODIS_GEOREFERENCE_NAME);
            }

            // File descriptor
            RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(
               RasterUtilities::generateAndSetFileDescriptor(pDescriptor, filename, std::string(), BIG_ENDIAN_ORDER));
            if (pFileDescriptor == NULL)
            {
               return std::vector<ImportDescriptor*>();
            }

            // Upgrade the data type to support applying scale and offset values.  This must occur
            // after generating the file descriptor so that the correct bits per element is set.
            ModisUtilities::RasterConversionType rasterConversion = ModisUtilities::getRasterConversion(pMetadata);
            if ((rasterConversion.isValid() == true) && (rasterConversion != ModisUtilities::NO_CONVERSION))
            {
               pDescriptor->setDataType(FLT4BYTES);
               pDescriptor->setValidDataTypes(std::vector<EncodingType>(1, FLT4BYTES));
            }

            // Units
            FactoryResource<Units> pUnits;
            switch (rasterConversion)
            {
               case ModisUtilities::NO_CONVERSION:    // Fall through
               default:
                  pUnits->setUnitType(DIGITAL_NO);
                  break;

               case ModisUtilities::CONVERT_TO_RADIANCE:
                  pUnits->setUnitType(RADIANCE);
                  break;

               case ModisUtilities::CONVERT_TO_REFLECTANCE:
                  pUnits->setUnitType(REFLECTANCE);
                  break;
            }

            pUnits->setRangeMin(rangeMin);
            pUnits->setRangeMax(rangeMax);

            pDescriptor->setUnits(pUnits.get());
            pFileDescriptor->setUnits(pUnits.get());

            // Add the import descriptor to the list of datasets available to import
            importDescriptors.push_back(pRasterImportDescriptor.release());
         }
      }
   }

   // Create import descriptors for all geolocation data as child data sets of the raster data
   std::vector<std::string> parentDesignator(1, filename);

   std::map<std::string, std::string> geolocationDatasets = ModisUtilities::getGeolocationDatasets(pMetadata);
   for (std::map<std::string, std::string>::iterator iter = geolocationDatasets.begin();
      iter != geolocationDatasets.end();
      ++iter)
   {
      std::string datasetName = iter->first;

      const Hdf4Dataset* pDataset = dynamic_cast<const Hdf4Dataset*>(pRootGroup->getElement(datasetName));
      if ((pDataset == NULL) || (pDataset->isCompoundDataset() == true))
      {
         continue;
      }

      ImportDescriptorResource pImportDescriptor(datasetName, TypeConverter::toString<RasterElement>(),
         parentDesignator);
      if (pImportDescriptor.get() == NULL)
      {
         continue;
      }

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         DynamicObject* pGeolocationMetadata = pDescriptor->getMetadata();
         if (pGeolocationMetadata != NULL)
         {
            // Metadata
            populateAttributes(pDataset->getAttributes(), pGeolocationMetadata);

            // Size
            HdfUtilities::Hdf4DatasetResource pHdfDataset(*pFile, datasetName);
            if ((pHdfDataset.get() != NULL) && (*pHdfDataset != FAIL))
            {
               int32 numDims = 0;
               int32 dimensions[MAX_VAR_DIMS];
               int32 hdfDataType = 0;
               int32 numAttr = 0;

               if (SDgetinfo(*pHdfDataset, NULL, &numDims, dimensions, &hdfDataType, &numAttr) == SUCCEED)
               {
                  unsigned int bandCount = 0;
                  unsigned int rowCount = 0;
                  unsigned int columnCount = 0;

                  if (numDims == 2)
                  {
                     bandCount = 1;
                     rowCount = static_cast<unsigned int>(dimensions[0]);
                     columnCount = static_cast<unsigned int>(dimensions[1]);
                  }
                  else if (numDims >= 3)
                  {
                     bandCount = static_cast<unsigned int>(dimensions[0]);
                     rowCount = static_cast<unsigned int>(dimensions[1]);
                     columnCount = static_cast<unsigned int>(dimensions[2]);
                  }

                  if ((bandCount > 0) && (rowCount > 0) && (columnCount > 0))
                  {
                     // Generate the rows and columns
                     std::vector<DimensionDescriptor> rows = RasterUtilities::generateDimensionVector(rowCount);
                     std::vector<DimensionDescriptor> columns = RasterUtilities::generateDimensionVector(columnCount);
                     std::vector<DimensionDescriptor> bands = RasterUtilities::generateDimensionVector(bandCount);

                     // Update the row and column original number
                     try
                     {
                        // Rows
                        std::string rowOffsetPath = STRUCT_METADATA +
                           std::string("/SwathStructure/SWATH_1/DimensionMap/DimensionMap_1/Offset");
                        std::string rowIncrementPath = STRUCT_METADATA +
                           std::string("/SwathStructure/SWATH_1/DimensionMap/DimensionMap_1/Increment");

                        int rowStartValue = dv_cast<int>(pMetadata->getAttributeByPath(rowOffsetPath));
                        int rowSkipFactor = dv_cast<int>(pMetadata->getAttributeByPath(rowIncrementPath));

                        if ((rowStartValue > -1) && (rowSkipFactor > -1))
                        {
                           unsigned int originalNumber = static_cast<unsigned int>(rowStartValue);
                           for (std::vector<DimensionDescriptor>::size_type i = 0; i < rows.size(); ++i)
                           {
                              rows[i].setOriginalNumber(originalNumber);
                              originalNumber += rowSkipFactor;
                           }
                        }

                        // Columns
                        std::string columnOffsetPath = STRUCT_METADATA +
                           std::string("/SwathStructure/SWATH_1/DimensionMap/DimensionMap_2/Offset");
                        std::string columnIncrementPath = STRUCT_METADATA +
                           std::string("/SwathStructure/SWATH_1/DimensionMap/DimensionMap_2/Increment");

                        int columnStartValue = dv_cast<int>(pMetadata->getAttributeByPath(columnOffsetPath));
                        int columnSkipFactor = dv_cast<int>(pMetadata->getAttributeByPath(columnIncrementPath));

                        if ((columnStartValue > -1) && (columnSkipFactor > -1))
                        {
                           unsigned int originalNumber = static_cast<unsigned int>(columnStartValue);
                           for (std::vector<DimensionDescriptor>::size_type i = 0; i < columns.size(); ++i)
                           {
                              columns[i].setOriginalNumber(originalNumber);
                              originalNumber += columnSkipFactor;
                           }
                        }
                     }
                     catch (const std::bad_cast&)
                     {
                        continue;
                     }

                     // Set the rows and columns into the data descriptor
                     pDescriptor->setRows(rows);
                     pDescriptor->setColumns(columns);
                     pDescriptor->setBands(bands);
                  }
               }
            }

            // Data type
            EncodingType encodingType;
            pDataset->getDataEncoding(encodingType);
            if (encodingType.isValid() == true)
            {
               pDescriptor->setDataType(encodingType);
               pDescriptor->setValidDataTypes(std::vector<EncodingType>(1, encodingType));
            }

            // Interleave format
            pDescriptor->setInterleaveFormat(BSQ);

            // Bad values
            Hdf4Attribute* pBadValueAttribute = pDataset->getAttribute(FILL_VALUE);
            if (pBadValueAttribute != NULL)
            {
               const DataVariant& badValue = pBadValueAttribute->getVariant();
               std::string badValueText = badValue.toDisplayString();
               if (badValueText.empty() == false)
               {
                  FactoryResource<BadValues> pBadValues;
                  pBadValues->addBadValue(badValueText);
                  pDescriptor->setBadValues(pBadValues.get());
               }
            }

            // Georeference
            GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
            if (pGeorefDescriptor != NULL)
            {
               // Only the parent raster data can be georeferenced
               pGeorefDescriptor->setGeoreferenceOnImport(false);
            }

            // File descriptor
            RasterFileDescriptor* pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(
               RasterUtilities::generateAndSetFileDescriptor(pDescriptor, filename, iter->second, BIG_ENDIAN_ORDER));
            if (pFileDescriptor == NULL)
            {
               continue;
            }

            // Units
            FactoryResource<Units> pUnits;

            Hdf4Attribute* pUnitsAttribute = pDataset->getAttribute(UNITS);
            if (pUnitsAttribute != NULL)
            {
               std::string units;
               if ((pUnitsAttribute->getValueAs(units) == true) && (units.empty() == false))
               {
                  pUnits->setUnitType(CUSTOM_UNIT);
                  pUnits->setUnitName(units);
               }
            }

            Hdf4Attribute* pRangeAttribute = pDataset->getAttribute(VALID_RANGE);
            if (pRangeAttribute != NULL)
            {
               const DataVariant& rangeValue = pRangeAttribute->getVariant();
               if (rangeValue.isValid() == true)
               {
                  std::string rangeText = rangeValue.toDisplayString();
                  if (rangeText.empty() == false)
                  {
                     std::vector<double> range = StringUtilities::fromDisplayString<std::vector<double> >(rangeText);
                     if (range.size() == 2)
                     {
                        pUnits->setRangeMin(range[0]);
                        pUnits->setRangeMax(range[1]);
                     }
                  }
               }
            }

            Hdf4Attribute* pScaleAttribute = pDataset->getAttribute(SCALE_FACTOR);
            if (pScaleAttribute != NULL)
            {
               double scaleFactor = 1.0;
               if (pScaleAttribute->getValueAs(scaleFactor) == true)
               {
                  pUnits->setScaleFromStandard(scaleFactor);
               }
            }

            pDescriptor->setUnits(pUnits.get());
            pFileDescriptor->setUnits(pUnits.get());

            // Upgrade the data type to support applying the unit scale factor
            if (pUnits->getScaleFromStandard() != 1.0)
            {
               pDescriptor->setDataType(FLT4BYTES);
               pDescriptor->setValidDataTypes(std::vector<EncodingType>(1, FLT4BYTES));
            }

            importDescriptors.push_back(pImportDescriptor.release());
         }
      }
   }

   return importDescriptors;
}

QWidget* ModisL1bImporter::getImportOptionsWidget(DataDescriptor* pDescriptor)
{
   RasterDataDescriptor* pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(pDescriptor);
   if (pRasterDescriptor == NULL)
   {
      return NULL;
   }

   if ((pRasterDescriptor->getParent() != NULL) || (pRasterDescriptor->getParentDesignator().empty() == false))
   {
      return NULL;
   }

   // Create the widget
   if (mpOptionsWidget.get() == NULL)
   {
      mpOptionsWidget.reset(new ModisL1bImportOptionsWidget(pRasterDescriptor));
   }

   return mpOptionsWidget.get();
}

bool ModisL1bImporter::validate(const DataDescriptor* pDescriptor,
                                const std::vector<const DataDescriptor*>& importedDescriptors,
                                std::string& errorMessage) const
{
   bool isValid = Hdf4ImporterShell::validate(pDescriptor, importedDescriptors, errorMessage);
   if (isValid == false)
   {
      ValidationTest errorTest = getValidationError();
      if ((errorTest == NO_ROW_SUBSETS) || (errorTest == NO_COLUMN_SUBSETS))
      {
         errorMessage = "Row and column subsets are not supported to preserve the accuracy of "
            "the georeferencing results.";
      }
   }
   else
   {
      const RasterDataDescriptor* pRasterDescriptor = dynamic_cast<const RasterDataDescriptor*>(pDescriptor);
      VERIFY(pRasterDescriptor != NULL);

      // If a units scale factor is applied, check if the data type is not FLT4BYTES
      const Units* pUnits = pRasterDescriptor->getUnits();
      VERIFY(pUnits != NULL);

      double scaleFactor = pUnits->getScaleFromStandard();
      EncodingType dataType = pRasterDescriptor->getDataType();

      if ((scaleFactor != 1.0) && (dataType != FLT4BYTES))
      {
         errorMessage = "The data type is invalid to support applying a unit scale factor.";
         return false;
      }

      // Perform checks on the top-level raster element
      std::vector<std::string> parentDesignator = pRasterDescriptor->getParentDesignator();
      if (parentDesignator.empty() == true)
      {
         // Perform checks specific to georeferencing with MODIS Georeference
         const GeoreferenceDescriptor* pGeorefDescriptor = pRasterDescriptor->getGeoreferenceDescriptor();
         VERIFY(pGeorefDescriptor != NULL);

         if ((pGeorefDescriptor->getGeoreferenceOnImport() == true) &&
            (pGeorefDescriptor->getGeoreferencePlugInName() == MODIS_GEOREFERENCE_NAME))
         {
            // Check if a spatial subset of the raster element is being imported
            const RasterFileDescriptor* pFileDescriptor =
               dynamic_cast<const RasterFileDescriptor*>(pRasterDescriptor->getFileDescriptor());
            VERIFY(pFileDescriptor != NULL);

            unsigned int fileRows = pFileDescriptor->getRowCount();
            unsigned int fileColumns = pFileDescriptor->getColumnCount();
            unsigned int importedRows = pRasterDescriptor->getRowCount();
            unsigned int importedColumns = pRasterDescriptor->getColumnCount();

            if ((importedRows != fileRows) || (importedColumns != fileColumns))
            {
               errorMessage = "Row and column subsets are not supported when georeferencing with " +
                  std::string(MODIS_GEOREFERENCE_NAME) + " because the accuracy of the results may "
                  "be greatly decreased.";
               return false;
            }

            // Check if the 'Latitude' and 'Longitude' child data sets are not being imported
            const RasterDataDescriptor* pLatitudeDescriptor = NULL;
            const RasterDataDescriptor* pLongitudeDescriptor = NULL;

            for (std::vector<const DataDescriptor*>::const_iterator iter = importedDescriptors.begin();
               iter != importedDescriptors.end();
               ++iter)
            {
               const RasterDataDescriptor* pCurrentDescriptor = dynamic_cast<const RasterDataDescriptor*>(*iter);
               if (pCurrentDescriptor != NULL)
               {
                  const std::string& elementName = pCurrentDescriptor->getName();
                  if (elementName == LATITUDE_ELEMENT_NAME)
                  {
                     pLatitudeDescriptor = pCurrentDescriptor;
                  }
                  else if (elementName == LONGITUDE_ELEMENT_NAME)
                  {
                     pLongitudeDescriptor = pCurrentDescriptor;
                  }
               }
            }

            if (pLatitudeDescriptor == NULL)
            {
               errorMessage = "The '" + std::string(LATITUDE_ELEMENT_NAME) + "' data set must also be imported "
                  "to georeference with " + std::string(MODIS_GEOREFERENCE_NAME) + ".";
               return false;
            }

            if (pLongitudeDescriptor == NULL)
            {
               errorMessage = "The '" + std::string(LONGITUDE_ELEMENT_NAME) + "' data set must also be imported "
                  "to georeference with " + std::string(MODIS_GEOREFERENCE_NAME) + ".";
               return false;
            }

            // Check if the 'Latitude' and 'Longitude' child data sets do not have matching rows and columns
            const std::vector<DimensionDescriptor>& latitudeRows = pLatitudeDescriptor->getRows();
            const std::vector<DimensionDescriptor>& latitudeColumns = pLatitudeDescriptor->getColumns();
            const std::vector<DimensionDescriptor>& longitudeRows = pLongitudeDescriptor->getRows();
            const std::vector<DimensionDescriptor>& longitudeColumns = pLongitudeDescriptor->getColumns();

            if ((latitudeRows != longitudeRows) || (latitudeColumns != longitudeColumns))
            {
               errorMessage = "The '" + std::string(LATITUDE_ELEMENT_NAME) + "' and '" +
                  std::string(LONGITUDE_ELEMENT_NAME) + "' data sets do not have matching rows and columns.";
               return false;
            }
         }

         // If the raster pixel values will be converted, check if the data type is not FLT4BYTES
         const DynamicObject* pMetadata = pRasterDescriptor->getMetadata();

         ModisUtilities::RasterConversionType rasterConversion = ModisUtilities::getRasterConversion(pMetadata);
         if ((rasterConversion.isValid() == true) && (rasterConversion != ModisUtilities::NO_CONVERSION) &&
            (dataType != FLT4BYTES))
         {
            errorMessage = "The data type is invalid to support converting the raster pixel values.";
            return false;
         }

         // Check if the unit type does not match the selected raster conversion units
         UnitType unitType = pUnits->getUnitType();
         if (((rasterConversion == ModisUtilities::NO_CONVERSION) && (unitType != DIGITAL_NO)) ||
            ((rasterConversion == ModisUtilities::CONVERT_TO_RADIANCE) && (unitType != RADIANCE)) ||
            ((rasterConversion == ModisUtilities::CONVERT_TO_REFLECTANCE) && (unitType != REFLECTANCE)))
         {
            // Report this message as a warning and do not return false, because this
            // problem does not prevent successful loading of the raster data
            errorMessage = "The '" + StringUtilities::toDisplayString(unitType) + "' unit type does not match the "
               "units of the imported raster pixel values.";
         }

         // Check if a unit scale factor will be applied and the raster pixels will be converted
         if ((scaleFactor != 1.0) && (rasterConversion != ModisUtilities::NO_CONVERSION))
         {
            // Report this message as a warning and do not return false, because this
            // problem does not prevent successful loading of the raster data
            if (errorMessage.empty() == false)
            {
               errorMessage += "\n";
            }

            errorMessage += "An additional units scale factor will be applied after the raster pixel values "
               "are automatically converted to " + StringUtilities::toDisplayString(unitType) + ".";
         }

         // Report a warning if converting to reflectance that the emissive bands will not be converted
         if (rasterConversion == ModisUtilities::CONVERT_TO_REFLECTANCE)
         {
            // Only report the warning if the user is overriding the default conversion type
            std::string rasterConversionText = ModisL1bImporter::getSettingRasterConversion();

            ModisUtilities::RasterConversionType defaultRasterConversion =
               StringUtilities::fromXmlString<ModisUtilities::RasterConversionType>(rasterConversionText);
            if (defaultRasterConversion != ModisUtilities::CONVERT_TO_REFLECTANCE)
            {
               // Only report the warning if the user is loading emissive bands
               const std::vector<DimensionDescriptor>& bands = pRasterDescriptor->getBands();
               VERIFY(bands.empty() == false);

               DimensionDescriptor lastBand = bands.back();
               if (lastBand.getOnDiskNumber() > 21)
               {
                  if (errorMessage.empty() == false)
                  {
                     errorMessage += "\n";
                  }

                  errorMessage += "The emissive bands (Bands 20-25 and 27-36) will not be converted to reflectance.";
               }
            }
         }
      }
   }

   return isValid;
}

bool ModisL1bImporter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   if (Hdf4ImporterShell::execute(pInArgList, pOutArgList) == false)
   {
      return false;
   }

   // Georeference the parent raster element if both the latitude and longitude child elements have finished loading
   RasterElement* pRaster = getRasterElement();
   if (pRaster != NULL)
   {
      const std::string& elementName = pRaster->getName();

      DataElement* pParent = pRaster->getParent();
      if (pParent == NULL)
      {
         // Parent elements are always imported before child elements, so add the parent data set to the map that is
         // used to determine when to perform georeference.  The data set needs to be added to the map here instead
         // of in getImportDescriptors() to successfully perform georeference when loading from the MRU file list.
         mLatLonImported[elementName] = std::make_pair(false, false);
      }
      else if ((elementName == LATITUDE_ELEMENT_NAME) || (elementName == LONGITUDE_ELEMENT_NAME))
      {
         const std::string& parentName = pParent->getName();

         std::map<std::string, std::pair<bool, bool> >::iterator iter = mLatLonImported.find(parentName);
         if (iter != mLatLonImported.end())
         {
            if (elementName == LATITUDE_ELEMENT_NAME)
            {
               iter->second.first = true;
            }
            else if (elementName == LONGITUDE_ELEMENT_NAME)
            {
               iter->second.second = true;
            }

            if ((iter->second.first == true) && (iter->second.second == true))
            {
               RasterElement* pParent = dynamic_cast<RasterElement*>(pRaster->getParent());
               if ((pParent != NULL) && (pParent->isGeoreferenced() == false))
               {
                  // Do not georeference if the user disabled auto-georeference in the import options dialog
                  const RasterDataDescriptor* pDescriptor =
                     dynamic_cast<const RasterDataDescriptor*>(pParent->getDataDescriptor());
                  VERIFY(pDescriptor != NULL);

                  const GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
                  VERIFY(pGeorefDescriptor != NULL);

                  if ((pGeorefDescriptor->getGeoreferenceOnImport() == true) &&
                     (pGeorefDescriptor->getGeoreferencePlugInName() == MODIS_GEOREFERENCE_NAME))
                  {
                     // Execute the MODIS Georeference plug-in
                     Progress* pProgress = getProgress();

                     ExecutableResource geoPlugIn(MODIS_GEOREFERENCE_NAME, std::string(), pProgress, true);
                     geoPlugIn->getInArgList().setPlugInArgValue(Executable::DataElementArg(), pParent);
                     if (geoPlugIn->execute() == true)
                     {
                        // Display the latitude/longitude layer if running the importer in interactive mode
                        if (isBatch() == false)
                        {
                           SpatialDataView* pView = NULL;

                           SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(
                              Service<DesktopServices>()->getWindow(pParent->getName(), SPATIAL_DATA_WINDOW));
                           if (pWindow != NULL)
                           {
                              pView = pWindow->getSpatialDataView();
                           }

                           bool createLayer = pGeorefDescriptor->getCreateLayer();
                           std::string layerName = pGeorefDescriptor->getLayerName();
                           bool displayLayer = pGeorefDescriptor->getDisplayLayer();
                           GeocoordType geocoordType = pGeorefDescriptor->getGeocoordType();
                           DmsFormatType latLonFormat = pGeorefDescriptor->getLatLonFormat();

                           ExecutableResource geoDisplayPlugIn("Georeference", std::string(), pProgress, true);
                           PlugInArgList& inArgList = geoDisplayPlugIn->getInArgList();
                           VERIFY(inArgList.setPlugInArgValue(Executable::DataElementArg(), pParent));
                           VERIFY(inArgList.setPlugInArgValue(Executable::ViewArg(), pView));
                           VERIFY(inArgList.setPlugInArgValue("Results Name", &layerName));
                           VERIFY(inArgList.setPlugInArgValue("Create Layer", &createLayer));
                           VERIFY(inArgList.setPlugInArgValue("Display Layer", &displayLayer));
                           VERIFY(inArgList.setPlugInArgValue("Coordinate Type", &geocoordType));
                           VERIFY(inArgList.setPlugInArgValue("Latitude/Longitude Format", &latLonFormat));
                           if (geoDisplayPlugIn->execute() == false)
                           {
                              if (pProgress != NULL)
                              {
                                 pProgress->updateProgress("Could not create the latitude/longitude layer for the "
                                    "georeference data.", 0, WARNING);
                              }
                           }
                        }
                     }
                     else if (pProgress != NULL)
                     {
                        // If the user aborted the georeference plug-in, report this as a warning; any warnings
                        // or errors from the georeference plug-in will automatically be displayed in the dialog
                        std::string message;
                        int percent = 0;
                        ReportingLevel level;
                        pProgress->getProgress(message, percent, level);

                        if (level == ABORT)
                        {
                           if (message.empty() == false)
                           {
                              message = "Could not georeference the data set.";
                           }

                           pProgress->updateProgress(message, 0, WARNING);
                        }
                     }
                  }
               }

               mLatLonImported.erase(iter);
            }
         }
      }
   }

   return true;
}

int ModisL1bImporter::getValidationTest(const DataDescriptor* pDescriptor) const
{
   int validationTest = Hdf4ImporterShell::getValidationTest(pDescriptor);
   if (pDescriptor != NULL)
   {
      const std::string& elementName = pDescriptor->getName();
      if ((elementName == LATITUDE_ELEMENT_NAME) || (elementName == LONGITUDE_ELEMENT_NAME))
      {
         validationTest |= NO_SUBSETS;
      }
   }

   return validationTest;
}

void ModisL1bImporter::performGeoreference() const
{
   RasterElement* pRaster = getRasterElement();
   if ((pRaster != NULL) && (pRaster->getParent() == NULL))
   {
      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
      VERIFYNRV(pDescriptor != NULL);

      GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
      VERIFYNRV(pGeorefDescriptor != NULL);

      // Prevent the MODIS Georeference plug-in from running now by not calling the base class so that the
      // latitude and longitude data can first be loaded.  Georeferencing will be performed in execute().
      if ((pGeorefDescriptor->getGeoreferenceOnImport() == true) &&
         (pGeorefDescriptor->getGeoreferencePlugInName() == MODIS_GEOREFERENCE_NAME))
      {
         return;
      }
   }

   Hdf4ImporterShell::performGeoreference();
}

SpatialDataView* ModisL1bImporter::createView() const
{
   RasterElement* pRaster = getRasterElement();
   if (pRaster != NULL)
   {
      DataElement* pParent = pRaster->getParent();
      if (pParent == NULL)
      {
         return Hdf4ImporterShell::createView();
      }

      if (isBatch() == false)
      {
         Service<DesktopServices> pDesktop;

         SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pParent->getName(),
            SPATIAL_DATA_WINDOW));
         if (pWindow != NULL)
         {
            return pWindow->getSpatialDataView();
         }
      }
   }

   return NULL;
}

bool ModisL1bImporter::createRasterPager(RasterElement* pRaster) const
{
   if (pRaster == NULL)
   {
      return false;
   }

   // Create the pager
   ExecutableResource pPlugIn("MODIS Pager");
   if (pPlugIn.get() == NULL)
   {
      return false;
   }

   // Set the input arg values
   FactoryResource<Filename> pFilename;
   pFilename->setFullPathAndName(pRaster->getFilename());

   PlugInArgList& inArgList = pPlugIn->getInArgList();
   inArgList.setPlugInArgValue(CachedPager::PagedFilenameArg(), pFilename.get());
   inArgList.setPlugInArgValue(CachedPager::PagedElementArg(), pRaster);

   // To set the custom input arg values, get a pointer to the raster element being imported since
   // the given raster element may be a temporary raster element used by RasterElementImporterShell
   // when the processing location is IN_MEMORY, which does not contain the metadata or the parent
   // information (see RasterUtilities::generateUnchippedRasterDataDescriptor())
   RasterElement* pImportRaster = getRasterElement();
   if (pImportRaster != NULL)
   {
      // Metadata
      DynamicObject* pMetadata = pImportRaster->getMetadata();
      VERIFY(pMetadata != NULL);

      inArgList.setPlugInArgValue(ModisPager::MetadataArg(), pMetadata);

      // Args for the top-level raster element
      if (pImportRaster->getParent() == NULL)
      {
         // Raster pixel conversion
         ModisUtilities::RasterConversionType rasterConversion = ModisUtilities::getRasterConversion(pMetadata);
         if (rasterConversion.isValid() == true)
         {
            inArgList.setPlugInArgValue(ModisUtilities::rasterConversionArg(), &rasterConversion);
         }
      }
      else     // Args for the child raster elements
      {
         // HDF dataset name
         std::string datasetName = pImportRaster->getName();
         inArgList.setPlugInArgValue(ModisPager::DatasetNameArg(), &datasetName);
      }
   }

   // Execute the pager to parse the input args and open the HDF file
   if (pPlugIn->execute() == false)
   {
      return false;
   }

   // Set the pager in the raster element
   ModisPager* pPager = dynamic_cast<ModisPager*>(pPlugIn->getPlugIn());
   if (pPager != NULL)
   {
      pRaster->setPager(pPager);
      pPlugIn->releasePlugIn();
      return true;
   }

   return false;
}

void ModisL1bImporter::populateAttributes(const Hdf4Element::AttributeContainer& attributes,
                                          DynamicObject* pMetadata) const
{
   HdfEosMetadataParser parser;
   for (std::map<std::string, Hdf4Attribute*>::const_iterator iter = attributes.begin();
      iter != attributes.end();
      ++iter)
   {
      Hdf4Attribute* pAttribute = iter->second;
      if (pAttribute != NULL)
      {
         const DataVariant& attributeValue = pAttribute->getVariant();

         const std::string& name = pAttribute->getName();
         if ((attributeValue.getTypeName() == TypeConverter::toString<std::string>()) &&
            (name == ARCHIVE_METADATA || name == CORE_METADATA || name == STRUCT_METADATA))
         {
            // These HDF attributes will be stored as separate DynamicObjects and require further parsing
            try
            {
               std::string value;
               if (attributeValue.getValue(value) == true)
               {
                  FactoryResource<DynamicObject> pChild;
                  if (parser.convert(pChild.get(), value) == true)
                  {
                     pMetadata->setAttributeByPath(name, *pChild);
                  }
               }
            }
            catch (const AssertException&)
            {
               // No clean up to perform, and a message has already been logged
            }
         }
         else
         {
            // Add the attribute to the metadata
            pMetadata->setAttributeByPath(name, attributeValue);
         }
      }
   }
}
