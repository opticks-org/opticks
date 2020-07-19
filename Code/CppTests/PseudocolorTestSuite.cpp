/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "ApplicationServices.h"
#include "ConfigurationSettings.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataElement.h"
#include "DesktopServicesImp.h"
#include "DimensionDescriptor.h"
#include "FilenameImp.h"
#include "HighResolutionTimer.h"
#include "LayerList.h"
#include "ModelServicesImp.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PseudocolorLayer.h"
#include "PseudocolorLayerImp.h"
#include "PseudocolorLayerAdapter.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataViewAdapter.h"
#include "SpatialDataWindowAdapter.h"
#include "SpatialDataWindow.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "ThresholdLayer.h"
#include "UtilityServices.h"
#include "xmlreader.h"
#include "xmlwriter.h"
using namespace std;

class GetRegionTestCase : public TestCase
{
public:
   GetRegionTestCase() : TestCase("GetRegion") {}
   bool run()
   {
      bool success = true;
      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataViewAdapter *pView = NULL;
      pView = dynamic_cast<SpatialDataViewAdapter*>( pWindow->getView() );
      issea( pView != NULL );

      ThresholdLayer *pProperties = NULL;
      //create threshold layer with 1198 values under 16 and under
      pProperties = TestUtilities::createThresholdLayer( pRasterElement, 16, 7557 );
      issea( pProperties != NULL );
      if( pProperties == NULL )
      {
         return false;
      }

      DataElement *pElement = NULL;
      pElement = dynamic_cast<DataElement*>( pProperties->getDataElement() );
      issea( pElement != NULL );
      if( pElement == NULL )
      {
         return false;
      }

      PseudocolorLayer *pPcProperties = NULL;
      pPcProperties = dynamic_cast<PseudocolorLayer*>( pView->convertLayer( pProperties, PSEUDOCOLOR ) );
      issea( pPcProperties != NULL );
      if( pPcProperties == NULL )
      {
         return false;
      }

      string name = pPcProperties->getName();
      issea( name != "" );

      int ids[32];
      //create classes for values 0 to 32, but only display those under or equal
      //to 16, ie. 1198 values.
      for( int i = 0; i < 32; i++ )
      {
         ids[i] = pPcProperties->addClass();
         pPcProperties->setClassValue( ids[i], i );
         pPcProperties->setClassColor( ids[i], ColorType( 8 * i, 8 * i, 8 * i ) );
         pPcProperties->setClassDisplayed( ids[i], i <= 16 );
         char buffer[256];
         sprintf( buffer, "%d", i );
         string className = buffer;
         pPcProperties->setClassName( ids[i], className );
      }

      PseudocolorLayerImp *pOverlay = NULL;

      LayerList *pLayerList = NULL;
      pLayerList = pView->getLayerList();
      issea( pLayerList != NULL );

      if( success )
      {
         pOverlay = dynamic_cast<PseudocolorLayerImp*>( pLayerList->getLayer( PSEUDOCOLOR, pPcProperties->getDataElement(), name ) );
      }
      issea( pOverlay != NULL );

      const BitMask *pMask = NULL;
      if( success )
      {
         pMask = pOverlay->getSelectedPixels();
      }
      issea( pMask != NULL );

      // verify that the correct number of points are 'true'
      int count = pMask->getCount();
      issea( count == 10000 );

      pView->deleteLayer( dynamic_cast<PseudocolorLayer*>( pOverlay ) );

      return success;
   }
};

class PseudocolorSubsetCreationTest : public TestCase
{
public:
   PseudocolorSubsetCreationTest() : TestCase("SubsetCreation") {}
   bool run()
   {
      bool success = true;

      Service<ApplicationServices> pApplication;
      issea( pApplication.get() != NULL );
      ObjectFactory *pFact = NULL;
      pFact = pApplication->getObjectFactory();
      issea( pFact != NULL );

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      RasterDataDescriptor *pDescriptor = NULL;
      pDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDescriptor != NULL );

      unsigned int numRows = pDescriptor->getRowCount();
      unsigned int numCols = pDescriptor->getColumnCount();
      unsigned int count = 0;
      unsigned int offsetRows = numRows - 25;
      issea( offsetRows == 156 );
      unsigned int offsetCols = numCols - 25;
      issea( offsetCols == 72 );

      ModelResource<RasterElement> pRMData(RasterUtilities::createRasterElement(
         "testMatrix", numRows, numCols, INT1UBYTE, true, pRasterElement));
      issea(pRMData.get() != NULL);
      
      RasterDataDescriptor *pRMDescriptor = dynamic_cast<RasterDataDescriptor*>(pRMData->getDataDescriptor());
      issea(pRMDescriptor != NULL);

      // scope the DataAccessor.  The DataAccessor must be destroyed before the 
      // layer is destroyed below, to prevent a dangling pointer within the DataAccessor
      {
         DataAccessor daMatrix = pRMData->getDataAccessor();
         issea( daMatrix.isValid() );

         unsigned char *pPtr = NULL;

         // fill the memory block with data such that the value of each pixel is its row number
         for( unsigned int row = 0; row < offsetRows; row++ )
         {
            pPtr = static_cast<unsigned char*>( daMatrix->getRow() );
            for( unsigned int col = 0; col < offsetCols; col++ )
            {
               *pPtr = ( unsigned char )row;
               pPtr++;
            }
            daMatrix->nextRow();
         }
      }

      DimensionDescriptor bandDesc = pDescriptor->getActiveBand(0);
      issea(bandDesc.isValid());

      issea( pRMData->getPixelValue( pRMDescriptor->getActiveColumn( 0 ), pRMDescriptor->getActiveRow( 0 ), bandDesc ) == static_cast<unsigned char>( 0 ) );
      issea( pRMData->getPixelValue( pRMDescriptor->getActiveColumn( 10 ), pRMDescriptor->getActiveRow( 10 ), bandDesc ) == static_cast<unsigned char>( 10 ) );
      issea( pRMData->getPixelValue( pRMDescriptor->getActiveColumn( 23 ), pRMDescriptor->getActiveRow( 4 ), bandDesc ) == static_cast<unsigned char>( 4 ) );
      issea( pRMData->getPixelValue( pRMDescriptor->getActiveColumn( offsetCols - 1 ), pRMDescriptor->getActiveRow( offsetRows - 1 ), bandDesc ) == static_cast<unsigned char>( offsetRows - 1 ) );

      // now add the Results Matrix to the viewer as a pseudocolor layer
      PseudocolorLayer *pLayer = pLayer = dynamic_cast<PseudocolorLayer*>( pView->createLayer( PSEUDOCOLOR, pRMData.release(), "resultsMatrix" ) );

      issea( pLayer != NULL );
      pLayer->setXOffset(12);
      pLayer->setYOffset(18);

      // now add offsetRows number of classes to the pseudocolor layer
      char className[15];
      for( count = 0; count < offsetRows; count++ )
      {
         sprintf( className, "class_#%d", count );
         issea( pLayer->addInitializedClass( className, count, ColorType( ( rand() / ( RAND_MAX / 255 + 1 ) ), ( rand() / ( RAND_MAX / 255 + 1 ) ), ( rand() / ( RAND_MAX / 255 + 1 ) ) ), true ) == count );
      }

      // make sure the data has not changed
      issea( pLayer->getXOffset() == 12 );
      issea( pLayer->getYOffset() == 18 );
      issea( pRMData->getPixelValue( pRMDescriptor->getActiveColumn( 0 ), pRMDescriptor->getActiveRow( 0 ), bandDesc ) == static_cast<unsigned char>( 0 ) );
      issea( pRMData->getPixelValue( pRMDescriptor->getActiveColumn( 10 ), pRMDescriptor->getActiveRow( 10 ), bandDesc ) == static_cast<unsigned char>( 10 ) );
      issea( pRMData->getPixelValue( pRMDescriptor->getActiveColumn( 23 ), pRMDescriptor->getActiveRow( 4 ), bandDesc ) == static_cast<unsigned char>( 4 ) );
      issea( pRMData->getPixelValue( pRMDescriptor->getActiveColumn( offsetCols - 1 ), pRMDescriptor->getActiveRow( offsetRows - 1 ), bandDesc ) == static_cast<unsigned char>( offsetRows - 1 ) );

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : The commented portion of PseudocolorSubsetCreationTest needs to be addressed.")
      /*PseudocolorLayerImp *pLayerImp = dynamic_cast<PseudocolorLayerImp*>(pLayer);
      issea(pLayerImp != NULL);
      if (success)
      {
         int hiddenClass = offsetRows/2;
         pLayer->setClassDisplayed(hiddenClass, false);
         pLayer->setSymbol(SOLID);
         pLayerImp->generateImage();
         Image *pImage = pLayerImp->mpImage;
         issea(pLayerImp->mpImage != NULL);
         issea(pImage->mInfo.mKey.mColorMap.size() == offsetRows+2);
         issea(pImage->mInfo.mKey.mColorMap[0].mAlpha == 0);
         issea(pImage->mInfo.mKey.mColorMap[0].mRed == 0);
         issea(pImage->mInfo.mKey.mColorMap[0].mGreen == 0);
         issea(pImage->mInfo.mKey.mColorMap[0].mBlue == 0);
         issea(pImage->mInfo.mKey.mColorMap[offsetRows+1].mAlpha == 0);
         issea(pImage->mInfo.mKey.mColorMap[offsetRows+1].mRed == 0);
         issea(pImage->mInfo.mKey.mColorMap[offsetRows+1].mGreen == 0);
         issea(pImage->mInfo.mKey.mColorMap[offsetRows+1].mBlue == 0);
         vector<ColorType> colors = pLayerImp->getColors();
         issea(colors.size() == offsetRows);
         if (success)
         {
            issea(pImage->mInfo.mKey.mStretchPoints1[0] == 0.0 - 1.0 - 0.5);
            issea(pImage->mInfo.mKey.mStretchPoints1[1] == offsetRows + 0.5);
            for (unsigned int i=0; i<offsetRows; ++i)
            {
               if (i == hiddenClass)
               {
                  issea(pImage->mInfo.mKey.mColorMap[i+1].mAlpha == 0);
                  issea(pImage->mInfo.mKey.mColorMap[i+1].mRed == 0);
                  issea(pImage->mInfo.mKey.mColorMap[i+1].mGreen == 0);
                  issea(pImage->mInfo.mKey.mColorMap[i+1].mBlue == 0);
               }
               else
               {
                  issea(pImage->mInfo.mKey.mColorMap[i+1].mAlpha == 255);
                  issea(colors[i].mRed == pImage->mInfo.mKey.mColorMap[i+1].mRed);
                  issea(colors[i].mGreen == pImage->mInfo.mKey.mColorMap[i+1].mGreen);
                  issea(colors[i].mBlue == pImage->mInfo.mKey.mColorMap[i+1].mBlue);
               }
            }
         }
      }*/

      pLayer->clear();
      issea( pLayer->getClassCount() == 0 );
      issea( pView->deleteLayer( pLayer ) == true );

      return success;
   }
};

class PseudocolorSerializeDeserializeTest : public TestCase
{
public:
   PseudocolorSerializeDeserializeTest() : TestCase("SerializeDeserialize") {}
   bool run()
   {
      bool success = true;

      Service<ModelServices> pModel;      
      Service<DesktopServices> pDesktop;
      Service<UtilityServices> pUtility;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( pDesktop->getWindow(
         pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      RasterDataDescriptor *pSensorDescriptor = NULL;
      pSensorDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pSensorDescriptor != NULL );

      unsigned int numRows = pSensorDescriptor->getRowCount();
      unsigned int numCols = pSensorDescriptor->getColumnCount();
      unsigned int count = 0;

      ModelResource<RasterElement> pRMData(RasterUtilities::createRasterElement(
         "testMatrix", numRows, numCols, INT1UBYTE, true, pRasterElement));
      issea(pRMData.get() != NULL);

      DataAccessor daMatrix = pRMData->getDataAccessor();
      issea( daMatrix.isValid() );

      unsigned char *pPtr = NULL;

      // fill the memory block with data such that the value of each pixel is its row number
      for( unsigned int row = 0; row < numRows; row++ )
      {
         pPtr = static_cast<unsigned char*>( daMatrix->getRow() );
         for( unsigned int col = 0; col < numCols; col++ )
         {
            *pPtr = static_cast<unsigned char>( row );
            pPtr++;
         }
         daMatrix->nextRow();
         issea( daMatrix.isValid() );
      }

      // now add the Results Matrix to the viewer as a pseudocolor layer
      PseudocolorLayer *pLayer = NULL;
      pLayer = dynamic_cast<PseudocolorLayer*>( pView->createLayer( PSEUDOCOLOR, pRMData.release(), "resultsMatrix" ) );
      issea( pLayer != NULL );

      // now add 10 classes to the pseudocolor layer
      char className[15];
      for( count = 0; count < 10; count++ )
      {
         sprintf( className, "class_#%d", count );
         issea( pLayer->addInitializedClass( className, count,
            ColorType( ( rand() / ( RAND_MAX / 255 + 1 ) ), ( rand() / ( RAND_MAX / 255 + 1 ) ),
            ( rand() / ( RAND_MAX / 255 + 1 ) ) ), true ) == count );
      }

      FILE *pFilePtr = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }
      pFilePtr = fopen( ( tempHome + "/pseudocolor.pseudocolorlayer" ).c_str(), "w" );
      issea( pFilePtr != NULL );

      if( success )
      {
         XMLWriter xwrite( "PseudocolorLayer" );
         success = pLayer->toXml( &xwrite );
         issea( success == true );
         xwrite.writeToFile( pFilePtr );
         fclose( pFilePtr );
      }

      pModel->setElementName( pRMData.get(), "testMatrixSerialized" );

      ModelResource<RasterElement> pRMDataCopy(RasterUtilities::createRasterElement(
         "testMatrix", numRows, numCols, INT1UBYTE, true, pRasterElement));
      issea(pRMDataCopy.get() != NULL);
      
      RasterDataDescriptor *pRMDescriptor = dynamic_cast<RasterDataDescriptor*>(pRMData->getDataDescriptor());
      issea(pRMDescriptor != NULL);
      RasterDataDescriptor *pRMDescriptorCopy = dynamic_cast<RasterDataDescriptor*>(pRMDataCopy->getDataDescriptor());
      issea(pRMDescriptorCopy != NULL);

      PseudocolorLayer *pLayerCopy = NULL;
      pLayerCopy = dynamic_cast<PseudocolorLayer*>( pView->createLayer( 
         PSEUDOCOLOR, pRMDataCopy.release(), "resultsMatrixCopy" ) );
      issea( pLayerCopy != NULL );

      pRMDataCopy->detach(Subject::signalDeleted(), 
         Slot(dynamic_cast<SpatialDataViewImp*>(pView), &SpatialDataViewImp::elementDeleted));

      if( success )
      {
         string fileName = tempHome + "/pseudocolor.pseudocolorlayer";
         FilenameImp pseudocolorFile( fileName );
         
         MessageLog *pLog( pUtility->getMessageLog()->getLog( "session" ) );
         XmlReader xml( pLog );    
      
         XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *pseudocolorDoc( NULL );
         try
         {
            pseudocolorDoc = xml.parse( &pseudocolorFile );
         }
         catch( XmlBase::XmlException & )
         {
            // do nothing
         }
         DOMElement *pRootelementPseudocolor( NULL );
         if( pseudocolorDoc != NULL )
         {
            pRootelementPseudocolor = pseudocolorDoc->getDocumentElement();
         }

         // deserialize the layer
         if( pRootelementPseudocolor != NULL )
         {
            unsigned int formatVersion = atoi( A( pRootelementPseudocolor->getAttribute( X( "version" ) ) ) );
            success = pLayerCopy->fromXml( pRootelementPseudocolor, formatVersion );
            issea( success == true );
         }
         else
         {
            success = false;
         }

         //during the deserialize the original results matrix was blown away
         //and a new one was created, get that for verification purposes
         RasterElement *pRMDataCopy2 = dynamic_cast<RasterElement*>( pLayerCopy->getDataElement() );
         RasterDataDescriptor *pRMDescriptorCopy2 = dynamic_cast<RasterDataDescriptor*>(pRMDataCopy2->getDataDescriptor());
         // verify that the contents are the same as the original
         issea( pLayerCopy->getClassCount() == pLayer->getClassCount() );
         string className = "";
         string classNameCopy = "";

         for( unsigned int classCount = 0; classCount < pLayer->getClassCount(); classCount++ )
         {
            pLayer->getClassName( classCount, className );
            pLayerCopy->getClassName( classCount, classNameCopy );
            issea( className == classNameCopy );
            issea( pLayerCopy->getClassValue( classCount ) == pLayer->getClassValue( classCount ) );
            issea( pLayerCopy->getClassColor( classCount ) == pLayer->getClassColor( classCount ) );
         }
         
         DimensionDescriptor invalid;
         for( unsigned int rowCount = 0; rowCount < numRows; rowCount++ )
         {
            for( unsigned int colCount = 0; colCount < numCols; colCount++ )
            {
               issea( pRMDataCopy2->getPixelValue( pRMDescriptorCopy2->getActiveColumn( colCount ), pRMDescriptorCopy2->getActiveRow( rowCount ), invalid ) == 
                  pRMData->getPixelValue( pRMDescriptor->getActiveColumn( colCount ), pRMDescriptor->getActiveRow( rowCount ), invalid ) );
            }
         }
      }     

      pView->deleteLayer( pLayer );
      pView->deleteLayer( pLayerCopy );
      
      return success;
   }
};

class PseudocolorTestSuite : public TestSuiteNewSession
{
public:
   PseudocolorTestSuite() : TestSuiteNewSession( "Pseudocolor" )
   {
      addTestCase( new GetRegionTestCase );
      addTestCase( new PseudocolorSubsetCreationTest );
      addTestCase( new PseudocolorSerializeDeserializeTest );
   }
};

REGISTER_SUITE( PseudocolorTestSuite )
