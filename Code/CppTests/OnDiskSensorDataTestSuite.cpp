/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ApplicationServices.h"
#include "AppVerify.h"
#include "assert.h"
#include "ConfigurationSettingsImp.h"
#include "ConnectionManager.h"
#include "DataAccessorImpl.h"
#include "DesktopServices.h"
#include "DimensionDescriptor.h"
#include "ImportDescriptor.h"
#include "ModelServicesImp.h"
#include "ObjectFactoryImp.h"
#include "ObjectResource.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "RasterLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialDataWindow.h"
#include "Statistics.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"

#include <sstream>
#include <string>

using namespace std;

const int SKIP_FACTOR = 1000;

class MultiFileOnDiskBsqTestCase : public TestCase
{
public:
   MultiFileOnDiskBsqTestCase() : TestCase("MultiFileOnDiskBsqTest")
   {
   }

   bool run()
   {
      bool success = true;
      Service<DesktopServices> pDesktop;


      ImporterResource imp("ENVI Importer", 
         TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bip.hdr",
         NULL, false);
      issearf(imp->execute());
      const vector<DataElement*> elements = imp->getImportedElements();
      issearf(elements.size() == 1);
      RasterElement *pRasterElement = dynamic_cast<RasterElement*>(elements.front());
      issea(pRasterElement != NULL);
      RasterDataDescriptor *pDdRaster = dynamic_cast<RasterDataDescriptor*>(
         pRasterElement->getDataDescriptor());

      std::vector<ProcessingLocation> loc;
      loc.push_back(ON_DISK);
      loc.push_back(ON_DISK_READ_ONLY);

      for (unsigned int i = 0; success && i < loc.size(); ++i)
      {
         ImporterResource impMult("Generic Importer", 
            TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bsq.raw000",
            NULL, false);
         vector<ImportDescriptor*> descriptors = impMult->getImportDescriptors();
         isseac(descriptors.size() == 1);
         ImportDescriptor* pImportDescriptor = descriptors.front();
         isseac(pImportDescriptor != NULL);
         RasterDataDescriptor* pDd = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
         isseac(pDd != NULL);
         RasterFileDescriptor *pFd = dynamic_cast<RasterFileDescriptor*>(
            pDd->getFileDescriptor());
         isseac(pDd != NULL);

         vector<string> bandFiles;
         for (unsigned int j = 0; j < 10; ++j)
         {
            stringstream str;
            str << TestUtilities::getTestDataPath() << 
               "CreateChip/cube512x512x10x4flsb.bsq.raw00" << j;

            bandFiles.push_back(str.str());
         }
         pFd->setBandFiles(bandFiles);
         pDd->setProcessingLocation(loc[i]);
         vector<DimensionDescriptor> bands = RasterUtilities::generateDimensionVector(10, true, false, true);
         vector<DimensionDescriptor> rows = RasterUtilities::generateDimensionVector(512, true, false, true);
         vector<DimensionDescriptor> cols = RasterUtilities::generateDimensionVector(512, true, false, true);
         pDd->setBands(bands);
         pFd->setBands(bands);
         pDd->setRows(rows);
         pFd->setRows(rows);
         pDd->setColumns(cols);
         pFd->setColumns(cols);

         pDd->setInterleaveFormat(BSQ);
         pFd->setInterleaveFormat(BSQ);
         pDd->setDataType(INT4UBYTES);
         pFd->setBitsPerElement(32);
         issea(impMult->execute());

         vector<DataElement*> elementsMult = impMult->getImportedElements();
         isseac(elementsMult.size() == 1);
         RasterElement *pRasterElementMult = dynamic_cast<RasterElement*>(elementsMult.front());
         isseac(pRasterElementMult != NULL);

         RasterDataDescriptor *pDdMult = dynamic_cast<RasterDataDescriptor*>(
            pRasterElement->getDataDescriptor());
         isseac(pDdMult != NULL);
         unsigned int rowCount = pDd->getRowCount();
         unsigned int colCount = pDd->getColumnCount();
         unsigned int bandCount = pDd->getBandCount();
         // test over some subset of the cube
         unsigned int rowSkip = rowCount / 7;
         unsigned int colSkip = colCount / 11;

         for (unsigned int band = 0; band < bandCount; ++band)
         {
            DimensionDescriptor bandDim = pDdRaster->getActiveBand(band);
            DimensionDescriptor bandMult = pDdMult->getActiveBand(band);
            for (unsigned int row = 0; row < rowCount; row += rowSkip)
            {
               DimensionDescriptor rowDim = pDdRaster->getActiveRow(row);
               DimensionDescriptor rowMult = pDdMult->getActiveRow(row);
               for (unsigned int col = 0; col < colCount; col += colSkip)
               {
                  DimensionDescriptor colDim = pDdRaster->getActiveColumn(col);
                  DimensionDescriptor colMult = pDdMult->getActiveColumn(col);
                  issea(pRasterElement->getPixelValue(colDim, rowDim, bandDim, COMPLEX_MAGNITUDE) ==
                     pRasterElementMult->getPixelValue(colMult, rowMult, bandMult, COMPLEX_MAGNITUDE));
               }
            }
         }
         WorkspaceWindow* pWindow = dynamic_cast<WorkspaceWindow*>(pDesktop->getWindow(pRasterElementMult->getName(),
            SPATIAL_DATA_WINDOW));
         issea(pWindow != NULL);
         issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      }
      WorkspaceWindow* pWindow = dynamic_cast<WorkspaceWindow*>(pDesktop->getWindow(pRasterElement->getName(),
         SPATIAL_DATA_WINDOW));
      issea(pWindow != NULL);
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class InterleaveConversionTestCase : public TestCase
{
public:
   struct DataSpec
   {
      string mFilename;
      string mName;
      InterleaveFormatType mTestInterleave;
      InterleaveFormatType mSourceInterleave;
   };

   InterleaveConversionTestCase() : TestCase("InterleaveConversionTest")
   {
   }

   bool run()
   {
      bool success = true;

      DataSpec spec[] = {
         { TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bip.hdr",
           TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bip", BSQ, BIP },
         { TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bsq.hdr",
           TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bsq", BIP, BSQ },
         { TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bil.hdr",
           TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bil", BSQ, BIL },
         { TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bil.hdr",
           TestUtilities::getTestDataPath() + "CreateChip/cube512x512x10x4flsb.bil", BIP, BIL }
      };

      for( unsigned int i = 0; success && ( i < sizeof( spec ) / sizeof( DataSpec ) ); ++i )
      {
         ImporterResource imp( "ENVI Importer", spec[i].mFilename, NULL, false );

         vector<ImportDescriptor*> importDescriptors = imp->getImportDescriptors();
         issea(!importDescriptors.empty());

         ImportDescriptor* pImportDescriptor = importDescriptors.front();
         issea(pImportDescriptor != NULL);

         RasterDataDescriptor* pDescriptor =
            dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
         issea( pDescriptor != NULL );

         pDescriptor->setProcessingLocation( ON_DISK_READ_ONLY );
         issea( imp->execute() );

         RasterElement *pRasterElement = NULL;
         pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( spec[i].mName, "RasterElement", NULL ) );
         issea( pRasterElement != NULL );

         if( !success )
         {
            continue;
         }
         issea( testDataset( pRasterElement, spec[i].mTestInterleave, spec[i].mSourceInterleave ) );

         string name = pRasterElement->getName();
         Service<DesktopServices> pDesktop;
         WorkspaceWindow* pWindow = dynamic_cast<WorkspaceWindow*>(pDesktop->getWindow(name, SPATIAL_DATA_WINDOW));
         issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      }

      return success;
   }

   bool testDataset( RasterElement *pRasterElement, InterleaveFormatType testInterleave,
      InterleaveFormatType sourceInterleave )
   {
      Service<ModelServices> pModel;
      VERIFY( pRasterElement != NULL );

      const RasterDataDescriptor *pDd = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      VERIFY( pDd != NULL );
      unsigned int rows = pDd->getRowCount();
      unsigned int cols = pDd->getColumnCount();
      unsigned int bands = pDd->getBandCount();
      unsigned int rowSkip = rows / 7; // check some subset of the loaded portion
      unsigned int colSkip = cols / 11; // check some subset of the loaded portion
      unsigned int bytesPerElement = pDd->getBytesPerElement();
      EncodingType encoding = pDd->getDataType();
      VERIFY( pDd->getInterleaveFormat() == sourceInterleave );

      if( testInterleave == BSQ )
      {
         const int COLS_TO_COMPARE = 3;
         for( unsigned int band = 0; band < bands; ++band )
         {
            FactoryResource<DataRequest> pRequest;
            pRequest->setInterleaveFormat(BSQ);
            pRequest->setBands(pDd->getActiveBand(band), pDd->getActiveBand(band));
            DataAccessor daBsq =  pRasterElement->getDataAccessor(pRequest.release());

            for( unsigned int row = 0; row < rows; row += rowSkip )
            {
               for( unsigned int col = 0; col < cols - COLS_TO_COMPARE; col += colSkip )
               {
                  daBsq->toPixel( row, col );
                  void *pBsqCol = daBsq->getColumn();
                  for( unsigned int cmp = 0; cmp < COLS_TO_COMPARE; ++cmp )
                  {
                     VERIFY( pModel->getDataValue( encoding, pBsqCol, COMPLEX_MAGNITUDE, cmp ) == 
                        pRasterElement->getPixelValue( pDd->getActiveColumn( col + cmp ), pDd->getActiveRow( row ),
                        pDd->getActiveBand( band ), COMPLEX_MAGNITUDE ) )
                  }
               }
            }
         }
      }
      else if( testInterleave == BIP )
      {
         FactoryResource<DataRequest> pRequest;
         pRequest->setInterleaveFormat(BIP);
         DataAccessor daBip =  pRasterElement->getDataAccessor( pRequest.release() );
         for( unsigned int row = 0; row < rows; row += rowSkip )
         {
            for( unsigned int col = 0; col < cols; col += colSkip )
            {
               void *pBipCol = daBip->getColumn();
               for( unsigned int band = 0; band < bands; ++band )
               {
                  VERIFY( pModel->getDataValue( encoding, pBipCol, COMPLEX_MAGNITUDE, band ) ==
                     pRasterElement->getPixelValue( pDd->getActiveColumn( col ), pDd->getActiveRow( row ), 
                     pDd->getActiveBand( band ), COMPLEX_MAGNITUDE ) )
               }
               daBip->nextColumn( colSkip );
            }
            daBip->nextRow( rowSkip, true );
         }
      }
      else
      {
         return false;
      }

      return true;
   }
};

class OnDiskSensorTestCase : public TestCase
{
public:
   OnDiskSensorTestCase() : TestCase("Main") {}
   bool run()
   {
      bool success = true;

      if( success )
      {
         success &= ( runWithProcLoc( ON_DISK_READ_ONLY ) == true );
      }
      if( success )
      {
         // now we need to clean up and delete the cube!
         Service<ModelServices> pModel;
         if( tst_assert( mpRasterElement != NULL ) )
         {
            pModel->destroyElement( mpRasterElement );
         }
         // now try to load the same cube and perform the same test with TEMPORARY_FILE
         success = success && tst_assert( runWithProcLoc( ON_DISK ) == true );
         if( tst_assert( mpRasterElement != NULL ) )
         {
            pModel->destroyElement( mpRasterElement );
         }
      }

      return success;
   }
   bool runWithProcLoc(ProcessingLocation procLoc)
   {
      bool success = true;

      string filename = TestUtilities::getTestDataPath() + "cube10000x10000x10x1umsb.bsq";

      ImporterResource genericImporter( "Generic Importer", filename, NULL, false );

      vector<ImportDescriptor*> importDescriptors = genericImporter->getImportDescriptors();
      issea(!importDescriptors.empty());

      ImportDescriptor* pImportDescriptor = importDescriptors.front();
      issea(pImportDescriptor != NULL);

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issea( pDescriptor != NULL );

      RasterFileDescriptor *pFileDescriptor = NULL;
      pFileDescriptor = dynamic_cast<RasterFileDescriptor*>( pDescriptor->getFileDescriptor() );
      issea( pFileDescriptor != NULL );

      vector<DimensionDescriptor> allBands = RasterUtilities::generateDimensionVector(10, true, false, true);   
      pFileDescriptor->setBands( allBands ); // all bands
      pDescriptor->setBands( allBands ); // active bands

      vector<DimensionDescriptor> rowInfo = RasterUtilities::generateDimensionVector(10000, true, false, true);
      vector<DimensionDescriptor> columnInfo = RasterUtilities::generateDimensionVector(10000, true, false, true);
      pFileDescriptor->setRows( rowInfo ); // all rows
      pDescriptor->setRows( rowInfo ); // active rows
      pFileDescriptor->setColumns( columnInfo ); // all columns
      pDescriptor->setColumns( columnInfo ); // active columns

      pDescriptor->setInterleaveFormat( BSQ );
      pDescriptor->setDataType( INT1UBYTE );
      pDescriptor->setProcessingLocation( procLoc );
      pFileDescriptor->setBitsPerElement( 8 );

      mpRasterElement = NULL;
      if( success )
      {
         issea( genericImporter->execute() );

         mpRasterElement = static_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
      }
      issea( mpRasterElement != NULL ); // check for NULL pointer

      unsigned int numRows = 0, numCols = 0, numBands = 0;

      if( success )
      {
         // now we can verify information about the SensorData
         pDescriptor = NULL;
         pDescriptor = dynamic_cast<RasterDataDescriptor*>( mpRasterElement->getDataDescriptor() );
         issea( pDescriptor != NULL );
         if( success )
         {
            numRows = pDescriptor->getRowCount();
            numCols = pDescriptor->getColumnCount();
            numBands = pDescriptor->getBandCount();
            success = success && tst_assert( numRows == 10000 );
            success = success && tst_assert( numCols == 10000 );
            success = success && tst_assert( numBands == 10 );
            success = success && tst_assert( pDescriptor->getDataType() == INT1UBYTE );
            success = success && tst_assert( pDescriptor->getInterleaveFormat() == BSQ );
            success = success && tst_assert( pDescriptor->getProcessingLocation() == procLoc );
         }

         try // to access the entire cube - this SHOULD throw an exception if there's not enough memory
         {
            FactoryResource<DataRequest> pRequest;
            pRequest->setRows(DimensionDescriptor(), DimensionDescriptor(), numRows);
            DataAccessor badDataAcc = mpRasterElement->getDataAccessor( pRequest.release() ); 
            /*
            output << "On disk raster failed to throw an exception when attempting to load the " << endl
               << "entire cube into memory!" << endl;
            */
            success &= true;
         }
         catch( out_of_range err )
         {
            success &= true;
         }

         // Test the computation of band statistics
         const vector<DimensionDescriptor> &loadedBands = pDescriptor->getBands();
         issea( loadedBands.size() == 10 );

         DimensionDescriptor band1 = loadedBands.at( 0 );
         DimensionDescriptor band2 = loadedBands.at( 1 );

         double dev, min, max, avg;
         dev = min = max = avg = 0;

         // for band 1
         dev = mpRasterElement->getStatistics( band1 )->getStandardDeviation();
         min = mpRasterElement->getStatistics( band1 )->getMin();
         max = mpRasterElement->getStatistics( band1 )->getMax();
         avg = mpRasterElement->getStatistics( band1 )->getAverage();
         issea( isWithin( dev, 63.6697, 0.1 ) );
         issea( isWithin( max, 254, 0 ) );
         issea( isWithin( avg, 63.1398, 0.1 ) );
         dev = min = max = avg = 0;

         // for band 2
         dev = mpRasterElement->getStatistics( band2 )->getStandardDeviation();
         min = mpRasterElement->getStatistics( band2 )->getMin();
         max = mpRasterElement->getStatistics( band2 )->getMax();
         avg = mpRasterElement->getStatistics( band2 )->getAverage();

         issea( isWithin( dev, 63.6066, 0.1 ) );
         issea( isWithin( max, 253, 0 ) );
         issea( isWithin( avg, 63.0157, 0.1 ) );
      }

      return success;
   }
private:
   RasterElement* mpRasterElement;
};

class OnDiskSensorAccessorTestCase : public TestCase
{
public:
   OnDiskSensorAccessorTestCase() : TestCase("Accessor") {}
   bool run()
   {
      bool success = true;
      ostringstream output;
      string filename = TestUtilities::getTestDataPath() + "cube14000x14000x2x2ulsb.bip";

      ImporterResource genericImporter( "Generic Importer", filename, NULL, false );

      vector<ImportDescriptor*> importDescriptors = genericImporter->getImportDescriptors();
      issea(!importDescriptors.empty());

      ImportDescriptor* pImportDescriptor = importDescriptors.front();
      issea(pImportDescriptor != NULL);

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issea( pDescriptor != NULL );

      RasterFileDescriptor *pFileDescriptor = NULL;
      pFileDescriptor = dynamic_cast<RasterFileDescriptor*>( pDescriptor->getFileDescriptor() );
      issea( pFileDescriptor != NULL );

      vector<DimensionDescriptor> allBands = RasterUtilities::generateDimensionVector(2, true, false, true);   
      pFileDescriptor->setBands( allBands ); // all bands
      pDescriptor->setBands( allBands ); // active bands

      vector<DimensionDescriptor> rowInfo = RasterUtilities::generateDimensionVector(14000, true, false, true);
      vector<DimensionDescriptor> columnInfo = RasterUtilities::generateDimensionVector(14000, true, false, true);
      pFileDescriptor->setRows( rowInfo ); // all rows
      pDescriptor->setRows( rowInfo ); // active rows
      pFileDescriptor->setColumns( columnInfo ); // all columns
      pDescriptor->setColumns( columnInfo ); // active columns

      pDescriptor->setInterleaveFormat( BIP );
      pDescriptor->setDataType( INT2UBYTES );
      pDescriptor->setProcessingLocation( ON_DISK_READ_ONLY );
      pFileDescriptor->setBitsPerElement( 16 );

      RasterElement* pRasterElement = NULL;
      if( success )
      {
         issea( genericImporter->execute() );

         pRasterElement = static_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
      }
      success = success && tst_assert( pRasterElement != NULL ); // check for NULL pointer

      unsigned int numRows = 0, numCols = 0, numBands = 0;

      if( success )
      {
         // now we can verify information about the SensorData

         pDescriptor = NULL;
         pDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
         issea( pDescriptor != NULL );

         if( success )
         {
            numRows = pDescriptor->getRowCount();
            numCols = pDescriptor->getColumnCount();
            numBands = pDescriptor->getBandCount();
            success = success && tst_assert( numRows == 14000 );
            success = success && tst_assert( numCols == 14000 );
            success = success && tst_assert( numBands == 2 );
            success = success && tst_assert( pDescriptor->getDataType() == INT2UBYTES );
            success = success && tst_assert( pDescriptor->getInterleaveFormat() == BIP );
            success = success && tst_assert( pDescriptor->getProcessingLocation() == ON_DISK_READ_ONLY );
         }

         try
         {
            // every 20th row, every 20th column
            for( unsigned int ui = 0; ui < numRows; ui += SKIP_FACTOR )
            {
               for( unsigned int uj = 0; uj < numCols; uj += SKIP_FACTOR )
               {
                  for( unsigned int uk = 0; uk < numBands; uk++ )
                  {
                     FactoryResource<DataRequest> pRequest;
                     pRequest->setRows(pDescriptor->getActiveRow(ui), pDescriptor->getActiveRow(ui), 1);
                     pRequest->setColumns(pDescriptor->getActiveColumn(uj), pDescriptor->getActiveColumn(uj), 1);
                     pRequest->setBands(pDescriptor->getActiveBand(uk), pDescriptor->getActiveBand(uk), 1);

                     DataAccessor dataAcc = pRasterElement->getDataAccessor( pRequest.release() );
                     issea( dataAcc.isValid() );
                     /* Given an accessor of only 1 row, 1 column, 1 band at a time, these pointers
                        SHOULD BE THE SAME POINTER */
                     unsigned short *pRow = NULL;
                     unsigned short *pCol = NULL;
                     if( success )
                     {
                        pRow = static_cast<unsigned short*>( dataAcc->getRow() );
                        pCol = static_cast<unsigned short*>( dataAcc->getColumn() );
                        success = success && tst_assert( pRow != NULL );
                        success = success && tst_assert( pCol != NULL );
                     }
                     issea( pRow == pCol );
                     issea( pRow != NULL );
                  }
               }
            }
         }
         catch( out_of_range err )
         {
            success &= false;
            output << "Failed to access large cube!" << endl;
         }

         ModelServicesImp::instance()->destroyElement( pRasterElement );
      }

      return success;
   }
};

class SmallOnDiskDataAccessorTestCase : public TestCase
{
public:
   SmallOnDiskDataAccessorTestCase() : TestCase("SmallAccessor") {}
   bool run()
   {
      bool success = true;
      string filename = TestUtilities::getTestDataPath() + "cube1x1x1x1slsb.bip";
      ImporterResource genericImporter( "Generic Importer", filename, NULL, false );

      vector<ImportDescriptor*> importDescriptors = genericImporter->getImportDescriptors();
      issea(!importDescriptors.empty());

      ImportDescriptor* pImportDescriptor = importDescriptors.front();
      issea(pImportDescriptor != NULL);

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issea( pDescriptor != NULL );

      RasterFileDescriptor *pFileDescriptor = NULL;
      pFileDescriptor = dynamic_cast<RasterFileDescriptor*>( pDescriptor->getFileDescriptor() );
      issea( pFileDescriptor != NULL );

      vector<DimensionDescriptor> bandInfo = RasterUtilities::generateDimensionVector(1, true, false, true);
      vector<DimensionDescriptor> rowInfo = RasterUtilities::generateDimensionVector(1, true, false, true);
      vector<DimensionDescriptor> columnInfo = RasterUtilities::generateDimensionVector(1, true, false, true);

      pFileDescriptor->setBands( bandInfo ); // all bands
      pDescriptor->setBands( bandInfo ); // active bands
      pFileDescriptor->setRows( rowInfo ); // all rows
      pDescriptor->setRows( rowInfo ); // active rows
      pFileDescriptor->setColumns( columnInfo ); // all columns
      pDescriptor->setColumns( columnInfo ); // active columns

      pDescriptor->setDataType( INT1SBYTE );
      pDescriptor->setInterleaveFormat( BIP );
      pDescriptor->setProcessingLocation( ON_DISK_READ_ONLY );
      pFileDescriptor->setBitsPerElement( 8 );

      // attempt to test that accessing a 1 x 1 x 1 cube as BIP, BIL, and BSQ are the same
      char bipChar = 0, bilChar, bsqChar; // set one different on purpose
      if( success )
      {
         issea( genericImporter->execute() );
         RasterElement* pRasterElement = NULL;
         pRasterElement = static_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
         pDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());

         if( success = success && tst_assert( pRasterElement != NULL ) )
         {
            // access as BIP
            FactoryResource<DataRequest> pBipRequest;
            pBipRequest->setInterleaveFormat(BIP);
            DataAccessor d = pRasterElement->getDataAccessor( pBipRequest.release() );
            if( success = success && tst_assert( d.isValid() ) )
            {
               char *pChar = static_cast<char*>( d->getRow() );
               // verify that getting the row is the same as getting the column of a 1 x 1 x 1 cube
               success = success && tst_assert( pChar != NULL );
               issea( pChar == d->getColumn() );
               if( success )
               {
                  bipChar = *pChar;
               }
            }

            // now try to access as BIL
            FactoryResource<DataRequest> pBilRequest;
            pBilRequest->setInterleaveFormat(BIL);
            d = pRasterElement->getDataAccessor( pBilRequest.release() );
            if( success = success && tst_assert( d.isValid() ) )
            {
               char *pChar = static_cast<char*>( d->getRow() );
               // verify that getting the row is the same as getting the column of a 1 x 1 x 1 cube
               issea( pChar == d->getColumn() );
               if( success )
               {
                  bilChar = *pChar;
               }
            }

            // now try to access as BSQ
            FactoryResource<DataRequest> pBsqRequest;
            pBsqRequest->setInterleaveFormat(BSQ);
            d = pRasterElement->getDataAccessor( pBsqRequest.release() );
            if( success = success && tst_assert( d.isValid() ) )
            {
               char *pChar = static_cast<char*>( d->getRow() );
               // verify that getting the row is the same as getting the column of a 1 x 1 x 1 cube
               issea( pChar == d->getColumn() );
               if( success )
               {
                  bsqChar = *pChar;
               }
            }

            if( success )
            {
               success = success && tst_assert( bipChar == bilChar && bipChar == bsqChar && bilChar == bsqChar );
            }
         }
         issea( ModelServicesImp::instance()->destroyElement( pRasterElement ) );
      }

      return success;
   }
};

class OnDiskSpectrumTestCase : public TestCase
{
public:
   OnDiskSpectrumTestCase() : TestCase("Spectrum") {}
   bool run()
   {
      bool success = true;
      string filename = TestUtilities::getTestDataPath() + "cube10000x10000x10x1umsb.bsq";

      ImporterResource genericImporter( "Generic Importer", filename, NULL, false );

      vector<ImportDescriptor*> importDescriptors = genericImporter->getImportDescriptors();
      issea(!importDescriptors.empty());

      ImportDescriptor* pImportDescriptor = importDescriptors.front();
      issea(pImportDescriptor != NULL);

      RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issea( pDescriptor != NULL );

      RasterFileDescriptor *pFileDescriptor = NULL;
      pFileDescriptor = dynamic_cast<RasterFileDescriptor*>( pDescriptor->getFileDescriptor() );
      issea( pFileDescriptor != NULL );

      vector<DimensionDescriptor> allBands = RasterUtilities::generateDimensionVector(10, true, false, true);
      pFileDescriptor->setBands( allBands ); // all bands
      pDescriptor->setBands( allBands ); // active bands

      vector<DimensionDescriptor> rowInfo = RasterUtilities::generateDimensionVector(10000, true, false, true);
      vector<DimensionDescriptor> columnInfo = RasterUtilities::generateDimensionVector(10000, true, false, true);
      pFileDescriptor->setRows( rowInfo ); // all rows
      pDescriptor->setRows( rowInfo ); // active rows
      pFileDescriptor->setColumns( columnInfo ); // all columns
      pDescriptor->setColumns( columnInfo ); // active columns

      pDescriptor->setDataType( INT1UBYTE );
      pDescriptor->setInterleaveFormat( BSQ );
      pDescriptor->setProcessingLocation( ON_DISK_READ_ONLY );
      pFileDescriptor->setBitsPerElement( 8 );

      RasterElement* pRasterElement = NULL;
      if( success )
      {
         issea( genericImporter->execute() );
         pRasterElement = static_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
      }
      issea( pRasterElement != NULL ); // check for NULL pointer

      unsigned int numRows = 0, numCols = 0;

      if( success )
      {
         // now we can verify information about the SensorData
         RasterDataDescriptor *pDataDescriptor = NULL;
         pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
         issea( pDataDescriptor != NULL );

         if( success )
         {
            numRows = pDataDescriptor->getRowCount();
            numCols = pDataDescriptor->getColumnCount();
            issea( numRows == 10000 );
            issea( numCols == 10000 );
            issea( pDataDescriptor->getBandCount() == 10 );
            issea( pDataDescriptor->getDataType() == INT1UBYTE );
            issea( pDataDescriptor->getInterleaveFormat() == BSQ );
            issea( pDataDescriptor->getProcessingLocation() == ON_DISK_READ_ONLY );
         }

         // these are the boundary cases - they should work
         /*Spectrum *pGoodSpectrum = NULL;
         pGoodSpectrum = pRasterElement->getPixelSpectrum( LocationType( 0, 0 ) );
         issea( pGoodSpectrum != NULL );

         pGoodSpectrum = pRasterElement->getPixelSpectrum( LocationType( numCols - 1, 0 ) );
         issea( pGoodSpectrum != NULL );

         pGoodSpectrum = pRasterElement->getPixelSpectrum( LocationType( 0, numRows - 1 ) );
         issea( pGoodSpectrum != NULL );

         pGoodSpectrum = pRasterElement->getPixelSpectrum( LocationType( numCols - 1, numRows - 1 ) );
         issea( pGoodSpectrum != NULL );

         // spectra outside the bounds of the cube - should FAIL
         Spectrum *pBadSpectrum = NULL;
         pBadSpectrum = pRasterElement->getPixelSpectrum( LocationType( -1, -1 ) );
         issea( pBadSpectrum == NULL );

         pBadSpectrum = pRasterElement->getPixelSpectrum( LocationType( -1, 0 ) );
         issea( pBadSpectrum == NULL );

         pBadSpectrum = pRasterElement->getPixelSpectrum( LocationType( 0, -1 ) );
         issea( pBadSpectrum == NULL );

         pBadSpectrum = pRasterElement->getPixelSpectrum( LocationType( numCols, numRows ) );
         issea( pBadSpectrum == NULL );

         pBadSpectrum = pRasterElement->getPixelSpectrum( LocationType( numCols + 1, numRows ) );
         issea( pBadSpectrum == NULL );

         pBadSpectrum = pRasterElement->getPixelSpectrum( LocationType( numCols, numRows + 1 ) );
         issea( pBadSpectrum == NULL );

         pBadSpectrum = pRasterElement->getPixelSpectrum( LocationType( numCols + 1, numRows + 1 ) );
         issea( pBadSpectrum == NULL );*/

         issea( ModelServicesImp::instance()->destroyElement( pRasterElement ) );
      }

      return success;
   }
};

class BadOnDiskImportTestCase : public TestCase
{
public:
   BadOnDiskImportTestCase() : TestCase("BadImport") {}
   bool run()
   {
      bool success = true;
      string filename = "/this/is/a bad path";

      ImporterResource genericImporter( "Generic Importer", filename, NULL, false );

      vector<ImportDescriptor*> importDescriptors = genericImporter->getImportDescriptors();
      issea(!importDescriptors.empty());

      return success;
   }
};

class ToPointDataAccessorTestCase : public TestCase
{
public:
   ToPointDataAccessorTestCase() : TestCase("ToPoint") {}
   bool run()
   {
      bool success = true;

      // InMemory test
      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      success = success && tst_assert( pRasterElement != NULL );
      if( success )
      {
         RasterDataDescriptor *pDataDescriptor = NULL;
         pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
         issea( pDataDescriptor != NULL );

         int numRows = pDataDescriptor->getRowCount();
         int numCols = pDataDescriptor->getColumnCount();
         int numBands = pDataDescriptor->getBandCount();
         
         vector<LocationType> points;
         points.push_back( LocationType( numCols / 2, numRows / 2 ) );
         for( int i = 1; i < 100; ++i )
         {
            LocationType point( rand() % numCols, rand() % numRows );
            points.push_back( point );
         }
         DataAccessor da = pRasterElement->getDataAccessor();

         short *pData = static_cast<short*>( pRasterElement->getRawData() );
         for( int i = 0; i < 100 && success; ++i )
         {
            int y = static_cast<int>( points[i].mY );
            int x = static_cast<int>( points[i].mX );
            da->toPixel( y, x );
            short *pFromDa = static_cast<short*>( da->getColumn() );
            issea( pFromDa == &pData[( y * numCols + x ) * numBands] );
         }
      }

      // OnDisk tests

      // Create ODSD
      string filename = TestUtilities::getTestDataPath() + "afghan.sio";
      ImporterResource sioImporter( "SIO Importer", filename );

      vector<ImportDescriptor*> importDescriptors = sioImporter->getImportDescriptors();
      issea(!importDescriptors.empty());

      ImportDescriptor* pImportDescriptor = importDescriptors.front();
      issea(pImportDescriptor != NULL);

      RasterDataDescriptor* pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issea( pDataDescriptor != NULL );   

      pDataDescriptor->setProcessingLocation( ON_DISK_READ_ONLY );

      issea( sioImporter->execute() );
      RasterElement *pRasterElement2 = NULL;
      pRasterElement2 = static_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
      issea( pRasterElement2 != NULL );
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement2->getDataDescriptor());
      // Test toPixel on 100 pixels against a one-pixel DA
      if( success )
      {
         int numRows = pDataDescriptor->getRowCount();
         int numCols = pDataDescriptor->getColumnCount();
         int numBands = pDataDescriptor->getBandCount();
         
         vector<LocationType> points;
         points.push_back( LocationType( numCols / 2, numRows / 2 ) );
         for( int i = 1; i < 100; ++i )
         {
            LocationType point( rand() % numCols, rand() % numRows );
            points.push_back( point );
         }
         DataAccessor da = pRasterElement2->getDataAccessor();
         for( int i = 0; i < 100 && success; ++i )
         {
            int y = static_cast<int>( points[i].mY );
            int x = static_cast<int>( points[i].mX );

            FactoryResource<DataRequest> pRequest;
            pRequest->setRows(pDataDescriptor->getActiveRow(y), pDataDescriptor->getActiveRow(y), 1);
            pRequest->setColumns(pDataDescriptor->getActiveColumn(x), pDataDescriptor->getActiveColumn(x), 1);

            DataAccessor da2 = pRasterElement2->getDataAccessor( pRequest.release() );
            da->toPixel( y, x );
            unsigned char data1 = *( reinterpret_cast<unsigned char*>( da->getColumn() ) );
            unsigned char data2 = *( reinterpret_cast<unsigned char*>( da2->getColumn() ) );
            issea( data1 == data2 );
         }
      }

      // Test toPixel on scene's diagonal pixels against full-scene DA using nextRow/nextColumn
      if( success )
      {
         int numRows = pDataDescriptor->getRowCount();
         int numCols = pDataDescriptor->getColumnCount();
         int numBands = pDataDescriptor->getBandCount();

         DataAccessor da = pRasterElement2->getDataAccessor();
         DataAccessor da2 = pRasterElement2->getDataAccessor();
         
         for( int i = 0; i < numRows && i < numCols && success; ++i )
         {
            da->toPixel( i, i );
            unsigned char data1 = *( reinterpret_cast<unsigned char*>( da->getColumn() ) );
            unsigned char data2 = *( reinterpret_cast<unsigned char*>( da2->getColumn() ) );
            issea( data1 == data2 );
            da2->nextRow( false );
            da2->nextColumn();
         }
      }

      // Test toPixel on 100 pixels with startRow & startColumn not 0
      if( success )
      {
         int numRows = pDataDescriptor->getRowCount();
         int numCols = pDataDescriptor->getColumnCount();
         int numBands = pDataDescriptor->getBandCount();
         
         vector<LocationType> points;
         points.push_back( LocationType( 3 * numCols / 4, 3 * numRows / 4 ) );
         for( int i = 1; i < 100; ++i )
         {
            LocationType point( rand() % ( numCols / 2 ) + numCols / 2, rand() % ( numRows / 2 ) + numRows / 2 );
            points.push_back( point );
         }

         FactoryResource<DataRequest> pRequest;
         pRequest->setRows(pDataDescriptor->getActiveRow(numRows/2), pDataDescriptor->getActiveRow(numRows-1));
         pRequest->setColumns(pDataDescriptor->getActiveColumn(numCols/2), pDataDescriptor->getActiveColumn(numCols-1));

         DataAccessor da = pRasterElement2->getDataAccessor( pRequest.release() );
         for( int i = 0; i < 100 && success; ++i )
         {
            int y = static_cast<int>( points[i].mY );
            int x = static_cast<int>( points[i].mX );

            FactoryResource<DataRequest> pRequest;
            pRequest->setRows(pDataDescriptor->getActiveRow(y), pDataDescriptor->getActiveRow(y), 1);
            pRequest->setColumns(pDataDescriptor->getActiveColumn(x), pDataDescriptor->getActiveColumn(x), 1);

            DataAccessor da2 = pRasterElement2->getDataAccessor( pRequest.release() );
            da->toPixel( y, x );
            unsigned char data1 = *( reinterpret_cast<unsigned char*>( da->getColumn() ) );
            unsigned char data2 = *( reinterpret_cast<unsigned char*>( da2->getColumn() ) );
            issea( data1 == data2 );
         }
      }
      issea( ModelServicesImp::instance()->destroyElement( pRasterElement2 ) );

      return success;
   }
};

class CreateTemporaryFileTestCase : public TestCase
{
public:
   CreateTemporaryFileTestCase() : TestCase("CreateTemporaryFile") {}
   bool run()
   {
      bool success = true;
      bool ok = false;
      string filename = TestUtilities::getTestDataPath() + "daytonchip.sio";

      ImporterResource sioImporter( "SIO Importer", filename, NULL, false );
      RasterElement *pRasterElement = NULL;

      vector<ImportDescriptor*> importDescriptors = sioImporter->getImportDescriptors();
      issea(!importDescriptors.empty());

      ImportDescriptor* pImportDescriptor = importDescriptors.front();
      issea(pImportDescriptor != NULL);

      RasterDataDescriptor* pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issea( pDataDescriptor != NULL );

      unsigned int numRows = 0;
      unsigned int numCols = 0;
      unsigned int numBands = 0;
      unsigned int numPix = 0;
      unsigned int rowcount = 0;
      unsigned int colcount = 0;
      unsigned int bandcount = 0;
      unsigned int num = 0;

      numRows = pDataDescriptor->getRowCount();
      issea( numRows == 620 );
      numCols = pDataDescriptor->getColumnCount();
      issea( numCols == 680 );
      numBands = pDataDescriptor->getBandCount();
      issea( numBands == 3 );
      numPix = numRows * numCols;

      if( success )
      {
         pDataDescriptor->setProcessingLocation( ON_DISK_READ_ONLY );
      }
      issea( sioImporter->execute() );

      pRasterElement = static_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      RasterDataDescriptor* pOnDiskDescriptor = dynamic_cast<RasterDataDescriptor*>
         ( pDataDescriptor->copy( "daytonchip_copy.sio", pRasterElement ) );
      issea( pOnDiskDescriptor != NULL );

      // create an on-disk RasterElement that will be a copy of the open cube
      RasterElement *pOnDisk = dynamic_cast<RasterElement*>( 
         ModelServicesImp::instance()->createElement( pOnDiskDescriptor ) );
      issea( pOnDisk != NULL );

      // create a temporary file to hold the copied data
      ok = pOnDisk->createTemporaryFile();
      issea( ok == true );

      if( success )
      {
         DataAccessor daOriginal = pRasterElement->getDataAccessor();
         issea( daOriginal.isValid() == true );

         DataAccessor daOnDiskAccessor = pOnDisk->getDataAccessor();
         issea( daOnDiskAccessor.isValid() == true );

         unsigned char *pPtr = NULL;
         unsigned char *pPtrOrig = NULL;

         // copy the original cube into the newly created cube
         for( rowcount = 0; rowcount < numRows; rowcount++ )
         {
            pPtrOrig = reinterpret_cast<unsigned char*>( daOriginal->getRow() );
            issea( pPtrOrig != NULL );
            pPtr = reinterpret_cast<unsigned char*>( daOnDiskAccessor->getRow() );
            issea( pPtr != NULL );

            for( colcount = 0; colcount < numCols; colcount++ )
            {
               for( bandcount = 0; bandcount < numBands; bandcount++ )
               {
                  pPtr[( colcount * numBands ) + bandcount] = pPtrOrig[( colcount * numBands ) + bandcount];
               }
            }
            daOnDiskAccessor->nextRow();
            daOriginal->nextRow();
         }
      }

      // compare the two cubes and verify that they are identical
      for( rowcount = 0; rowcount < numRows; rowcount++ )
      {
         for( colcount = 0; colcount < numCols; colcount++ )
         {
            for( bandcount = 0; bandcount < numBands; bandcount++ )
            {
               issea( pOnDisk->getPixelValue( pOnDiskDescriptor->getActiveColumn( colcount ), pOnDiskDescriptor->getActiveRow( rowcount ), pOnDiskDescriptor->getActiveBand( bandcount ), COMPLEX_MAGNITUDE ) == 
                  pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( colcount ), pDataDescriptor->getActiveRow( rowcount ), pDataDescriptor->getActiveBand( bandcount ), COMPLEX_MAGNITUDE ) );
            }
         }
      }

      // grab a pointer to the original cube's window so it can be closed
      WorkspaceWindow* pTempWindow = dynamic_cast<WorkspaceWindow*>(Service<DesktopServices>()->getWindow(filename,
         SPATIAL_DATA_WINDOW));
      issea(pTempWindow != NULL);
      issea(TestUtilities::destroyWorkspaceWindow(pTempWindow));

      ModelServicesImp::instance()->destroyDataDescriptor( pOnDiskDescriptor );
      pOnDiskDescriptor = NULL;

      return success;
   }
};

class OnDiskColumnWiseTestCase : public TestCase
{
public:
   OnDiskColumnWiseTestCase() : TestCase("ColumnWise") {}
   bool run()
   {
      bool success = true;
      bool ok = false;
      string filename = TestUtilities::getTestDataPath() + "daytonchip.sio";

      ImporterResource sioImporter( "SIO Importer", filename, NULL, false );
      RasterElement *pRasterElement = NULL;

      vector<ImportDescriptor*> importDescriptors = sioImporter->getImportDescriptors();
      issea(!importDescriptors.empty());

      ImportDescriptor* pImportDescriptor = importDescriptors.front();
      issea(pImportDescriptor != NULL);

      RasterDataDescriptor* pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issea( pDataDescriptor != NULL );

      unsigned int numRows = 0;
      unsigned int numCols = 0;
      unsigned int numBands = 0;
      unsigned int numPix = 0;
      unsigned int rowcount = 0;
      unsigned int colcount = 0;
      unsigned int bandcount = 0;
      unsigned int num = 0;

      numRows = pDataDescriptor->getRowCount();
      issea( numRows == 620 );
      numCols = pDataDescriptor->getColumnCount();
      issea( numCols == 680 );
      numBands = pDataDescriptor->getBandCount();
      issea( numBands == 3 );
      numPix = numRows * numCols;

      if( success )
      {
         pDataDescriptor->setProcessingLocation( ON_DISK_READ_ONLY );
      }
      issea( sioImporter->execute() );

      pRasterElement = static_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      RasterDataDescriptor* pOnDiskDescriptor = dynamic_cast<RasterDataDescriptor*>
         ( pDataDescriptor->copy( "daytonchip_copy3.sio", pRasterElement ) );
      issea( pOnDiskDescriptor != NULL );

      // create an on-disk RasterElement that will be a copy of the open cube
      RasterElement *pOnDisk = dynamic_cast<RasterElement*>( 
         ModelServicesImp::instance()->createElement( pOnDiskDescriptor ) );
      issea( pOnDisk != NULL );

      // create a temporary file to hold the copied data
      ok = pOnDisk->createTemporaryFile();
      issea( ok == true );

      if( success )
      {
         DataAccessor daOriginal = pRasterElement->getDataAccessor();
         issea( daOriginal.isValid() == true );

         DataAccessor daOnDiskAccessor = pOnDisk->getDataAccessor();
         issea( daOnDiskAccessor.isValid() == true );

         unsigned char *pPtr = NULL;
         unsigned char *pPtrOrig = NULL;

         // copy the original cube into the newly created cube
         for( colcount = 0; colcount < numCols; colcount++ )
         {
            daOnDiskAccessor->toPixel( 0, colcount );
            daOriginal->toPixel( 0, colcount );

            for( rowcount = 0; rowcount < numRows; rowcount++ )
            {
               pPtrOrig = reinterpret_cast<unsigned char*>( daOriginal->getColumn() );
               issea( pPtrOrig != NULL );
               pPtr = reinterpret_cast<unsigned char*>( daOnDiskAccessor->getColumn() );
               issea( pPtr != NULL );

               for( bandcount = 0; bandcount < numBands; bandcount++ )
               {
                  pPtr[bandcount] = pPtrOrig[bandcount];
               }
               daOnDiskAccessor->nextRow( false );
               daOriginal->nextRow( false );

            }
         }
      }

      // compare the two cubes and verify that they are identical
      for( rowcount = 0; rowcount < numRows; rowcount++ )
      {
         for( colcount = 0; colcount < numCols; colcount++ )
         {
            for( bandcount = 0; bandcount < numBands; bandcount++ )
            {
               issea( pOnDisk->getPixelValue( pOnDiskDescriptor->getActiveColumn( colcount ), pOnDiskDescriptor->getActiveRow( rowcount ), pOnDiskDescriptor->getActiveBand( bandcount ), COMPLEX_MAGNITUDE ) == 
                  pRasterElement->getPixelValue( pDataDescriptor->getActiveColumn( colcount ), pDataDescriptor->getActiveRow( rowcount ), pDataDescriptor->getActiveBand( bandcount ), COMPLEX_MAGNITUDE ) );
            }
         }
      }

      // grab a pointer to the original cube's window so it can be closed
      WorkspaceWindow* pTempWindow = dynamic_cast<WorkspaceWindow*>(Service<DesktopServices>()->getWindow(filename,
         SPATIAL_DATA_WINDOW));
      issea(pTempWindow != NULL);
      issea(TestUtilities::destroyWorkspaceWindow(pTempWindow));

      // close the newly created cube
      ModelServicesImp::instance()->destroyDataDescriptor( pOnDiskDescriptor );
      pOnDiskDescriptor = NULL;

      return success;

   }
};

class OnDiskSubsetTestCase : public TestCase
{
public:
   OnDiskSubsetTestCase() : TestCase("Subset") {}
   bool run()
   {
      bool success = true;
      bool ok = false;
      string filename = TestUtilities::getTestDataPath() + "daytonchip.sio";
      string filename2 = TestUtilities::getTestDataPath() + "daytonchip2.sio";

      ImporterResource sioImporter( "SIO Importer", filename, NULL, false );
      RasterElement *pInMemory = NULL;
      RasterElement *pOnDisk = NULL;

      vector<ImportDescriptor*> importDescriptors = sioImporter->getImportDescriptors();
      issea(!importDescriptors.empty());

      ImportDescriptor* pImportDescriptor = importDescriptors.front();
      issea(pImportDescriptor != NULL);

      RasterDataDescriptor* pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issea( pDataDescriptor != NULL );

      unsigned int numRows = 0;
      unsigned int numCols = 0;
      unsigned int numBands = 0;
      unsigned int numPix = 0;
      unsigned int rowcount = 0;
      unsigned int colcount = 0;
      unsigned int bandcount = 0;
      unsigned int num = 0;

      numRows = pDataDescriptor->getRowCount();
      issea( numRows == 620 );
      numCols = pDataDescriptor->getColumnCount();
      issea( numCols == 680 );
      numBands = pDataDescriptor->getBandCount();
      issea( numBands == 3 );
      numPix = numRows * numCols;

      if( success )
      {
         pDataDescriptor->setProcessingLocation( ON_DISK_READ_ONLY );
      }

      issea( sioImporter->execute() );

      pOnDisk = static_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
      issea( pOnDisk != NULL );

      pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pOnDisk->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      // load a copy of the same cube into memory
      SpatialDataWindow *pWindowInMemory = NULL;
      pWindowInMemory = TestUtilities::loadDataSet( filename2, "SIO Importer" );
      issea( pWindowInMemory != NULL );

      pInMemory = NULL;
      pInMemory = static_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename2, "RasterElement", NULL ) );
      issea( pInMemory != NULL );


      if( success )
      {
         RasterDataDescriptor *pInMemoryDescriptor = 
            dynamic_cast<RasterDataDescriptor*>(pInMemory->getDataDescriptor());
         issea(pInMemoryDescriptor != NULL);
         FactoryResource<DataRequest> pInMemoryRequest;
         pInMemoryRequest->setInterleaveFormat(BIP);
         pInMemoryRequest->setRows(pInMemoryDescriptor->getActiveRow(200), pInMemoryDescriptor->getActiveRow(299), 100);
         pInMemoryRequest->setColumns(pInMemoryDescriptor->getActiveColumn(250), pInMemoryDescriptor->getActiveColumn(349), 100);
         DataAccessor daInMemoryAccessor = pInMemory->getDataAccessor( pInMemoryRequest.release() );
         issea( daInMemoryAccessor.isValid() == true );

         RasterDataDescriptor *pOnDiskDescriptor = 
            dynamic_cast<RasterDataDescriptor*>(pOnDisk->getDataDescriptor());
         issea(pOnDiskDescriptor != NULL);
         FactoryResource<DataRequest> pOnDiskRequest;
         pOnDiskRequest->setInterleaveFormat(BIP);
         pOnDiskRequest->setRows(pOnDiskDescriptor->getActiveRow(200), pOnDiskDescriptor->getActiveRow(299), 100);
         pOnDiskRequest->setColumns(pOnDiskDescriptor->getActiveColumn(250), pOnDiskDescriptor->getActiveColumn(349), 100);
         DataAccessor daOnDiskAccessor = pOnDisk->getDataAccessor( pOnDiskRequest.release() );
         issea( daOnDiskAccessor.isValid() == true );   

         // pixel(265, 217) 27,23,64 (band2, band3, band4) *
         // pixel(331, 258) 29,30,59 (band2, band3, band4) *
         // pixel(273, 259) 20,15,39 (band2, band3, band4) *
         // pixel(266, 218) 24,21,64 (band2, band3, band4) *
         // pixel(336, 205) 30,31,82 (band2, band3, band4) *
         // pixel(266, 202) 23,19,71 (band2, band3, band4) *
         // pixel(284, 245) 29,28,72 (band2, band3, band4) *
         // pixel(294, 215) 30,31,63 (band2, band3, band4) *
         // pixel(252, 294) 42,49,57 (band2, band3, band4) *
         // pixel(264, 270) 34,43,13 (band2, band3, band4) *
         // pixel(339, 257) 29,28,58 (band2, band3, band4) *

         unsigned char *pPtrInMemory = NULL;
         unsigned char *pPtrOnDisk = NULL;

         pPtrInMemory = static_cast<unsigned char*>( daInMemoryAccessor->getRow() );  // should be at pixel (250,200)
         pPtrOnDisk = static_cast<unsigned char*>( daOnDiskAccessor->getRow() );
         issea( pPtrInMemory[0] == 24 && pPtrOnDisk[0] == 24 );
         issea( pPtrInMemory[1] == 23 && pPtrOnDisk[1] == 23 );
         issea( pPtrInMemory[2] == 56 && pPtrOnDisk[2] == 56 );

         daInMemoryAccessor->toPixel( 217, 265 );
         daOnDiskAccessor->toPixel( 217, 265 );
         pPtrInMemory = static_cast<unsigned char*>( daInMemoryAccessor->getColumn() ); // should be at pixel (265,217)
         pPtrOnDisk = static_cast<unsigned char*>( daOnDiskAccessor->getColumn() );
         issea( pPtrInMemory[0] == 27 && pPtrOnDisk[0] == 27 );
         issea( pPtrInMemory[1] == 23 && pPtrOnDisk[1] == 23 );
         issea( pPtrInMemory[2] == 64 && pPtrOnDisk[2] == 64 );

         daInMemoryAccessor->nextRow( false );
         daOnDiskAccessor->nextRow( false );
         daInMemoryAccessor->nextColumn();
         daOnDiskAccessor->nextColumn();
         pPtrInMemory = static_cast<unsigned char*>( daInMemoryAccessor->getColumn() );  // should be at pixel (266,218)
         pPtrOnDisk = static_cast<unsigned char*>( daOnDiskAccessor->getColumn() );
         issea( pPtrInMemory[0] == 24 && pPtrOnDisk[0] == 24 ); 
         issea( pPtrInMemory[1] == 21 && pPtrOnDisk[1] == 21 );
         issea( pPtrInMemory[2] == 64 && pPtrOnDisk[2] == 64 );

         daInMemoryAccessor->toPixel( 202, 266 );
         daOnDiskAccessor->toPixel( 202, 266 );
         pPtrInMemory = static_cast<unsigned char*>( daInMemoryAccessor->getColumn() );  // should be at pixel (266,202)
         pPtrOnDisk = static_cast<unsigned char*>( daOnDiskAccessor->getColumn() );
         issea( pPtrInMemory[0] == 23 && pPtrOnDisk[0] == 23 );
         issea( pPtrInMemory[1] == 19 && pPtrOnDisk[1] == 19 );
         issea( pPtrInMemory[2] == 71 && pPtrOnDisk[2] == 71 );

         // move to pixel (294,215)
         for( rowcount = 202; rowcount < 215; rowcount++ )
         {
            daInMemoryAccessor->nextRow( false );
            daOnDiskAccessor->nextRow( false );
         }
         for( colcount = 266; colcount < 294; colcount++ )
         {
            daInMemoryAccessor->nextColumn();
            daOnDiskAccessor->nextColumn();
         }
         pPtrInMemory = static_cast<unsigned char*>( daInMemoryAccessor->getColumn() );  // should be at pixel (294,215)
         pPtrOnDisk = static_cast<unsigned char*>( daOnDiskAccessor->getColumn() );
         issea( pPtrInMemory[0] == 30 && pPtrOnDisk[0] == 30 );
         issea( pPtrInMemory[1] == 31 && pPtrOnDisk[1] == 31 );
         issea( pPtrInMemory[2] == 63 && pPtrOnDisk[2] == 63 );

         // move to pixel (284,245)
         for( rowcount = 215; rowcount < 245; rowcount++ )
         {
            daInMemoryAccessor->nextRow();
            daOnDiskAccessor->nextRow();
         }
         for( colcount = 250; colcount < 284; colcount++ )
         {
            daInMemoryAccessor->nextColumn();
            daOnDiskAccessor->nextColumn();
         }
         pPtrInMemory = static_cast<unsigned char*>( daInMemoryAccessor->getColumn() );  // should be at pixel (284,245)
         pPtrOnDisk = static_cast<unsigned char*>( daOnDiskAccessor->getColumn() );
         issea( pPtrInMemory[0] == 29 && pPtrOnDisk[0] == 29 );
         issea( pPtrInMemory[1] == 28 && pPtrOnDisk[1] == 28 );
         issea( pPtrInMemory[2] == 72 && pPtrOnDisk[2] == 72 );

         // move to pixel (252,294)
         for( rowcount = 245; rowcount < 294; rowcount++ )
         {
            daInMemoryAccessor->nextRow();
            daOnDiskAccessor->nextRow();
         }
         for( colcount = 250; colcount < 252; colcount++ )
         {
            daInMemoryAccessor->nextColumn();
            daOnDiskAccessor->nextColumn();
         }
         pPtrInMemory = static_cast<unsigned char*>( daInMemoryAccessor->getColumn() );  // should be at pixel (252,294)
         pPtrOnDisk = static_cast<unsigned char*>( daOnDiskAccessor->getColumn() );
         issea( pPtrInMemory[0] == 42 && pPtrOnDisk[0] == 42 );
         issea( pPtrInMemory[1] == 49 && pPtrOnDisk[1] == 49 );
         issea( pPtrInMemory[2] == 57 && pPtrOnDisk[2] == 57 );

         daInMemoryAccessor->toPixel( 205, 336 );
         daOnDiskAccessor->toPixel( 205, 336 );
         pPtrInMemory = static_cast<unsigned char*>( daInMemoryAccessor->getColumn() );  // should be at pixel (336,205)
         pPtrOnDisk = static_cast<unsigned char*>( daOnDiskAccessor->getColumn() );
         issea( pPtrInMemory[0] == 30 && pPtrOnDisk[0] == 30 );
         issea( pPtrInMemory[1] == 31 && pPtrOnDisk[1] == 31 );
         issea( pPtrInMemory[2] == 82 && pPtrOnDisk[2] == 82 );

         // move to pixel (339,257)
         for( rowcount = 205; rowcount < 257; rowcount++ )
         {
            daInMemoryAccessor->nextRow( false );
            daOnDiskAccessor->nextRow( false );
         }
         for( colcount = 336; colcount < 339; colcount++ )
         {
            daInMemoryAccessor->nextColumn();
            daOnDiskAccessor->nextColumn();
         }
         pPtrInMemory = static_cast<unsigned char*>( daInMemoryAccessor->getColumn() );  // should be at pixel (339,257)
         pPtrOnDisk = static_cast<unsigned char*>( daOnDiskAccessor->getColumn() );
         issea( pPtrInMemory[0] == 29 && pPtrOnDisk[0] == 29 );
         issea( pPtrInMemory[1] == 28 && pPtrOnDisk[1] == 28 );
         issea( pPtrInMemory[2] == 58 && pPtrOnDisk[2] == 58 );

         // move to pixel (331,258)
         for( rowcount = 257; rowcount < 258; rowcount++ )
         {
            daInMemoryAccessor->nextRow();
            daOnDiskAccessor->nextRow();
         }
         for( colcount = 250; colcount < 331; colcount++ )
         {
            daInMemoryAccessor->nextColumn();
            daOnDiskAccessor->nextColumn();
         }
         pPtrInMemory = static_cast<unsigned char*>( daInMemoryAccessor->getColumn() );  // should be at pixel (331,258)
         pPtrOnDisk = static_cast<unsigned char*>( daOnDiskAccessor->getColumn() );
         issea( pPtrInMemory[0] == 29 && pPtrOnDisk[0] == 29 );
         issea( pPtrInMemory[1] == 30 && pPtrOnDisk[1] == 30 );
         issea( pPtrInMemory[2] == 59 && pPtrOnDisk[2] == 59 );

         // move to pixel (273,259)
         for( rowcount = 258; rowcount < 259; rowcount++ )
         {
            daInMemoryAccessor->nextRow();
            daOnDiskAccessor->nextRow();
         }
         for( colcount = 250; colcount < 273; colcount++ )
         {
            daInMemoryAccessor->nextColumn();
            daOnDiskAccessor->nextColumn();
         }
         pPtrInMemory = static_cast<unsigned char*>( daInMemoryAccessor->getColumn() );  // should be at pixel (273,259)
         pPtrOnDisk = static_cast<unsigned char*>( daOnDiskAccessor->getColumn() );
         issea( pPtrInMemory[0] == 20 && pPtrOnDisk[0] == 20 );
         issea( pPtrInMemory[1] == 15 && pPtrOnDisk[1] == 15 );
         issea( pPtrInMemory[2] == 39 && pPtrOnDisk[2] == 39 );

         // move to pixel (264,270)
         for( rowcount = 259; rowcount < 270; rowcount++ )
         {
            daInMemoryAccessor->nextRow();
            daOnDiskAccessor->nextRow();
         }
         for( colcount = 250; colcount < 264; colcount++ )
         {
            daInMemoryAccessor->nextColumn();
            daOnDiskAccessor->nextColumn();
         }
         pPtrInMemory = static_cast<unsigned char*>( daInMemoryAccessor->getColumn() );  // should be at pixel (264,270)
         pPtrOnDisk = static_cast<unsigned char*>( daOnDiskAccessor->getColumn() );
         issea( pPtrInMemory[0] == 34 && pPtrOnDisk[0] == 34 );
         issea( pPtrInMemory[1] == 43 && pPtrOnDisk[1] == 43 );
         issea( pPtrInMemory[2] == 13 && pPtrOnDisk[2] == 13 );

         daInMemoryAccessor->toPixel( 200, 250 );
         daOnDiskAccessor->toPixel( 200, 250 );
         pPtrInMemory = static_cast<unsigned char*>( daInMemoryAccessor->getColumn() );  // should be at pixel (336,205)
         pPtrOnDisk = static_cast<unsigned char*>( daOnDiskAccessor->getColumn() );
         issea( pPtrInMemory[0] == 24 && pPtrOnDisk[0] == 24 );
         issea( pPtrInMemory[1] == 23 && pPtrOnDisk[1] == 23 );
         issea( pPtrInMemory[2] == 56 && pPtrOnDisk[2] == 56 );
      }

      // grab a pointer to the cube windows so they can be closed
      Service<DesktopServices> pDesktop;
      WorkspaceWindow* pWindowOnDisk = dynamic_cast<WorkspaceWindow*>(pDesktop->getWindow(filename,
         SPATIAL_DATA_WINDOW));
      issea(pWindowOnDisk != NULL);
      issea(TestUtilities::destroyWorkspaceWindow(pWindowOnDisk));
      issea(TestUtilities::destroyWorkspaceWindow(pWindowInMemory));

      return success;
   }
};

class OnDiskImporterTestCase : public TestCase
{
public:
   OnDiskImporterTestCase() : TestCase("Importer") {}
   bool run()
   {
      bool success = true;
      bool ok = false;

      string filename = TestUtilities::getTestDataPath() + "Dted/W109/N37.dt1";

      ImporterResource dtedImporter( "DTED Importer", filename, NULL, false );
      RasterElement *pOnDisk = NULL;

      vector<ImportDescriptor*> importDescriptors = dtedImporter->getImportDescriptors();
      issea(!importDescriptors.empty());

      ImportDescriptor *pImportDescriptor = importDescriptors.front();
      issea(pImportDescriptor != NULL);

      RasterDataDescriptor *pDataDescriptor = dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issea( pDataDescriptor != NULL );

      unsigned int numRows = pDataDescriptor->getRowCount();
      issea( numRows == 1201 );
      unsigned int numCols = pDataDescriptor->getColumnCount();
      issea( numCols == 1201 );
      unsigned int numBands = pDataDescriptor->getBandCount();
      issea( numBands == 1 );
      unsigned int numPix = numRows * numCols;

      if( success )
      {
         pDataDescriptor->setProcessingLocation( ON_DISK );
      }

      issea( dtedImporter->execute() );

      pOnDisk = static_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
      issea( pOnDisk != NULL );

      if ( pOnDisk != NULL)
      {
         pDataDescriptor = NULL;
         pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pOnDisk->getDataDescriptor() );
         issea( pDataDescriptor != NULL );

         if( success )
         {
            // create a DataAccessor
            DataAccessor daOnDiskAccessor = pOnDisk->getDataAccessor();
            issea( daOnDiskAccessor.isValid() == true );

            unsigned short *pPtrOnDisk = NULL;

            pPtrOnDisk = reinterpret_cast<unsigned short*>( daOnDiskAccessor->getRow() );
            issea( pPtrOnDisk[0] == 1412 );

            daOnDiskAccessor->toPixel( 475, 928 );
            pPtrOnDisk = reinterpret_cast<unsigned short*>( daOnDiskAccessor->getColumn() );
            issea( pPtrOnDisk[0] == 2722 );

            daOnDiskAccessor->toPixel( 97, 1030 );
            pPtrOnDisk = reinterpret_cast<unsigned short*>( daOnDiskAccessor->getColumn() );
            issea( pPtrOnDisk[0] == 2163 );
         }

         WorkspaceWindow* pWindow = dynamic_cast<WorkspaceWindow*>(
            Service<DesktopServices>()->getWindow(pOnDisk->getName(), SPATIAL_DATA_WINDOW));
         issea(pWindow != NULL);

         issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      }

      return success;
   }
};

class OnDiskSensorDataTestSuite : public TestSuiteNewSession
{
public:
   OnDiskSensorDataTestSuite() : TestSuiteNewSession( "OnDiskSensorData" )
   {
      addTestCase( new MultiFileOnDiskBsqTestCase );
      addTestCase( new InterleaveConversionTestCase );
      addTestCase( new OnDiskSensorAccessorTestCase );
      addTestCase( new OnDiskSensorTestCase );
      addTestCase( new SmallOnDiskDataAccessorTestCase );
      addTestCase( new OnDiskSpectrumTestCase );
      addTestCase( new BadOnDiskImportTestCase );
      addTestCase( new ToPointDataAccessorTestCase );
      addTestCase( new CreateTemporaryFileTestCase );   
      addTestCase( new OnDiskColumnWiseTestCase );
      addTestCase( new OnDiskSubsetTestCase );
      addTestCase( new OnDiskImporterTestCase );
   }
};

REGISTER_SUITE( OnDiskSensorDataTestSuite )
