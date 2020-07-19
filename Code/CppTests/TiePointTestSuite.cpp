/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "ConfigurationSettingsImp.h"
#include "ConnectionManager.h"
#include "DesktopServicesImp.h"
#include "FilenameImp.h"
#include "HighResolutionTimer.h"
#include "LayerList.h"
#include "ModelServicesImp.h"
#include "RasterElement.h"
#include "SpatialDataWindow.h"
#include "SpatialDataView.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "TiePointLayer.h"
#include "TiePointLayerImp.h"
#include "TiePointList.h"
#include "UtilityServicesImp.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include "QtCore/QCoreApplication"

#include <time.h>

XERCES_CPP_NAMESPACE_USE
using namespace std;

class TiePointListCreationTest : public TestCase
{
public:
   TiePointListCreationTest() : TestCase("ListCreation") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      TiePointList* pTiePointList = NULL;
      pTiePointList = dynamic_cast<TiePointList*>( ModelServicesImp::instance()->createElement( "TestList", "TiePointList", NULL ) );
      issea( pTiePointList != NULL );

      pTiePointList->setMissionDatasetName( "cppTestMissionCube" );

      vector<TiePoint> tiePointVector;

      TiePoint point1;
      point1.mReferencePoint.mX = 16;
      point1.mReferencePoint.mY = 44;
      point1.mMissionOffset.mX = 1.0;
      point1.mMissionOffset.mY = 2.0;
      point1.mConfidence = 5;
      point1.mPhi = 4;
      tiePointVector.push_back( point1 );

      TiePoint point2;
      point2.mReferencePoint.mX = 10;
      point2.mReferencePoint.mY = 23;
      point2.mMissionOffset.mX = 3.0;
      point2.mMissionOffset.mY = 4.0;
      point2.mConfidence = 5;
      point2.mPhi = 4;
      tiePointVector.push_back( point2 );

      TiePoint point3;
      point3.mReferencePoint.mX = 45;
      point3.mReferencePoint.mY = 95;
      point3.mMissionOffset.mX = 5.0;
      point3.mMissionOffset.mY = 6.0;
      point3.mConfidence = 4;
      point3.mPhi = 3;
      tiePointVector.push_back( point3 );

      TiePoint point4;
      point4.mReferencePoint.mX = 68;
      point4.mReferencePoint.mY = 26;
      point4.mMissionOffset.mX = 7.0;
      point4.mMissionOffset.mY = 8.0;
      point4.mConfidence = 3;
      point4.mPhi = 2;
      tiePointVector.push_back( point4 );

      TiePoint point5;
      point5.mReferencePoint.mX = 90;
      point5.mReferencePoint.mY = 31;
      point5.mMissionOffset.mX = 9.0;
      point5.mMissionOffset.mY = 1.0;
      point5.mConfidence = 2;
      point5.mPhi = 1;
      tiePointVector.push_back( point5 );

      pTiePointList->adoptTiePoints( tiePointVector );

      TiePointLayer* pProperties = NULL;
      pProperties = dynamic_cast<TiePointLayer*>( pView->createLayer( TIEPOINT_LAYER, pTiePointList, "TestTiePointLayer" ) );
      issea( pProperties != NULL );

      LayerList *pLayerList = NULL;
      pLayerList = pView->getLayerList();
      issea( pLayerList != NULL );

      TiePointList *tempList = NULL;
      tempList = dynamic_cast<TiePointList*>( pLayerList->getLayer( TIEPOINT_LAYER, pTiePointList, "TestTiePointLayer" )->getDataElement() );
      issea( tempList != NULL );

      string listName = "";
      listName = tempList->getMissionDatasetName();
      issea( listName == "cppTestMissionCube" );

      vector<TiePoint> pTempPoints = pTiePointList->getTiePoints();
      issea( pTempPoints.size() == 5 );

      issea( pTempPoints.at( 0 ).mReferencePoint.mX == 16 && pTempPoints.at( 0 ).mReferencePoint.mY == 44 && 
         pTempPoints.at( 0 ).mMissionOffset.mX == 1.0 && pTempPoints.at( 0 ).mMissionOffset.mY == 2.0 &&
         pTempPoints.at( 0 ).mConfidence == 5 && pTempPoints.at( 0 ).mPhi == 4 );

      issea( pTempPoints.at( 1 ).mReferencePoint.mX == 10 && pTempPoints.at( 1 ).mReferencePoint.mY == 23 && 
         pTempPoints.at( 1 ).mMissionOffset.mX == 3.0 && pTempPoints.at( 1 ).mMissionOffset.mY == 4.0 &&
         pTempPoints.at( 1 ).mConfidence == 5 && pTempPoints.at( 1 ).mPhi == 4 );

      issea( pTempPoints.at( 2 ).mReferencePoint.mX == 45 && pTempPoints.at( 2 ).mReferencePoint.mY == 95 && 
         pTempPoints.at( 2 ).mMissionOffset.mX == 5.0 && pTempPoints.at( 2 ).mMissionOffset.mY == 6.0 &&
         pTempPoints.at( 2 ).mConfidence == 4 && pTempPoints.at( 2 ).mPhi == 3 );

      issea( pTempPoints.at( 3 ).mReferencePoint.mX == 68 && pTempPoints.at( 3 ).mReferencePoint.mY == 26 && 
         pTempPoints.at( 3 ).mMissionOffset.mX == 7.0 && pTempPoints.at( 3 ).mMissionOffset.mY == 8.0 &&
         pTempPoints.at( 3 ).mConfidence == 3 && pTempPoints.at( 3 ).mPhi == 2 );

      issea( pTempPoints.at( 4 ).mReferencePoint.mX == 90 && pTempPoints.at( 4 ).mReferencePoint.mY == 31 && 
         pTempPoints.at( 4 ).mMissionOffset.mX == 9.0 && pTempPoints.at( 4 ).mMissionOffset.mY == 1.0 &&
         pTempPoints.at( 4 ).mConfidence == 2 && pTempPoints.at( 4 ).mPhi == 1 );

      // do not destroy, these are needed for the TiePointListRetrievalTest
      //issea( pView->deleteLayer( pProperties ) == true );

      return success;
   }
};

class TiePointListRetrievalTest : public TestCase
{
public:
   TiePointListRetrievalTest() : TestCase("ListRetrieval") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      TiePointLayer* pLayer = dynamic_cast<TiePointLayer*>(pView->getTopMostLayer(TIEPOINT_LAYER));
      issea( pLayer != NULL );

      TiePointList* pTiePointList = NULL;
      pTiePointList = dynamic_cast<TiePointList*>( pLayer->getDataElement() );
      issea( pTiePointList != NULL );

      string listName = "";
      listName = pTiePointList->getMissionDatasetName();
      issea( listName == "cppTestMissionCube" );

      vector<TiePoint> pPoints = pTiePointList->getTiePoints();
      issea( pPoints.size() == 5 );

      issea( pPoints.at( 0 ).mReferencePoint.mX == 16 && pPoints.at( 0 ).mReferencePoint.mY == 44 && 
         pPoints.at( 0 ).mMissionOffset.mX == 1.0 && pPoints.at( 0 ).mMissionOffset.mY == 2.0 &&
         pPoints.at( 0 ).mConfidence == 5 && pPoints.at( 0 ).mPhi == 4 );

      issea( pPoints.at( 1 ).mReferencePoint.mX == 10 && pPoints.at( 1 ).mReferencePoint.mY == 23 && 
         pPoints.at( 1 ).mMissionOffset.mX == 3.0 && pPoints.at( 1 ).mMissionOffset.mY == 4.0 &&
         pPoints.at( 1 ).mConfidence == 5 && pPoints.at( 1 ).mPhi == 4 );

      issea( pPoints.at( 2 ).mReferencePoint.mX == 45 && pPoints.at( 2 ).mReferencePoint.mY == 95 && 
         pPoints.at( 2 ).mMissionOffset.mX == 5.0 && pPoints.at( 2 ).mMissionOffset.mY == 6.0 &&
         pPoints.at( 2 ).mConfidence == 4 && pPoints.at( 2 ).mPhi == 3 );

      issea( pPoints.at( 3 ).mReferencePoint.mX == 68 && pPoints.at( 3 ).mReferencePoint.mY == 26 && 
         pPoints.at( 3 ).mMissionOffset.mX == 7.0 && pPoints.at( 3 ).mMissionOffset.mY == 8.0 &&
         pPoints.at( 3 ).mConfidence == 3 && pPoints.at( 3 ).mPhi == 2 );

      issea( pPoints.at( 4 ).mReferencePoint.mX == 90 && pPoints.at( 4 ).mReferencePoint.mY == 31 && 
         pPoints.at( 4 ).mMissionOffset.mX == 9.0 && pPoints.at( 4 ).mMissionOffset.mY == 1.0 &&
         pPoints.at( 4 ).mConfidence == 2 && pPoints.at( 4 ).mPhi == 1 );

      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pLayer ) ) && success;
      }

      return success;
   }
};

class TiePointSerializeDeserializeTest : public TestCase
{
public:
   TiePointSerializeDeserializeTest() : TestCase("SerializeList") {}
   bool run()
   {
      bool success = true;
      bool ok = false;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      TiePointList* pTiePointList = NULL;
      pTiePointList = dynamic_cast<TiePointList*>( ModelServicesImp::instance()->createElement( "TestList", "TiePointList", NULL ) );
      issea( pTiePointList != NULL );

      pTiePointList->setMissionDatasetName( pRasterElement->getName() );

      vector<TiePoint> tiePointVector;

      TiePoint point1;
      point1.mReferencePoint.mX = 15;
      point1.mReferencePoint.mY = 43;
      point1.mMissionOffset.mX = 2.0;
      point1.mMissionOffset.mY = 3.0;
      point1.mConfidence = 5;
      point1.mPhi = 4;
      tiePointVector.push_back( point1 );

      TiePoint point2;
      point2.mReferencePoint.mX = 9;
      point2.mReferencePoint.mY = 22;
      point2.mMissionOffset.mX = 4.0;
      point2.mMissionOffset.mY = 5.0;
      point2.mConfidence = 5;
      point2.mPhi = 4;
      tiePointVector.push_back( point2 );

      TiePoint point3;
      point3.mReferencePoint.mX = 44;
      point3.mReferencePoint.mY = 94;
      point3.mMissionOffset.mX = 6.0;
      point3.mMissionOffset.mY = 7.0;
      point3.mConfidence = 4;
      point3.mPhi = 3;
      tiePointVector.push_back( point3 );

      TiePoint point4;
      point4.mReferencePoint.mX = 67;
      point4.mReferencePoint.mY = 25;
      point4.mMissionOffset.mX = 8.0;
      point4.mMissionOffset.mY = 9.0;
      point4.mConfidence = 3;
      point4.mPhi = 2;
      tiePointVector.push_back( point4 );

      TiePoint point5;
      point5.mReferencePoint.mX = 89;
      point5.mReferencePoint.mY = 30;
      point5.mMissionOffset.mX = 10.0;
      point5.mMissionOffset.mY = 11.0;
      point5.mConfidence = 2;
      point5.mPhi = 1;
      tiePointVector.push_back( point5 );

      pTiePointList->adoptTiePoints( tiePointVector );

      TiePointLayer* pLayer = NULL;
      pLayer = dynamic_cast<TiePointLayer*>( pView->createLayer( TIEPOINT_LAYER, pTiePointList, "TestTiePointLayer" ) );
      issea( pLayer != NULL );

      // serialize the TiePointList
      FILE *pOutputFile = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }
      pOutputFile = fopen( ( tempHome + "/TiePointList.tie" ).c_str(), "w" );
      issea( pOutputFile != NULL );

      XMLWriter xwrite( "TiePointList" );
      ok = pTiePointList->toXml( &xwrite );
      issea( ok == true );
      xwrite.writeToFile( pOutputFile );
      fclose( pOutputFile );
      issea( ok == true );

      ModelServicesImp::instance()->setElementName(pTiePointList, "TestListSerialized");

      TiePointList* pTiePointListCopy = NULL;
      pTiePointListCopy = dynamic_cast<TiePointList*>( ModelServicesImp::instance()->createElement( "TestList", "TiePointList", NULL ) );
      issea( pTiePointListCopy != NULL );

      // deserialize the TiePointList
      MessageLog *pLog( UtilityServicesImp::instance()->getMessageLog()->getLog( "session" ) );
      XmlReader xml( pLog );

      FilenameImp tiepointFile( tempHome + "/TiePointList.tie" );

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *tiepointDoc( NULL );
      try
      {
         tiepointDoc = xml.parse( &tiepointFile );
      }
      catch( XmlBase::XmlException & )
      {
         // do nothing
      }
      DOMElement *rootelement( NULL );
      if( tiepointDoc != NULL )
      {
         rootelement = tiepointDoc->getDocumentElement();
      }

      unsigned int formatVersion = atoi( A( rootelement->getAttribute( X( "version" ) ) ) );
      try
      {
         ok = pTiePointListCopy->fromXml( rootelement, formatVersion );
      }
      catch (...)
      {
         ok = false;
      }
      issea( ok == true );

      // verify the deserialize
      string listName = "";
      listName = pTiePointList->getMissionDatasetName();
      issea( listName == pRasterElement->getName() );

      vector<TiePoint> pPoints = pTiePointList->getTiePoints();
      issea( pPoints.size() == 5 );

      issea( pPoints.at( 0 ).mReferencePoint.mX == 15 && pPoints.at( 0 ).mReferencePoint.mY == 43 && 
         pPoints.at( 0 ).mMissionOffset.mX == 2.0 && pPoints.at( 0 ).mMissionOffset.mY == 3.0 &&
         pPoints.at( 0 ).mConfidence == 5 && pPoints.at( 0 ).mPhi == 4 );

      issea( pPoints.at( 1 ).mReferencePoint.mX == 9 && pPoints.at( 1 ).mReferencePoint.mY == 22 && 
         pPoints.at( 1 ).mMissionOffset.mX == 4.0 && pPoints.at( 1 ).mMissionOffset.mY == 5.0 &&
         pPoints.at( 1 ).mConfidence == 5 && pPoints.at( 1 ).mPhi == 4 );

      issea( pPoints.at( 2 ).mReferencePoint.mX == 44 && pPoints.at( 2 ).mReferencePoint.mY == 94 && 
         pPoints.at( 2 ).mMissionOffset.mX == 6.0 && pPoints.at( 2 ).mMissionOffset.mY == 7.0 &&
         pPoints.at( 2 ).mConfidence == 4 && pPoints.at( 2 ).mPhi == 3 );

      issea( pPoints.at( 3 ).mReferencePoint.mX == 67 && pPoints.at( 3 ).mReferencePoint.mY == 25 && 
         pPoints.at( 3 ).mMissionOffset.mX == 8.0 && pPoints.at( 3 ).mMissionOffset.mY == 9.0 &&
         pPoints.at( 3 ).mConfidence == 3 && pPoints.at( 3 ).mPhi == 2 );

      issea( pPoints.at( 4 ).mReferencePoint.mX == 89 && pPoints.at( 4 ).mReferencePoint.mY == 30 && 
         pPoints.at( 4 ).mMissionOffset.mX == 10.0 && pPoints.at( 4 ).mMissionOffset.mY == 11.0 &&
         pPoints.at( 4 ).mConfidence == 2 && pPoints.at( 4 ).mPhi == 1 );

      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pLayer ) ) && success;
      }
      if( pTiePointListCopy != NULL )
      {
         success = tst_assert( ModelServicesImp::instance()->destroyElement( pTiePointListCopy ) ) && success;
      }

      return success;
   }
};

class TiePointSerializeDeserializeLayerTest : public TestCase
{
public:
   TiePointSerializeDeserializeLayerTest() : TestCase("SerializeLayer") {}
   bool run()
   {
      bool success = true;
      bool ok = false;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      TiePointList* pTiePointList = NULL;
      pTiePointList = dynamic_cast<TiePointList*>( ModelServicesImp::instance()->createElement( "TestList", "TiePointList", NULL ) );
      issea( pTiePointList != NULL );

      pTiePointList->setMissionDatasetName( pRasterElement->getName() );

      vector<TiePoint> tiePointVector;

      TiePoint point1;
      point1.mReferencePoint.mX = 15;
      point1.mReferencePoint.mY = 43;
      point1.mMissionOffset.mX = 2.0;
      point1.mMissionOffset.mY = 3.0;
      point1.mConfidence = 5;
      point1.mPhi = 4;
      tiePointVector.push_back( point1 );

      TiePoint point2;
      point2.mReferencePoint.mX = 9;
      point2.mReferencePoint.mY = 22;
      point2.mMissionOffset.mX = 4.0;
      point2.mMissionOffset.mY = 5.0;
      point2.mConfidence = 5;
      point2.mPhi = 4;
      tiePointVector.push_back( point2 );

      TiePoint point3;
      point3.mReferencePoint.mX = 44;
      point3.mReferencePoint.mY = 94;
      point3.mMissionOffset.mX = 6.0;
      point3.mMissionOffset.mY = 7.0;
      point3.mConfidence = 4;
      point3.mPhi = 3;
      tiePointVector.push_back( point3 );

      TiePoint point4;
      point4.mReferencePoint.mX = 67;
      point4.mReferencePoint.mY = 25;
      point4.mMissionOffset.mX = 8.0;
      point4.mMissionOffset.mY = 9.0;
      point4.mConfidence = 3;
      point4.mPhi = 2;
      tiePointVector.push_back( point4 );

      TiePoint point5;
      point5.mReferencePoint.mX = 89;
      point5.mReferencePoint.mY = 30;
      point5.mMissionOffset.mX = 10.0;
      point5.mMissionOffset.mY = 11.0;
      point5.mConfidence = 2;
      point5.mPhi = 1;
      tiePointVector.push_back( point5 );

      pTiePointList->adoptTiePoints( tiePointVector );

      TiePointLayer* pLayer = NULL;
      pLayer = dynamic_cast<TiePointLayer*>( pView->createLayer( TIEPOINT_LAYER, pTiePointList, "TestTiePointLayer" ) );
      issea( pLayer != NULL );

      // serialize the TiePointList
      FILE *pOutputFile = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }
      pOutputFile = fopen( ( tempHome + "/TiePointLayer.tielayer" ).c_str(), "w" );
      issea( pOutputFile != NULL );

      XMLWriter xwrite( "TiePointLayer" );
      ok = pTiePointList->toXml( &xwrite );
      issea( ok == true );
      xwrite.writeToFile( pOutputFile );
      fclose( pOutputFile );
      issea( ok == true );

      ModelServicesImp::instance()->setElementName( pTiePointList, "TestListSerialized" );

      TiePointList* pTiePointListCopy = NULL;
      pTiePointListCopy = dynamic_cast<TiePointList*>( ModelServicesImp::instance()->createElement( "TestList", "TiePointList", NULL ) );
      issea( pTiePointListCopy != NULL );

      TiePointLayer* pLayerCopy = NULL;
      pLayerCopy = dynamic_cast<TiePointLayer*>( pView->createLayer( TIEPOINT_LAYER, pTiePointListCopy, "TestTiePointLayerCopy" ) );
      issea( pLayerCopy != NULL );

      // deserialize the TiePointList
      MessageLog *pLog( UtilityServicesImp::instance()->getMessageLog()->getLog( "session" ) );
      XmlReader xml( pLog );

      FilenameImp tiepointFile( tempHome + "/TiePointLayer.tielayer" );

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *tiepointDoc( NULL );
      try
      {
         tiepointDoc = xml.parse( &tiepointFile );
      }
      catch( XmlBase::XmlException & )
      {
         // do nothing
      }
      DOMElement *rootelement( NULL );
      if( tiepointDoc != NULL )
      {
         rootelement = tiepointDoc->getDocumentElement();
      }

      if( rootelement != NULL )
      {
         unsigned int formatVersion = atoi( A( rootelement->getAttribute( X( "version" ) ) ) );
         ok = pLayerCopy->fromXml( rootelement, formatVersion );
         issea( ok == true );
      }
      else
      {
         success = false;
      }

      // verify the deserialize
      string listName = "";
      listName = static_cast<TiePointList*>( pLayerCopy->getDataElement() )->getMissionDatasetName();
      issea( listName == pRasterElement->getName() );

      vector<TiePoint> pPoints = static_cast<TiePointList*>( pLayerCopy->getDataElement() )->getTiePoints();
      issea( pPoints.size() == 5 );

      issea( pPoints.at( 0 ).mReferencePoint.mX == 15 && pPoints.at( 0 ).mReferencePoint.mY == 43 && 
         pPoints.at( 0 ).mMissionOffset.mX == 2.0 && pPoints.at( 0 ).mMissionOffset.mY == 3.0 &&
         pPoints.at( 0 ).mConfidence == 5 && pPoints.at( 0 ).mPhi == 4 );

      issea( pPoints.at( 1 ).mReferencePoint.mX == 9 && pPoints.at( 1 ).mReferencePoint.mY == 22 && 
         pPoints.at( 1 ).mMissionOffset.mX == 4.0 && pPoints.at( 1 ).mMissionOffset.mY == 5.0 &&
         pPoints.at( 1 ).mConfidence == 5 && pPoints.at( 1 ).mPhi == 4 );

      issea( pPoints.at( 2 ).mReferencePoint.mX == 44 && pPoints.at( 2 ).mReferencePoint.mY == 94 && 
         pPoints.at( 2 ).mMissionOffset.mX == 6.0 && pPoints.at( 2 ).mMissionOffset.mY == 7.0 &&
         pPoints.at( 2 ).mConfidence == 4 && pPoints.at( 2 ).mPhi == 3 );

      issea( pPoints.at( 3 ).mReferencePoint.mX == 67 && pPoints.at( 3 ).mReferencePoint.mY == 25 && 
         pPoints.at( 3 ).mMissionOffset.mX == 8.0 && pPoints.at( 3 ).mMissionOffset.mY == 9.0 &&
         pPoints.at( 3 ).mConfidence == 3 && pPoints.at( 3 ).mPhi == 2 );

      issea( pPoints.at( 4 ).mReferencePoint.mX == 89 && pPoints.at( 4 ).mReferencePoint.mY == 30 && 
         pPoints.at( 4 ).mMissionOffset.mX == 10.0 && pPoints.at( 4 ).mMissionOffset.mY == 11.0 &&
         pPoints.at( 4 ).mConfidence == 2 && pPoints.at( 4 ).mPhi == 1 );

      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pLayer ) ) && success; 
         success = tst_assert( pView->deleteLayer( pLayerCopy ) ) && success;
      }

      return success;
   }
};

class TiePoint100PointsTest : public TestCase
{
public:
   TiePoint100PointsTest() : TestCase("100Points") {}
   bool run()
   {
      bool success = true;
      double timeFor100 = 0.0;
      int count = 0;

      string filename = TestUtilities::getTestDataPath() + "cube1024x1024x7x1ulsb.sio";

      SpatialDataWindow *pWindow = NULL;
      pWindow = TestUtilities::loadDataSet( filename, "SIO Importer" );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      TiePointList* pTiePointList = NULL;
      pTiePointList = dynamic_cast<TiePointList*>( ModelServicesImp::instance()->createElement(
         "TestList", "TiePointList", NULL ) );
      issea( pTiePointList != NULL );

      pTiePointList->setMissionDatasetName( "cppTestMissionCube" );

      vector<TiePoint> tiePointVector;

      // how long does it take to create a layer of 100?
      TiePointLayer* pLayer = NULL;
      {
         HrTimer::Resource timer( &timeFor100, false );
         for( int row = 0; row < 100; row++ )
         {
            TiePoint point;
            point.mReferencePoint.mX = 10;
            point.mReferencePoint.mY = row;
            point.mMissionOffset.mX = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
            point.mMissionOffset.mY = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
            point.mConfidence = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
            point.mPhi = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
            tiePointVector.push_back( point );
         }         
         pTiePointList->adoptTiePoints( tiePointVector );

         pLayer = dynamic_cast<TiePointLayer*>( pView->createLayer( TIEPOINT_LAYER, pTiePointList, "TestTiePointLayer" ) );
         issea( pLayer != NULL );
      }
      QCoreApplication::instance()->processEvents();

      printf( "Time to complete 100 TiePoints is: %f seconds.\n", timeFor100 ); 

      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pLayer ) ) && success;
      }

      return success;
   }
};

class TiePoint1000PointsTest : public TestCase
{
public:
   TiePoint1000PointsTest() : TestCase("1000Points") {}
   bool run()
   {
      bool success = true;
      double timeFor1000 = 0.0;
      int count = 0;
      string filename = TestUtilities::getTestDataPath() + "cube1024x1024x7x1ulsb.sio";

      RasterElement* pRasterElement = NULL;
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      if( pWindow == NULL )
      {
         pWindow = TestUtilities::loadDataSet( filename, "SIO Importer" );
      }
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      TiePointList* pTiePointList = NULL;
      pTiePointList = dynamic_cast<TiePointList*>( ModelServicesImp::instance()->createElement(
         "TestList", "TiePointList", NULL ) );
      issea( pTiePointList != NULL );

      pTiePointList->setMissionDatasetName( "cppTestMissionCube" );

      vector<TiePoint> tiePointVector;

      // how long does it take to create a layer of 1000?
      TiePointLayer* pLayer = NULL;
      {
         HrTimer::Resource timer( &timeFor1000, false );
         for( int row = 0; row < 100; row++ )
         {
            for( int col = 0; col < 10; col++ )
            {
               TiePoint point;
               point.mReferencePoint.mX = col;
               point.mReferencePoint.mY = row;
               point.mMissionOffset.mX = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
               point.mMissionOffset.mY = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
               point.mConfidence = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
               point.mPhi = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
               tiePointVector.push_back( point );
            }
         }
         pTiePointList->adoptTiePoints( tiePointVector );

         pLayer = dynamic_cast<TiePointLayer*>( pView->createLayer( TIEPOINT_LAYER, pTiePointList, "TestTiePointLayer" ) );
         issea( pLayer != NULL );
      }
      QCoreApplication::instance()->processEvents();

      printf( "Time to complete 1000 TiePoints is: %f seconds.\n", timeFor1000 ); 

      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pLayer ) ) && success;
      }

      return success;
   }
};

class TiePoint10000PointsTest : public TestCase
{
public:
   TiePoint10000PointsTest() : TestCase("10000Points") {}
   bool run()
   {
      bool success = true;
      double timeFor10000 = 0.0;
      int count = 0;
      string filename = TestUtilities::getTestDataPath() + "cube1024x1024x7x1ulsb.sio";

      RasterElement* pRasterElement = NULL;
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      if( pWindow == NULL )
      {
         pWindow = TestUtilities::loadDataSet( filename, "SIO Importer" );
      }
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      TiePointList* pTiePointList = NULL;
      pTiePointList = dynamic_cast<TiePointList*>( ModelServicesImp::instance()->createElement(
         "TestList", "TiePointList", NULL ) );
      issea( pTiePointList != NULL );

      pTiePointList->setMissionDatasetName( "cppTestMissionCube" );

      vector<TiePoint> tiePointVector;

      // how long does it take to create a layer of 10000?
      TiePointLayer* pLayer = NULL;
      {
         HrTimer::Resource timer( &timeFor10000, false );
         for( int row = 0; row < 100; row++ )
         {
            for( int col = 0; col < 100; col++ )
            {
               TiePoint point;
               point.mReferencePoint.mX = col;
               point.mReferencePoint.mY = row;
               point.mMissionOffset.mX = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
               point.mMissionOffset.mY = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
               point.mConfidence = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
               point.mPhi = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
               tiePointVector.push_back( point );
            }
         }
         pTiePointList->adoptTiePoints( tiePointVector );

         pLayer = dynamic_cast<TiePointLayer*>( pView->createLayer( TIEPOINT_LAYER, pTiePointList, "TestTiePointLayer" ) );
         issea( pLayer != NULL );
      }
      QCoreApplication::instance()->processEvents();

      printf( "Time to complete 10000 TiePoints is: %f seconds.\n", timeFor10000 ); 

      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pLayer ) ) && success;
      }

      return success;
   }
};

class TiePoint100000PointsTest : public TestCase
{
public:
   TiePoint100000PointsTest() : TestCase("100000Points") {}
   bool run()
   {
      bool success = true;
      double timeFor100000 = 0.0;
      int count = 0;
      string filename = TestUtilities::getTestDataPath() + "cube1024x1024x7x1ulsb.sio";

      RasterElement* pRasterElement = NULL;
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      if( pWindow == NULL )
      {
         pWindow = TestUtilities::loadDataSet( filename, "SIO Importer" );
      }
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      TiePointList* pTiePointList = NULL;
      pTiePointList = dynamic_cast<TiePointList*>( ModelServicesImp::instance()->createElement(
         "TestList", "TiePointList", NULL ) );
      issea( pTiePointList != NULL );

      pTiePointList->setMissionDatasetName( "cppTestMissionCube" );

      vector<TiePoint> tiePointVector;

      // how long does it take to create a layer of 100000?
      TiePointLayer* pLayer = NULL;
      {
         HrTimer::Resource timer( &timeFor100000, false );
         for( int row = 0; row < 1000; row++ )
         {
            for( int col = 0; col < 100; col++ )
            {
               TiePoint point;
               point.mReferencePoint.mX = col;
               point.mReferencePoint.mY = row;
               point.mMissionOffset.mX = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
               point.mMissionOffset.mY = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
               point.mConfidence = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
               point.mPhi = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
               tiePointVector.push_back( point );
            }
         }
         pTiePointList->adoptTiePoints( tiePointVector );

         pLayer = dynamic_cast<TiePointLayer*>( pView->createLayer( TIEPOINT_LAYER, pTiePointList, "TestTiePointLayer" ) );
         issea( pLayer != NULL );
      }
      QCoreApplication::instance()->processEvents();

      printf( "Time to complete 100000 TiePoints is: %f seconds.\n", timeFor100000 ); 

      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pLayer ) ) && success;
      }

      return success;
   }
};

class TiePoint1000000PointsTest : public TestCase
{
public:
   TiePoint1000000PointsTest() : TestCase("1000000Points") {}
   bool run()
   {
      bool success = true;
      double timeFor1000000 = 0.0;
      int count = 0;
      string filename = TestUtilities::getTestDataPath() + "cube1024x1024x7x1ulsb.sio";

      RasterElement* pRasterElement = NULL;
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( filename, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      if( pWindow == NULL )
      {
         pWindow = TestUtilities::loadDataSet( filename, "SIO Importer" );
      }
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      TiePointList* pTiePointList = NULL;
      pTiePointList = dynamic_cast<TiePointList*>( ModelServicesImp::instance()->createElement(
         "TestList", "TiePointList", NULL ) );
      issea( pTiePointList != NULL );

      pTiePointList->setMissionDatasetName( "cppTestMissionCube" );

      vector<TiePoint> tiePointVector;

      // how long does it take to create a layer of 1000000?
      TiePointLayer* pLayer = NULL;
      {
         HrTimer::Resource timer( &timeFor1000000, false );
         for( int row = 0; row < 1000; row++ )
         {
            for( int col = 0; col < 1000; col++ )
            {
               TiePoint point;
               point.mReferencePoint.mX = col;
               point.mReferencePoint.mY = row;
               point.mMissionOffset.mX = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
               point.mMissionOffset.mY = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
               point.mConfidence = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
               point.mPhi = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
               tiePointVector.push_back( point );
            }
         }
         pTiePointList->adoptTiePoints( tiePointVector );

         pLayer = dynamic_cast<TiePointLayer*>( pView->createLayer( TIEPOINT_LAYER, pTiePointList, "TestTiePointLayer" ) );
         issea( pLayer != NULL );
      }
      QCoreApplication::instance()->processEvents();

      printf( "Time to complete 1000000 TiePoints is: %f seconds.\n", timeFor1000000 ); 

      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pLayer ) ) && success;
      }

      if (pWindow != NULL)
      {
         issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      }

      return success;
   }
};

class TiePointListRenameTest : public TestCase
{
public:
   TiePointListRenameTest() : TestCase("ListRename") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement(true);
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      TiePointList* pTiePointList = NULL;
      pTiePointList = dynamic_cast<TiePointList*>( ModelServicesImp::instance()->createElement(
         "TestTiePointList", "TiePointList", pRasterElement ) );
      issea( pTiePointList != NULL );

      pTiePointList->setMissionDatasetName( "cppTestMissionCube" );

      // add 100 semi-random points
      vector<TiePoint> tiePointVector;
      for( int row = 0; row < 10; row++ )
      {
         for( int col = 0; col < 10; col++ )
         {
            TiePoint point;
            point.mReferencePoint.mX = col + ( rand() / ( RAND_MAX / 50 + 1 ) );
            point.mReferencePoint.mY = row + ( rand() / ( RAND_MAX / 50 + 1 ) );
            point.mMissionOffset.mX = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
            point.mMissionOffset.mY = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
            point.mConfidence = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
            point.mPhi = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
            tiePointVector.push_back( point );
         }
      }
      pTiePointList->adoptTiePoints( tiePointVector );

      TiePointLayer* pLayer = NULL;
      pLayer = dynamic_cast<TiePointLayer*>( pView->createLayer( TIEPOINT_LAYER, pTiePointList, "TestTiePointList" ) );
      issea( pLayer != NULL );

      unsigned int numLists = 0;
      unsigned int numLayers = 0;
      string name = "";
      vector<Layer*> layerVector;
      vector<string> listVector;
      LayerList *pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );
      numLayers = pList->getNumLayers();
      issea( numLayers == 2 ); // raster layer, TestTiePointList

      listVector = ModelServicesImp::instance()->getElementNames( pRasterElement, "TiePointList" );
      numLists = listVector.size();
      issea( numLists == 1 );
      issea( listVector.size() == 1 );
      issea( listVector.at( 0 ) == "TestTiePointList" );

      pList->getLayers( TIEPOINT_LAYER, layerVector );
      issea( layerVector.size() == 1 );
      name = layerVector.at(0)->getName();
      issea( name == "TestTiePointList" );
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea( name == pRasterElement->getName() );

      // rename the layer
      layerVector.clear();
      listVector.clear();
      dynamic_cast<TiePointLayerImp*>(pLayer)->setName("TestTiePointListRename");
      ModelServicesImp::instance()->setElementName( pTiePointList, "TestTiePointListRename" );
      pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );
      numLayers = pList->getNumLayers();
      issea( numLayers == 2 ); // raster layer, TestTiePointListRename

      listVector = ModelServicesImp::instance()->getElementNames( pRasterElement, "TiePointList" );
      numLists = listVector.size();
      issea( numLists == 1 );
      issea( listVector.size() == 1 );
      issea( listVector.at( 0 ) == "TestTiePointListRename" );

      pList->getLayers( TIEPOINT_LAYER, layerVector );
      issea( layerVector.size() == 1 );
      name = layerVector.at(0)->getName();
      issea( name == "TestTiePointListRename" );
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea( name == pRasterElement->getName() );

      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pLayer ) ) && success;
      }

      return success;
   }
};

class TiePointListRenameTest2 : public TestCase
{
public:
   TiePointListRenameTest2() : TestCase("ListRename2") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement(true);
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      TiePointList* pTiePointList = NULL;
      pTiePointList = dynamic_cast<TiePointList*>( ModelServicesImp::instance()->createElement(
         "TestTiePointList", "TiePointList", pRasterElement ) );
      issea( pTiePointList != NULL );

      pTiePointList->setMissionDatasetName( "cppTestMissionCube" );

      // add 100 semi-random points
      vector<TiePoint> tiePointVector;
      for( int row = 0; row < 10; row++ )
      {
         for( int col = 0; col < 10; col++ )
         {
            TiePoint point;
            point.mReferencePoint.mX = col + ( rand() / ( RAND_MAX / 50 + 1 ) );
            point.mReferencePoint.mY = row + ( rand() / ( RAND_MAX / 50 + 1 ) );
            point.mMissionOffset.mX = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
            point.mMissionOffset.mY = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
            point.mConfidence = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
            point.mPhi = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
            tiePointVector.push_back( point );
         }
      }
      pTiePointList->adoptTiePoints( tiePointVector );

      TiePointLayer* pLayer = NULL;
      pLayer = dynamic_cast<TiePointLayer*>( pView->createLayer( TIEPOINT_LAYER, pTiePointList, "TestTiePointList" ) );
      issea( pLayer != NULL );

      TiePointList* pTiePointList2 = NULL;
      pTiePointList2 = dynamic_cast<TiePointList*>( ModelServicesImp::instance()->createElement(
         "TestTiePointList2", "TiePointList", pRasterElement ) );
      issea( pTiePointList2 != NULL );

      pTiePointList->setMissionDatasetName( "cppTestMissionCube2" );

      // add 100 semi-random points
      vector<TiePoint> tiePointVector2;
      for( int row = 0; row < 10; row++ )
      {
         for( int col = 0; col < 10; col++ )
         {
            TiePoint point;
            point.mReferencePoint.mX = col + ( rand() / ( RAND_MAX / 50 + 1 ) );
            point.mReferencePoint.mY = row + ( rand() / ( RAND_MAX / 50 + 1 ) );
            point.mMissionOffset.mX = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
            point.mMissionOffset.mY = rand() / ( RAND_MAX / 15 + 1 ); // rand between 0 and 15
            point.mConfidence = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
            point.mPhi = rand() / ( RAND_MAX / 10 + 1 ); // rand between 0 and 10
            tiePointVector2.push_back( point );
         }
      }
      pTiePointList->adoptTiePoints( tiePointVector2 );

      TiePointLayer* pLayer2 = NULL;
      pLayer2 = dynamic_cast<TiePointLayer*>( pView->createLayer( TIEPOINT_LAYER, pTiePointList2, "TestTiePointList2" ) );
      issea( pLayer2 != NULL );

      unsigned int numLists = 0;
      unsigned int numLayers = 0;
      string name = "";
      vector<Layer*> layerVector;
      vector<string> listVector;
      LayerList *pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );
      numLayers = pList->getNumLayers();
      issea( numLayers == 3 ); // raster layer, TestTiePointList, TestTiePointList2

      listVector = ModelServicesImp::instance()->getElementNames( pRasterElement, "TiePointList" );
      numLists = listVector.size();
      issea( numLists == 2 );
      issea( listVector.size() == 2 );
      issea( listVector.at( 0 ) == "TestTiePointList" );
      issea( listVector.at( 1 ) == "TestTiePointList2" );

      pList->getLayers( TIEPOINT_LAYER, layerVector );
      issea( layerVector.size() == 2 );
      name = layerVector.at(0)->getName();
      issea( name == "TestTiePointList" );
      name = layerVector.at(1)->getName();
      issea( name == "TestTiePointList2" );
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea( name == pRasterElement->getName() );

      // rename the layer
      layerVector.clear();
      listVector.clear();
      dynamic_cast<TiePointLayerImp*>(pLayer)->setName("RenameTestTiePointList");
      ModelServicesImp::instance()->setElementName( pTiePointList, "RenameTestTiePointList" );
      pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );
      numLayers = pList->getNumLayers();
      issea( numLayers == 3 ); // raster layer, TestTiePointListRename, TestTiePointList2

      listVector = ModelServicesImp::instance()->getElementNames( pRasterElement, "TiePointList" );
      numLists = listVector.size();
      issea( numLists == 2 );
      issea( listVector.size() == 2 );      
      issea( listVector.at( 0 ) == "RenameTestTiePointList" );
      issea( listVector.at( 1 ) == "TestTiePointList2" );

      pList->getLayers( TIEPOINT_LAYER, layerVector );
      issea( layerVector.size() == 2 );
      name = layerVector.at(0)->getName();
      issea( name == "RenameTestTiePointList" );
      name = layerVector.at(1)->getName();
      issea( name == "TestTiePointList2" );
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea( name == pRasterElement->getName() );

      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pLayer ) ) && success;
         success = tst_assert( pView->deleteLayer( pLayer2 ) ) && success;
      }

      return success;
   }
};

class TiePointTestSuite : public TestSuiteNewSession
{
public:
   TiePointTestSuite() : TestSuiteNewSession( "TiePoint" )
   {
      addTestCase( new TiePointListCreationTest );
      addTestCase( new TiePointListRetrievalTest );
      addTestCase( new TiePointSerializeDeserializeTest );
      addTestCase( new TiePointSerializeDeserializeLayerTest );
      addTestCase( new TiePoint100PointsTest );
      addTestCase( new TiePoint1000PointsTest );
      addTestCase( new TiePoint10000PointsTest );
      addTestCase( new TiePoint100000PointsTest );
      addTestCase( new TiePoint1000000PointsTest );
      addTestCase( new TiePointListRenameTest );
      addTestCase( new TiePointListRenameTest2 );
   }
};

REGISTER_SUITE( TiePointTestSuite )
