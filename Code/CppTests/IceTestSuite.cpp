/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVerify.h"
#include "assert.h"
#include "BadValues.h"
#include "Classification.h"
#include "ColorType.h"
#include "ComplexData.h"
#include "ConfigurationSettings.h"
#include "DataAccessorImpl.h"
#include "DateTime.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "DynamicObject.h"
#include "FileResource.h"
#include "ImportDescriptor.h"
#include "Layer.h"
#include "LayerList.h"
#include "ModelServices.h"
#include "ObjectResource.h"
#include "PlugInResource.h"
#include "PseudocolorLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "SpecialMetadata.h"
#include "Statistics.h"
#include "StringUtilities.h"
#include "TestBedTestUtilities.h"
#include "TestCase.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "TypesFile.h"
#include "Units.h"
#include "UtilityServicesImp.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

namespace
{
   ModelResource<RasterElement> batchImportCube(const std::string& importerName, const std::string& filename)
   {
      ModelResource<RasterElement> importElement(reinterpret_cast<RasterElement*>(NULL));
      ImporterResource import(importerName, filename, NULL, true);
      DO_IF(!import->execute(), return importElement);
      vector<DataElement*> importElements = import->getImportedElements();
      DO_IF(importElements.size() != 1, return importElement);
      importElement = ModelResource<RasterElement>(dynamic_cast<RasterElement*>(importElements.front()));
      return importElement;
   }

}
/**
 * Test the loading of Ice files that are version 0.0
 */
class IceVersion0TestCase : public TestCase
{
public:
   IceVersion0TestCase() : TestCase("IceVersion0")
   {
      mDataPath = TestUtilities::getTestDataPath() + "Ice/Version0_0/";
   }

   bool run()
   {
      bool success = true;

      {
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", mDataPath + "cube1024x1024x7x1u-bip.ice.h5"));
         issearf(importElement1.get() != NULL);
         RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
         issearf(importElement1Desc != NULL);
         issearf(importElement1Desc->getRowCount() == 1024);
         issearf(importElement1Desc->getColumnCount() == 1024);
         issearf(importElement1Desc->getBandCount() == 7);
         issearf(importElement1Desc->getDataType() == INT1UBYTE);
         issearf(importElement1Desc->getInterleaveFormat() == BIP);
         issearf(importElement1Desc->getActiveRow(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveRow(1023).getOriginalNumber() == 1023);
         issearf(importElement1Desc->getActiveColumn(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveColumn(1023).getOriginalNumber() == 1023);
         issearf(importElement1Desc->getActiveBand(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveBand(6).getOriginalNumber() == 6);
      }

      {
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", mDataPath + "cube1024x1024x7x1u-bsq.ice.h5"));
         issearf(importElement1.get() != NULL);
         RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
         issearf(importElement1Desc != NULL);
         issearf(importElement1Desc->getRowCount() == 1024);
         issearf(importElement1Desc->getColumnCount() == 1024);
         issearf(importElement1Desc->getBandCount() == 7);
         issearf(importElement1Desc->getDataType() == INT1UBYTE);
         issearf(importElement1Desc->getInterleaveFormat() == BSQ);
         issearf(importElement1Desc->getActiveRow(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveRow(1023).getOriginalNumber() == 1023);
         issearf(importElement1Desc->getActiveColumn(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveColumn(1023).getOriginalNumber() == 1023);
         issearf(importElement1Desc->getActiveBand(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveBand(6).getOriginalNumber() == 6);
      }

      {
         //exported from getTestDataPath() + "/Ice/cube1024x1024x7x1u-bip.ice.h5" with rows 10-900 skip 1, cols 100-500 skip 1, bands 2, 4, 5, 7 in the export dialog 
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", mDataPath + "cube1024x1024x7x1u-bip-chipped.ice.h5"));
         issearf(importElement1.get() != NULL);
         RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
         issearf(importElement1Desc != NULL);
         issearf(importElement1Desc->getRowCount() == 446);
         issearf(importElement1Desc->getColumnCount() == 201);
         issearf(importElement1Desc->getBandCount() == 4);
         issearf(importElement1Desc->getDataType() == INT1UBYTE);
         issearf(importElement1Desc->getInterleaveFormat() == BIP);
         issearf(importElement1Desc->getActiveRow(0).getOriginalNumber() == 9);
         issearf(importElement1Desc->getActiveRow(1).getOriginalNumber() == 11);
         issearf(importElement1Desc->getActiveRow(445).getOriginalNumber() == 899);
         issearf(importElement1Desc->getActiveColumn(0).getOriginalNumber() == 99);
         issearf(importElement1Desc->getActiveColumn(1).getOriginalNumber() == 101);
         issearf(importElement1Desc->getActiveColumn(200).getOriginalNumber() == 499);
         issearf(importElement1Desc->getActiveBand(0).getOriginalNumber() == 1);
         issearf(importElement1Desc->getActiveBand(1).getOriginalNumber() == 3);
         issearf(importElement1Desc->getActiveBand(2).getOriginalNumber() == 4);
         issearf(importElement1Desc->getActiveBand(3).getOriginalNumber() == 6);
      }

      {
         //exported from getTestDataPath() + "/Ice/cube1024x1024x7x1u-bsq.ice.h5" with rows 10-900 skip 1, cols 100-500 skip 1, bands 2, 4, 5, 7 in the export dialog 
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", mDataPath + "cube1024x1024x7x1u-bsq-chipped.ice.h5"));
         issearf(importElement1.get() != NULL);
         RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
         issearf(importElement1Desc != NULL);
         issearf(importElement1Desc->getRowCount() == 446);
         issearf(importElement1Desc->getColumnCount() == 201);
         issearf(importElement1Desc->getBandCount() == 4);
         issearf(importElement1Desc->getDataType() == INT1UBYTE);
         issearf(importElement1Desc->getInterleaveFormat() == BSQ);
         issearf(importElement1Desc->getActiveRow(0).getOriginalNumber() == 9);
         issearf(importElement1Desc->getActiveRow(1).getOriginalNumber() == 11);
         issearf(importElement1Desc->getActiveRow(445).getOriginalNumber() == 899);
         issearf(importElement1Desc->getActiveColumn(0).getOriginalNumber() == 99);
         issearf(importElement1Desc->getActiveColumn(1).getOriginalNumber() == 101);
         issearf(importElement1Desc->getActiveColumn(200).getOriginalNumber() == 499);
         issearf(importElement1Desc->getActiveBand(0).getOriginalNumber() == 1);
         issearf(importElement1Desc->getActiveBand(1).getOriginalNumber() == 3);
         issearf(importElement1Desc->getActiveBand(2).getOriginalNumber() == 4);
         issearf(importElement1Desc->getActiveBand(3).getOriginalNumber() == 6);
      }

      {
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", mDataPath + "cube512x512x7x2s-bsq-littleendian.ice.h5"));
         issearf(importElement1.get() != NULL);
         RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
         issearf(importElement1Desc != NULL);
         issearf(importElement1Desc->getRowCount() == 512);
         issearf(importElement1Desc->getColumnCount() == 512);
         issearf(importElement1Desc->getBandCount() == 7);
         issearf(importElement1Desc->getDataType() == INT2SBYTES);
         issearf(importElement1Desc->getInterleaveFormat() == BSQ);
         issearf(importElement1Desc->getActiveRow(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveRow(511).getOriginalNumber() == 511);
         issearf(importElement1Desc->getActiveColumn(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveColumn(511).getOriginalNumber() == 511);
         issearf(importElement1Desc->getActiveBand(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveBand(6).getOriginalNumber() == 6);
         DataAccessor importData1 = importElement1->getDataAccessor();

         ModelResource<RasterElement> importElement2(batchImportCube("Ice Importer", mDataPath + "cube512x512x7x2s-bsq-bigendian.ice.h5"));
         issearf(importElement2.get() != NULL);
         RasterDataDescriptor* importElement2Desc = dynamic_cast<RasterDataDescriptor*>(importElement2->getDataDescriptor());
         issearf(importElement2Desc != NULL);
         issearf(importElement2Desc->getRowCount() == 512);
         issearf(importElement2Desc->getColumnCount() == 512);
         issearf(importElement2Desc->getBandCount() == 7);
         issearf(importElement2Desc->getDataType() == INT2SBYTES);
         issearf(importElement2Desc->getInterleaveFormat() == BSQ);
         issearf(importElement2Desc->getActiveRow(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveRow(511).getOriginalNumber() == 511);
         issearf(importElement2Desc->getActiveColumn(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveColumn(511).getOriginalNumber() == 511);
         issearf(importElement2Desc->getActiveBand(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveBand(6).getOriginalNumber() == 6);
         DataAccessor importData2 = importElement2->getDataAccessor();
         issearf(importData1.isValid());
         issearf(importData2.isValid());

         importData1->toPixel(0, 0);
         importData2->toPixel(0, 0);
         issearf(*static_cast<short*>(importData1->getColumn()) == -32768);
         issearf(*static_cast<short*>(importData1->getColumn()) == *static_cast<short*>(importData2->getColumn()));

         importData1->toPixel(245, 135);
         importData2->toPixel(245, 135);
         issearf(*static_cast<short*>(importData1->getColumn()) == -15743);
         issearf(*static_cast<short*>(importData1->getColumn()) == *static_cast<short*>(importData2->getColumn()));

         importData1->toPixel(387, 323);
         importData2->toPixel(387, 323);
         issearf(*static_cast<short*>(importData1->getColumn()) == 8856);
         issearf(*static_cast<short*>(importData1->getColumn()) == *static_cast<short*>(importData2->getColumn()));

         importData1->toPixel(501, 236);
         importData2->toPixel(501, 236);
         issearf(*static_cast<short*>(importData1->getColumn()) == 30398);
         issearf(*static_cast<short*>(importData1->getColumn()) == *static_cast<short*>(importData2->getColumn()));
      }

      {
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", mDataPath + "cube512x512x7x4complex-bsq-littleendian.ice.h5"));
         issearf(importElement1.get() != NULL);
         RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
         issearf(importElement1Desc != NULL);
         issearf(importElement1Desc->getRowCount() == 512);
         issearf(importElement1Desc->getColumnCount() == 512);
         issearf(importElement1Desc->getBandCount() == 7);
         issearf(importElement1Desc->getDataType() == INT4SCOMPLEX);
         issearf(importElement1Desc->getInterleaveFormat() == BSQ);
         issearf(importElement1Desc->getActiveRow(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveRow(511).getOriginalNumber() == 511);
         issearf(importElement1Desc->getActiveColumn(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveColumn(511).getOriginalNumber() == 511);
         issearf(importElement1Desc->getActiveBand(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveBand(6).getOriginalNumber() == 6);
         DataAccessor importData1 = importElement1->getDataAccessor();

         ModelResource<RasterElement> importElement2(batchImportCube("Ice Importer", mDataPath + "cube512x512x7x4complex-bsq-bigendian.ice.h5"));
         issearf(importElement2.get() != NULL);
         RasterDataDescriptor* importElement2Desc = dynamic_cast<RasterDataDescriptor*>(importElement2->getDataDescriptor());
         issearf(importElement2Desc != NULL);
         issearf(importElement2Desc->getRowCount() == 512);
         issearf(importElement2Desc->getColumnCount() == 512);
         issearf(importElement2Desc->getBandCount() == 7);
         issearf(importElement2Desc->getDataType() == INT4SCOMPLEX);
         issearf(importElement2Desc->getInterleaveFormat() == BSQ);
         issearf(importElement2Desc->getActiveRow(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveRow(511).getOriginalNumber() == 511);
         issearf(importElement2Desc->getActiveColumn(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveColumn(511).getOriginalNumber() == 511);
         issearf(importElement2Desc->getActiveBand(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveBand(6).getOriginalNumber() == 6);
         DataAccessor importData2 = importElement2->getDataAccessor();
         issearf(importData1.isValid());
         issearf(importData2.isValid());

         importData1->toPixel(319, 235);
         importData2->toPixel(319, 235);
         IntegerComplex* pData1 = static_cast<IntegerComplex*>(importData1->getColumn());
         IntegerComplex* pData2 = static_cast<IntegerComplex*>(importData2->getColumn());
         issearf(pData1->mReal == 7389 && pData1->mImaginary == -32094);
         issearf(pData1->mReal == pData2->mReal && pData1->mImaginary == pData2->mImaginary);

         importData1->toPixel(468, 440);
         importData2->toPixel(468, 440);
         pData1 = static_cast<IntegerComplex*>(importData1->getColumn());
         pData2 = static_cast<IntegerComplex*>(importData2->getColumn());
         issearf(pData1->mReal == -21818 && pData1->mImaginary == 16184);
         issearf(pData1->mReal == pData2->mReal && pData1->mImaginary == pData2->mImaginary);
      }

      {
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", mDataPath + "cube512x512x7x8complex-bsq-littleendian.ice.h5"));
         issearf(importElement1.get() != NULL);
         RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
         issearf(importElement1Desc != NULL);
         issearf(importElement1Desc->getRowCount() == 512);
         issearf(importElement1Desc->getColumnCount() == 512);
         issearf(importElement1Desc->getBandCount() == 7);
         issearf(importElement1Desc->getDataType() == FLT8COMPLEX);
         issearf(importElement1Desc->getInterleaveFormat() == BSQ);
         issearf(importElement1Desc->getActiveRow(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveRow(511).getOriginalNumber() == 511);
         issearf(importElement1Desc->getActiveColumn(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveColumn(511).getOriginalNumber() == 511);
         issearf(importElement1Desc->getActiveBand(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveBand(6).getOriginalNumber() == 6);
         DataAccessor importData1 = importElement1->getDataAccessor();

         ModelResource<RasterElement> importElement2(batchImportCube("Ice Importer", mDataPath + "cube512x512x7x8complex-bsq-bigendian.ice.h5"));
         issearf(importElement2.get() != NULL);
         RasterDataDescriptor* importElement2Desc = dynamic_cast<RasterDataDescriptor*>(importElement2->getDataDescriptor());
         issearf(importElement2Desc != NULL);
         issearf(importElement2Desc->getRowCount() == 512);
         issearf(importElement2Desc->getColumnCount() == 512);
         issearf(importElement2Desc->getBandCount() == 7);
         issearf(importElement2Desc->getDataType() == FLT8COMPLEX);
         issearf(importElement2Desc->getInterleaveFormat() == BSQ);
         issearf(importElement2Desc->getActiveRow(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveRow(511).getOriginalNumber() == 511);
         issearf(importElement2Desc->getActiveColumn(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveColumn(511).getOriginalNumber() == 511);
         issearf(importElement2Desc->getActiveBand(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveBand(6).getOriginalNumber() == 6);
         DataAccessor importData2 = importElement2->getDataAccessor();
         issearf(importData1.isValid());
         issearf(importData2.isValid());

         float tolerance = static_cast<float>(0.0001);
         importData1->toPixel(476, 257);
         importData2->toPixel(476, 257);
         FloatComplex* pData1 = static_cast<FloatComplex*>(importData1->getColumn());
         FloatComplex* pData2 = static_cast<FloatComplex*>(importData2->getColumn());
         issearf(fabs(pData1->mReal - 9296.5254) < tolerance && fabs(pData1->mImaginary - 0.35002145) < tolerance);
         issearf(fabs(pData1->mReal - pData2->mReal) < tolerance && fabs(pData1->mImaginary - pData2->mImaginary) < tolerance);

         importData1->toPixel(473, 66);
         importData2->toPixel(473, 66);
         pData1 = static_cast<FloatComplex*>(importData1->getColumn());
         pData2 = static_cast<FloatComplex*>(importData2->getColumn());
         issearf(fabs(pData1->mReal - 1434.0559) < tolerance && fabs(pData1->mImaginary - 7804.2251) < tolerance);
         issearf(fabs(pData1->mReal - pData2->mReal) < tolerance && fabs(pData1->mImaginary - pData2->mImaginary) < tolerance);
      }

      return success;
   }

   string mDataPath;
};

/**
 * Test the loading of Ice files that are version 0.70
 */
class IceVersion0_70TestCase : public TestCase
{
public:
   IceVersion0_70TestCase() : TestCase("IceVersion0_70"), mRunEarlierSuites(true)
   {
      mDataPath = TestUtilities::getTestDataPath() + "Ice/Version0_70/";
   }

   bool run()
   {
      bool success = true;
      if (mRunEarlierSuites)
      {
         IceVersion0TestCase* pTestCase = new IceVersion0TestCase();
         pTestCase->mDataPath = mDataPath + "/Version0_0_Upgraded/";
         issearf(pTestCase->run());
      }

      //Load cube with custom metadata
      {
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", mDataPath + "cube-with-metadata.ice.h5"));
         issearf(importElement1.get() != NULL);
         DynamicObject* pMetadata = importElement1->getMetadata();
         issearf(pMetadata != NULL);
         vector<double> doubleValues = dv_cast<vector<double> >(pMetadata->getAttribute("testVectorDouble"));
         issearf(doubleValues.size() == 3);
         issearf(doubleValues[0] == 3);
         issearf(doubleValues[1] == -9.65);
         issearf(doubleValues[2] == 32.48);
         float floatVal = dv_cast<float>(pMetadata->getAttribute("testFloatVal"));
         issearf(floatVal == -34.5);
         DateTime* pDateTime = dv_cast<DateTime>(&pMetadata->getAttribute("testDateTime"));
         issearf(pDateTime != NULL);
         issearf(pDateTime->isValid());
         issearf(pDateTime->isTimeValid());
         string formatString;
#if defined(WIN_API)
            formatString = "%B %#d, %#Y, %H:%M:%S";
#else
            formatString = "%B %d, %Y, %H:%M:%S";
#endif
         string formattedTime = pDateTime->getFormattedUtc(formatString); 
         issearf(formattedTime == "July 10, 2007, 16:59:40");
         bool boolVal = dv_cast<bool>(pMetadata->getAttribute("testBoolValue"));
         issearf(boolVal == false);
         string stringVal = dv_cast<string>(pMetadata->getAttributeByPath("TopObject/ChildObject/testStringVal"));
         issearf(stringVal == "FooBar");
         short shortVal = dv_cast<short>(pMetadata->getAttributeByPath("TopObject/ChildObject/testShortVal"));
         issearf(shortVal == 5);
       
      }

      //Load cube with wavelengths
      {
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", mDataPath + "cube-with-wavelengths.ice.h5"));
         issearf(importElement1.get() != NULL);
         DynamicObject* pMetadata1 = importElement1->getMetadata();
         issearf(pMetadata1 != NULL);

         ModelResource<RasterElement> importElement2(batchImportCube("Ice Importer", mDataPath + "cube-with-startwavelengths.ice.h5"));
         issearf(importElement2.get() != NULL);
         DynamicObject* pMetadata2 = importElement2->getMetadata();
         issearf(pMetadata2 != NULL);

         ModelResource<RasterElement> importElement3(batchImportCube("Ice Importer", mDataPath + "cube-with-centerwavelengths.ice.h5"));
         issearf(importElement3.get() != NULL);
         DynamicObject* pMetadata3 = importElement3->getMetadata();
         issearf(pMetadata3 != NULL);

         ModelResource<RasterElement> importElement4(batchImportCube("Ice Importer", mDataPath + "cube-with-endwavelengths.ice.h5"));
         issearf(importElement4.get() != NULL);
         DynamicObject* pMetadata4 = importElement4->getMetadata();
         issearf(pMetadata4 != NULL);

         string pCenterWavelengthPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, 
            CENTER_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
         string pStartWavelengthPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
            START_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
         string pEndWavelengthPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME,
            END_WAVELENGTHS_METADATA_NAME, END_METADATA_NAME };
         vector<double> centerWavelengths1 = dv_cast<vector<double> >(pMetadata1->getAttributeByPath(pCenterWavelengthPath));
         vector<double> startWavelengths1 = dv_cast<vector<double> >(pMetadata1->getAttributeByPath(pStartWavelengthPath));
         vector<double> endWavelengths1 = dv_cast<vector<double> >(pMetadata1->getAttributeByPath(pEndWavelengthPath));
         vector<double> centerWavelengths2 = dv_cast<vector<double> >(pMetadata3->getAttributeByPath(pCenterWavelengthPath));
         vector<double> startWavelengths2 = dv_cast<vector<double> >(pMetadata2->getAttributeByPath(pStartWavelengthPath));
         vector<double> endWavelengths2 = dv_cast<vector<double> >(pMetadata4->getAttributeByPath(pEndWavelengthPath));

         issearf(centerWavelengths1.size() == 7);
         issearf(centerWavelengths1 == centerWavelengths2);
         issearf(centerWavelengths1[0] == 34.5);
         issearf(centerWavelengths1[1] == 10.5);
         issearf(centerWavelengths1[2] == 100);
         issearf(centerWavelengths1[3] == -.98);
         issearf(centerWavelengths1[4] == 34.2);
         issearf(centerWavelengths1[5] == 12);
         issearf(centerWavelengths1[6] == 56.8);

         issearf(startWavelengths1.size() == 7);
         issearf(startWavelengths1 == startWavelengths2);
         issearf(startWavelengths1[0] == 3.4);
         issearf(startWavelengths1[1] == 5.6);
         issearf(startWavelengths1[2] == 7.8);
         issearf(startWavelengths1[3] == 9);
         issearf(startWavelengths1[4] == -23);
         issearf(startWavelengths1[5] == 34.5);
         issearf(startWavelengths1[6] == 10.5);

         issearf(endWavelengths1.size() == 7);
         issearf(endWavelengths1 == endWavelengths2);
         issearf(endWavelengths1[0] == 9);
         issearf(endWavelengths1[1] == -23);
         issearf(endWavelengths1[2] == 34.5);
         issearf(endWavelengths1[3] == 10.5);
         issearf(endWavelengths1[4] == 4.5);
         issearf(endWavelengths1[5] == 12);
         issearf(endWavelengths1[6] == 56);
      }

      //Load cube with band names
      {
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", mDataPath + "cube-with-bandnames.ice.h5"));
         issearf(importElement1.get() != NULL);
         DynamicObject* pMetadata1 = importElement1->getMetadata();
         issearf(pMetadata1 != NULL);

         string pBandNamesPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, NAMES_METADATA_NAME, END_METADATA_NAME };
         vector<string> bandNames = dv_cast<vector<string> >(pMetadata1->getAttributeByPath(pBandNamesPath));
         issearf(bandNames.size() == 7);

         issearf(bandNames[0] == "Band1");
         issearf(bandNames[1] == "Band2");
         issearf(bandNames[2] == "BigBand");
         issearf(bandNames[3] == "Band4");
         issearf(bandNames[4] == "RockBand");
         issearf(bandNames[5] == "ClassicalBand");
         issearf(bandNames[6] == "GreenBand");
      }

      return success;
   }

   bool mRunEarlierSuites;
   string mDataPath;
};

/**
 * Test the loading of Ice files that are version 1.0
 */
class IceVersion0_90TestCase : public TestCase
{
public:
   IceVersion0_90TestCase() : TestCase("IceVersion0_90")
   {
   }

   bool run()
   {
      bool success = true;

      {
         IceVersion0TestCase* pTestCase0 = new IceVersion0TestCase();
         pTestCase0->mDataPath = TestUtilities::getTestDataPath() + "Ice/Version0_90/Version0_0_Upgraded/";
         issearf(pTestCase0->run());

         IceVersion0_70TestCase* pTestCase0_70 = new IceVersion0_70TestCase();
         pTestCase0_70->mRunEarlierSuites = false;
         pTestCase0_70->mDataPath = TestUtilities::getTestDataPath() + "Ice/Version0_90/Version0_70_Upgraded/";
         issearf(pTestCase0_70->run());

         //test loading of classification
         UtilityServicesImp::instance()->overrideDefaultClassification("T"); //set default classification to top secret
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", TestUtilities::getTestDataPath() +
            "Ice/Version0_90/cube-test-classification-markings.ice.h5"));
         issearf(importElement1.get() != NULL);
         const Classification* pClass = importElement1->getClassification();
         issearf(pClass != NULL);
         issearf(pClass->getLevel() == "C"); //make sure value of confidential was read out of file instead of defaulting to top secret.
         issearf(pClass->getSystem() == "Test\\ System");
         issearf(pClass->getCodewords() == "SI");
         issearf(pClass->getFileControl() == "");
         issearf(pClass->getFileReleasing() == "ORCON REL\\ TO");
         issearf(pClass->getClassificationReason() == "C");
         issearf(pClass->getDeclassificationType() == "DD");
         const DateTime* pDeclassDate = pClass->getDeclassificationDate();
         issearf(pDeclassDate != NULL);
         issearf(pDeclassDate->isValid() == false);
         issearf(pClass->getDeclassificationExemption() == "MR");
         issearf(pClass->getFileDowngrade() == "");
         const DateTime* pDowngradeDate = pClass->getDowngradeDate();
         issearf(pDowngradeDate != NULL);
         issearf(pDowngradeDate->isValid() == true);
         issearf(pDowngradeDate->isTimeValid() == false);
         issearf(pDowngradeDate->getStructured() == 1185926400);
         issearf(pClass->getCountryCode() == "USA");
         issearf(pClass->getDescription() == "Test Classification to make sure Ice is working properly.");
         issearf(pClass->getAuthority() == "");
         issearf(pClass->getAuthorityType() == "");
         const DateTime* pSourceDate = pClass->getSecuritySourceDate();
         issearf(pSourceDate != NULL);
         issearf(pSourceDate->isValid() == false);
         issearf(pClass->getSecurityControlNumber() == "");
         issearf(pClass->getFileCopyNumber() == "1");
         issearf(pClass->getFileNumberOfCopies() == "2");   

         //verify that no gcp's were loaded from the file.
         RasterDataDescriptor* pDataDesc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
         issearf(pDataDesc != NULL);
         RasterFileDescriptor* pFileDesc = dynamic_cast<RasterFileDescriptor*>(pDataDesc->getFileDescriptor());
         issearf(pFileDesc != NULL);
         issearf(pFileDesc->getGcps().empty() == true);
      }

      //Make sure that when loading an pre 0.90 file that the classification is defaulted to the highest level of 
      //the system.
      {
         UtilityServicesImp::instance()->overrideDefaultClassification("R"); //set default classification to restricted
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", TestUtilities::getTestDataPath() +
            "Ice/Version0_70/Version0_0_Upgraded/cube512x512x7x2s-bsq-littleendian.ice.h5"));
         issearf(importElement1.get() != NULL);
         const Classification* pClass = importElement1->getClassification();
         issearf(pClass->getLevel() == "R"); //make sure level defaulted to restricted because version 0.70 files don't have a classification marking
         UtilityServicesImp::instance()->overrideDefaultClassification("");
      }

      {
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", TestUtilities::getTestDataPath() +
            "Ice/Version0_90/cube-with-dayton-ohio-gcps.ice.h5"));
         issearf(importElement1.get() != NULL);
         RasterDataDescriptor* pDataDesc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
         issearf(pDataDesc != NULL);
         RasterFileDescriptor* pFileDesc = dynamic_cast<RasterFileDescriptor*>(pDataDesc->getFileDescriptor());
         issearf(pFileDesc != NULL);
         list<GcpPoint> gcps = pFileDesc->getGcps();
         issearf(gcps.size() == 5);
         issearf(fabs(gcps.front().mPixel.mX - 0.0) < 0.000000000000001);
         issearf(fabs(gcps.front().mPixel.mY - 0.0) < 0.000000000000001);
         issearf(fabs(gcps.front().mCoordinate.mX - 39.75020002887268) < 0.000000000000001);
         issearf(fabs(gcps.front().mCoordinate.mY - -84.2) < 0.000000000000001);
         gcps.pop_front();
         issearf(fabs(gcps.front().mPixel.mX - 511.0) < 0.000000000000001);
         issearf(fabs(gcps.front().mPixel.mY - 0.0) < 0.000000000000001);
         issearf(fabs(gcps.front().mCoordinate.mX - 39.75020002887265) < 0.000000000000001);
         issearf(fabs(gcps.front().mCoordinate.mY - -84.99982782571257) < 0.000000000000001);
         gcps.pop_front();
         issearf(fabs(gcps.front().mPixel.mX - 0.0) < 0.000000000000001);
         issearf(fabs(gcps.front().mPixel.mY - 511.0) < 0.000000000000001);
         issearf(fabs(gcps.front().mCoordinate.mX - 39.000000000000256) < 0.000000000000001);
         issearf(fabs(gcps.front().mCoordinate.mY - -84.20000000000053) < 0.000000000000001);
         gcps.pop_front();
         issearf(fabs(gcps.front().mPixel.mX - 511.0) < 0.000000000000001);
         issearf(fabs(gcps.front().mPixel.mY - 511.0) < 0.000000000000001);
         issearf(fabs(gcps.front().mCoordinate.mX - 39.00000000000022) < 0.000000000000001);
         issearf(fabs(gcps.front().mCoordinate.mY - -84.9998278257131) < 0.000000000000001);
         gcps.pop_front();
         issearf(fabs(gcps.front().mPixel.mX - 255.0) < 0.000000000000001);
         issearf(fabs(gcps.front().mPixel.mY - 255.0) < 0.000000000000001);
         issearf(fabs(gcps.front().mCoordinate.mX - 39.37583406534533) < 0.000000000000001);
         issearf(fabs(gcps.front().mCoordinate.mY - -84.59913130245958) < 0.000000000000001);

      }

      return success;
   }
};

/**
 * Test the loading of Ice files that are version 1.0
 */
class IceVersion1_0TestCase : public TestCase
{
public:
   IceVersion1_0TestCase() : TestCase("IceVersion1_0")
   {
   }

   bool run()
   {
      bool success = true;
      {
         {
            ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer",
               TestUtilities::getTestDataPath() +
               "Ice/Version1_0/cube-grayscale-displayband3-band3stats-normalunits.ice.h5"));
            issearf(importElement1.get() != NULL);
            RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
            issearf(importElement1Desc != NULL);
            issearf(importElement1Desc->getDisplayMode() == GRAYSCALE_MODE);
            DimensionDescriptor displayBand = importElement1Desc->getOriginalBand(importElement1Desc->getDisplayBand(GRAY).getOriginalNumber());
            issearf(displayBand.isValid());
            issearf(displayBand.isOnDiskNumberValid() && displayBand.getOnDiskNumber() == 2);
            issearf(displayBand.isActiveNumberValid() && displayBand.getActiveNumber() == 2);
            issearf(importElement1Desc->getXPixelSize() == 1);
            issearf(importElement1Desc->getYPixelSize() == 1);
            Units* pUnits = importElement1Desc->getUnits();
            issearf(pUnits != NULL);
            issearf(pUnits->getUnitName() == "Digital Number");
            issearf(pUnits->getUnitType() == DIGITAL_NO);
            issearf(pUnits->getRangeMin() == 0);
            issearf(pUnits->getRangeMax() == 0);
            issearf(pUnits->getScaleFromStandard() == 1);
            Statistics* pStats1 = importElement1->getStatistics(importElement1Desc->getActiveBand(2));
            issearf(pStats1 != NULL);
            issearf(pStats1->areStatisticsCalculated() == true);
            Statistics* pStats2 = importElement1->getStatistics(importElement1Desc->getActiveBand(0));
            issearf(pStats2 != NULL);
            issearf(pStats2->areStatisticsCalculated() == false);
         }

         {
            ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer",
               TestUtilities::getTestDataPath() + "Ice/Version1_0/cube-with-unique-units-custom-pixel-size.ice.h5"));
            issearf(importElement1.get() != NULL);
            RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
            issearf(importElement1Desc != NULL);
            issearf(importElement1Desc->getDisplayMode() == GRAYSCALE_MODE);
            DimensionDescriptor displayBand = importElement1Desc->getOriginalBand(importElement1Desc->getDisplayBand(GRAY).getOriginalNumber());
            issearf(displayBand.isValid());
            issearf(displayBand.isOnDiskNumberValid() && displayBand.getOnDiskNumber() == 0);
            issearf(displayBand.isActiveNumberValid() && displayBand.getActiveNumber() == 0);
            issearf(importElement1Desc->getXPixelSize() == 5);
            issearf(importElement1Desc->getYPixelSize() == 10);
            Units* pUnits = importElement1Desc->getUnits();
            issearf(pUnits != NULL);
            issearf(pUnits->getUnitName() == "Ice Test Units");
            issearf(pUnits->getUnitType() == RADIANCE);
            issearf(pUnits->getRangeMin() == 345);
            issearf(pUnits->getRangeMax() == -987);
            issearf(pUnits->getScaleFromStandard() == 0.56);
            Statistics* pStats1 = importElement1->getStatistics(importElement1Desc->getActiveBand(0));
            issearf(pStats1 != NULL);
            issearf(pStats1->areStatisticsCalculated() == true);
            Statistics* pStats2 = importElement1->getStatistics(importElement1Desc->getActiveBand(2));
            issearf(pStats2 != NULL);
            issearf(pStats2->areStatisticsCalculated() == false);
         }

         {
            ImporterResource import1("Ice Importer", TestUtilities::getTestDataPath() +
               "Ice/Version1_0/cube-rgb-r4-g2-b6-bandstats-normalunits.ice.h5", NULL, true);
            vector<ImportDescriptor*> descriptors1 = import1->getImportDescriptors();
            issearf(descriptors1.size() == 1);
            ImportDescriptor* pDesc1 = descriptors1.front();
            issearf(pDesc1 != NULL);
            RasterDataDescriptor* pDataDesc1 = dynamic_cast<RasterDataDescriptor*>(pDesc1->getDataDescriptor());
            issearf(pDataDesc1 != NULL);
            pDataDesc1->setProcessingLocation(IN_MEMORY);
            issearf(import1->execute());
            vector<DataElement*> importElements1 = import1->getImportedElements();
            issearf(importElements1.size() == 1);
            ModelResource<RasterElement> importElement1(dynamic_cast<RasterElement*>(importElements1.front()));
            issearf(importElement1.get() != NULL);
            RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
            issearf(importElement1Desc != NULL);
            issearf(importElement1Desc->getDisplayMode() == RGB_MODE);
            DimensionDescriptor redBand = importElement1Desc->getOriginalBand(importElement1Desc->getDisplayBand(RED).getOriginalNumber());
            issearf(redBand.isValid());
            issearf(redBand.isOnDiskNumberValid() && redBand.getOnDiskNumber() == 3);
            issearf(redBand.isActiveNumberValid() && redBand.getActiveNumber() == 3);
            DimensionDescriptor greenBand = importElement1Desc->getOriginalBand(importElement1Desc->getDisplayBand(GREEN).getOriginalNumber());
            issearf(greenBand.isValid());
            issearf(greenBand.isOnDiskNumberValid() && greenBand.getOnDiskNumber() == 1);
            issearf(greenBand.isActiveNumberValid() && greenBand.getActiveNumber() == 1);
            DimensionDescriptor blueBand = importElement1Desc->getOriginalBand(importElement1Desc->getDisplayBand(BLUE).getOriginalNumber());
            issearf(blueBand.isValid());
            issearf(blueBand.isOnDiskNumberValid() && blueBand.getOnDiskNumber() == 5);
            issearf(blueBand.isActiveNumberValid() && blueBand.getActiveNumber() == 5);
            Statistics* pStats1 = importElement1->getStatistics(importElement1Desc->getActiveBand(0));
            issearf(pStats1 != NULL);
            issearf(pStats1->areStatisticsCalculated() == true);
            issearf(fabs(pStats1->getAverage() - -16416.749271392822) < 0.000000000001);
            issearf(fabs(pStats1->getMin() - -32768.0) < 0.000000000001);
            issearf(fabs(pStats1->getMax() - 32639.0) < 0.000000000001);
            issearf(fabs(pStats1->getStandardDeviation() - 16367.771936833415) < 0.000000000001);
            issearf(pStats1->getStatisticsResolution() == 1);
            Statistics* pStats2 = importElement1->getStatistics(importElement1Desc->getActiveBand(3));
            issearf(pStats2 != NULL);
            issearf(pStats2->areStatisticsCalculated() == true);
            issearf(fabs(pStats2->getAverage() - -16512.74644470215) < 0.000000000001);
            issearf(fabs(pStats2->getMin() - -32768.0) < 0.000000000001);
            issearf(fabs(pStats2->getMax() - 32255.0) < 0.000000000001);
            issearf(fabs(pStats2->getStandardDeviation() - 16319.656926855709) < 0.000000000001);
            issearf(pStats2->getStatisticsResolution() == 1);
            Statistics* pStats3 = importElement1->getStatistics(importElement1Desc->getActiveBand(4));
            issearf(pStats3 != NULL);
            issearf(pStats3->areStatisticsCalculated() == false);
            issearf(pStats3->getStatisticsResolution() == 0);
         }

         {
            //Load the same cube as just before but with a processing location of ON_DISK_READ_ONLY.  This is because
            //the method used by the Ice importer to load statistics with a processing location of ON_DISK_READ_ONLY is
            //slightly different, so we need to make sure it still works.
            ImporterResource import1("Ice Importer", TestUtilities::getTestDataPath() +
               "Ice/Version1_0/cube-rgb-r4-g2-b6-bandstats-normalunits.ice.h5", NULL, true);
            vector<ImportDescriptor*> descriptors1 = import1->getImportDescriptors();
            issearf(descriptors1.size() == 1);
            ImportDescriptor* pDesc1 = descriptors1.front();
            issearf(pDesc1 != NULL);
            RasterDataDescriptor* pDataDesc1 = dynamic_cast<RasterDataDescriptor*>(pDesc1->getDataDescriptor());
            issearf(pDataDesc1 != NULL);
            pDataDesc1->setProcessingLocation(ON_DISK_READ_ONLY);
            issearf(import1->execute());
            vector<DataElement*> importElements1 = import1->getImportedElements();
            issearf(importElements1.size() == 1);
            ModelResource<RasterElement> importElement1(dynamic_cast<RasterElement*>(importElements1.front()));
            issearf(importElement1.get() != NULL);
            RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
            issearf(importElement1Desc != NULL);
            Statistics* pStats1 = importElement1->getStatistics(importElement1Desc->getActiveBand(0));
            issearf(pStats1 != NULL);
            issearf(pStats1->areStatisticsCalculated() == true);
            issearf(fabs(pStats1->getAverage() - -16416.749271392822) < 0.000000000001);
            issearf(fabs(pStats1->getMin() - -32768.0) < 0.000000000001);
            issearf(fabs(pStats1->getMax() - 32639.0) < 0.000000000001);
            issearf(fabs(pStats1->getStandardDeviation() - 16367.771936833415) < 0.000000000001);
            issearf(pStats1->getStatisticsResolution() == 1);
            Statistics* pStats2 = importElement1->getStatistics(importElement1Desc->getActiveBand(3));
            issearf(pStats2 != NULL);
            issearf(pStats2->areStatisticsCalculated() == true);
            issearf(fabs(pStats2->getAverage() - -16512.74644470215) < 0.000000000001);
            issearf(fabs(pStats2->getMin() - -32768.0) < 0.000000000001);
            issearf(fabs(pStats2->getMax() - 32255.0) < 0.000000000001);
            issearf(fabs(pStats2->getStandardDeviation() - 16319.656926855709) < 0.000000000001);
            issearf(pStats2->getStatisticsResolution() == 1);
            Statistics* pStats3 = importElement1->getStatistics(importElement1Desc->getActiveBand(4));
            issearf(pStats3 != NULL);
            issearf(pStats3->areStatisticsCalculated() == false);
            issearf(pStats3->getStatisticsResolution() == 0);
         }

         {
            ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer",
               TestUtilities::getTestDataPath() + "Ice/Version1_0/cube-with-different-bad-values-per-band.ice.h5"));
            issearf(importElement1.get() != NULL);
            RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
            issearf(importElement1Desc != NULL);

            Statistics* pStats1 = importElement1->getStatistics(importElement1Desc->getActiveBand(0));
            issearf(pStats1 != NULL);
            issearf(pStats1->areStatisticsCalculated() == false);
            issearf(pStats1->getStatisticsResolution() == 1);
            const BadValues* pBadValues1 = pStats1->getBadValues();
            issearf(pBadValues1 != NULL);
            const vector<string> badValues1 = pBadValues1->getIndividualBadValues();
            issearf(badValues1.size() == 4);
            issearf(badValues1[0] == "3");
            issearf(badValues1[1] == "4");
            issearf(badValues1[2] == "5");
            issearf(badValues1[3] == "10");
            issearf(pBadValues1->isBadValue(3.0));
            issearf(pBadValues1->isBadValue(4.0));
            issearf(pBadValues1->isBadValue(5.0));
            issearf(pBadValues1->isBadValue(10.0));
            const double tolerance = StringUtilities::fromDisplayString<double>(pBadValues1->getBadValueTolerance());
            issearf(pBadValues1->isBadValue(3.0 + tolerance) == false);
            issearf(pBadValues1->isBadValue(4.0 + tolerance) == false);
            issearf(pBadValues1->isBadValue(5.0 - tolerance) == false);
            issearf(pBadValues1->isBadValue(10.0 - tolerance) == false);

            Statistics* pStats2 = importElement1->getStatistics(importElement1Desc->getActiveBand(1));
            issearf(pStats2 != NULL);
            issearf(pStats2->areStatisticsCalculated() == false);
            issearf(pStats2->getStatisticsResolution() == 0);
            const BadValues* pBadValues2 = pStats2->getBadValues();
            issearf(pBadValues2 != NULL);
            const vector<string> badValues2 = pBadValues2->getIndividualBadValues();
            issearf(badValues2.size() == 4);
            issearf(badValues2[0] == "3");
            issearf(badValues2[1] == "4");
            issearf(badValues2[2] == "5");
            issearf(badValues2[3] == "10");
            issearf(pBadValues2->isBadValue(3.0));
            issearf(pBadValues2->isBadValue(4.0));
            issearf(pBadValues2->isBadValue(5.0));
            issearf(pBadValues2->isBadValue(10.0));
            issearf(pBadValues2->isBadValue(3.0 + tolerance) == false);
            issearf(pBadValues2->isBadValue(4.0 + tolerance) == false);
            issearf(pBadValues2->isBadValue(5.0 - tolerance) == false);
            issearf(pBadValues2->isBadValue(10.0 - tolerance) == false);

            Statistics* pStats3 = importElement1->getStatistics(importElement1Desc->getActiveBand(2));
            issearf(pStats3 != NULL);
            issearf(pStats3->areStatisticsCalculated() == false);
            issearf(pStats3->getStatisticsResolution() == 1);
            const BadValues* pBadValues3 = pStats3->getBadValues();
            issearf(pBadValues3 != NULL);
            issearf(pBadValues3->empty());

            Statistics* pStats4 = importElement1->getStatistics(importElement1Desc->getActiveBand(5));
            issearf(pStats4 != NULL);
            issearf(pStats4->areStatisticsCalculated() == true);
            issearf(pStats4->getStatisticsResolution() == 1);
            const BadValues* pBadValues4 = pStats4->getBadValues();
            issearf(pBadValues4 != NULL);
            const vector<string> badValues4 = pBadValues4->getIndividualBadValues();
            issearf(badValues4.size() == 1);
            issearf(badValues4[0] == "20");
            issearf(pBadValues4->isBadValue(20.0));
            issearf(pBadValues4->isBadValue(20.0 + tolerance) == false);
            issearf(pBadValues4->isBadValue(20.0 - tolerance) == false);
         }

         {
            ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer",
               TestUtilities::getTestDataPath() + "Ice/Version1_0/cube-no-saved-stats.ice.h5"));
            issearf(importElement1.get() != NULL);
            RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
            issearf(importElement1Desc != NULL);
            const vector<DimensionDescriptor>& bands = importElement1Desc->getBands();
            for (vector<DimensionDescriptor>::const_iterator iter = bands.begin();
                 iter != bands.end();
                 ++iter)
            {
               Statistics* pStats = importElement1->getStatistics(*iter);
               issearf(pStats != NULL);
               issearf(pStats->areStatisticsCalculated() == false);
               issearf(pStats->getStatisticsResolution() == 0);
            }
         }

         {
            //Before importing cube, specifically set bad values on the data descriptor.
            //This should cause the Ice Importer to not load the saved statistics in the ice file, since the bad values
            //are now different.  After load, verify for all Statistics, that Statistics::areStatisticsCalculated() returns false.
            ImporterResource import("Ice Importer", TestUtilities::getTestDataPath() +
               "Ice/Version1_0/cube-with-different-bad-values-per-band.ice.h5", NULL, true);
            vector<ImportDescriptor*> descriptors = import->getImportDescriptors();
            issearf(descriptors.size() == 1);
            ImportDescriptor* pDesc = descriptors.front();
            issearf(pDesc != NULL);
            RasterDataDescriptor* pDataDesc = dynamic_cast<RasterDataDescriptor*>(pDesc->getDataDescriptor());
            issearf(pDataDesc != NULL);
            vector<int> badValues;
            badValues.push_back(3);
            pDataDesc->setBadValues(badValues);
            issearf(import->execute());
            vector<DataElement*> importElements = import->getImportedElements();
            issearf(importElements.size() == 1);
            ModelResource<RasterElement> importElement1(dynamic_cast<RasterElement*>(importElements.front()));
            issearf(importElement1.get() != NULL);
            RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
            issearf(importElement1Desc != NULL);
            const vector<DimensionDescriptor>& bands = importElement1Desc->getBands();
            for (vector<DimensionDescriptor>::const_iterator iter = bands.begin();
                 iter != bands.end();
                 ++iter)
            {
               Statistics* pStats = importElement1->getStatistics(*iter);
               issearf(pStats != NULL);
               issearf(pStats->areStatisticsCalculated() == false);
               issearf(pStats->getStatisticsResolution() == 0);
            }
         }
      }
      return success;
   }
};

/**
 * Test the loading of Ice files that are version 1.10
 */
class IceVersion1_10TestCase : public TestCase
{
public:
   IceVersion1_10TestCase() : TestCase("IceVersion1_10")
   {
   }

   bool run()
   {
      bool success = true;
      {
         // Test RasterElement Import
         {
            ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer",
               TestUtilities::getTestDataPath() + "Ice/Version1_10/i_3130s.re.ice.h5"));
            issearf(importElement1.get() != NULL);
            RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
            issearf(importElement1Desc != NULL);
            issearf(importElement1Desc->getDisplayMode() == GRAYSCALE_MODE);
            issearf(importElement1Desc->getBandCount() == 1);
            DimensionDescriptor displayBand = importElement1Desc->getOriginalBand(importElement1Desc->getDisplayBand(GRAY).getOriginalNumber());
            issearf(displayBand.isValid());
            issearf(displayBand.isOnDiskNumberValid() && displayBand.getOnDiskNumber() == 0);
            issearf(displayBand.isActiveNumberValid() && displayBand.getActiveNumber() == 0);
            issearf(importElement1Desc->getXPixelSize() == 1);
            issearf(importElement1Desc->getYPixelSize() == 1);
            Units* pUnits = importElement1Desc->getUnits();
            issearf(pUnits != NULL);
            issearf(pUnits->getUnitName() == "Digital Number");
            issearf(pUnits->getUnitType() == DIGITAL_NO);
            issearf(pUnits->getRangeMin() == 0);
            issearf(pUnits->getRangeMax() == 0);
            issearf(pUnits->getScaleFromStandard() == 1);
            Statistics* pStats = importElement1->getStatistics(importElement1Desc->getActiveBand(0));
            issearf(pStats != NULL);
            issearf(pStats->areStatisticsCalculated() == true);
         }

         // Test PseudocolorLayer Import and Export
         {
            SpatialDataWindow* pWindow = TestUtilities::loadDataSet(TestUtilities::getTestDataPath() +
               "Ice/Version1_10/i_3130s.re.ice.h5", "Ice Importer");
            issearf(pWindow != NULL);

            // These files contain similar layers. The only differences are the symbols and the colors.
            string filename = TestUtilities::getTestDataPath() +
               "Ice/Version1_10/i_3130s_Name-Asterisk_LayerType-PseudocolorLayer_XScale1_YScale1__XOffset0_YOffset0_Symbol-Asterisk_176Red_177Green_178Blue_179Purple_180Orange.psl.ice.h5";
            issearf(TestUtilities::loadDataSet(filename, "Ice PseudocolorLayer Importer", pWindow) == pWindow);

            filename = TestUtilities::getTestDataPath() +
               "Ice/Version1_10/i_3130s_Name-Box_LayerType-PseudocolorLayer_XScale1_YScale1__XOffset0_YOffset0_Symbol-Box_176Pink_177Yellow_178LightBlue_179Brown_180Orange.psl.ice.h5";
            issearf(TestUtilities::loadDataSet(filename, "Ice PseudocolorLayer Importer", pWindow) == pWindow);

            // Get the PseudocolorLayer
            SpatialDataView* pView = pWindow->getSpatialDataView();
            issearf(pView != NULL);

            LayerList* pLayerList = pView->getLayerList();
            issearf(pLayerList != NULL);

            vector<Layer*> pLayers;
            pLayerList->getLayers(PSEUDOCOLOR, pLayers);
            issearf(pLayers.size() == 2);

            PseudocolorLayer* pAsteriskLayer = dynamic_cast<PseudocolorLayer*>(pLayers.front());
            issearf(pAsteriskLayer != NULL);

            PseudocolorLayer* pBoxLayer = dynamic_cast<PseudocolorLayer*>(pLayers.back());
            issearf(pBoxLayer != NULL);

            // Test Layer Properties
            issearf(checkLayerProperties(pAsteriskLayer, "Asterisk", PSEUDOCOLOR, 1.0, 1.0, 0.0, 0.0));
            issearf(checkLayerProperties(pBoxLayer, "Box", PSEUDOCOLOR, 1.0, 1.0, 0.0, 0.0));

            // Test PseudocolorLayer Properties for Asterisk Layer
            vector<int> classValues;
            classValues.push_back(176);
            classValues.push_back(177);
            classValues.push_back(178);
            classValues.push_back(179);
            classValues.push_back(180);

            vector<string> classNames;
            classNames.push_back("176-Red");
            classNames.push_back("177-Green");
            classNames.push_back("178-Blue");
            classNames.push_back("179-Purple");
            classNames.push_back("180-Orange");

            vector<ColorType> classColors;
            classColors.push_back(ColorType(255, 0, 0));
            classColors.push_back(ColorType(0, 255, 0));
            classColors.push_back(ColorType(0, 0, 255));
            classColors.push_back(ColorType(85, 0, 127));
            classColors.push_back(ColorType(255, 170, 0));
            issearf(checkPseudocolorLayerProperties(pAsteriskLayer, ASTERISK, classNames, classValues, classColors));

            // Test PseudocolorLayer Properties for Box Layer
            classNames.clear();
            classNames.push_back("176-Pink");
            classNames.push_back("177-Yellow");
            classNames.push_back("178-LightBlue");
            classNames.push_back("179-Brown");
            classNames.push_back("180-Orange");

            classColors.clear();
            classColors.push_back(ColorType(255, 0, 255));
            classColors.push_back(ColorType(255, 255, 0));
            classColors.push_back(ColorType(85, 170, 255));
            classColors.push_back(ColorType(170, 85, 0));
            classColors.push_back(ColorType(255, 170, 0));
            issearf(checkPseudocolorLayerProperties(pBoxLayer, BOX, classNames, classValues, classColors));

            // Copy the Box layer and change it to a Solid layer. Export the Solid layer and destroy it
            {
               PseudocolorLayer* pSolidLayer = dynamic_cast<PseudocolorLayer*>(pBoxLayer->copy("Solid"));
               issearf(pSolidLayer != NULL);
               issearf(pView->addLayer(pSolidLayer));

               pSolidLayer->setSymbol(SOLID);
               issearf(pSolidLayer->setClassColor(0, ColorType(255, 255, 255)));
               issearf(pSolidLayer->setClassName(0, "176-White"));
               pSolidLayer->addInitializedClass("175-Red", 175, ColorType(255, 0, 0));

               const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
               issearf(pTempPath != NULL);
               filename = pTempPath->getFullPathAndName() + SLASH + "testPseudocolorLayerExport.psl.ice.h5";

               DataElement* pElement = pSolidLayer->getDataElement();
               issearf(pElement != NULL);
               const DataDescriptor* pDd = pElement->getDataDescriptor();
               issearf(pDd != NULL);

               FactoryResource<RasterFileDescriptor> pExporterFileDescriptor(
                  dynamic_cast<RasterFileDescriptor*>(RasterUtilities::generateFileDescriptorForExport(pDd, filename)));
               issearf(pExporterFileDescriptor.get() != NULL);

               ExporterResource exporter("Ice PseudocolorLayer Exporter", pSolidLayer, pExporterFileDescriptor.get(), NULL);
               issearf(exporter->execute());

               issearf(pView->deleteLayer(pSolidLayer));
               pSolidLayer = NULL;
            }

            //Now reload the recently exported layer
            issearf(TestUtilities::loadDataSet(filename, "Ice PseudocolorLayer Importer", pWindow) == pWindow);

            pLayers.clear();
            pLayerList->getLayers(PSEUDOCOLOR, pLayers);
            issearf(pLayers.size() == 3);

            PseudocolorLayer* pSolidLayer = dynamic_cast<PseudocolorLayer*>(pLayers.back());
            issearf(pBoxLayer != NULL);

            // Test Layer Properties
            issearf(checkLayerProperties(pSolidLayer, "Solid", PSEUDOCOLOR, 1.0, 1.0, 0.0, 0.0));

            // Test PseudocolorLayer Properties for Solid Layer
            classValues.clear();
            classValues.push_back(176);
            classValues.push_back(177);
            classValues.push_back(178);
            classValues.push_back(179);
            classValues.push_back(180);
            classValues.push_back(175);

            classNames.clear();
            classNames.push_back("176-White");
            classNames.push_back("177-Yellow");
            classNames.push_back("178-LightBlue");
            classNames.push_back("179-Brown");
            classNames.push_back("180-Orange");
            classNames.push_back("175-Red");

            classColors.clear();
            classColors.push_back(ColorType(255, 255, 255));
            classColors.push_back(ColorType(255, 255, 0));
            classColors.push_back(ColorType(85, 170, 255));
            classColors.push_back(ColorType(170, 85, 0));
            classColors.push_back(ColorType(255, 170, 0));
            classColors.push_back(ColorType(255, 0, 0));
            issearf(checkPseudocolorLayerProperties(pSolidLayer, SOLID, classNames, classValues, classColors));
         }
      }
      return success;
   }

   bool checkLayerProperties(Layer* pLayer, string name, LayerType layerType, double xScaleFactor, double yScaleFactor, double xOffset, double yOffset)
   {
      DO_IF(pLayer == NULL, return false);
      DO_IF(pLayer->getName() != name, return false);
      DO_IF(pLayer->getLayerType() != layerType, return false);
      DO_IF(pLayer->getXScaleFactor() != xScaleFactor, return false);
      DO_IF(pLayer->getYScaleFactor() != yScaleFactor, return false);
      DO_IF(pLayer->getXOffset() != xOffset, return false);
      DO_IF(pLayer->getYOffset() != yOffset, return false);
      return true;
   }

   bool checkPseudocolorLayerProperties(PseudocolorLayer* pLayer, SymbolType symbolType, const vector<string>& classNames,
      const vector<int>& classValues, const vector<ColorType>& classColors)
   {
      DO_IF(pLayer == NULL, return false);
      DO_IF(pLayer->getSymbol() != symbolType, return false);

      vector<int> classIds;
      pLayer->getClassIDs(classIds);
      DO_IF(classNames.size() != classValues.size() || classNames.size() != classColors.size() || classNames.size() != classIds.size(), return false);

      const unsigned int numClasses = classIds.size();
      for (unsigned int i = 0; i < numClasses; ++i)
      {
         string className;
         DO_IF(pLayer->getClassName(classIds[i], className) == false, return false);
         DO_IF(className != classNames[i], return false);
         DO_IF(pLayer->getClassValue(classIds[i]) != classValues[i], return false);
         DO_IF(pLayer->getClassColor(classIds[i]) != classColors[i], return false);
         DO_IF(pLayer->isClassDisplayed(classIds[i]) == false, return false);
      }

      return true;
   }
};

/**
 * Test the loading of Ice files that are version 1.3
 */
class IceVersion1_30TestCase : public TestCase
{
public:
   IceVersion1_30TestCase() : TestCase("IceVersion1_30")
   {}

   bool run()
   {
      bool success = true;
      {
         ModelResource<RasterElement> pImportElement(batchImportCube("Ice Importer",
            TestUtilities::getTestDataPath() +
            "Ice/Version1_30/cube-with-different-bad-values-per-band.re.ice.h5"));
         issearf(pImportElement.get() != NULL);
         RasterDataDescriptor* pImportElementDesc =
            dynamic_cast<RasterDataDescriptor*>(pImportElement->getDataDescriptor());
         issearf(pImportElementDesc != NULL);

         Statistics* pStats1 = pImportElement->getStatistics(pImportElementDesc->getActiveBand(0));
         issearf(pStats1 != NULL);
         issea(pStats1->areStatisticsCalculated() == true);
         issea(pStats1->getStatisticsResolution() == 1);
         const BadValues* pBadValues1 = pStats1->getBadValues();
         issearf(pBadValues1 != NULL);
         string badValues1Str = pBadValues1->getBadValuesString();
         issea(badValues1Str == "<-10.5, 0, 5<>1.4e1, >200");
         issea(pBadValues1->isBadValue(-10.5));
         issea(pBadValues1->isBadValue(0));
         issea(pBadValues1->isBadValue(6.0));
         issea(pBadValues1->isBadValue(210.0));
         const double tolerance1 = StringUtilities::fromDisplayString<double>(pBadValues1->getBadValueTolerance());
         issea(pBadValues1->isBadValue(-10.5 + tolerance1) == false);
         issea(pBadValues1->isBadValue(0 + tolerance1) == false);
         issea(pBadValues1->isBadValue(5.0 - tolerance1) == false);
         issea(pBadValues1->isBadValue(200.0 - tolerance1) == false);

         Statistics* pStats2 = pImportElement->getStatistics(pImportElementDesc->getActiveBand(1));
         issearf(pStats2 != NULL);
         issea(pStats2->areStatisticsCalculated() == false);
         issea(pStats2->getStatisticsResolution() == 0);
         const BadValues* pBadValues2 = pStats2->getBadValues();
         issearf(pBadValues2 != NULL);
         const vector<string> badValues2 = pBadValues2->getIndividualBadValues();
         issea(badValues2.size() == 4);
         issea(badValues2[0] == "3");
         issea(badValues2[1] == "4");
         issea(badValues2[2] == "5");
         issea(badValues2[3] == "10");
         issea(pBadValues2->isBadValue(3.0));
         issea(pBadValues2->isBadValue(4.0));
         issea(pBadValues2->isBadValue(5.0));
         issea(pBadValues2->isBadValue(10.0));
         const double tolerance2 = StringUtilities::fromDisplayString<double>(pBadValues2->getBadValueTolerance());
         issea(pBadValues2->isBadValue(3.0 + tolerance2) == false);
         issea(pBadValues2->isBadValue(4.0 + tolerance2) == false);
         issea(pBadValues2->isBadValue(5.0 - tolerance2) == false);
         issea(pBadValues2->isBadValue(10.0 - tolerance2) == false);

         Statistics* pStats3 = pImportElement->getStatistics(pImportElementDesc->getActiveBand(2));
         issearf(pStats3 != NULL);
         issea(pStats3->areStatisticsCalculated() == false);
         issea(pStats3->getStatisticsResolution() == 1);
         const BadValues* pBadValues3 = pStats3->getBadValues();
         issearf(pBadValues3 != NULL);
         issea(pBadValues3->empty());

         Statistics* pStats4 = pImportElement->getStatistics(pImportElementDesc->getActiveBand(5));
         issearf(pStats4 != NULL);
         issea(pStats4->areStatisticsCalculated() == true);
         issea(pStats4->getStatisticsResolution() == 1);
         const BadValues* pBadValues4 = pStats4->getBadValues();
         issearf(pBadValues4 != NULL);
         const vector<string> badValues4 = pBadValues4->getIndividualBadValues();
         issearf(badValues4.size() == 1);
         issea(badValues4[0] == "20");
         issea(pBadValues4->isBadValue(20.0));
         const double tolerance4 = StringUtilities::fromDisplayString<double>(pBadValues4->getBadValueTolerance());
         issea(pBadValues4->isBadValue(20.0 + tolerance4) == false);
         issea(pBadValues4->isBadValue(20.0 - tolerance4) == false);

         issearf(pImportElementDesc->getBadValues() == NULL);
         pImportElementDesc->setBadValues(pBadValues1);
         const BadValues* pNewBadValues = pImportElementDesc->getBadValues();
         issearf(pNewBadValues != NULL);

         // Note: pBadValues2 points to the bad values instance for band 2. The pointer isn't changed, but the values
         // within the instance are now set to the values from the band 1 instance as result of setting the bad values
         // in the import element descriptor to band 1's bad values.
         issea(pBadValues2->compare(pNewBadValues) == true);
      }

      {
         //Before importing cube, specifically set bad values on the data descriptor.
         //This should cause the Ice Importer to not load the saved statistics in the ice file, since the bad values
         //are now different.  After load, verify for all Statistics, that Statistics::areStatisticsCalculated() returns false.
         ImporterResource import("Ice Importer", TestUtilities::getTestDataPath() +
            "Ice/Version1_30/cube-with-different-bad-values-per-band.re.ice.h5", NULL, true);
         vector<ImportDescriptor*> descriptors = import->getImportDescriptors();
         issearf(descriptors.size() == 1);
         ImportDescriptor* pDesc = descriptors.front();
         issearf(pDesc != NULL);
         RasterDataDescriptor* pDataDesc = dynamic_cast<RasterDataDescriptor*>(pDesc->getDataDescriptor());
         issearf(pDataDesc != NULL);
         FactoryResource<BadValues> pNewBadValues;
         pNewBadValues->addBadValue("3");
         pDataDesc->setBadValues(pNewBadValues.get());
         issearf(import->execute());
         vector<DataElement*> importElements = import->getImportedElements();
         issearf(importElements.size() == 1);
         ModelResource<RasterElement> pImportElement(dynamic_cast<RasterElement*>(importElements.front()));
         issearf(pImportElement.get() != NULL);
         RasterDataDescriptor* pImportElementDesc = dynamic_cast<RasterDataDescriptor*>(pImportElement->getDataDescriptor());
         issearf(pImportElementDesc != NULL);
         const BadValues* pBadValues = pImportElementDesc->getBadValues();
         issearf(pBadValues != NULL);
         const vector<DimensionDescriptor>& bands = pImportElementDesc->getBands();
         for (vector<DimensionDescriptor>::const_iterator iter = bands.begin();
               iter != bands.end();
               ++iter)
         {
            Statistics* pStats = pImportElement->getStatistics(*iter);
            issearf(pStats != NULL);
            issearf(pStats->areStatisticsCalculated() == false);
            issearf(pStats->getStatisticsResolution() == 0);
            issearf(pBadValues->compare(pStats->getBadValues()));
         }

         pNewBadValues->clear();
         Statistics* pStats = pImportElement->getStatistics(bands.front());
         issearf(pStats != NULL);
         pStats->setBadValues(pNewBadValues.get());
         issearf(pImportElementDesc->getBadValues() == NULL);
      }
      return success;
   }
};

class IceWriteComplexDataTestCase : public TestCase
{
public:
   IceWriteComplexDataTestCase() : TestCase("WriteComplexData") {}
   bool run()
   {
      bool success = true;

      //Test Integer Complex
      {
         //Load existing integer complex cube
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", TestUtilities::getTestDataPath() +
            "Ice/Version0_0/cube512x512x7x4complex-bsq-littleendian.ice.h5"));
         issearf(importElement1.get() != NULL);
         RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
         issearf(importElement1Desc != NULL);
         issearf(importElement1Desc->getRowCount() == 512);
         issearf(importElement1Desc->getColumnCount() == 512);
         issearf(importElement1Desc->getBandCount() == 7);
         issearf(importElement1Desc->getDataType() == INT4SCOMPLEX);
         issearf(importElement1Desc->getInterleaveFormat() == BSQ);
         issearf(importElement1Desc->getActiveRow(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveRow(511).getOriginalNumber() == 511);
         issearf(importElement1Desc->getActiveColumn(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveColumn(511).getOriginalNumber() == 511);
         issearf(importElement1Desc->getActiveBand(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveBand(6).getOriginalNumber() == 6);
         DataAccessor importData1 = importElement1->getDataAccessor();

         //Export the Integer Complex cube using current version of Ice Exporter
         string tempPath;
         const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
         if (pTempPath != NULL)
         {
            tempPath = pTempPath->getFullPathAndName();
         }
         string newComplexCubePath = tempPath + SLASH + "testIntegerComplexCube.ice.h5";
         FactoryResource<RasterFileDescriptor> pExporterFileDescriptor(
            dynamic_cast<RasterFileDescriptor*>(RasterUtilities::generateFileDescriptorForExport(
            importElement1Desc, newComplexCubePath)));

         ExporterResource exporter("Ice Exporter", importElement1.get(), pExporterFileDescriptor.get(), NULL);
         issearf(exporter->execute());

         //Now load the recently exported cube.
         ModelResource<RasterElement> importElement2(batchImportCube("Ice Importer", newComplexCubePath));
         issearf(importElement2.get() != NULL);
         RasterDataDescriptor* importElement2Desc = dynamic_cast<RasterDataDescriptor*>(importElement2->getDataDescriptor());
         issearf(importElement2Desc != NULL);
         issearf(importElement2Desc->getRowCount() == 512);
         issearf(importElement2Desc->getColumnCount() == 512);
         issearf(importElement2Desc->getBandCount() == 7);
         issearf(importElement2Desc->getDataType() == INT4SCOMPLEX);
         issearf(importElement2Desc->getInterleaveFormat() == BSQ);
         issearf(importElement2Desc->getActiveRow(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveRow(511).getOriginalNumber() == 511);
         issearf(importElement2Desc->getActiveColumn(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveColumn(511).getOriginalNumber() == 511);
         issearf(importElement2Desc->getActiveBand(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveBand(6).getOriginalNumber() == 6);
         DataAccessor importData2 = importElement2->getDataAccessor();

         //Now compare original complex cube to the recently exported complex cube.
         importData1->toPixel(319, 235);
         importData2->toPixel(319, 235);
         IntegerComplex* pData1 = static_cast<IntegerComplex*>(importData1->getColumn());
         IntegerComplex* pData2 = static_cast<IntegerComplex*>(importData2->getColumn());
         issearf(pData1->mReal == 7389 && pData1->mImaginary == -32094);
         issearf(pData1->mReal == pData2->mReal && pData1->mImaginary == pData2->mImaginary);

         importData1->toPixel(468, 440);
         importData2->toPixel(468, 440);
         pData1 = static_cast<IntegerComplex*>(importData1->getColumn());
         pData2 = static_cast<IntegerComplex*>(importData2->getColumn());
         issearf(pData1->mReal == -21818 && pData1->mImaginary == 16184);
         issearf(pData1->mReal == pData2->mReal && pData1->mImaginary == pData2->mImaginary);
      }

      //Test Float Complex
      {
         ModelResource<RasterElement> importElement1(batchImportCube("Ice Importer", TestUtilities::getTestDataPath() +
            "Ice/Version0_0/cube512x512x7x8complex-bsq-littleendian.ice.h5"));
         issearf(importElement1.get() != NULL);
         RasterDataDescriptor* importElement1Desc = dynamic_cast<RasterDataDescriptor*>(importElement1->getDataDescriptor());
         issearf(importElement1Desc != NULL);
         issearf(importElement1Desc->getRowCount() == 512);
         issearf(importElement1Desc->getColumnCount() == 512);
         issearf(importElement1Desc->getBandCount() == 7);
         issearf(importElement1Desc->getDataType() == FLT8COMPLEX);
         issearf(importElement1Desc->getInterleaveFormat() == BSQ);
         issearf(importElement1Desc->getActiveRow(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveRow(511).getOriginalNumber() == 511);
         issearf(importElement1Desc->getActiveColumn(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveColumn(511).getOriginalNumber() == 511);
         issearf(importElement1Desc->getActiveBand(0).getOriginalNumber() == 0);
         issearf(importElement1Desc->getActiveBand(6).getOriginalNumber() == 6);
         DataAccessor importData1 = importElement1->getDataAccessor();

         //Export the Float Complex cube using current version of Ice Exporter
         string tempPath;
         const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
         if (pTempPath != NULL)
         {
            tempPath = pTempPath->getFullPathAndName();
         }
         string newComplexCubePath = tempPath + SLASH + "testFloatComplexCube.ice.h5";
         FactoryResource<RasterFileDescriptor> pExporterFileDescriptor(
            dynamic_cast<RasterFileDescriptor*>(RasterUtilities::generateFileDescriptorForExport(
            importElement1Desc, newComplexCubePath)));

         ExporterResource exporter("Ice Exporter", importElement1.get(), pExporterFileDescriptor.get(), NULL);
         issearf(exporter->execute());

         //Now load the recently exported cube.
         ModelResource<RasterElement> importElement2(batchImportCube("Ice Importer", newComplexCubePath));
         issearf(importElement2.get() != NULL);
         RasterDataDescriptor* importElement2Desc = dynamic_cast<RasterDataDescriptor*>(importElement2->getDataDescriptor());
         issearf(importElement2Desc != NULL);
         issearf(importElement2Desc->getRowCount() == 512);
         issearf(importElement2Desc->getColumnCount() == 512);
         issearf(importElement2Desc->getBandCount() == 7);
         issearf(importElement2Desc->getDataType() == FLT8COMPLEX);
         issearf(importElement2Desc->getInterleaveFormat() == BSQ);
         issearf(importElement2Desc->getActiveRow(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveRow(511).getOriginalNumber() == 511);
         issearf(importElement2Desc->getActiveColumn(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveColumn(511).getOriginalNumber() == 511);
         issearf(importElement2Desc->getActiveBand(0).getOriginalNumber() == 0);
         issearf(importElement2Desc->getActiveBand(6).getOriginalNumber() == 6);
         DataAccessor importData2 = importElement2->getDataAccessor();

         //Now compare original complex cube to the recently exported complex cube.
         float tolerance = static_cast<float>(0.0001);
         importData1->toPixel(476, 257);
         importData2->toPixel(476, 257);
         FloatComplex* pData1 = static_cast<FloatComplex*>(importData1->getColumn());
         FloatComplex* pData2 = static_cast<FloatComplex*>(importData2->getColumn());
         issearf(fabs(pData1->mReal - 9296.5254) < tolerance && fabs(pData1->mImaginary - 0.35002145) < tolerance);
         issearf(fabs(pData1->mReal - pData2->mReal) < tolerance && fabs(pData1->mImaginary - pData2->mImaginary) < tolerance);

         importData1->toPixel(473, 66);
         importData2->toPixel(473, 66);
         pData1 = static_cast<FloatComplex*>(importData1->getColumn());
         pData2 = static_cast<FloatComplex*>(importData2->getColumn());
         issearf(fabs(pData1->mReal - 1434.0559) < tolerance && fabs(pData1->mImaginary - 7804.2251) < tolerance);
         issearf(fabs(pData1->mReal - pData2->mReal) < tolerance && fabs(pData1->mImaginary - pData2->mImaginary) < tolerance);

      }

      return success;
   }
};

class IceWriteClassificationAndUnitsTestCase : public TestCase
{
public:
   IceWriteClassificationAndUnitsTestCase() : TestCase("WriteClassificationAndUnits")
   {
   }

   bool run()
   {
      bool success = true;

      ModelResource<RasterElement> pElement(RasterUtilities::createRasterElement("TestIceFileForClassificationAndUnitWrite", 5, 5, INT1SBYTE, true));
      issearf(pElement.get() != NULL);
      RasterDataDescriptor* pDataDesc = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      issearf(pDataDesc != NULL);
      FactoryResource<Classification> pClass;
      issearf(pClass.get() != NULL);

      pClass->setLevel("R");
      pClass->setSystem("Test\\ System1 Test\\ System2");
      pClass->setCodewords("TestWord1 TestWord2");
      pClass->setFileControl("TestFileControl");
      pClass->setFileReleasing("REL\\ TO ORCON");
      pClass->setClassificationReason("C");
      pClass->setDeclassificationType("DD");
      FactoryResource<DateTime> pDateTime1;
      pDateTime1->set(2007, 8, 3);
      pClass->setDeclassificationDate(pDateTime1.get());
      pClass->setDeclassificationExemption("MR"); //this clears the internal declassification date
      const DateTime* pOrgDeclassDate = pClass->getDeclassificationDate();
      pClass->setFileDowngrade("TestDowngrade");
      FactoryResource<DateTime> pDateTime2;
      pDateTime2->set(2007, 8, 5, 10, 33, 56);
      pClass->setDowngradeDate(pDateTime2.get());
      pClass->setCountryCode("USA TestCountry");
      pClass->setDescription("Test description\nSecond line of description.");
      pClass->setAuthority("TestAuthority");
      pClass->setAuthorityType("TestAuthorityType");
      FactoryResource<DateTime> pDateTime3;
      pDateTime3->set(2001, 5, 2);
      pClass->setSecuritySourceDate(pDateTime3.get());
      pClass->setSecurityControlNumber("TestControl");
      pClass->setFileCopyNumber("TestCopy1");
      pClass->setFileNumberOfCopies("TestNumberCopies");
      pClass->setAttribute("TestAttribute1", static_cast<unsigned int>(3));
      pClass->setAttributeByPath("TestPath1/TestPath2/TestAttribute2", string("TestValue"));
      string temp;
      issearf(pClass->isValid(temp) == true);
      pDataDesc->setClassification(pClass.get());

      FactoryResource<Units> pUnits;
      pUnits->setUnitName("Test Units");
      pUnits->setUnitType(ABSORPTANCE);
      pUnits->setRangeMin(-3956.234);
      pUnits->setRangeMax(4.3456590821);
      pUnits->setScaleFromStandard(0.45678);
      pDataDesc->setUnits(pUnits.get());

      //export cube now
      string tempPath;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempPath = pTempPath->getFullPathAndName();
      }
      string newCubePath = tempPath + SLASH + "testClassificationAndUnitWrite.ice.h5";
      FactoryResource<RasterFileDescriptor> pExporterFileDescriptor(
         dynamic_cast<RasterFileDescriptor*>(RasterUtilities::generateFileDescriptorForExport(
         pDataDesc, newCubePath)));

      ExporterResource exporter("Ice Exporter", pElement.get(), pExporterFileDescriptor.get(), NULL);
      issearf(exporter->execute());

      //import now
      ModelResource<RasterElement> importElement(batchImportCube("Ice Importer", newCubePath));
      issearf(importElement.get() != NULL);
      RasterDataDescriptor* pReadDataDesc = dynamic_cast<RasterDataDescriptor*>(importElement->getDataDescriptor());
      const Classification* pReadClass = importElement->getClassification();
      issearf(pReadClass != NULL);

      issearf(pReadClass->getLevel() == pClass->getLevel());
      issearf(pReadClass->getSystem() == pClass->getSystem());
      issearf(pReadClass->getCodewords() == pClass->getCodewords());
      issearf(pReadClass->getFileControl() == pClass->getFileControl());
      issearf(pReadClass->getFileReleasing() == pClass->getFileReleasing());
      issearf(pReadClass->getClassificationReason() == pClass->getClassificationReason());
      issearf(pReadClass->getDeclassificationType() == pClass->getDeclassificationType());
      const DateTime* pDeclassificationDate = pReadClass->getDeclassificationDate();
      issearf(pDeclassificationDate != NULL);
      issearf(pDeclassificationDate->isValid() == pOrgDeclassDate->isValid());
      issearf(pDeclassificationDate->isTimeValid() == pOrgDeclassDate->isTimeValid());
      issearf(pDeclassificationDate->getStructured() == pOrgDeclassDate->getStructured());
      issearf(pReadClass->getDeclassificationExemption() == pClass->getDeclassificationExemption());
      issearf(pReadClass->getFileDowngrade() == pClass->getFileDowngrade());
      const DateTime* pDowngradeDate = pReadClass->getDowngradeDate();
      issearf(pDowngradeDate != NULL);
      issearf(pDowngradeDate->isValid() == pDateTime2->isValid());
      issearf(pDowngradeDate->isTimeValid() == pDateTime2->isTimeValid());
      issearf(pDowngradeDate->getStructured() == pDateTime2->getStructured());
      issearf(pReadClass->getCountryCode() == pClass->getCountryCode());
      issearf(pReadClass->getDescription() == pClass->getDescription());
      issearf(pReadClass->getAuthority() == pClass->getAuthority());
      issearf(pReadClass->getAuthorityType() == pClass->getAuthorityType());
      const DateTime* pSourceDate = pReadClass->getSecuritySourceDate();
      issearf(pSourceDate != NULL);
      issearf(pSourceDate->isValid() == pDateTime3->isValid());
      issearf(pSourceDate->isTimeValid() == pDateTime3->isTimeValid());
      issearf(pSourceDate->getStructured() == pDateTime3->getStructured());
      issearf(pReadClass->getSecurityControlNumber() == pClass->getSecurityControlNumber());
      issearf(pReadClass->getFileCopyNumber() == pClass->getFileCopyNumber());
      issearf(pReadClass->getFileNumberOfCopies() == pClass->getFileNumberOfCopies());
      issearf(dv_cast<unsigned int>(pClass->getAttribute("TestAttribute1")) == 3);
      issearf(dv_cast<string>(pClass->getAttributeByPath("TestPath1/TestPath2/TestAttribute2")) == "TestValue");
      issearf(pClass->getAttribute("TestAttribute2").isValid() == false);

      const Units* pReadUnits = pReadDataDesc->getUnits();
      issearf(pReadUnits != NULL);
      issearf(pUnits->getUnitName() == "Test Units");
      issearf(pUnits->getUnitType() == ABSORPTANCE);
      issearf(fabs(pUnits->getRangeMin() - -3956.234) < 0.0000000001);
      issearf(fabs(pUnits->getRangeMax() - 4.3456590821) < 0.0000000001);
      issearf(fabs(pUnits->getScaleFromStandard() - 0.45678) < 0.0000000001);

      return success;
   }
};

class IceChipper
{
public:
   IceChipper() :
      mpOriginalElement(NULL),
      mExportFilename(),
      mpImportedElement(NULL),
      mProcLoc(IN_MEMORY)
   {
      setImportedRows(-1, -1, 0);
      setImportedColumns(-1, -1, 0);
      setImportedBands(-1, -1, 0);
   }

   ~IceChipper()
   {
      clearChip();
   }

   void clearChip()
   {
      if (!mExportFilename.empty())
      {
         unlink(mExportFilename.c_str());
         mExportFilename = "";
      }
      if (mpImportedElement != NULL)
      {
         Service<ModelServices>()->destroyElement(mpImportedElement);
         mpImportedElement = NULL;
      }
      setImportedRows(-1, -1, 0);
      setImportedColumns(-1, -1, 0);
      setImportedBands(-1, -1, 0);
      setExportedRows(-1, -1, 0);
      setExportedColumns(-1, -1, 0);
      setExportedBands(-1, -1, 0);
   }

   void setOriginalElement(RasterElement* pElement)
   {
      mpOriginalElement = pElement;
      setExportedRows(-1, -1, 0);
      setExportedColumns(-1, -1, 0);
      vector<unsigned int> bands;
      setExportedBands(-1, -1, 0);
   }

   void setImportProcessingLocation(ProcessingLocation procLoc)
   {
      mProcLoc = procLoc;
   }

   /**
    * Uses active numbers
    */
   void setExportedRows(int startRow, int stopRow, unsigned int skipFactor)
   {
      DO_IF(mpOriginalElement == NULL, return);
      RasterDataDescriptor* pDataDesc = dynamic_cast<RasterDataDescriptor*>(mpOriginalElement->getDataDescriptor());
      DO_IF(pDataDesc == NULL, return);
      if (startRow == -1)
      {
         mExportStartRow = pDataDesc->getRows().front();
      }
      else
      {
         mExportStartRow = pDataDesc->getActiveRow(startRow);
      }
      if (stopRow == -1)
      {
         mExportStopRow = pDataDesc->getRows().back();
      }
      else
      {
         mExportStopRow = pDataDesc->getActiveRow(stopRow);
      }
      mExportSkipRows = skipFactor;
   }

   /**
    * Uses active numbers
    */
   void setExportedColumns(int startColumn, int stopColumn, unsigned int skipFactor)
   {
      DO_IF(mpOriginalElement == NULL, return);
      RasterDataDescriptor* pDataDesc = dynamic_cast<RasterDataDescriptor*>(mpOriginalElement->getDataDescriptor());
      DO_IF(pDataDesc == NULL, return);
      if (startColumn == -1)
      {
         mExportStartColumn = pDataDesc->getColumns().front();
      }
      else
      {
         mExportStartColumn = pDataDesc->getActiveColumn(startColumn);
      }
      if (stopColumn == -1)
      {
         mExportStopColumn = pDataDesc->getColumns().back();
      }
      else
      {
         mExportStopColumn = pDataDesc->getActiveColumn(stopColumn);
      }
      mExportSkipColumns = skipFactor;
   }

   /**
    * Uses active numbers
    */
   void setExportedBands(int startBand, int stopBand, unsigned int skipFactor)
   {
      DO_IF(mpOriginalElement == NULL, return);
      RasterDataDescriptor* pDataDesc = dynamic_cast<RasterDataDescriptor*>(mpOriginalElement->getDataDescriptor());
      DO_IF(pDataDesc == NULL, return);
      mExportBands.clear();
      if (startBand != -1 && stopBand != -1)
      {
         for (int i = startBand; i <= stopBand; i += (skipFactor + 1))
         {
            mExportBands.push_back(pDataDesc->getActiveBand(i));
         }
      }
   }

   /**
    * Uses on-disk numbers
    */
   void setImportedRows(int startRow, int stopRow, unsigned int skipFactor)
   {
      mImportStartRow = startRow;
      mImportStopRow = stopRow;
      mImportSkipRows = skipFactor;
   }

   /**
    * Uses on-disk numbers
    */
   void setImportedColumns(int startColumn, int stopColumn, unsigned int skipFactor)
   {
      mImportStartColumn = startColumn;
      mImportStopColumn = stopColumn;
      mImportSkipColumns = skipFactor;
   }

   /**
    * Uses on-disk numbers
    */
   void setImportedBands(int startBand, int stopBand, unsigned int skipFactor)
   {
      mImportBands.clear();
      if (startBand != -1 && stopBand != -1)
      {
         for (int i = startBand; i <= stopBand; i += (skipFactor + 1))
         {
            mImportBands.push_back(i);
         }
      }
   }

   bool performChipOnExportAndReImport()
   {
      bool success = true;
      issearf(mpOriginalElement != NULL);
      string tempPath;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempPath = pTempPath->getFullPathAndName();
      }
      mExportFilename = tempPath + SLASH + "iceChipExport.ice.h5";
      FactoryResource<RasterFileDescriptor> pOutputFileDesc(
            RasterUtilities::generateRasterFileDescriptorForExport(
         dynamic_cast<RasterDataDescriptor*>(mpOriginalElement->getDataDescriptor()),
         mExportFilename, mExportStartRow, mExportStopRow, mExportSkipRows,
         mExportStartColumn, mExportStopColumn, mExportSkipColumns,
         mExportBands));
      issearf(pOutputFileDesc.get() != NULL);

      //export as ice file
      ExporterResource exporter("Ice Exporter", mpOriginalElement, NULL);
      exporter->setFileDescriptor(pOutputFileDesc.get());
      issearf(exporter->execute());

      ImporterResource iceImport("Ice Importer", mExportFilename);
      vector<ImportDescriptor*> iceDesc = iceImport->getImportDescriptors();
      issearf(iceDesc.size() == 1);
      ImportDescriptor* pImportDesc = iceDesc.front();
      issearf(pImportDesc != NULL);
      RasterDataDescriptor* pDataDesc = dynamic_cast<RasterDataDescriptor*>(pImportDesc->getDataDescriptor());
      DimensionDescriptor startRow;
      if (mImportStartRow == -1)
      {
         startRow = pDataDesc->getRows().front();
      }
      else
      {
         startRow = pDataDesc->getOnDiskRow(mImportStartRow);
      }
      DimensionDescriptor stopRow;
      if (mImportStopRow == -1)
      {
         stopRow = pDataDesc->getRows().back();
      }
      else
      {
         stopRow = pDataDesc->getOnDiskRow(mImportStopRow);
      }
      DimensionDescriptor startColumn;
      if (mImportStartColumn == -1)
      {
         startColumn = pDataDesc->getColumns().front();
      }
      else
      {
         startColumn = pDataDesc->getOnDiskColumn(mImportStartColumn);
      }
      DimensionDescriptor stopColumn;
      if (mImportStopColumn == -1)
      {
         stopColumn = pDataDesc->getColumns().back();
      }
      else
      {
         stopColumn = pDataDesc->getOnDiskColumn(mImportStopColumn);
      }
      vector<DimensionDescriptor> bands;
      for (vector<unsigned int>::iterator iter = mImportBands.begin(); iter != mImportBands.end(); ++iter)
      {
         bands.push_back(pDataDesc->getOnDiskBand(*iter));
      }
      RasterUtilities::subsetDataDescriptor(pDataDesc, startRow, stopRow, 
         mImportSkipRows, startColumn, stopColumn,
         mImportSkipColumns, bands);
      pDataDesc->setProcessingLocation(mProcLoc);
      iceImport->execute();
      vector<DataElement*> iceElements = iceImport->getImportedElements();
      issearf(iceElements.size() == 1);
      mpImportedElement = dynamic_cast<RasterElement*>(iceElements.front());
      issearf(mpImportedElement != NULL);

      return success;
   }

   bool testChip()
   {
      bool success = true;
      issearf(mpImportedElement != NULL);
      issearf(mpOriginalElement != NULL);
      RasterDataDescriptor* pChipDesc = dynamic_cast<RasterDataDescriptor*>(mpImportedElement->getDataDescriptor());
      issearf(pChipDesc != NULL);
      RasterDataDescriptor* pSrcDesc = dynamic_cast<RasterDataDescriptor*>(mpOriginalElement->getDataDescriptor());
      issearf(pSrcDesc != NULL);
      const vector<DimensionDescriptor> chipRows = pChipDesc->getRows();
      const vector<DimensionDescriptor> chipColumns = pChipDesc->getColumns();
      const vector<DimensionDescriptor> chipBands = pChipDesc->getBands();
      const vector<DimensionDescriptor> srcRows = pSrcDesc->getRows();
      const vector<DimensionDescriptor> srcColumns = pSrcDesc->getColumns();
      const vector<DimensionDescriptor> srcBands = pSrcDesc->getBands();
      mOriginalRows = extractOriginalNumbers(srcRows);
      mOriginalColumns = extractOriginalNumbers(srcColumns);
      mOriginalBands = extractOriginalNumbers(srcBands);
      mExportedOriginalRows = chipVector(mOriginalRows,
         mExportStartRow.getActiveNumber(), mExportStopRow.getActiveNumber(), mExportSkipRows);
      mImportedOriginalRows = chipVector(mExportedOriginalRows, 
         mImportStartRow, mImportStopRow, mImportSkipRows);
      mExportedOriginalColumns = chipVector(mOriginalColumns,
         mExportStartColumn.getActiveNumber(), mExportStopColumn.getActiveNumber(), mExportSkipColumns);
      mImportedOriginalColumns = chipVector(mExportedOriginalColumns, 
         mImportStartColumn, mImportStopColumn, mImportSkipColumns);
      vector<unsigned int> bandsToExport;
      bandsToExport.resize(mExportBands.size());
      for (vector<DimensionDescriptor>::const_iterator iter = mExportBands.begin(); iter != mExportBands.end(); ++iter)
      {
         DimensionDescriptor dim = *iter;
         if (dim.isActiveNumberValid())
         {
            bandsToExport.push_back(dim.getActiveNumber());
         }
      }
      mExportedOriginalBands = chipVectorByPos(mOriginalBands, bandsToExport);
      mImportedOriginalBands = chipVectorByPos(mExportedOriginalBands, mImportBands);
      vector<unsigned int> chipActualRowOrigNums = extractOriginalNumbers(chipRows);
      vector<unsigned int> chipActualColumnOrigNums = extractOriginalNumbers(chipColumns);
      vector<unsigned int> chipActualBandOrigNums = extractOriginalNumbers(chipBands);

      //Test Original Numbers Were Chipped Correctly.
      issearf(chipActualRowOrigNums.size() == mImportedOriginalRows.size());
      issearf(std::equal(chipActualRowOrigNums.begin(), chipActualRowOrigNums.end(), mImportedOriginalRows.begin()));
      issearf(chipActualColumnOrigNums.size() == mImportedOriginalColumns.size());
      issearf(std::equal(chipActualColumnOrigNums.begin(), chipActualColumnOrigNums.end(), mImportedOriginalColumns.begin()));
      issearf(chipActualBandOrigNums.size() == mImportedOriginalBands.size());
      issearf(std::equal(chipActualBandOrigNums.begin(), chipActualBandOrigNums.end(), mImportedOriginalBands.begin()));

      //Test that cube data was chipped correctly.
      int rowSpread = ceil(chipRows.size() / 10.0);
      int columnSpread = ceil(chipColumns.size() / 10.0);
      int bandSpread = ceil(chipBands.size() / 10.0);

      for (unsigned int row = 0; row < chipRows.size(); row += rowSpread)
      {
         DimensionDescriptor chipRowDim = chipRows[row];
         DimensionDescriptor srcRowDim = getOriginalRow(pSrcDesc, chipRowDim);
         issearf(testDimension(srcRowDim, chipRowDim));
         for (unsigned int column = 0; column < chipColumns.size(); column += columnSpread)
         {
            DimensionDescriptor chipColumnDim = chipColumns[column];
            DimensionDescriptor srcColumnDim = getOriginalColumn(pSrcDesc, chipColumnDim);
            issearf(testDimension(srcColumnDim, chipColumnDim));
            for (unsigned int band = 0; band < chipBands.size(); band += bandSpread)
            {
               DimensionDescriptor chipBandDim = chipBands[band];
               DimensionDescriptor srcBandDim = getOriginalBand(pSrcDesc, chipBandDim);
               issearf(testDimension(srcBandDim, chipBandDim));

               //check raw cube data to ensure it was chipped correctly.
               issearf(mpImportedElement->getPixelValue(chipColumnDim, chipRowDim, 
                  chipBandDim, COMPLEX_MAGNITUDE) == 
                  mpOriginalElement->getPixelValue(srcColumnDim, srcRowDim, 
                  srcBandDim, COMPLEX_MAGNITUDE));
            }
         }
      }

      //Test that displayed bands were chipped correctly.
      issearf(checkDisplayedBand(GRAY));
      issearf(checkDisplayedBand(RED));
      issearf(checkDisplayedBand(GREEN));
      issearf(checkDisplayedBand(BLUE));

      //Test Statistics chipping
      for (unsigned int band = 0; band < chipBands.size(); band += bandSpread)
      {
         DimensionDescriptor chipBandDim = chipBands[band];
         DimensionDescriptor srcBandDim = getOriginalBand(pSrcDesc, chipBandDim);

         //check statistics to ensure they were chipped correctly.
         Statistics* pSrcStats = mpOriginalElement->getStatistics(srcBandDim);
         issearf(pSrcStats != NULL);
         Statistics* pChipStats = mpImportedElement->getStatistics(chipBandDim);
         issearf(pChipStats != NULL);
         if (pChipDesc->getRows() == pSrcDesc->getRows() &&
            pChipDesc->getColumns() == pSrcDesc->getColumns())
         {
            issearf(pSrcStats->areStatisticsCalculated() == pChipStats->areStatisticsCalculated());
            if (pSrcStats->areStatisticsCalculated())
            {
               issearf(pSrcStats->getAverage() == pChipStats->getAverage());
               issearf(pSrcStats->getMax() == pChipStats->getMax());
               issearf(pSrcStats->getMin() == pChipStats->getMin());
               issearf(pSrcStats->getStandardDeviation() == pChipStats->getStandardDeviation());
            }
         
            issearf(pSrcStats->getStatisticsResolution() == pChipStats->getStatisticsResolution());
            const BadValues* pSrcBadValues = pSrcStats->getBadValues();
            const BadValues* pChipBadValues = pChipStats->getBadValues();
            issearf(pSrcBadValues != NULL && pChipBadValues != NULL);
            issearf(pSrcBadValues->compare(pChipBadValues));
         }
      }

      return success;
   }

   template<typename T>
   bool checkBandMetadataChipping(const string& path)
   {
      bool success = true;
      issearf(mpImportedElement != NULL);
      issearf(mpOriginalElement != NULL);
      RasterDataDescriptor* pChipDesc = dynamic_cast<RasterDataDescriptor*>(mpImportedElement->getDataDescriptor());
      issearf(pChipDesc != NULL);
      RasterDataDescriptor* pSrcDesc = dynamic_cast<RasterDataDescriptor*>(mpOriginalElement->getDataDescriptor());
      issearf(pSrcDesc != NULL);

      DynamicObject* pSrcMetadata = pSrcDesc->getMetadata();
      issearf(pSrcMetadata != NULL);
      DynamicObject* pChipMetadata = pChipDesc->getMetadata();
      issearf(pChipMetadata != NULL);

      vector<T>* pSrcBandMetadata = dv_cast<vector<T> >(&pSrcMetadata->getAttributeByPath(path));
      vector<T>* pChipBandMetadata = dv_cast<vector<T> >(&pChipMetadata->getAttributeByPath(path));
      if (pSrcBandMetadata != NULL)
      {
         issearf(pChipBandMetadata != NULL);        
         issearf(pSrcBandMetadata->size() == mOriginalBands.size());
         issearf(pChipBandMetadata->size() == mImportedOriginalBands.size());
         vector<unsigned int> bandsToExport;
         bandsToExport.resize(mExportBands.size());
         for (vector<DimensionDescriptor>::const_iterator iter = mExportBands.begin(); iter != mExportBands.end(); ++iter)
         {
            DimensionDescriptor dim = *iter;
            if (dim.isActiveNumberValid())
            {
               bandsToExport.push_back(dim.getActiveNumber());
            }
         }
         vector<T> expectedChipMetadata = chipVectorByPos(chipVectorByPos(*pSrcBandMetadata, bandsToExport), mImportBands);
         issearf(expectedChipMetadata.size() == pChipBandMetadata->size());
         issearf(equal(expectedChipMetadata.begin(), expectedChipMetadata.end(), pChipBandMetadata->begin()));
      }
      return success;
   }

   template<typename T>
   bool checkRowMetadataChipping(const string& path)
   {
      bool success = true;
      issearf(mpImportedElement != NULL);
      issearf(mpOriginalElement != NULL);
      RasterDataDescriptor* pChipDesc = dynamic_cast<RasterDataDescriptor*>(mpImportedElement->getDataDescriptor());
      issearf(pChipDesc != NULL);
      RasterDataDescriptor* pSrcDesc = dynamic_cast<RasterDataDescriptor*>(mpOriginalElement->getDataDescriptor());
      issearf(pSrcDesc != NULL);

      DynamicObject* pSrcMetadata = pSrcDesc->getMetadata();
      issearf(pSrcMetadata != NULL);
      DynamicObject* pChipMetadata = pChipDesc->getMetadata();
      issearf(pChipMetadata != NULL);

      vector<T>* pSrcRowMetadata = dv_cast<vector<T> >(&pSrcMetadata->getAttributeByPath(path));
      vector<T>* pChipRowMetadata = dv_cast<vector<T> >(&pChipMetadata->getAttributeByPath(path));
      if (pSrcRowMetadata != NULL)
      {
         issearf(pChipRowMetadata != NULL);        
         issearf(pSrcRowMetadata->size() == mOriginalRows.size());
         issearf(pChipRowMetadata->size() == mImportedOriginalRows.size());
         vector<T> expectedChipMetadata = chipVector(chipVector(*pSrcRowMetadata,
            mExportStartRow.getActiveNumber(), mExportStopRow.getActiveNumber(), mExportSkipRows),
            mImportStartRow, mImportStopRow, mImportSkipRows);
         issearf(expectedChipMetadata.size() == pChipRowMetadata->size());
         issearf(equal(expectedChipMetadata.begin(), expectedChipMetadata.end(), pChipRowMetadata->begin()));
      }
      return success;
   }

   template<typename T>
   bool checkColumnMetadataChipping(const string& path)
   {
      bool success = true;
      issearf(mpImportedElement != NULL);
      issearf(mpOriginalElement != NULL);
      RasterDataDescriptor* pChipDesc = dynamic_cast<RasterDataDescriptor*>(mpImportedElement->getDataDescriptor());
      issearf(pChipDesc != NULL);
      RasterDataDescriptor* pSrcDesc = dynamic_cast<RasterDataDescriptor*>(mpOriginalElement->getDataDescriptor());
      issearf(pSrcDesc != NULL);

      DynamicObject* pSrcMetadata = pSrcDesc->getMetadata();
      issearf(pSrcMetadata != NULL);
      DynamicObject* pChipMetadata = pChipDesc->getMetadata();
      issearf(pChipMetadata != NULL);

      vector<T>* pSrcColumnMetadata = dv_cast<vector<T> >(&pSrcMetadata->getAttributeByPath(path));
      vector<T>* pChipColumnMetadata = dv_cast<vector<T> >(&pChipMetadata->getAttributeByPath(path));
      if (pSrcColumnMetadata != NULL)
      {
         issearf(pChipColumnMetadata != NULL);        
         issearf(pSrcColumnMetadata->size() == mOriginalColumns.size());
         issearf(pChipColumnMetadata->size() == mImportedOriginalColumns.size());
         vector<T> expectedChipMetadata = chipVector(chipVector(*pSrcColumnMetadata,
            mExportStartColumn.getActiveNumber(), mExportStopColumn.getActiveNumber(), mExportSkipColumns),
            mImportStartColumn, mImportStopColumn, mImportSkipColumns);
         issearf(expectedChipMetadata.size() == pChipColumnMetadata->size());
         issearf(equal(expectedChipMetadata.begin(), expectedChipMetadata.end(), pChipColumnMetadata->begin()));
      }
      return success;
   }

private:

   bool checkDisplayedBand(RasterChannelType channel)
   {
      bool success = true;
      issearf(mpImportedElement != NULL);
      issearf(mpOriginalElement != NULL);
      RasterDataDescriptor* pChipDesc = dynamic_cast<RasterDataDescriptor*>(mpImportedElement->getDataDescriptor());
      issearf(pChipDesc != NULL);
      RasterDataDescriptor* pSrcDesc = dynamic_cast<RasterDataDescriptor*>(mpOriginalElement->getDataDescriptor());
      issearf(pSrcDesc != NULL);

      DimensionDescriptor srcBand = pSrcDesc->getDisplayBand(channel);
      DimensionDescriptor chipBand = pChipDesc->getDisplayBand(channel);
      issearf(chipBand.isOriginalNumberValid());
      issearf(srcBand.isOriginalNumberValid());
      if (find(mExportedOriginalBands.begin(), mExportedOriginalBands.end(), srcBand.getOriginalNumber()) != mExportedOriginalBands.end())
      {
         issearf(chipBand.getOriginalNumber() == srcBand.getOriginalNumber());
      }
      else
      {
         issearf(chipBand.getOriginalNumber() == mExportedOriginalBands.front());
      }

      return success;
   }

   vector<unsigned int> extractOriginalNumbers(const vector<DimensionDescriptor>& dims)
   {
      vector<unsigned int> origNums;
      origNums.reserve(dims.size());
      for (vector<DimensionDescriptor>::const_iterator iter = dims.begin(); iter != dims.end(); ++iter)
      {
         DimensionDescriptor dim = *iter;
         if (dim.isOriginalNumberValid())
         {
            origNums.push_back(dim.getOriginalNumber());
         }
      }
      return origNums;
   }

   template<typename T>
   vector<T> chipVector(const vector<T>& originalVector, int startPos, int stopPos, unsigned int skipFactor)
   {
      vector<T> outputVector;
      if (startPos == -1)
      {
         startPos = 0;
      }
      if (stopPos == -1)
      {
         stopPos = originalVector.size();
      }
      outputVector.reserve(stopPos - startPos);
      for (int i = startPos; i <= stopPos; i += (skipFactor + 1))
      {
         if (i >= 0 && i < static_cast<int>(originalVector.size()))
         {
            outputVector.push_back(originalVector[i]);
         }
      }
      return outputVector;
   }

   template<typename T>
   vector<T> chipVectorByPos(const vector<T>& originalVector, const vector<unsigned int>& positionsToKeep)
   {
      if (!positionsToKeep.empty())
      {
         vector<T> outputVector;
         outputVector.reserve(positionsToKeep.size());
         for (unsigned int i = 0; i < positionsToKeep.size(); ++i)
         {
            unsigned int indexToKeep = positionsToKeep[i];
            if (indexToKeep > 0 && indexToKeep < originalVector.size())
            {
               outputVector.push_back(originalVector[indexToKeep]);
            }
         }
         return outputVector;
      }
      else
      {
         return originalVector;
      }
   }

   bool testDimension(const DimensionDescriptor& origDim, const DimensionDescriptor& chipDim)
   {
      bool success = true;
      issearf(origDim.isValid());
      issearf(chipDim.isValid());
      issearf(origDim.isOriginalNumberValid() == chipDim.isOriginalNumberValid() == true);
      issearf(origDim.getOriginalNumber() == chipDim.getOriginalNumber());
      return success;
   }

   DimensionDescriptor getOriginalRow(RasterDataDescriptor* pDesc, DimensionDescriptor row)
   {
      DimensionDescriptor retVal;
      if (row.isOriginalNumberValid())
      {
         retVal = pDesc->getOriginalRow(row.getOriginalNumber());
      }
      return retVal;
   }

   DimensionDescriptor getOriginalColumn(RasterDataDescriptor* pDesc, DimensionDescriptor column)
   {
      DimensionDescriptor retVal;
      if (column.isOriginalNumberValid())
      {
         retVal = pDesc->getOriginalColumn(column.getOriginalNumber());
      }
      return retVal;
   }

   DimensionDescriptor getOriginalBand(RasterDataDescriptor* pDesc, DimensionDescriptor band)
   {
      DimensionDescriptor retVal;
      if (band.isOriginalNumberValid())
      {
         retVal = pDesc->getOriginalBand(band.getOriginalNumber());
      }
      return retVal;
   }

   RasterElement* mpOriginalElement;
   std::string mExportFilename;
   RasterElement* mpImportedElement;
   ProcessingLocation mProcLoc;

   DimensionDescriptor mExportStartRow;
   DimensionDescriptor mExportStopRow;
   unsigned int mExportSkipRows;
   DimensionDescriptor mExportStartColumn;
   DimensionDescriptor mExportStopColumn;
   unsigned int mExportSkipColumns;
   vector<DimensionDescriptor> mExportBands;

   int mImportStartRow;
   int mImportStopRow;
   unsigned int mImportSkipRows;
   int mImportStartColumn;
   int mImportStopColumn;
   unsigned int mImportSkipColumns;
   vector<unsigned int> mImportBands;

   vector<unsigned int> mOriginalRows;
   vector<unsigned int> mOriginalColumns;
   vector<unsigned int> mOriginalBands;

   vector<unsigned int> mExportedOriginalRows;
   vector<unsigned int> mExportedOriginalColumns;
   vector<unsigned int> mExportedOriginalBands;

   vector<unsigned int> mImportedOriginalRows;
   vector<unsigned int> mImportedOriginalColumns;
   vector<unsigned int> mImportedOriginalBands;

};

class IceChipTestCase : public TestCase
{
public:
   IceChipTestCase() : TestCase("IceChip")
   {
   }

   static const std::string& getRowMetadataName()
   {
      static string retValue = SPECIAL_METADATA_NAME + "/" + ROW_METADATA_NAME + "/Row Data";
      return retValue;
   }

   static const std::string& getColumnMetadataName()
   {
      static string retValue = SPECIAL_METADATA_NAME + "/" + COLUMN_METADATA_NAME + "/Column Data";
      return retValue;
   }

   RasterElement* createTestCube(const std::string& name, unsigned int rows, unsigned int columns,
      unsigned int bands, InterleaveFormatType interleave)
   {
      RasterElement* pElement = RasterUtilities::createRasterElement(name, rows,
                  columns, bands, INT4UBYTES, interleave);
      DO_IF(pElement == NULL, return NULL);
      unsigned int* pData = reinterpret_cast<unsigned int*>(pElement->getRawData());
      size_t totalSize = rows * columns * bands;
      for (size_t count = 0; count < totalSize; ++count)
      {
         pData[count] = count;
      }

      FactoryResource<DynamicObject> pMetadata;
      vector<double> startWave;
      startWave.reserve(bands);
      vector<double> centerWave;
      centerWave.reserve(bands);
      vector<double> endWave;
      endWave.reserve(bands);
      vector<string> bandNames;
      bandNames.reserve(bands);
      for (unsigned int currentBand = 1; currentBand <= bands; ++currentBand)
      {
         startWave.push_back(currentBand);
         centerWave.push_back(currentBand + bands);
         endWave.push_back(currentBand + (bands * 2));
         ostringstream formatter;
         formatter << "NameForBand" << currentBand;
         bandNames.push_back(formatter.str());
      }
      pMetadata->setAttributeByPath(START_WAVELENGTHS_METADATA_PATH, startWave);
      pMetadata->setAttributeByPath(CENTER_WAVELENGTHS_METADATA_PATH, centerWave);
      pMetadata->setAttributeByPath(END_WAVELENGTHS_METADATA_PATH, endWave);
      string bandNamesPath[] = { SPECIAL_METADATA_NAME, BAND_METADATA_NAME, NAMES_METADATA_NAME, END_METADATA_NAME };
      pMetadata->setAttributeByPath(bandNamesPath, bandNames);

      vector<unsigned int> rowData;
      rowData.reserve(rows);
      for (unsigned int currentRow = 1; currentRow <= rows; ++currentRow)
      {
         rowData.push_back(currentRow);
      }
      pMetadata->setAttributeByPath(getRowMetadataName(), rowData);

      vector<unsigned int> columnData;
      columnData.reserve(rows);
      for (unsigned int currentColumn = 1; currentColumn <= columns; ++currentColumn)
      {
         columnData.push_back(currentColumn);
      }
      pMetadata->setAttributeByPath(getColumnMetadataName(), columnData);
      RasterDataDescriptor* pDataDesc = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      pDataDesc->setMetadata(pMetadata.get());

      return pElement;
   }

   bool run()
   {
      bool success = true;

      vector<InterleaveFormatType> interleaves;
      interleaves.push_back(BSQ);
      interleaves.push_back(BIP);
      interleaves.push_back(BIL);

      vector<ProcessingLocation> processingLocations = StringUtilities::getAllEnumValues<ProcessingLocation>();
      for (vector<InterleaveFormatType>::iterator interleaveIter = interleaves.begin();
           interleaveIter != interleaves.end();
           ++interleaveIter)
      {
         for (vector<ProcessingLocation>::iterator procLocIter = processingLocations.begin();
              procLocIter != processingLocations.end();
              ++procLocIter)
         {
            ProcessingLocation procLoc = *procLocIter;
            InterleaveFormatType interleave = *interleaveIter;
            string interleaveStr = StringUtilities::toDisplayString(interleave);
            string procLocStr = StringUtilities::toDisplayString(procLoc);
            unsigned int numRows = 50;
            unsigned int numColumns = 70;
            ModelResource<RasterElement> pElement(createTestCube("iceChip1", numRows, numColumns, 40, interleave));
            issearf(pElement.get() != NULL);
            RasterDataDescriptor* pDataDesc = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
            pDataDesc->setDisplayBand(GRAY, pDataDesc->getActiveBand(7)); //will be chipped out during export for Test3 and Test4
            pDataDesc->setDisplayBand(RED, pDataDesc->getActiveBand(10)); //never chipped
            pDataDesc->setDisplayBand(GREEN, pDataDesc->getActiveBand(22)); //never chipped
            pDataDesc->setDisplayBand(BLUE, pDataDesc->getActiveBand(35)); //will be chipped out during export for Test3 and Test4
            vector<int> badValues1;
            badValues1.push_back(20);
            badValues1.push_back(30);
            vector<int> badValues2;
            badValues2.push_back(-20);
            badValues2.push_back(50);
            pElement->getStatistics(pDataDesc->getActiveBand(16))->setBadValues(badValues1); //never chipped
            pElement->getStatistics(pDataDesc->getActiveBand(16))->setStatisticsResolution(2); //never chipped
            pElement->getStatistics(pDataDesc->getActiveBand(28))->setBadValues(badValues2); //never chipped
            pElement->getStatistics(pDataDesc->getActiveBand(28))->setStatisticsResolution(3); //never chipped
            pElement->getStatistics(pDataDesc->getActiveBand(11))->setBadValues(badValues1); //will be chipped out during export for Test3 and Test4
            pElement->getStatistics(pDataDesc->getActiveBand(11))->setStatisticsResolution(2); //will be chipped out during export for Test3 and Test4
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Put in code to test geocoordinate chipping in the Ice file. (kstreith)")

            IceChipper chipper;
            chipper.setOriginalElement(pElement.get());
            chipper.setImportProcessingLocation(procLoc);

            //Test1: import and export with no chipping (make sure no statistics have been generated up to this point.
            cout << "Running Test1 with " << interleaveStr << " interleave and " << procLocStr << " processing location." << endl;
            issearf(chipper.performChipOnExportAndReImport());
            issearf(chipper.testChip());
            issearf(chipper.checkBandMetadataChipping<double>(START_WAVELENGTHS_METADATA_PATH));
            issearf(chipper.checkBandMetadataChipping<double>(CENTER_WAVELENGTHS_METADATA_PATH));
            issearf(chipper.checkBandMetadataChipping<double>(END_WAVELENGTHS_METADATA_PATH));
            issearf(chipper.checkBandMetadataChipping<string>(SPECIAL_METADATA_NAME + "/" + BAND_METADATA_NAME + "/" + NAMES_METADATA_NAME));
            issearf(chipper.checkRowMetadataChipping<unsigned int>(getRowMetadataName()));
            issearf(chipper.checkColumnMetadataChipping<unsigned int>(getColumnMetadataName()));
            chipper.clearChip();

            //Test2: import and export with no chipping

            //force statistics generation (this is after Test1 to ensure that Test1 tests exporting cubes with no generated statistics)
            cout << "Running Test2 with " << interleaveStr << " interleave and " << procLocStr << " processing location." << endl;
            pElement->getStatistics(pDataDesc->getActiveBand(16))->getAverage(); //never chipped
            pElement->getStatistics(pDataDesc->getActiveBand(28))->getAverage(); //never chipped
            pElement->getStatistics(pDataDesc->getActiveBand(11))->getAverage(); //will be chipped out during export for Test3 and Test4
            issearf(chipper.performChipOnExportAndReImport());
            issearf(chipper.testChip());
            issearf(chipper.checkBandMetadataChipping<double>(START_WAVELENGTHS_METADATA_PATH));
            issearf(chipper.checkBandMetadataChipping<double>(CENTER_WAVELENGTHS_METADATA_PATH));
            issearf(chipper.checkBandMetadataChipping<double>(END_WAVELENGTHS_METADATA_PATH));
            issearf(chipper.checkBandMetadataChipping<string>(SPECIAL_METADATA_NAME + "/" + BAND_METADATA_NAME + "/" + NAMES_METADATA_NAME));
            issearf(chipper.checkRowMetadataChipping<unsigned int>(getRowMetadataName()));
            issearf(chipper.checkColumnMetadataChipping<unsigned int>(getColumnMetadataName()));
            chipper.clearChip();

            //Test3: export chipping only
            cout << "Running Test3 with " << interleaveStr << " interleave and " << procLocStr << " processing location." << endl;
            chipper.setExportedRows(4, 40, 1);
            chipper.setExportedColumns(5, 66, 1);
            chipper.setExportedBands(2, 36, 1);
            issearf(chipper.performChipOnExportAndReImport());
            issearf(chipper.testChip());
            issearf(chipper.checkBandMetadataChipping<double>(START_WAVELENGTHS_METADATA_PATH));
            issearf(chipper.checkBandMetadataChipping<double>(CENTER_WAVELENGTHS_METADATA_PATH));
            issearf(chipper.checkBandMetadataChipping<double>(END_WAVELENGTHS_METADATA_PATH));
            issearf(chipper.checkBandMetadataChipping<string>(SPECIAL_METADATA_NAME + "/" + BAND_METADATA_NAME + "/" + NAMES_METADATA_NAME));
            issearf(chipper.checkRowMetadataChipping<unsigned int>(getRowMetadataName()));
            issearf(chipper.checkColumnMetadataChipping<unsigned int>(getColumnMetadataName()));
            chipper.clearChip();

            //Test4: import and export chipping
            if (interleave == BSQ || procLoc != ON_DISK_READ_ONLY)
            {
               cout << "Running Test4 with " << interleaveStr << " interleave and " << procLocStr << " processing location." << endl;
               chipper.setExportedRows(4, 40, 1);
               chipper.setExportedColumns(5, 66, 1);
               chipper.setExportedBands(2, 36, 1);
               chipper.setImportedRows(2, 14, 2);
               chipper.setImportedColumns(3, 12, 2);
               chipper.setImportedBands(4, 13, 2);
               issearf(chipper.performChipOnExportAndReImport());
               issearf(chipper.testChip());
               issearf(chipper.checkBandMetadataChipping<double>(START_WAVELENGTHS_METADATA_PATH));
               issearf(chipper.checkBandMetadataChipping<double>(CENTER_WAVELENGTHS_METADATA_PATH));
               issearf(chipper.checkBandMetadataChipping<double>(END_WAVELENGTHS_METADATA_PATH));
               issearf(chipper.checkBandMetadataChipping<string>(SPECIAL_METADATA_NAME + "/" + BAND_METADATA_NAME + "/" + NAMES_METADATA_NAME));
               issearf(chipper.checkRowMetadataChipping<unsigned int>(getRowMetadataName()));
               issearf(chipper.checkColumnMetadataChipping<unsigned int>(getColumnMetadataName()));
               chipper.clearChip();
            }
         }
      }

      // Test On-Disk Read-Only chipping of bands for BSQ interleave
      unsigned int numRows = 50;
      unsigned int numColumns = 70;
      ModelResource<RasterElement> pElement(createTestCube("iceChip1", numRows, numColumns, 40, BSQ));
      issearf(pElement.get() != NULL);
      RasterDataDescriptor* pDataDesc = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      pDataDesc->setDisplayBand(GRAY, pDataDesc->getActiveBand(4)); //never chipped
      pDataDesc->setDisplayBand(RED, pDataDesc->getActiveBand(4)); //never chipped
      pDataDesc->setDisplayBand(GREEN, pDataDesc->getActiveBand(4)); //never chipped
      pDataDesc->setDisplayBand(BLUE, pDataDesc->getActiveBand(4)); //never chipped
      IceChipper chipper;
      chipper.setOriginalElement(pElement.get());
      chipper.setImportProcessingLocation(ON_DISK_READ_ONLY);
      chipper.setImportedBands(4, 13, 2);
      issearf(chipper.performChipOnExportAndReImport());
      issearf(chipper.testChip());
      issearf(chipper.checkBandMetadataChipping<double>(START_WAVELENGTHS_METADATA_PATH));
      issearf(chipper.checkBandMetadataChipping<double>(CENTER_WAVELENGTHS_METADATA_PATH));
      issearf(chipper.checkBandMetadataChipping<double>(END_WAVELENGTHS_METADATA_PATH));
      issearf(chipper.checkBandMetadataChipping<string>(SPECIAL_METADATA_NAME + "/" + BAND_METADATA_NAME + "/" + NAMES_METADATA_NAME));
      issearf(chipper.checkRowMetadataChipping<unsigned int>(getRowMetadataName()));
      issearf(chipper.checkColumnMetadataChipping<unsigned int>(getColumnMetadataName()));
      chipper.clearChip();



      return success;
   }
};

class IcePagerTestCase : public TestCase
{
public:
   IcePagerTestCase() : TestCase("IcePager")
   {
   }

   bool run()
   {
      bool success = true;
      string filename = TestUtilities::getTestDataPath() + "Ice/cube512x512x256x2ulsb.re.ice.h5";
      ImporterResource import1("Ice Importer", filename, NULL, true);
      vector<ImportDescriptor*> descriptors1 = import1->getImportDescriptors();
      issearf(descriptors1.size() == 1);
      ImportDescriptor* pDesc1 = descriptors1.front();
      issearf(pDesc1 != NULL);
      RasterDataDescriptor* pDataDesc1 = dynamic_cast<RasterDataDescriptor*>(pDesc1->getDataDescriptor());
      issearf(pDataDesc1 != NULL);
      pDataDesc1->setProcessingLocation(ON_DISK_READ_ONLY);
      issearf(import1->execute());
      vector<DataElement*> importElements1 = import1->getImportedElements();
      issearf(importElements1.size() == 1);
      ModelResource<RasterElement> pIceCube(dynamic_cast<RasterElement*>(importElements1.front()));
      issearf(pIceCube.get() != NULL);

      const int rows = 512;
      const int columns = 512;
      const int bands = 256;

      RasterDataDescriptor* pIceCubeDesc = dynamic_cast<RasterDataDescriptor*>(pIceCube->getDataDescriptor());
      issearf(pIceCubeDesc != NULL);
      issearf(pIceCubeDesc->getRowCount() == rows);
      issearf(pIceCubeDesc->getColumnCount() == columns);
      issearf(pIceCubeDesc->getBandCount() == bands);
      issearf(pIceCubeDesc->getDataType() == INT2UBYTES);
      issearf(pIceCubeDesc->getInterleaveFormat() == BSQ);

      int minRows[] = {91, 35, 122, 220, 258, 1, 192};
      int maxRows[] = {132, 184, 337, 271, 303, 250, 401};
      int minCols[] = {143, 161, 192, 284, 122, 215, 140};
      int maxCols[] = {347, 408, 376, 437, 212, 254, 205};

      int64_t totalSum[2] = {0, 0};
      for (int runNumber=0; runNumber<2; ++runNumber)
      {
         for (int rep=0; rep<7; rep+=1)
         {
            FactoryResource<DataRequest> pRequest;
            pRequest->setRows(pIceCubeDesc->getActiveRow(minRows[rep]), pIceCubeDesc->getActiveRow(maxRows[rep]), 1);
            pRequest->setColumns(pIceCubeDesc->getActiveColumn(minCols[rep]), pIceCubeDesc->getActiveColumn(maxCols[rep]), 
               maxCols[rep]-minCols[rep]+1);
            pRequest->setInterleaveFormat(BSQ);
            pRequest->setBands(pIceCubeDesc->getActiveBand(1), pIceCubeDesc->getActiveBand(1), 1);
            DataAccessor pAccessor = pIceCube->getDataAccessor(pRequest.release());
            // Ensure that the data we asked for is really available
            for (int row=minRows[rep]; row<=maxRows[rep]; ++row)
            {
               issearf(pAccessor.isValid());
               for (int col=minCols[rep]; col<=maxCols[rep]; ++col)
               {
                  unsigned short *pValue = static_cast<unsigned short*>(pAccessor->getColumn());
                  unsigned short v = *pValue;
                  totalSum[runNumber] += v;
                  pAccessor->nextColumn();
               }
               pAccessor->nextRow();
            }
         }
      }

      issea_eq(totalSum[0], totalSum[1]);

      return success;
   }
};

class IceTestSuite : public TestSuiteNewSession
{
public:
   IceTestSuite() : TestSuiteNewSession( "Ice" )
   {
      addTestCase(new IceChipTestCase);
      addTestCase(new IceVersion0TestCase);
      addTestCase(new IceVersion0_70TestCase);
      addTestCase(new IceVersion0_90TestCase);
      addTestCase(new IceVersion1_0TestCase);
      addTestCase(new IceVersion1_10TestCase);
      addTestCase(new IceVersion1_30TestCase);
      addTestCase(new IceWriteClassificationAndUnitsTestCase);
      addTestCase(new IceWriteComplexDataTestCase);
      addTestCase(new IcePagerTestCase);
   }
};

REGISTER_SUITE( IceTestSuite )
