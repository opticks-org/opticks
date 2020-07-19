/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "Testable.h"

// to access a plug-in
#include "ConnectionManager.h"
#include "DataDescriptor.h"
#include "DataVariant.h"
#include "DynamicObject.h"
#include "PlugInManagerServices.h"
#include "ApplicationServices.h"
#include "ImportDescriptor.h"
#include "ModelServicesImp.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterFileDescriptor.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"

#include <string>
#include <sstream>

using namespace std;

class GeoTiffImporterTestCase : public TestCase
{
public:
   GeoTiffImporterTestCase() : TestCase("Importer") {}
   bool run()
   {
      string fileName = TestUtilities::getTestDataPath() + "landsat6band.tif";
      ImporterResource impResource("GeoTIFF Importer", fileName);
      bool success = true;

      vector<ImportDescriptor*> importDescriptors = impResource->getImportDescriptors();
      issearf(importDescriptors.size() == 1);

      ImportDescriptor* pImportDescriptor = importDescriptors.front();
      issearf(pImportDescriptor != NULL);

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issearf(pDescriptor != NULL);

      RasterFileDescriptor *pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
      issearf(pFileDescriptor != NULL);

      issearf(impResource->execute() == true);
      issearf(pDescriptor->getRowCount() == 512);
      issearf(pDescriptor->getColumnCount() == 512);
      issearf(pDescriptor->getBandCount() == 6);
      issearf(pDescriptor->getDataType() == INT1UBYTE);
      issearf(pDescriptor->getInterleaveFormat() == BIP);
      issearf(pFileDescriptor->getHeaderBytes() == 0);
      issearf(pFileDescriptor->getTrailerBytes() == 0);
      issearf(pFileDescriptor->getPrelineBytes() == 0);
      issearf(pFileDescriptor->getPostlineBytes() == 0);
      issearf(pFileDescriptor->getPrebandBytes() == 0);
      issearf(pFileDescriptor->getPostbandBytes() == 0);
      issearf(pFileDescriptor->getInterleaveFormat() == BSQ);

      return success;
   }
};

class GeoTiffImporterChipTestCase : public TestCase
{
public:
   GeoTiffImporterChipTestCase() : TestCase("ImporterChip") {}
   bool run()
   {
      // Test subcubing a file with rowsPerStrip != 1
      // This was added to test COMETIV-805
      string fileName = TestUtilities::getTestDataPath() + "ikonos_msi_wrightpatt.tif";
      ImporterResource impResource("GeoTIFF Importer", fileName);
      bool success = true;

      vector<ImportDescriptor*> importDescriptors = impResource->getImportDescriptors();
      issearf(importDescriptors.size() == 1);

      ImportDescriptor* pImportDescriptor = importDescriptors.front();
      issearf(pImportDescriptor != NULL);

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issearf(pDescriptor != NULL);

      RasterFileDescriptor *pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
      issearf(pFileDescriptor != NULL);

      vector<DimensionDescriptor> rows = pDescriptor->getRows();
      issearf(rows.size() == 4204);
      rows.erase(rows.begin());
      issearf(rows.size() == 4203);
      pDescriptor->setRows(rows);

      issearf(impResource->execute() == true);
      issearf(pDescriptor->getRowCount() == 4203);
      issearf(pDescriptor->getColumnCount() == 4198);
      issearf(pDescriptor->getBandCount() == 4);
      issearf(pDescriptor->getDataType() == INT2UBYTES);
      issearf(pDescriptor->getInterleaveFormat() == BIP);
      issearf(pFileDescriptor->getHeaderBytes() == 0);
      issearf(pFileDescriptor->getTrailerBytes() == 0);
      issearf(pFileDescriptor->getPrelineBytes() == 0);
      issearf(pFileDescriptor->getPostlineBytes() == 0);
      issearf(pFileDescriptor->getPrebandBytes() == 0);
      issearf(pFileDescriptor->getPostbandBytes() == 0);
      issearf(pFileDescriptor->getInterleaveFormat() == BIP);

      return success;
   }
};

class GeoTiffImporterMetadataTestCase : public TestCase
{
public:
   GeoTiffImporterMetadataTestCase() : TestCase("ImporterMetadata") {}
   bool run()
   {
      // This file was chosen because it is a tiled GeoTIFF where numColumns % tileWidth != 0
      string fileName = TestUtilities::getTestDataPath() + "ikonos_msi_wrightpatt_rotated.tif";
      ImporterResource impResource("GeoTIFF Importer", fileName);
      bool success = true;

      vector<ImportDescriptor*> importDescriptors = impResource->getImportDescriptors();
      issearf(importDescriptors.size() == 1);

      ImportDescriptor* pImportDescriptor = importDescriptors.front();
      issearf(pImportDescriptor != NULL);

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issearf(pDescriptor != NULL);

      RasterFileDescriptor *pFileDescriptor = dynamic_cast<RasterFileDescriptor*>(pDescriptor->getFileDescriptor());
      issearf(pFileDescriptor != NULL);

      issearf(impResource->execute() == true);
      issearf(pDescriptor->getRowCount() == 4204);
      issearf(pDescriptor->getColumnCount() == 4198);
      issearf(pDescriptor->getBandCount() == 4);
      issearf(pDescriptor->getDataType() == INT2UBYTES);
      issearf(pDescriptor->getInterleaveFormat() == BIP);
      issearf(pFileDescriptor->getHeaderBytes() == 0);
      issearf(pFileDescriptor->getTrailerBytes() == 0);
      issearf(pFileDescriptor->getPrelineBytes() == 0);
      issearf(pFileDescriptor->getPostlineBytes() == 0);
      issearf(pFileDescriptor->getPrebandBytes() == 0);
      issearf(pFileDescriptor->getPostbandBytes() == 0);
      issearf(pFileDescriptor->getInterleaveFormat() == BIP);

      try
      {
         DynamicObject* pMetadata = pDescriptor->getMetadata();
         issearf(pMetadata != NULL);
         issearf(dv_cast<unsigned short>(pMetadata->getAttributeByPath("TIFF/BitsPerSample")) == 16);
         issearf(dv_cast<unsigned short>(pMetadata->getAttributeByPath("TIFF/Compression")) == 1);
         issearf(dv_cast<unsigned short>(pMetadata->getAttributeByPath("TIFF/Orientation")) == 1);
         issearf(dv_cast<unsigned short>(pMetadata->getAttributeByPath("TIFF/PhotometricInterpretation")) == 2);
         issearf(dv_cast<unsigned short>(pMetadata->getAttributeByPath("TIFF/PlanarConfiguration")) == 1);
         issearf(dv_cast<unsigned short>(pMetadata->getAttributeByPath("TIFF/SamplesPerPixel")) == 4);
         issearf(dv_cast<unsigned int>(pMetadata->getAttributeByPath("TIFF/ImageLength")) == 4204);
         issearf(dv_cast<unsigned int>(pMetadata->getAttributeByPath("TIFF/ImageWidth")) == 4198);
         issearf(dv_cast<unsigned int>(pMetadata->getAttributeByPath("TIFF/TileLength")) == 1024);
         issearf(dv_cast<unsigned int>(pMetadata->getAttributeByPath("TIFF/TileWidth")) == 1024);
         issearf(dv_cast<string>(pMetadata->getAttributeByPath("TIFF/DateTime")) == string("2007:05:08 11:26:49"));
         issearf(dv_cast<string>(pMetadata->getAttributeByPath("TIFF/Software")) ==
            string("Sensor Systems RemoteView"));

         vector<double> geoPixelScale;
         geoPixelScale.push_back(4);
         geoPixelScale.push_back(4);
         geoPixelScale.push_back(0);
         issearf(dv_cast<vector<double> >(pMetadata->getAttributeByPath("TIFF/GeoPixelScale")) == geoPixelScale);
      }
      catch (const bad_cast&)
      {
         issearf(false);
      }

      return success;
   }
};

class GeoTiffTestSuite : public TestSuiteNewSession
{
public:
   GeoTiffTestSuite() : TestSuiteNewSession("GeoTiff")
   {
      addTestCase(new GeoTiffImporterTestCase);
      addTestCase(new GeoTiffImporterChipTestCase);
      addTestCase(new GeoTiffImporterMetadataTestCase);
   }
};

REGISTER_SUITE(GeoTiffTestSuite)
