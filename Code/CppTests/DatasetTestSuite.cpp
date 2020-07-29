/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "AnimationController.h"
#include "AnimationServices.h"
#include "AnnotationElement.h"
#include "AnnotationLayer.h"
#include "AoiElementAdapter.h"
#include "ConfigurationSettings.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "ImportDescriptor.h"
#include "LayerList.h"
#include "ModelServicesImp.h"
#include "ObjectFactoryImp.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialDataWindow.h"
#include "SpatialDataView.h"
#include "SpecialMetadata.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "Units.h"

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <vector>
using namespace std;

class DatasetChangeEventTest : public TestCase
{
public:
   DatasetChangeEventTest() :
      TestCase("ChangeEvent"),
      mpCurrentWindow(NULL)
   {
   }

   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      Service<DesktopServices> pDesktop;
      pDesktop->attach(SIGNAL_NAME(DesktopServices, WindowActivated),
         Slot(this, &DatasetChangeEventTest::windowActivated));

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      SpatialDataWindow* pWindow =
         dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pDataDescriptor->getName(), SPATIAL_DATA_WINDOW));
      issea( pWindow != NULL );

      issea(mpCurrentWindow == NULL);

      string secondFileName = TestUtilities::getTestDataPath() + "tipjul5bands.sio";
      SpatialDataWindow *pWindowNew = NULL;
      pWindowNew = TestUtilities::loadDataSet( secondFileName, "SIO Importer" );
      issea( pWindowNew != NULL );

      SpatialDataWindow* pSDW = static_cast<SpatialDataWindow*>(mpCurrentWindow);
      issea( pSDW != NULL );
      issea( pSDW == pWindowNew );
      issea( pSDW != pWindow );

      string thirdFileName = TestUtilities::getTestDataPath() + "daytonchip.sio";
      SpatialDataWindow *pWindowNew2 = NULL;
      pWindowNew2 = TestUtilities::loadDataSet( thirdFileName, "SIO Importer" );
      issea( pWindowNew2 != NULL );

      pSDW = static_cast<SpatialDataWindow*>(mpCurrentWindow);
      issea( pSDW != NULL );
      issea( pSDW == pWindowNew2 );
      issea( pSDW != pWindow );
      issea( pSDW != pWindowNew );

      issea(TestUtilities::destroyWorkspaceWindow(pWindowNew));
      issea(TestUtilities::destroyWorkspaceWindow(pWindowNew2));
      pDesktop->detach(SIGNAL_NAME(DesktopServices, WindowActivated),
         Slot(this, &DatasetChangeEventTest::windowActivated));

      return success;
   }

   void windowActivated(Subject& subject, const string& signal, const boost::any& value)
   {
      if (dynamic_cast<DesktopServices*>(&subject) != NULL)
      {
         mpCurrentWindow = boost::any_cast<WorkspaceWindow*>(value);
      }
   }

private:
   WorkspaceWindow* mpCurrentWindow;
};

class CreateChipTestCase : public TestCase
{
public:
   CreateChipTestCase() : TestCase("CreateChipTest")
   {
   }

   struct DataSpec
   {
      string mFilename;
      ProcessingLocation mProc;
      // One-based values
      int mStartRow;
      int mStopRow;
      int mSkipRow;
      int mStartCol;
      int mStopCol;
      int mSkipCol;
      int mStartBand;
      int mStopBand;
      int mSkipBand;

      vector<DimensionDescriptor> getRows(const RasterElement *pSrcRasterElement) const
      {
         const RasterDataDescriptor *pSrcDescriptor = 
            dynamic_cast<const RasterDataDescriptor*>(pSrcRasterElement->getDataDescriptor());
         VERIFYRV(pSrcDescriptor != NULL, vector<DimensionDescriptor>());
         const vector<DimensionDescriptor> &srcRows = pSrcDescriptor->getRows();

         return getDims(srcRows, mStartRow, mStopRow, mSkipRow);
      }

      vector<DimensionDescriptor> getColumns(const RasterElement *pSrcRasterElement) const
      {
         const RasterDataDescriptor *pSrcDescriptor = 
            dynamic_cast<const RasterDataDescriptor*>(pSrcRasterElement->getDataDescriptor());
         VERIFYRV(pSrcDescriptor != NULL, vector<DimensionDescriptor>());
         const vector<DimensionDescriptor> &srcCols = pSrcDescriptor->getColumns();

         return getDims(srcCols, mStartCol, mStopCol, mSkipCol);
      }

      vector<DimensionDescriptor> getBands(const RasterElement *pSrcRasterElement) const
      {
         const RasterDataDescriptor *pSrcDescriptor = 
            dynamic_cast<const RasterDataDescriptor*>(pSrcRasterElement->getDataDescriptor());
         VERIFYRV(pSrcDescriptor != NULL, vector<DimensionDescriptor>());
         const vector<DimensionDescriptor> &srcBands = pSrcDescriptor->getBands();

         return getDims(srcBands, mStartBand, mStopBand, mSkipBand);
      }

   private:
      static vector<DimensionDescriptor> getDims(const vector<DimensionDescriptor> &srcDims, int start, int stop, int skip)
      {
         // given to us in 1-based, we need 0-based
         --start;
         --stop;

         DimensionDescriptor startDim;
         if (static_cast<size_t>(start) < srcDims.size() && start >= 0)
         {
            startDim = srcDims[start];
         }
         DimensionDescriptor stopDim;
         if (static_cast<size_t>(stop) < srcDims.size() && stop >= 0)
         {
            stopDim = srcDims[stop];
         }

         if (skip == -1)
         {
            skip = 0;
         }

         return RasterUtilities::subsetDimensionVector(srcDims, startDim, stopDim, skip);
      }
   };

   bool run()
   {
      bool success = true;

      DataSpec spec[] = 
      {
         // test BIP in memory and on disk, full cube
         {TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bip.hdr", IN_MEMORY, 
         -1, -1, -1, -1, -1, -1, -1, -1, -1},
         {TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bip.hdr", ON_DISK_READ_ONLY, 
         -1, -1, -1, -1, -1, -1, -1, -1, -1},
         // test BIP with subsetting rows/columns
         {TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bip.hdr", IN_MEMORY,
         20, 200, -1, 30, 400, -1, -1, -1, -1},
         // test BIP with skip row/columns
         {TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bip.hdr", IN_MEMORY,
         1, 512, 2, 1, 512, 2, -1, -1, -1},
         // test BIP with skip bands
         {TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bip.hdr", IN_MEMORY,
         -1, -1, -1, -1, -1, -1, 1, 5, 2},
         // test BSQ on disk (since it would be rearranged to BIP in memory)
         {TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bsq.hdr",
         ON_DISK_READ_ONLY, -1, -1, -1, 
         -1, -1, -1, -1, -1, -1},
         // test BSQ subsetting rows/columns
         {TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bsq.hdr",
         ON_DISK_READ_ONLY, 20, 200, -1, 
         30, 400, -1, -1, -1, -1},
         // test BSQ skipping rows/columns
         {TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bsq.hdr",
         ON_DISK_READ_ONLY, 1, 512, 2, 
         1, 512, 2, -1, -1, -1},
         // test BSQ skipping bands
         {TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bsq.hdr",
         ON_DISK_READ_ONLY, -1, -1, -1, 
         -1, -1, -1, 1, 6, 2},
         // test BIL on disk (since it would be rearranged to BIP in memory)
         {TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bil.hdr",
         ON_DISK_READ_ONLY, -1, -1, -1, 
         -1, -1, -1, -1, -1, -1},
         // test BIL subsetting rows/columns
         {TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bil.hdr",
         ON_DISK_READ_ONLY, 20, 200, -1, 
         30, 400, -1, -1, -1, -1},
         // test BIL skipping rows/columns
         {TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bil.hdr",
         ON_DISK_READ_ONLY, 1, 512, 2, 
         1, 512, 2, -1, -1, -1},
         // test BIL skipping bands
         {TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bil.hdr",
         ON_DISK_READ_ONLY, -1, -1, -1, 
         -1, -1, -1, 1, 6, 2},
      };

      for (unsigned int i = 0; i < sizeof(spec)/sizeof(spec[0]); ++i)
      {

         ImporterResource imp("ENVI Importer", spec[i].mFilename);
         vector<ImportDescriptor*> descriptors = imp->getImportDescriptors();
         issearf(descriptors.size() == 1);
         ImportDescriptor* pImportDescriptor = descriptors.front();
         issearf(pImportDescriptor != NULL);
         RasterDataDescriptor *pDd = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
         issearf(pDd != NULL);
         if (pDd->getProcessingLocation() != spec[i].mProc)
         {
            pDd->setProcessingLocation(spec[i].mProc);
         }
         imp->execute();
         vector<DataElement*> elements = imp->getImportedElements();
         issearf(elements.size() == 1);
         ModelResource<RasterElement> pSrc(dynamic_cast<RasterElement*>(elements.front()));
         issearf(pSrc.get() != NULL);

         ModelResource<RasterElement> pChip(pSrc->createChip(NULL, "_chip", spec[i].getRows(pSrc.get()),
            spec[i].getColumns(pSrc.get()), spec[i].getBands(pSrc.get())));
         issearf(pChip.get() != NULL);

         issea(testChip(pSrc.get(), pChip.get(), &spec[i]));

      }

      // test chip of chip
      ImporterResource imp("ENVI Importer", TestUtilities::getTestDataPath() + 
         "CreateChip/cube512x512x10x4flsb.bip.hdr");
      imp->execute();
      vector<DataElement*> elements = imp->getImportedElements();
      issearf(elements.size() == 1);
      ModelResource<RasterElement> pSrc(dynamic_cast<RasterElement*>(elements.front()));
      issearf(pSrc.get() != NULL);

      DataSpec chipChipSpec = {"", IN_MEMORY, 20, 200, -1,
         30, 400, -1, -1, -1, -1};

      ModelResource<RasterElement> pChip1(pSrc->createChip(NULL, "_chip1", chipChipSpec.getRows(pSrc.get()),
         chipChipSpec.getColumns(pSrc.get()), chipChipSpec.getBands(pSrc.get())));
      issearf(pChip1.get() != NULL);

      issea(testChip(pSrc.get(), pChip1.get(), &chipChipSpec));

      RasterDataDescriptor *pDdChip1 = dynamic_cast<RasterDataDescriptor*>(pChip1->getDataDescriptor());
      issearf(pDdChip1 != NULL);
      DataSpec chipChipSpec2 = {"", IN_MEMORY, 10, 50, -1,
         20, 60, -1, -1, -1, -1};

      ModelResource<RasterElement> pChip2(pChip1->createChip(NULL, "_chip2", chipChipSpec2.getRows(pChip1.get()),
         chipChipSpec2.getColumns(pChip1.get()), chipChipSpec2.getBands(pChip1.get())));
      issearf(pChip2.get() != NULL);

      issea(testChip(pChip1.get(), pChip2.get(), &chipChipSpec2));

      // test with empty vectors from a full scene
      ModelResource<RasterElement> pChip3(pSrc->createChip(NULL, "_chip3", 
         vector<DimensionDescriptor>(),
         vector<DimensionDescriptor>(),
         vector<DimensionDescriptor>()));
      issearf(pChip3.get() != NULL);

      DataSpec fullCubeSpec = {"", IN_MEMORY, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

      issea(testChip(pSrc.get(), pChip3.get(), &fullCubeSpec));

      // test with empty vectors from a chip
      ModelResource<RasterElement> pChip4(pChip1->createChip(NULL, "_chip4",
         vector<DimensionDescriptor>(),
         vector<DimensionDescriptor>(),
         vector<DimensionDescriptor>()));
      issearf(pChip4.get() != NULL);

      issea(testChip(pChip1.get(), pChip4.get(), &fullCubeSpec));

      return success;
   }

   bool testChip(RasterElement *pSrc, RasterElement *pChip, const DataSpec *pSpec)
   {
      bool success = true;

      RasterDataDescriptor *pSrcDescriptor = 
         dynamic_cast<RasterDataDescriptor*>(pSrc->getDataDescriptor());
      issea(pSrcDescriptor != NULL);
      RasterFileDescriptor *pSrcFDescriptor = 
         dynamic_cast<RasterFileDescriptor*>(pSrcDescriptor->getFileDescriptor());
      issea(pSrcFDescriptor != NULL);

      RasterDataDescriptor *pChipDescriptor =
         dynamic_cast<RasterDataDescriptor*>(pChip->getDataDescriptor());
      issea(pChipDescriptor != NULL);
      RasterFileDescriptor *pChipFDescriptor = 
         dynamic_cast<RasterFileDescriptor*>(pChipDescriptor->getFileDescriptor());
      issea(pChipFDescriptor != NULL);

      const vector<DimensionDescriptor> &srcOnDiskRows = pSrcFDescriptor->getRows();
      const vector<DimensionDescriptor> &srcOnDiskCols = pSrcFDescriptor->getColumns();
      const vector<DimensionDescriptor> &srcOnDiskBands = pSrcFDescriptor->getBands();

      const vector<DimensionDescriptor> &chipOnDiskRows = pChipFDescriptor->getRows();
      const vector<DimensionDescriptor> &chipOnDiskCols = pChipFDescriptor->getColumns();
      const vector<DimensionDescriptor> &chipOnDiskBands = pChipFDescriptor->getBands();

      const vector<DimensionDescriptor> &srcActiveRows = pSrcDescriptor->getRows();
      const vector<DimensionDescriptor> &srcActiveCols = pSrcDescriptor->getColumns();
      const vector<DimensionDescriptor> &srcActiveBands = pSrcDescriptor->getBands();

      const vector<DimensionDescriptor> &chipActiveRows = pChipDescriptor->getRows();
      const vector<DimensionDescriptor> &chipActiveCols = pChipDescriptor->getColumns();
      const vector<DimensionDescriptor> &chipActiveBands = pChipDescriptor->getBands();

      issea(srcOnDiskRows.size() == chipOnDiskRows.size());
      issea(srcOnDiskCols.size() == chipOnDiskCols.size());
      issea(srcOnDiskBands.size() == chipOnDiskBands.size());

      issea(chipActiveRows.size() == pSpec->getRows(pSrc).size());
      issea(chipActiveCols.size() == pSpec->getColumns(pSrc).size());
      issea(chipActiveBands.size() == pSpec->getBands(pSrc).size());

      issea(pChipDescriptor->getInterleaveFormat() ==
         pSrcDescriptor->getInterleaveFormat());

      int rowSpread = ceil(chipActiveRows.size() / 10.0);
      int colSpread = ceil(chipActiveCols.size() / 10.0);
      int bandSpread = ceil(chipActiveBands.size() / 10.0);
      for (unsigned int row = 0; row < chipActiveRows.size(); row += rowSpread)
      {
         unsigned int onDiskRow = chipActiveRows[row].getOnDiskNumber();
         issea(testDimension(chipOnDiskRows[onDiskRow], srcOnDiskRows[onDiskRow]));
         issea(chipOnDiskRows[onDiskRow].getActiveNumber() == row);

         issea(testDimension(chipActiveRows[row], chipOnDiskRows[onDiskRow]));
         issea(chipActiveRows[row].getActiveNumber() == chipOnDiskRows[onDiskRow].getActiveNumber());
         issea(chipActiveRows[row].getActiveNumber() == row);
         for (unsigned int col = 0; col < chipActiveCols.size(); col += colSpread)
         {
            unsigned int onDiskCol = chipActiveCols[col].getOnDiskNumber();
            issea(testDimension(chipOnDiskCols[onDiskCol], srcOnDiskCols[onDiskCol]));
            issea(chipOnDiskCols[onDiskCol].getActiveNumber() == col);

            issea(testDimension(chipActiveCols[col], chipOnDiskCols[onDiskCol]));
            issea(chipActiveCols[col].getActiveNumber() == chipOnDiskCols[onDiskCol].getActiveNumber());
            issea(chipActiveCols[col].getActiveNumber() == col);
            for (unsigned int band = 0; band < chipActiveBands.size(); band += bandSpread)
            {
               unsigned int onDiskBand = chipActiveBands[band].getOnDiskNumber();
               issea(testDimension(chipOnDiskBands[onDiskBand], srcOnDiskBands[onDiskBand]));
               issea(chipOnDiskBands[onDiskBand].getActiveNumber() == 
                  band);

               issea(testDimension(chipActiveBands[band], chipOnDiskBands[onDiskBand]));
               issea(chipActiveBands[band].getActiveNumber() == 
                  chipOnDiskBands[onDiskBand].getActiveNumber());
               issea(chipActiveBands[band].getActiveNumber() == 
                  band);

               issea(pChip->getPixelValue(chipActiveCols[col], chipActiveRows[row], 
                  chipActiveBands[band], COMPLEX_MAGNITUDE) == 
                  pSrc->getPixelValue(srcOnDiskCols[onDiskCol], srcOnDiskRows[onDiskRow], 
                  srcOnDiskBands[onDiskBand], COMPLEX_MAGNITUDE));

            }
         }
      }

      return success;   
   }

   bool testDimension(DimensionDescriptor left,
      DimensionDescriptor right)
   {
      bool success = true;

      bool valid = left.isOnDiskNumberValid();
      issea(valid == right.isOnDiskNumberValid());
      if (valid)
      {
         issea(left.getOnDiskNumber() == right.getOnDiskNumber());
      }
      valid = left.isOriginalNumberValid();
      issea(valid == right.isOriginalNumberValid());
      if (valid)
      {
         issea(left.getOriginalNumber() == right.getOriginalNumber());
      }

      return success;
   }
};

class ChipMetadataTest : public TestCase
{
public:
   ChipMetadataTest() : TestCase("ChipMetadataTest") {}
   bool run()
   {
      bool success = true;

      ImporterResource imp("ENVI Importer", 
         TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bip.hdr");

      imp->execute();
      vector<DataElement*> elements = imp->getImportedElements();
      issearf(elements.size() == 1);

      ModelResource<RasterElement> pRaster(dynamic_cast<RasterElement*>(elements.front()));
      issearf(pRaster.get() != NULL);

      RasterDataDescriptor *pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
      issearf(pDescriptor != NULL);

      DynamicObject *pMetadata = pRaster->getMetadata();
      issearf(pMetadata != NULL);

      issearf(generateAndSetOriginalVector<int>(ROW_METADATA_NAME, "rowValue", END_METADATA_NAME, 
         pDescriptor->getRowCount(), pMetadata));
      issearf(generateAndSetOriginalVector<double>(COLUMN_METADATA_NAME, "colValue", END_METADATA_NAME,
         pDescriptor->getColumnCount(), pMetadata));
      issearf(generateAndSetOriginalVector<string>(BAND_METADATA_NAME, "bandValue", END_METADATA_NAME,
         pDescriptor->getBandCount(), pMetadata));
      issearf(generateAndSetOriginalVector<float>(ROW_METADATA_NAME, "childDynObj", "rowValues",
         pDescriptor->getRowCount(), pMetadata));

      issearf(pMetadata != NULL);

      vector<DimensionDescriptor> selectedRows = 
         RasterUtilities::subsetDimensionVector(pDescriptor->getRows(),
         pDescriptor->getActiveRow(10), pDescriptor->getActiveRow(30), 2);
      vector<DimensionDescriptor> selectedCols = 
         RasterUtilities::subsetDimensionVector(pDescriptor->getColumns(),
         pDescriptor->getActiveColumn(50), pDescriptor->getActiveColumn(100), 3);
      vector<DimensionDescriptor> selectedBands = 
         RasterUtilities::subsetDimensionVector(pDescriptor->getBands(),
         pDescriptor->getActiveBand(2), pDescriptor->getActiveBand(8), 0);

      issearf(RasterUtilities::chipMetadata(pRaster->getMetadata(), selectedRows, selectedCols, selectedBands));

      const DynamicObject *pChippedMetadata = pDescriptor->getMetadata();
      issearf(pChippedMetadata != NULL);

      issearf(testChippedVector<int>(ROW_METADATA_NAME, "rowValue", END_METADATA_NAME, selectedRows, pChippedMetadata));
      issearf(testChippedVector<double>(COLUMN_METADATA_NAME, "colValue", END_METADATA_NAME,  selectedCols, pChippedMetadata));
      issearf(testChippedVector<string>(BAND_METADATA_NAME, "bandValue", END_METADATA_NAME,  selectedBands, pChippedMetadata));
      issearf(testChippedVector<float>(ROW_METADATA_NAME, "childDynObj", "rowValues", selectedRows, pChippedMetadata));

      return success;
   }

protected:
   template<typename T>
   static bool generateAndSetOriginalVector(const string &metadataNameFirst,
      const string &metadataNameSecond, const string &metadataNameThird,
      unsigned int count, DynamicObject *pMetadata)
   {
      bool success = true;

      vector<T> dimValue;
      for (unsigned int i = 0; i < count; ++i)
      {
         dimValue.push_back(boost::lexical_cast<T>(i));
      }
      string pPath[] = { SPECIAL_METADATA_NAME, metadataNameFirst, metadataNameSecond,
         metadataNameThird, END_METADATA_NAME };
      pMetadata->setAttributeByPath(pPath, dimValue);

      return success;

   }

   template<typename T>
   static bool testChippedVector(const string &metadataNameFirst,
      const string &metadataNameSecond, const string &metadataNameThird,
      const vector<DimensionDescriptor> &selectedDims, const DynamicObject *pChippedMetadata)
   {
      bool success = true;

      string pPath[] = { SPECIAL_METADATA_NAME, metadataNameFirst, 
         metadataNameSecond, metadataNameThird, END_METADATA_NAME };
      const vector<T> *pChippedDimValue = 
         pChippedMetadata->getAttributeByPath(pPath).getPointerToValue<vector<T> >();
      issearf(pChippedDimValue != NULL);
      issearf(pChippedDimValue->size() == selectedDims.size());
      for (unsigned int i = 0; i < selectedDims.size(); ++i)
      {
         issearf(selectedDims[i].getActiveNumber() == boost::lexical_cast<int>(pChippedDimValue->at(i)));
      }

      return success;

   }
};

class DataDescriptorMetadataTest : public TestCase
{
public:
   DataDescriptorMetadataTest() : TestCase("Copy Metadata Object") {}
   bool run()
   {
      bool success = true;

      Service<ObjectFactory> pFactory;
      Service<ModelServices> pModel;

      RasterElement* pCube = TestUtilities::getStandardRasterElement();
      issea( pCube != NULL );

      DataDescriptor *pDescriptor = NULL;
      pDescriptor = pCube->getDataDescriptor()->copy( "MetadataTestSensorData", NULL );
      issea( pDescriptor != NULL );

      DynamicObject *pDynObj = NULL;
      pDynObj = static_cast<DynamicObject*>( pFactory->createObject( "DynamicObject" ) );
      issea( pDynObj != NULL );

      string testData = "This is a test.";

      pDynObj->setAttribute( "testEntry", testData );
      issea( pDynObj->getNumAttributes() == 1 );

      pDescriptor->setMetadata( pDynObj );

      RasterElement* pRasterElement = NULL;
      pRasterElement = static_cast<RasterElement*>( pModel->createElement( pDescriptor ) );
      issea( pRasterElement != NULL );

      pModel->destroyDataDescriptor( pDescriptor );

      const DynamicObject *pObj = NULL;
      pObj = pRasterElement->getDataDescriptor()->getMetadata();
      issea( pObj != NULL );
      issea( pObj->getNumAttributes() != 0 );
      issea( pObj->getNumAttributes() == 1 );

      vector<string> nameVector;
      pObj->getAttributeNames( nameVector );
      issea( nameVector.size() == 1 );
      issea( nameVector.at( 0 ) == "testEntry" );

      const string *pData = NULL;
      pData = pObj->getAttribute( nameVector.at( 0 ) ).getPointerToValue<string>();
      issea( ( pData != NULL ) && ( *pData == "This is a test." ) );

      pFactory->destroyObject( static_cast<void*>( pDynObj ), "DynamicObject" );
      success = tst_assert( pModel->destroyElement( pRasterElement ) ) && success;

      return success;
   }
};

class DatasetAutoImportTest : public TestCase
{
public:
   DatasetAutoImportTest() : TestCase("AutoImport") {}
   bool run()
   {
      bool success = true;

      // The purpose of this test is to test if the Auto Importer can handle a path with slashes
      // in two different directions.  dataDir will initially be "T:/cppTestData/" and I am changing it to
      // be "T:/cppTestData\"
      QString dataDir = QString::fromStdString(TestUtilities::getTestDataPath());
      dataDir.remove( "cppTestData/", Qt::CaseInsensitive ); // I'm changing the direction of the slash on purpose
      string imageFn = ( dataDir + "cppTestData\\fs_test_image.tif" ).toStdString();

      ImporterResource imp( "Auto Importer", imageFn, NULL, false );
      issea( imp->getPlugIn() != NULL );

      issea( imp->execute() );

      RasterElement* pCube = NULL;
      pCube = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( imageFn, "RasterElement", NULL ) );
      issea( pCube != NULL );

      SpatialDataWindow* pWindow =
         dynamic_cast<SpatialDataWindow*>(Service<DesktopServices>()->getWindow(pCube->getName(), SPATIAL_DATA_WINDOW));
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      return success;
   }
};

class DataRequestTestCase : public TestCase
{
public:
   DataRequestTestCase() : TestCase("DataRequest") {}
   bool run()
   {
      bool success = true;

      const unsigned int numRows = 39;          // Arbitrary.
      const unsigned int numColumns = 45;       // Arbitrary.
      const unsigned int numBands = 7;          // Arbitrary.
      const EncodingType dataType = INT4UBYTES; // Must match NativeDataType. Arbitrary but larger than 1 byte.
      typedef unsigned int NativeDataType;      // Must match dataType.

      // Data structures should all contain equivalent data for checking the results of the converter pagers.
      // The values here are arbitrary but each pixel should have a unique value to ensure proper testing.
      const unsigned int numElements = numRows * numColumns * numBands;
      vector<NativeDataType> bipData(numElements);
      vector<NativeDataType> bilData(numElements);
      vector<NativeDataType> bsqData(numElements);
      for (unsigned int value = 0; value < numElements; ++value)
      {
         bipData[((value * numBands) / numElements) + ((value * numBands) % numElements)] = value + 1;
         bilData[(numColumns * ((numBands * numColumns) * (value / numColumns) / numElements)) +
            (((numBands * numColumns) * (value / numColumns)) % numElements) + (value % numColumns)] = value + 1;
         bsqData[value] = value + 1;
      }

      // For example, a 3x3x3 representation could also be defined as follows:
      //NativeDataType bipData[] =
      //{
      //   // Row 1
      //   1, 10, 19,  // Column 1
      //   2, 11, 20,  // Column 2
      //   3, 12, 21,  // Column 3

      //   // Row 2
      //   4, 13, 22,  // Column 1
      //   5, 14, 23,  // Column 2
      //   6, 15, 24,  // Column 3

      //   // Row 3
      //   7, 16, 25,  // Column 1
      //   8, 17, 26,  // Column 2
      //   9, 18, 27   // Column 3
      //};

      //NativeDataType bilData[] =
      //{  // Row 1
      //   1,  2,  3,  // Band 1
      //   10, 11, 12, // Band 2
      //   19, 20, 21, // Band 3

      //   // Row 2
      //   4,  5,  6,  // Band 1
      //   13, 14, 15, // Band 2
      //   22, 23, 24, // Band 3

      //   // Row 3
      //   7,  8,  9,  // Band 1
      //   16, 17, 18, // Band 2
      //   25, 26, 27  // Band 3
      //};

      //NativeDataType bsqData[] =
      //{  // Band 1
      //   1, 2, 3, // Row 1
      //   4, 5, 6, // Row 2
      //   7, 8, 9, // Row 3

      //   // Band 2
      //   10, 11, 12, // Row 1
      //   13, 14, 15, // Row 2
      //   16, 17, 18, // Row 3

      //   // Band 3
      //   19, 20, 21, // Row 1
      //   22, 23, 24, // Row 2
      //   25, 26, 27  // Row 3
      //};

      // Define and execute the tests.
      vector<pair<InterleaveFormatType, const void*> > interleaveTests;
      interleaveTests.push_back(pair<InterleaveFormatType, const void*>(BIP, &bipData[0]));
      interleaveTests.push_back(pair<InterleaveFormatType, const void*>(BIL, &bilData[0]));
      interleaveTests.push_back(pair<InterleaveFormatType, const void*>(BSQ, &bsqData[0]));

      for (vector<pair<InterleaveFormatType, const void*> >::const_iterator testIter = interleaveTests.begin();
         testIter != interleaveTests.end();
         ++testIter)
      {
         // Create a RasterElement and set its data.
         ModelResource<RasterElement> pRaster(RasterUtilities::createRasterElement(
            "test dataset", numRows, numColumns, numBands, dataType, testIter->first));
         memcpy(pRaster->getRawData(), testIter->second, numRows * numColumns * numBands * sizeof(NativeDataType));

         // Test the data against a default data request.
         issearf(testDefault(pRaster.get(), testIter->second));

         // Test the data against every interleave (including its own).
         for (vector<pair<InterleaveFormatType, const void*> >::const_iterator convertIter = interleaveTests.begin();
            convertIter != interleaveTests.end();
            ++convertIter)
         {
            issearf(testInterleaveBand(pRaster.get(), convertIter->second, convertIter->first));
            issearf(testInterleaveColumn(pRaster.get(), convertIter->second, convertIter->first));
            issearf(testInterleaveConcurrentRows(pRaster.get(), convertIter->second, convertIter->first));
         }
      }

      return success;
   }

private:
   bool testDefault(RasterElement* pRaster, const void* pData)
   {
      bool success = true;
      issearf(pRaster != NULL);
      issearf(pData != NULL);

      RasterDataDescriptor* pDescriptor =  dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
      issearf(pDescriptor != NULL);

      // Create a DataRequest and call polish and validate on it with the given descriptor.
      FactoryResource<DataRequest> pRequest;
      issearf(pRequest->polish(pDescriptor));
      issearf(pRequest->validate(pDescriptor));
      issearf(pRequest->getRequestVersion(pDescriptor) == 1);
      issearf(pRequest->getWritable() == false);
      issearf(pRequest->getInterleaveFormat() == pDescriptor->getInterleaveFormat());

      issearf(pRequest->getStartRow() == pDescriptor->getActiveRow(0));
      issearf(pRequest->getStopRow() == pDescriptor->getActiveRow(pDescriptor->getRowCount() - 1));
      issearf(pRequest->getConcurrentRows() == 1);

      issearf(pRequest->getStartColumn() == pDescriptor->getActiveColumn(0));
      issearf(pRequest->getStopColumn() == pDescriptor->getActiveColumn(pDescriptor->getColumnCount() - 1));
      issearf(pRequest->getConcurrentColumns() == pDescriptor->getColumnCount());

      if (pDescriptor->getInterleaveFormat() == BSQ)
      {
         // Request only the first band.
         issearf(pRequest->getStartBand() == pDescriptor->getActiveBand(0));
         issearf(pRequest->getStopBand() == pDescriptor->getActiveBand(0));
         issearf(pRequest->getConcurrentBands() == 1);
      }
      else
      {
         // Request all bands concurrently.
         issearf(pRequest->getStartBand() == pDescriptor->getActiveBand(0));
         issearf(pRequest->getStopBand() == pDescriptor->getActiveBand(pDescriptor->getBandCount() - 1));
         issearf(pRequest->getConcurrentBands() == pDescriptor->getBandCount());
      }

      // Use values from the request since the default request should be used for pRaster->getDataAccessor().
      const unsigned int rowSizeBytes = pRequest->getConcurrentColumns() *
         pRequest->getConcurrentBands() * pDescriptor->getBytesPerElement();

      DataAccessor dataAccessor = pRaster->getDataAccessor();
      issearf(dataAccessor.isValid());
      issearf(dataAccessor->getAssociatedRasterElement() == pRaster);

      const vector<DimensionDescriptor>& rows = pDescriptor->getRows();
      for (vector<DimensionDescriptor>::const_iterator iter = rows.begin(); iter != rows.end(); ++iter)
      {
         issearf(dataAccessor.isValid());
         const void* const pDst = dataAccessor->getRow();
         issearf(memcmp(pDst, pData, rowSizeBytes) == 0);
         pData = reinterpret_cast<const unsigned char*>(pData) + rowSizeBytes;
         dataAccessor->nextRow();
      }

      return success;
   }

   bool testInterleaveBand(RasterElement* pRaster, const void* pData, InterleaveFormatType interleave)
   {
      bool success = true;
      issearf(pRaster != NULL);
      issearf(pData != NULL);
      issearf(interleave.isValid() == true);

      RasterDataDescriptor* pDescriptor =  dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
      issearf(pDescriptor != NULL);

      unsigned int columnSkipSizeBytes = pDescriptor->getBytesPerElement();
      if (interleave == BIP)
      {
         columnSkipSizeBytes *= pDescriptor->getBandCount();
      }

      unsigned int rowSkipSizeBytes = 0;
      if (interleave == BIL)
      {
         rowSkipSizeBytes = (pDescriptor->getBandCount() - 1) *
            pDescriptor->getColumnCount() * pDescriptor->getBytesPerElement();
      }

      unsigned int bandOffset = pDescriptor->getBytesPerElement();
      if (interleave == BSQ)
      {
         bandOffset *= pDescriptor->getRowCount() * pDescriptor->getColumnCount();
      }
      else if (interleave == BIL)
      {
         bandOffset *= pDescriptor->getColumnCount();
      }

      // Test each pixel one at a time for each band.
      for (unsigned int band = 0, offset = 0; band < pDescriptor->getBandCount(); ++band, offset += bandOffset)
      {
         // Create a DataRequest and call polish and validate on it with the given descriptor.
         // The calls to polish and validate are unnecessary but are done to ensure that the interleave does not change.
         FactoryResource<DataRequest> pRequest;
         pRequest->setInterleaveFormat(interleave);
         pRequest->setBands(pDescriptor->getActiveBand(band), pDescriptor->getActiveBand(band));
         issearf(pRequest->polish(pDescriptor));
         issearf(pRequest->validate(pDescriptor));
         issearf(pRequest->getInterleaveFormat() == interleave);
         DataAccessor dataAccessor = pRaster->getDataAccessor(pRequest.release());
         issearf(dataAccessor.isValid());
         issearf(dataAccessor->getAssociatedRasterElement() == pRaster);

         const unsigned char* pExpectedData = reinterpret_cast<const unsigned char*>(pData) + offset;
         for (unsigned int row = 0; row < pDescriptor->getRowCount(); ++row)
         {
            for (unsigned int column = 0; column < pDescriptor->getColumnCount(); ++column)
            {
               issearf(dataAccessor.isValid());
               const void* const pActualData = dataAccessor->getColumn();
               issearf(memcmp(pActualData, pExpectedData, pDescriptor->getBytesPerElement()) == 0);
               pExpectedData += columnSkipSizeBytes;
               dataAccessor->nextColumn();
            }

            pExpectedData += rowSkipSizeBytes;
            dataAccessor->nextRow();
         }
      }

      return success;
   }

   bool testInterleaveColumn(RasterElement* pRaster, const void* pData, InterleaveFormatType interleave)
   {
      bool success = true;
      issearf(pRaster != NULL);
      issearf(pData != NULL);
      issearf(interleave.isValid() == true);

      RasterDataDescriptor* pDescriptor =  dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
      issearf(pDescriptor != NULL);

      unsigned int bandSkipSizeBytes = pDescriptor->getBytesPerElement();
      if (interleave == BSQ)
      {
         bandSkipSizeBytes *= pDescriptor->getRowCount() * pDescriptor->getColumnCount();
      }

      if (interleave == BIL)
      {
         bandSkipSizeBytes *= pDescriptor->getColumnCount();
      }

      unsigned int columnOffset = pDescriptor->getBytesPerElement();
      if (interleave == BIP)
      {
         columnOffset *= pDescriptor->getBandCount();
      }

      unsigned int rowOffset = pDescriptor->getBytesPerElement() * pDescriptor->getColumnCount();
      if (interleave == BIP || interleave == BIL)
      {
         rowOffset *= pDescriptor->getBandCount();
      }

      // Test each pixel one at a time for each column.
      for (unsigned int column = 0; column < pDescriptor->getColumnCount(); ++column)
      {
         for (unsigned int row = 0; row < pDescriptor->getRowCount(); ++row)
         {
            const unsigned char* pExpectedData = reinterpret_cast<const unsigned char*>(pData) +
               column * columnOffset + row * rowOffset;
            for (unsigned int band = 0; band < pDescriptor->getBandCount(); ++band)
            {
               // Create a DataRequest and call polish and validate on it with the given descriptor.
               // The calls to polish and validate are unnecessary
               // but are done to ensure that the interleave does not change.
               FactoryResource<DataRequest> pRequest;
               pRequest->setInterleaveFormat(interleave);
               pRequest->setColumns(pDescriptor->getActiveColumn(column), pDescriptor->getActiveColumn(column));
               pRequest->setRows(pDescriptor->getActiveRow(row), pDescriptor->getActiveRow(row));
               pRequest->setBands(pDescriptor->getActiveBand(band), pDescriptor->getActiveBand(band));
               issearf(pRequest->polish(pDescriptor));
               issearf(pRequest->validate(pDescriptor));
               issearf(pRequest->getInterleaveFormat() == interleave);
               DataAccessor dataAccessor = pRaster->getDataAccessor(pRequest.release());
               issearf(dataAccessor.isValid());
               issearf(dataAccessor->getAssociatedRasterElement() == pRaster);

               const void* const pActualData = dataAccessor->getColumn();
               issearf(memcmp(pActualData, pExpectedData, pDescriptor->getBytesPerElement()) == 0);
               pExpectedData += bandSkipSizeBytes;
            }
         }
      }

      return success;
   }

   bool testInterleaveConcurrentRows(RasterElement* pRaster, const void* pData, InterleaveFormatType interleave)
   {
      bool success = true;
      issearf(pRaster != NULL);
      issearf(pData != NULL);
      issearf(interleave.isValid() == true);

      RasterDataDescriptor* pDescriptor =  dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
      issearf(pDescriptor != NULL);

      // Loop one time for BIP/BIL and one time for each band for BSQ.
      unsigned int band = 0;
      const unsigned char* pExpectedData = reinterpret_cast<const unsigned char*>(pData);
      while (band < pDescriptor->getBandCount())
      {
         // Create a DataRequest and call polish and validate on it with the given descriptor.
         // The calls to polish and validate are unnecessary but are done to ensure that the interleave does not change.
         FactoryResource<DataRequest> pRequest;
         pRequest->setInterleaveFormat(interleave);
         pRequest->setRows(pDescriptor->getActiveRow(0),
            pDescriptor->getActiveRow(pDescriptor->getRowCount() - 1), pDescriptor->getRowCount());
         pRequest->setBands(pDescriptor->getActiveBand(band), DimensionDescriptor());
         issearf(pRequest->polish(pDescriptor));
         issearf(pRequest->validate(pDescriptor));
         issearf(pRequest->getInterleaveFormat() == interleave);
         DataAccessor dataAccessor = pRaster->getDataAccessor(pRequest.release());
         issearf(dataAccessor.isValid());
         issearf(dataAccessor->getAssociatedRasterElement() == pRaster);

         const void* const pActualData = dataAccessor->getColumn();
         const unsigned int dataSizeBytes = pDescriptor->getBytesPerElement() * pDescriptor->getRowCount() *
            pDescriptor->getColumnCount() * pRequest->getConcurrentBands();
         issearf(memcmp(pActualData, pExpectedData, dataSizeBytes) == 0);
         band += pRequest->getConcurrentBands();
         pExpectedData += dataSizeBytes;
      }

      return success;
   }
};

class MovieExportTest : public TestCase
{
public:
   MovieExportTest() : TestCase("MovieExport") {}
   bool run()
   {
      bool success = true;
      string cubeName = TestUtilities::getTestDataPath() + "landsat6band.tif";
      Service<ObjectFactory> pFactory;
      Service<ModelServices> pModel;

      SpatialDataWindow *pWindow = NULL;
      pWindow = TestUtilities::loadDataSet( cubeName, "Auto Importer" );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = pWindow->getSpatialDataView();
      issea( pView != NULL );

      RasterElement *pRasterElement = NULL;
      pRasterElement = dynamic_cast<RasterElement*>( pModel->getElement( cubeName, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      AnnotationElement *pAnnotation = NULL;
      pAnnotation = dynamic_cast<AnnotationElement*>( pModel->createElement(
         "scaleObject", "AnnotationElement", pRasterElement ) );
      issea( pAnnotation != NULL );

      AnnotationLayer *pLayer = NULL;
      pLayer = dynamic_cast<AnnotationLayer*>( pView->createLayer( ANNOTATION, pAnnotation ) );
      issea( pLayer != NULL );

      GraphicObject *pObject = NULL;
      pObject = pLayer->addObject( SCALEBAR_OBJECT );
      issea( pObject != NULL );

      issea( pObject->setBoundingBox( LocationType( 30, 40 ), LocationType( 140, 60 ) ) );

      Animation* pAnimation = pView->createDefaultAnimation();
      issea( pAnimation != NULL );

      AnimationController* pController = pView->getAnimationController();
      issea( pController != NULL );

      pAnimation = pController->createAnimation( "landsat6band" );
      issea( pAnimation != NULL );

      FileDescriptor *pFile = NULL;
      pFile = reinterpret_cast<FileDescriptor*>( pFactory->createObject( "FileDescriptor" ) );
      issea( pFile != NULL );

      string filename;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      issearf(pTempPath != NULL);
      filename = pTempPath->getFullPathAndName() + "/movieTest.avi";
      pFile->setFilename( filename );

      ExporterResource animationExporter( "AVI Product Exporter", pView, pFile, NULL, true );
      issea( animationExporter->execute() );

      Service<AnimationServices> pAnimationServices;
      pAnimationServices->destroyAnimationController(pController);

      issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      return success;
   }
};

class MultiDataSetFileImportTest : public TestCase
{
public:
   MultiDataSetFileImportTest() : TestCase("MultDataSetFileImport") {}
   bool run()
   {
      bool success = true;
      string fileName = TestUtilities::getTestDataPath() + "Nitf/nine_large.ntf";

      ImporterResource nitfImporter("NITF Importer", fileName, NULL, false);

      vector<ImportDescriptor*> importDes = nitfImporter->getImportDescriptors();
      issea(nitfImporter->execute());

      vector<DataElement*> elements = nitfImporter->getImportedElements();
      issea(elements.size() == 9);
      issea(importDes.size() == elements.size());
      
      if (success)
      {
         for (unsigned int index = 0; index < importDes.size(); ++index)
         {
            // Data Set 1
            ImportDescriptor* pImportDes = importDes[index]; 
            issea(pImportDes != NULL);
            
            DataDescriptor* pDataDes = pImportDes->getDataDescriptor();
            issea(pDataDes != NULL);

            FileDescriptor* pFileDes = pDataDes->getFileDescriptor();
            issea(pFileDes != NULL);

            issea(elements[index] != NULL);

            // Data Set 2
            DataDescriptor* pDataDes2 = elements[index]->getDataDescriptor();
            issea(pDataDes2 != NULL);

            FileDescriptor* pFileDes2 = pDataDes2->getFileDescriptor();
            issea(pFileDes2 != NULL);

            // compare the two data sets.
            string fileName = pFileDes->getDatasetLocation();
            string fileName2 = pFileDes2->getDatasetLocation();
            isseab(fileName == fileName2);
         }
      }

      return success;
   }
};

class DatasetTestSuite : public TestSuiteNewSession
{
public:
   DatasetTestSuite() : TestSuiteNewSession( "Dataset" )
   {
      addTestCase( new ChipMetadataTest );
      addTestCase( new DataRequestTestCase );
      addTestCase( new CreateChipTestCase );
      addTestCase( new DatasetChangeEventTest );
      addTestCase( new DataDescriptorMetadataTest );
      addTestCase( new DatasetAutoImportTest );
      addTestCase( new MovieExportTest );
      addTestCase( new MultiDataSetFileImportTest );
   }
};

REGISTER_SUITE( DatasetTestSuite )
