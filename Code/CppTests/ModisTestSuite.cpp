/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "FileFinder.h"
#include "GeoreferenceDescriptor.h"
#include "ImportDescriptor.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "StringUtilities.h"
#include "TestCase.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "TypeConverter.h"
#include "TypesFile.h"

#include <string>
#include <vector>

class ModisL1bImporterTestCase : public TestCase
{
public:
   ModisL1bImporterTestCase() :
      TestCase("Import")
   {}

   bool run()
   {
      bool success = true;

      // For each file in the Modis test directory, import the file with each interleave format,
      // processing location, and raster conversion
      std::vector<InterleaveFormatType> interleaves = StringUtilities::getAllEnumValues<InterleaveFormatType>();
      std::vector<ProcessingLocation> processingLocations = StringUtilities::getAllEnumValues<ProcessingLocation>();

      std::vector<std::string> rasterConversions(3);
      rasterConversions[0] = "NoConversion";
      rasterConversions[1] = "ConvertToRadiance";
      rasterConversions[2] = "ConvertToReflectance";

      FactoryResource<FileFinder> pFileFinder;
      issearf(pFileFinder.get() != NULL);

      std::string directory = TestUtilities::getTestDataPath() + "Modis";
      issearf(pFileFinder->findFile(directory, "*.hdf", false));

      int numFilesImported = 0;
      while (pFileFinder->findNextFile() == true)
      {
         std::string filename;
         issearf(pFileFinder->getFullPath(filename));
         issearf(filename.empty() == false);

         std::cout << " Importing file: " << filename << std::endl;

         for (std::vector<InterleaveFormatType>::iterator interleaveIter = interleaves.begin();
            interleaveIter != interleaves.end();
            ++interleaveIter)
         {
            std::cout << "    Interleave = " << StringUtilities::toDisplayString(*interleaveIter) << std::endl;

            for (std::vector<ProcessingLocation>::iterator processingIter = processingLocations.begin();
               processingIter != processingLocations.end();
               ++processingIter)
            {
               std::cout << "       Processing Location = " << StringUtilities::toDisplayString(*processingIter) <<
                  std::endl;

               for (std::vector<std::string>::iterator conversionIter = rasterConversions.begin();
                  conversionIter != rasterConversions.end();
                  ++conversionIter)
               {
                  std::cout << "          Raster Conversion = " << *conversionIter << "..." << std::endl;

                  // Import the data in batch mode to avoid creating a view
                  issearf(importModisData(filename, *interleaveIter, *processingIter, *conversionIter) == true);

                  // Delete the top-level raster element so the file can be imported again
                  Service<ModelServices> pModel;

                  DataElement* pElement = pModel->getElement(filename, TypeConverter::toString<RasterElement>(), NULL);
                  if (pElement != NULL)   // Will be NULL for interleave conversions with on-disk read-only
                  {
                     issearf(pModel->destroyElement(pElement) == true);
                  }
               }
            }
         }

         ++numFilesImported;
      }

      issea(numFilesImported == 7);
      return success;
   }

protected:
   bool importModisData(const std::string& filename, InterleaveFormatType interleave,
                        ProcessingLocation processingLocation, const std::string& rasterConversion)
   {
      if (filename.empty() == true)
      {
         return false;
      }

      bool success = true;

      // Create the importer
      ImporterResource pImporter("MODIS L1B Importer", filename);

      // Update the import parameters in the raster data descriptors
      std::vector<ImportDescriptor*> importDescriptors = pImporter->getImportDescriptors();
      issearf(importDescriptors.empty() == false);

      for (std::vector<ImportDescriptor*>::iterator iter = importDescriptors.begin();
         iter != importDescriptors.end();
         ++iter)
      {
         ImportDescriptor* pImportDescriptor = *iter;
         issearf(pImportDescriptor != NULL);

         RasterDataDescriptor* pDescriptor =
            dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
         issearf(pDescriptor != NULL);

         // Interleave format
         pDescriptor->setInterleaveFormat(interleave);

         // Processing location
         pDescriptor->setProcessingLocation(processingLocation);

         if ((pDescriptor->getParent() == NULL) && (pDescriptor->getParentDesignator().empty() == true))
         {
            // Raster conversion
            DynamicObject* pMetadata = pDescriptor->getMetadata();
            issearf(pMetadata != NULL);
            issearf(pMetadata->setAttribute("Raster Conversion Type", rasterConversion));

            if ((rasterConversion == "ConvertToRadiance") || (rasterConversion == "ConvertToReflectance"))
            {
               pDescriptor->setDataType(FLT4BYTES);
               pDescriptor->setValidDataTypes(std::vector<EncodingType>(1, FLT4BYTES));
            }
         }
      }

      // Execute the importer
      if ((interleave != BSQ) && (processingLocation == ON_DISK_READ_ONLY))
      {
         // Interleave conversions with on-disk read-only are not supported, so importing should fail
         issearf(pImporter->execute() == false);
      }
      else
      {
         issearf(pImporter->execute() == true);
      }

      return success;
   }
};

class ModisL1bImporterChipTestCase : public TestCase
{
public:
   ModisL1bImporterChipTestCase() :
      TestCase("ImportChip")
   {}

   bool run()
   {
      bool success = true;

      std::string filename = TestUtilities::getTestDataPath() + "Modis/MYD021KM.A2010091.0000.005.2010092140854.hdf";
      ImporterResource pImporter("MODIS L1B Importer", filename, NULL, false);

      std::vector<ImportDescriptor*> importDescriptors = pImporter->getImportDescriptors();
      issearf(importDescriptors.size() == 9);

      // Iterate over each data set setting a row and column subset
      for (std::vector<ImportDescriptor*>::iterator iter = importDescriptors.begin();
         iter != importDescriptors.end();
         ++iter)
      {
         ImportDescriptor* pImportDescriptor = *iter;
         issearf(pImportDescriptor != NULL);

         RasterDataDescriptor* pDescriptor =
            dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
         issearf(pDescriptor != NULL);

         // Specify a row subset
         std::vector<DimensionDescriptor> rows = pDescriptor->getRows();
         issearf(rows.size() > 50);

         DimensionDescriptor rowStart = *(rows.begin() + 50);
         DimensionDescriptor rowStop = *(rows.end() - 50);

         std::vector<DimensionDescriptor> subsetRows = RasterUtilities::subsetDimensionVector(rows, rowStart,
            rowStop, 3);
         pDescriptor->setRows(subsetRows);

         // Specify a column subset
         std::vector<DimensionDescriptor> columns = pDescriptor->getColumns();
         issearf(columns.size() > 50);

         DimensionDescriptor columnStart = *(columns.begin() + 50);
         DimensionDescriptor columnStop = *(columns.end() - 50);

         std::vector<DimensionDescriptor> subsetColumns = RasterUtilities::subsetDimensionVector(columns,
            columnStart, columnStop, 2);
         pDescriptor->setColumns(subsetColumns);

         if ((pDescriptor->getParent() == NULL) && (pDescriptor->getParentDesignator().empty() == true))
         {
            // Enable auto-georeferencing
            GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
            issearf(pGeorefDescriptor != NULL);
            pGeorefDescriptor->setGeoreferenceOnImport(true);
            pGeorefDescriptor->setGeoreferencePlugInName("MODIS Georeference");

            // Execute
            issearf(pImporter->execute() == false);      // Fails because georeferencing with row and/or column
                                                         // subsets is not allowed

            // Disable auto-georeferencing
            pGeorefDescriptor->setGeoreferenceOnImport(false);
         }
         else
         {
            const std::string& elementName = pDescriptor->getName();
            if ((elementName == "Latitude") || (elementName == "Longitude"))
            {
               pDescriptor->setRows(rows);
               pDescriptor->setColumns(columns);
            }
         }
      }

      // Import the raster data
      issearf(pImporter->execute() == true);

      // Check for the proper number of imported rows and columns in each imported element
      std::vector<DataElement*> rasterElements = pImporter->getImportedElements();
      issearf(rasterElements.size() == 9);

      DataElement* pParentElement = NULL;

      for (std::vector<DataElement*>::iterator elementIter = rasterElements.begin();
         elementIter != rasterElements.end();
         ++elementIter)
      {
         DataElement* pElement = *elementIter;
         issearf(pElement != NULL);

         const RasterDataDescriptor* pImportedDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pElement->getDataDescriptor());
         issearf(pImportedDescriptor != NULL);

         if (pElement->getParent() == NULL)
         {
            issearf(pImportedDescriptor->getRowCount() == 483);
            issearf(pImportedDescriptor->getColumnCount() == 419);
            pParentElement = pElement;
         }
         else
         {
            const std::string& elementName = pImportedDescriptor->getName();
            if ((elementName == "Latitude") || (elementName == "Longitude"))
            {
               issearf(pImportedDescriptor->getRowCount() == 406);
               issearf(pImportedDescriptor->getColumnCount() == 271);
            }
            else
            {
               issearf(pImportedDescriptor->getRowCount() == 77);
               issearf(pImportedDescriptor->getColumnCount() == 58);
            }
         }
      }

      // Destroy the top-level raster element
      issearf(pParentElement != NULL);
      issearf(Service<ModelServices>()->destroyElement(pParentElement) == true);

      return success;
   }
};

class ModisL1bImporterMetadataTestCase : public TestCase
{
public:
   ModisL1bImporterMetadataTestCase() :
      TestCase("ImportMetadata")
   {}

   bool run()
   {
      bool success = true;

      std::string filename = TestUtilities::getTestDataPath() + "Modis/MOD021KM.A2010091.0000.005.2010092155638.hdf";
      ImporterResource pImporter("MODIS L1B Importer", filename);

      std::vector<ImportDescriptor*> importDescriptors = pImporter->getImportDescriptors();
      issearf(importDescriptors.size() == 9);

      for (std::vector<ImportDescriptor*>::iterator iter = importDescriptors.begin();
         iter != importDescriptors.end();
         ++iter)
      {
         ImportDescriptor* pImportDescriptor = *iter;
         issearf(pImportDescriptor != NULL);

         const RasterDataDescriptor* pDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
         issearf(pDescriptor != NULL);

         const DynamicObject* pMetadata = pDescriptor->getMetadata();
         issearf(pMetadata != NULL);

         if ((pDescriptor->getParent() == NULL) && (pDescriptor->getParentDesignator().empty() == true))
         {
            // Check for attributes in various areas of the top-level raster element metadata that
            // represent various data types and values
            try
            {
               // General metadata
               issearf(dv_cast<unsigned int>(pMetadata->getAttributeByPath("Bit QA Flags Change")) == 8200);
               issearf(dv_cast<unsigned char>(pMetadata->getAttributeByPath(
                  "Noise in Scan Mirror Thermistor Average")) == 18);
               issearf(dv_cast<char>(pMetadata->getAttributeByPath("Doors and Screens Configuration")) == -32);
               issearf(dv_cast<int>(pMetadata->getAttributeByPath("Number of Scans")) == 203);

               std::vector<unsigned char> noise = dv_cast<std::vector<unsigned char> >(
                  pMetadata->getAttributeByPath("Noise in Black Body Thermistors"));
               issearf(noise.size() == 12);
               issearf(noise[0] == 1);
               issearf(noise[1] == 1);
               issearf(noise[2] == 1);
               issearf(noise[3] == 1);
               issearf(noise[4] == 3);
               issearf(noise[5] == 1);
               issearf(noise[6] == 4);
               issearf(noise[7] == 6);
               issearf(noise[8] == 1);
               issearf(noise[9] == 4);
               issearf(noise[10] == 1);
               issearf(noise[11] == 2);

               // Band metadata
               issearf(dv_cast<std::string>(pMetadata->getAttributeByPath("EV_250_Aggr1km_RefSB/radiance_units")) ==
                  "Watts/m^2/micrometer/steradian");

               std::vector<unsigned short> validRange = dv_cast<std::vector<unsigned short> >(
                  pMetadata->getAttributeByPath("EV_1KM_Emissive/valid_range"));
               issearf(validRange.size() == 2);
               issearf(validRange[0] == 0);
               issearf(validRange[1] == 32767);

               // Archive metadata
               issearf(dv_cast<std::string>(pMetadata->getAttributeByPath(
                  "ArchiveMetadata.0/ARCHIVEDMETADATA/PROJECT/INSTRUMENTNAME")) ==
                  "Moderate Resolution Imaging SpectroRadiometer");

               // Core metadata
               issearf(dv_cast<std::string>(pMetadata->getAttributeByPath(
                  "CoreMetadata.0/INVENTORYMETADATA/ADDITIONALATTRIBUTES/ADDITIONALATTRIBUTESCONTAINER.1/"
                  "ADDITIONALATTRIBUTENAME.1")) == "AveragedBlackBodyTemperature");
               issearf(dv_cast<std::string>(pMetadata->getAttributeByPath(
                  "CoreMetadata.0/INVENTORYMETADATA/ADDITIONALATTRIBUTES/ADDITIONALATTRIBUTESCONTAINER.9/"
                  "ADDITIONALATTRIBUTENAME.9")) == "NadirPointing");

               // Struct metadata
               issearf(dv_cast<std::string>(pMetadata->getAttributeByPath(
                  "StructMetadata.0/SwathStructure/SWATH_1/Dimension/Dimension_1/DimensionName")) == "Band_250M");
               issearf(dv_cast<int>(pMetadata->getAttributeByPath(
                  "StructMetadata.0/SwathStructure/SWATH_1/Dimension/Dimension_6/Size")) == 1354);

               const DynamicObject* pObject = dv_cast<DynamicObject>(&pMetadata->getAttributeByPath(
                  "StructMetadata.0/SwathStructure/SWATH_1/IndexDimensionMap"));
               issearf(pObject != NULL);
               issearf(pObject->getNumAttributes() == 0);
            }
            catch (const std::bad_cast&)
            {
               issearf(false);
            }
         }
         else
         {
            // These attributes should be present for all child raster elements
            issearf(pMetadata->getAttributeByPath("_FillValue").isValid() == true);
            issearf(pMetadata->getAttributeByPath("frame_numbers").isValid() == true);
            issearf(pMetadata->getAttributeByPath("units").isValid() == true);
            issearf(pMetadata->getAttributeByPath("valid_range").isValid() == true);
         }
      }

      return success;
   }
};

class ModisGeoreferenceTestCase : public TestCase
{
public:
   ModisGeoreferenceTestCase() :
      TestCase("Georeference")
   {}

   bool run()
   {
      bool success = true;

      std::string filename = TestUtilities::getTestDataPath() + "Modis/MOD02HKM.A2010060.0035.005.2010060084407.hdf";
      ImporterResource pImporter("MODIS L1B Importer", filename);

      std::vector<ImportDescriptor*> importDescriptors = pImporter->getImportDescriptors();
      issearf(importDescriptors.size() == 3);

      for (std::vector<ImportDescriptor*>::iterator iter = importDescriptors.begin();
         iter != importDescriptors.end();
         ++iter)
      {
         ImportDescriptor* pImportDescriptor = *iter;
         issearf(pImportDescriptor != NULL);

         RasterDataDescriptor* pDescriptor =
            dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
         issearf(pDescriptor != NULL);

         // Can only georeference the top-level raster element
         if ((pDescriptor->getParent() != NULL) || (pDescriptor->getParentDesignator().empty() == false))
         {
            continue;
         }

         // Enable auto-georeferencing
         GeoreferenceDescriptor* pGeorefDescriptor = pDescriptor->getGeoreferenceDescriptor();
         issearf(pGeorefDescriptor != NULL);

         pGeorefDescriptor->setGeoreferenceOnImport(true);
         pGeorefDescriptor->setGeoreferencePlugInName("MODIS Georeference");

         // Import the raster data
         issearf(pImporter->execute() == true);

         // Get the top level raster element
         Service<ModelServices> pModel;
         RasterElement* pRaster = dynamic_cast<RasterElement*>(pModel->getElement(pDescriptor));
         issearf(pRaster != NULL);

         // Check the coordinate transformations at the raster corners, which is invalid because the MODIS
         // Georeference plug-in does not perform extrapolation outside of the latitude/longitude raster extents
         LocationType pixelLowerLeft(0.0, 0.0);
         LocationType pixelUpperLeft(0.0, 4060.0);
         LocationType pixelUpperRight(2708.0, 4060.0);
         LocationType pixelLowerRight(2708.0, 0.0);

         bool accurate = false;
         LocationType geoLowerLeft = pRaster->convertPixelToGeocoord(pixelLowerLeft, false, &accurate);
         issearf(accurate == false);

         LocationType geoUpperLeft = pRaster->convertPixelToGeocoord(pixelUpperLeft, false, &accurate);
         issearf(accurate == false);

         LocationType geoUpperRight = pRaster->convertPixelToGeocoord(pixelUpperRight, false, &accurate);
         issearf(accurate == false);

         LocationType geoLowerRight = pRaster->convertPixelToGeocoord(pixelLowerRight, false, &accurate);
         issearf(accurate == false);

         // Check the coordinate transformations at the latitude/longitude raster corners
         pixelLowerLeft = LocationType(0.5, 0.5);
         pixelUpperLeft = LocationType(0.5, 4058.49999999); // Specify a little less than the actual coordinate since
                                                            // the georeference plug-in will not be able to determine
                                                            // valid coordinates for the bilinear interpolation
         pixelUpperRight = LocationType(2706.49999999, 4058.49999999);
         pixelLowerRight = LocationType(2706.49999999, 0.5);

         geoLowerLeft = pRaster->convertPixelToGeocoord(pixelLowerLeft, false, &accurate);
         issearf(accurate == true);
         issearf(fabs(geoLowerLeft.mX - 69.18072) < 1e-5);
         issearf(fabs(geoLowerLeft.mY - 140.38902) < 1e-5);

         geoUpperLeft = pRaster->convertPixelToGeocoord(pixelUpperLeft, false, &accurate);
         issearf(accurate == true);
         issearf(fabs(geoUpperLeft.mX - 51.267277) < 1e-5);
         issearf(fabs(geoUpperLeft.mY - 140.80424) < 1e-5);

         geoUpperRight = pRaster->convertPixelToGeocoord(pixelUpperRight, false, &accurate);
         issearf(accurate == true);
         issearf(fabs(geoUpperRight.mX - 46.70654) < 1e-5);
         issearf(fabs(geoUpperRight.mY - 172.32478) < 1e-5);

         geoLowerRight = pRaster->convertPixelToGeocoord(pixelLowerRight, false, &accurate);
         issearf(accurate == true);
         issearf(fabs(geoLowerRight.mX - 61.95871) < 1e-5);
         issearf(fabs(geoLowerRight.mY + 169.84145) < 1e-5);

         // Convert back to pixel coordinates
         pixelLowerLeft = pRaster->convertGeocoordToPixel(geoLowerLeft, false, &accurate);
         issearf(accurate == false);   // The RMS error on the inverse polynomial places this coordinate outside
                                       // of the data set
         issearf(fabs(pixelLowerLeft.mX - 16.82840) < 1e-5);
         issearf(fabs(pixelLowerLeft.mY + 7.33822) < 1e-5);

         pixelUpperLeft = pRaster->convertGeocoordToPixel(geoUpperLeft, false, &accurate);
         issearf(accurate == false);   // The RMS error on the inverse polynomial places this coordinate outside
                                       // of the data set
         issearf(fabs(pixelUpperLeft.mX - 12.00126) < 1e-5);
         issearf(fabs(pixelUpperLeft.mY - 4070.06339) < 1e-5);

         pixelUpperRight = pRaster->convertGeocoordToPixel(geoUpperRight, false, &accurate);
         issearf(accurate == false);   // The RMS error on the inverse polynomial places this coordinate outside
                                       // of the data set
         issearf(fabs(pixelUpperRight.mX - 2659.66799) < 1e-5);
         issearf(fabs(pixelUpperRight.mY - 4070.29031) < 1e-5);

         pixelLowerRight = pRaster->convertGeocoordToPixel(geoLowerRight, false, &accurate);
         issearf(accurate == false);   // The RMS error on the inverse polynomial places this coordinate outside
                                       // of the data set
         issearf(fabs(pixelLowerRight.mX - 6075932.97707) < 1e-5);
         issearf(fabs(pixelLowerRight.mY + 86136.62797) < 1e-5);

         // Destroy the raster element
         issearf(pModel->destroyElement(pRaster) == true);
      }

      return success;
   }
};

class ModisTestSuite : public TestSuiteNewSession
{
public:
   ModisTestSuite() :
      TestSuiteNewSession("Modis")
   {
      addTestCase(new ModisL1bImporterTestCase);
      addTestCase(new ModisL1bImporterChipTestCase);
      addTestCase(new ModisL1bImporterMetadataTestCase);
      addTestCase(new ModisGeoreferenceTestCase);
   }
};

REGISTER_SUITE(ModisTestSuite)
