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
#include "DesktopServicesImp.h"
#include "FilenameImp.h"
#include "GcpLayer.h"
#include "GcpLayerImp.h"
#include "GcpList.h"
#include "HighResolutionTimer.h"
#include "LayerList.h"
#include <math.h>
#include "ModelServicesImp.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "RasterElement.h"
#include "SpatialDataWindow.h"
#include "SpatialDataViewAdapter.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "UtilityServicesImp.h"
#include "xmlreader.h"
#include "xmlwriter.h"

#include <QtCore/QCoreApplication>

XERCES_CPP_NAMESPACE_USE
using namespace std;

class GcpCreationTest : public TestCase
{
public:
   GcpCreationTest() : TestCase("Creation") {}
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

      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList", "GcpList", pRasterElement ) );
      issea( pGcpList != NULL );

      GcpPoint point1;
      point1.mPixel.mX = 20;
      point1.mPixel.mY = 22;
      point1.mCoordinate.mX = 1;
      point1.mCoordinate.mY = 2;
      pGcpList->addPoint( point1 );

      GcpLayer* pProperties = NULL;
      pProperties = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestGcpLayer" ) );
      issea( pProperties != NULL );

      // make sure the new gcp layer is there
      LayerList *pLayerList = NULL;
      pLayerList = pView->getLayerList();
      issea( pLayerList != NULL );

      // make sure the gcp list is there
      GcpList *pTempList = NULL;
      pTempList = dynamic_cast<GcpList*>( pLayerList->getLayer( GCP_LAYER, pGcpList, "TestGcpLayer" )->getDataElement() );
      issea( pTempList != NULL );
      int pointCount = 0;
      pointCount = pTempList->getCount();
      issea( pointCount != 0 );
      issea( pointCount == 1 );
      issea( pTempList->getSelectedPoints().size() != 0 );
      issea( pTempList->getSelectedPoints().size() == 1 );
      GcpPoint thePoint = pTempList->getSelectedPoints().front();
      issea( thePoint.mPixel.mX == 20 && thePoint.mPixel.mY == 22 &&
         thePoint.mCoordinate.mX == 1 && thePoint.mCoordinate.mY == 2 );

      QCoreApplication::instance()->processEvents();
      if (pView != NULL)
      {
         success = tst_assert(pView->deleteLayer( pProperties )) && success;
      }

      return success;
   }
};

class GcpDeletionTest : public TestCase
{
public:
   GcpDeletionTest() : TestCase("Deletion") {}
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

      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList", "GcpList", pRasterElement ) );
      issea( pGcpList != NULL );

      GcpPoint point1;
      point1.mPixel.mX = 15;
      point1.mPixel.mY = 17;
      point1.mCoordinate.mX = 3;
      point1.mCoordinate.mY = 4;
      pGcpList->addPoint( point1 );

      GcpLayer* pProperties = NULL;
      pProperties = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestGcpLayer" ) );
      issea( pProperties != NULL );

      QCoreApplication::instance()->processEvents();

      // make sure the new gcp layer is there
      LayerList *pLayerList = NULL;
      pLayerList = pView->getLayerList();
      issea( pLayerList != NULL );

      // make sure the gcp list is there
      GcpList *pTempList = NULL;
      pTempList = dynamic_cast<GcpList*>( pLayerList->getLayer( GCP_LAYER, pGcpList, "TestGcpLayer" )->getDataElement() );
      issea( pTempList != NULL );
      int pointCount = 0;
      pointCount = pTempList->getCount();
      issea( pointCount != 0 );
      issea( pointCount == 1 );
      issea( pTempList->getSelectedPoints().size() != 0 );
      issea( pTempList->getSelectedPoints().size() == 1 );
      GcpPoint thePoint = pTempList->getSelectedPoints().front();
      issea( thePoint.mPixel.mX == 15 && thePoint.mPixel.mY == 17 &&
         thePoint.mCoordinate.mX == 3 && thePoint.mCoordinate.mY == 4 );      

      // now delete the Gcp point
      pGcpList->removePoint( point1 );
      pointCount = pGcpList->getCount();
      issea( pointCount != 1 );
      issea( pointCount == 0 );

      pointCount = pTempList->getCount();
      issea( pointCount != 1 );
      issea( pointCount == 0 );
      issea( pTempList->getSelectedPoints().size() == 0 );
      issea( pTempList->getSelectedPoints().size() != 1 );

      pGcpList = NULL;
      pTempList = NULL;
      if (pView != NULL)
      {
         success = tst_assert(pView->deleteLayer( pProperties )) && success;
      }

      return success;
   }
};

class GcpListCreationTest : public TestCase
{
public:
   GcpListCreationTest() : TestCase("ListCreation") {}
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

      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList", "GcpList", pRasterElement ) );
      issea( pGcpList != NULL );

      list<GcpPoint> gcpListPoints;

      GcpPoint point1, point2, point3, point4, point5;
      point1.mPixel.mX = 10;
      point1.mPixel.mY = 15;
      point1.mCoordinate.mX = 1.1;
      point1.mCoordinate.mY = 1.2;
      gcpListPoints.push_back( point1 );

      point2.mPixel.mX = 20;
      point2.mPixel.mY = 25;
      point2.mCoordinate.mX = 2.1;
      point2.mCoordinate.mY = 2.2;
      gcpListPoints.push_back( point2 );

      point3.mPixel.mX = 30;
      point3.mPixel.mY = 35;
      point3.mCoordinate.mX = 3.1;
      point3.mCoordinate.mY = 3.2;
      gcpListPoints.push_back( point3 );

      point4.mPixel.mX = 40;
      point4.mPixel.mY = 45;
      point4.mCoordinate.mX = 4.1;
      point4.mCoordinate.mY = 4.2;
      gcpListPoints.push_back( point4 );

      point5.mPixel.mX = 50;
      point5.mPixel.mY = 55;
      point5.mCoordinate.mX = 5.1;
      point5.mCoordinate.mY = 5.2;
      gcpListPoints.push_back( point5 );
      pGcpList->addPoints( gcpListPoints );

      GcpLayer* pProperties = NULL;
      pProperties = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestGcpLayer" ) );
      issea( pProperties != NULL );

      QCoreApplication::instance()->processEvents();

      int pointCount = 0;
      pointCount = pGcpList->getCount();
      issea( pointCount != 0 );
      issea( pointCount == 5 );

      // make sure the new gcp layer is there
      LayerList *pLayerList = NULL;
      pLayerList = pView->getLayerList();
      issea( pLayerList != NULL );

      // make sure the gcp list is there
      GcpList *pTempList = NULL;
      pTempList = dynamic_cast<GcpList*>( pLayerList->getLayer( GCP_LAYER, pGcpList, "TestGcpLayer" )->getDataElement() );
      issea( pTempList != NULL );
      pointCount = pTempList->getCount();
      issea( pointCount != 0 );
      issea( pointCount == 5 );
      list<GcpPoint> tempGcpPoints = pTempList->getSelectedPoints();
      issea( gcpListPoints.size() == tempGcpPoints.size() );

      if( success )
      {
         for( int i = 0; i < pointCount; i++ )
         {
            GcpPoint thePoint1 = gcpListPoints.front();
            GcpPoint thePoint2 = tempGcpPoints.front();
            issea( thePoint1.mPixel.mX == thePoint2.mPixel.mX && thePoint1.mPixel.mY == thePoint2.mPixel.mY &&
               thePoint1.mCoordinate.mX == thePoint2.mCoordinate.mX && thePoint1.mCoordinate.mY == thePoint2.mCoordinate.mY );
            gcpListPoints.pop_front();
            tempGcpPoints.pop_front();
         }
      }

      pGcpList = NULL;
      pTempList = NULL;
      issea( pView->deleteLayer( pProperties ) == true );

      return success;
   }
};

class GcpSerializeDeserializeTest : public TestCase
{
public:
   GcpSerializeDeserializeTest() : TestCase("Serialize") {}
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

      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList", "GcpList", pRasterElement ) );
      issea( pGcpList != NULL );

      GcpPoint point1;
      point1.mPixel.mX = 16;
      point1.mPixel.mY = 18;
      point1.mCoordinate.mX = 3.5;
      point1.mCoordinate.mY = 4.5;
      pGcpList->addPoint( point1 );

      GcpLayer* pProperties = NULL;
      pProperties = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestGcpLayer" ) );
      issea( pProperties != NULL );

      QCoreApplication::instance()->processEvents();

      bool ok = false;
      // serialize the GcpList
      FILE *pOutputFile = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }

      QString temp = tempHome.c_str();
      temp.replace( "\\", "/" );
      tempHome = temp.toStdString();

      pOutputFile = fopen( ( tempHome + "/GcpList.gcp" ).c_str(), "w" );
      issea( pOutputFile != NULL );

      XMLWriter xwrite( "GcpList" );
      ok = pGcpList->toXml( &xwrite );
      issea( ok == true );
      xwrite.writeToFile( pOutputFile );
      fclose( pOutputFile );
      issea( ok == true );

      ModelServicesImp::instance()->setElementName(pGcpList, "TestListSerialized");

      GcpList* pGcpListCopy = NULL;
      pGcpListCopy = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList", "GcpList", pRasterElement ) );
      issea( pGcpListCopy != NULL );

      // deserialize the GcpList
      MessageLog *pLog( UtilityServicesImp::instance()->getMessageLog()->getLog( "session" ) );
      XmlReader xml( pLog );

      FilenameImp gcpFile( tempHome + "/GcpList.gcp" );

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *gcpDoc( NULL );
      try
      {
         gcpDoc = xml.parse( &gcpFile );
      }
      catch( XmlBase::XmlException & )
      {
         // do nothing
      }
      DOMElement *rootelement( NULL );
      if( gcpDoc != NULL )
      {
         rootelement = gcpDoc->getDocumentElement();
      }
      issea( rootelement != NULL );
      if( success )
      {
         unsigned int formatVersion = atoi( A( rootelement->getAttribute( X( "version" ) ) ) );
         try
         {
            ok = pGcpListCopy->fromXml( rootelement, formatVersion );
         }
         catch (...)
         {
            ok = false;
         }
         issea( ok == true );

         issea( pGcpListCopy->getCount() == pGcpList->getCount() );
      }

      if (success)
      {
         GcpPoint thePoint1 = pGcpListCopy->getSelectedPoints().front();
         GcpPoint thePoint2 = pGcpList->getSelectedPoints().front();
         issea( thePoint1.mPixel.mX == thePoint2.mPixel.mX && thePoint1.mPixel.mY == thePoint2.mPixel.mY &&
            thePoint1.mCoordinate.mX == thePoint2.mCoordinate.mX && thePoint1.mCoordinate.mY == thePoint2.mCoordinate.mY );
      }

      pGcpList = NULL;
      if (pView != NULL)
      {
         success = tst_assert(pView->deleteLayer( pProperties )) && success;
      }
      ModelServicesImp::instance()->destroyElement(pGcpListCopy);

      return success;
   }
};

class GcpSerializeDeserializeListTest : public TestCase
{
public:
   GcpSerializeDeserializeListTest() : TestCase("SerializeList") {}
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

      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList", "GcpList", pRasterElement ) );
      issea( pGcpList != NULL );

      list<GcpPoint> gcpListPoints;

      GcpPoint point1, point2, point3, point4, point5;
      point1.mPixel.mX = 10;
      point1.mPixel.mY = 15;
      point1.mCoordinate.mX = 1.1;
      point1.mCoordinate.mY = 1.2;
      gcpListPoints.push_back( point1 );

      point2.mPixel.mX = 20;
      point2.mPixel.mY = 25;
      point2.mCoordinate.mX = 2.1;
      point2.mCoordinate.mY = 2.2;
      gcpListPoints.push_back( point2 );

      point3.mPixel.mX = 30;
      point3.mPixel.mY = 35;
      point3.mCoordinate.mX = 3.1;
      point3.mCoordinate.mY = 3.2;
      gcpListPoints.push_back( point3 );

      point4.mPixel.mX = 40;
      point4.mPixel.mY = 45;
      point4.mCoordinate.mX = 4.1;
      point4.mCoordinate.mY = 4.2;
      gcpListPoints.push_back( point4 );

      point5.mPixel.mX = 50;
      point5.mPixel.mY = 55;
      point5.mCoordinate.mX = 5.1;
      point5.mCoordinate.mY = 5.2;
      gcpListPoints.push_back( point5 );
      pGcpList->addPoints( gcpListPoints );

      GcpLayer* pProperties = NULL;
      pProperties = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestGcpLayer" ) );
      issea( pProperties != NULL );

      QCoreApplication::instance()->processEvents();

      int pointCount = 0;
      pointCount = pGcpList->getCount();
      issea( pointCount != 0 );
      issea( pointCount == 5 );

      bool ok = false;
      // serialize the GcpList
      FILE *pOutputFile = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }

      pOutputFile = fopen( ( tempHome + "/GcpLargeList.gcp" ).c_str(), "w" );
      issea( pOutputFile != NULL );

      XMLWriter xwrite( "GcpList" );
      ok = pGcpList->toXml( &xwrite );
      issea( ok == true );
      xwrite.writeToFile( pOutputFile );
      fclose( pOutputFile );
      issea( ok == true );

      ModelServicesImp::instance()->setElementName(pGcpList, "TestListSerialized");

      GcpList* pGcpListCopy = NULL;
      pGcpListCopy = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList", "GcpList", pRasterElement ) );
      issea( pGcpListCopy != NULL );

      // deserialize the GcpList
      MessageLog *pLog( UtilityServicesImp::instance()->getMessageLog()->getLog( "session" ) );
      XmlReader xml( pLog );

      FilenameImp gcpFile( tempHome + "/GcpLargeList.gcp" );

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *gcpDoc( NULL );
      try
      {
         gcpDoc = xml.parse( &gcpFile );
      }
      catch( XmlBase::XmlException & )
      {
         // do nothing
      }
      DOMElement *rootelement( NULL );
      if( gcpDoc != NULL )
      {
         rootelement = gcpDoc->getDocumentElement();
      }
      issea( rootelement != NULL );

      if( success )
      {
         unsigned int formatVersion = atoi( A( rootelement->getAttribute( X( "version" ) ) ) );
         try
         {
            ok = pGcpListCopy->fromXml( rootelement, formatVersion );
         }
         catch (...)
         {
            ok = false;
         }
         issea( ok == true );

         pointCount = 0;
         pointCount = pGcpListCopy->getCount();
         issea( pointCount != 0 );
         issea( pointCount == 5 );

         list<GcpPoint> GcpPointsCopy;
         GcpPointsCopy = pGcpListCopy->getSelectedPoints();
         issea( gcpListPoints.size() == GcpPointsCopy.size() );
         if( success )
         {
            for( int i = 0; i < pointCount; i++ )
            {
               GcpPoint thePoint1 = gcpListPoints.front(); // original list
               GcpPoint thePoint2 = GcpPointsCopy.front(); // deserialized list
               issea( thePoint1.mPixel.mX == thePoint2.mPixel.mX && thePoint1.mPixel.mY == thePoint2.mPixel.mY &&
                  thePoint1.mCoordinate.mX == thePoint2.mCoordinate.mX && thePoint1.mCoordinate.mY == thePoint2.mCoordinate.mY );
               gcpListPoints.pop_front();
               GcpPointsCopy.pop_front();
            }
         }
      }

      if (pView != NULL)
      {
         success = tst_assert(pView->deleteLayer( pProperties )) && success;
      }

      ModelServicesImp::instance()->destroyElement(pGcpListCopy);

      return success;
   }
};

class GcpSerializeDeserializeLayerTest : public TestCase
{
public:
   GcpSerializeDeserializeLayerTest() : TestCase("SerializeLayer") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow(
         pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList", "GcpList", pRasterElement ) );
      issea( pGcpList != NULL );

      list<GcpPoint> gcpListPoints;

      GcpPoint point1, point2, point3, point4, point5;
      point1.mPixel.mX = 10;
      point1.mPixel.mY = 15;
      point1.mCoordinate.mX = 1.1;
      point1.mCoordinate.mY = 1.2;
      gcpListPoints.push_back( point1 );

      point2.mPixel.mX = 20;
      point2.mPixel.mY = 25;
      point2.mCoordinate.mX = 2.1;
      point2.mCoordinate.mY = 2.2;
      gcpListPoints.push_back( point2 );

      point3.mPixel.mX = 30;
      point3.mPixel.mY = 35;
      point3.mCoordinate.mX = 3.1;
      point3.mCoordinate.mY = 3.2;
      gcpListPoints.push_back( point3 );

      point4.mPixel.mX = 40;
      point4.mPixel.mY = 45;
      point4.mCoordinate.mX = 4.1;
      point4.mCoordinate.mY = 4.2;
      gcpListPoints.push_back( point4 );

      point5.mPixel.mX = 50;
      point5.mPixel.mY = 55;
      point5.mCoordinate.mX = 5.1;
      point5.mCoordinate.mY = 5.2;
      gcpListPoints.push_back( point5 );
      pGcpList->addPoints( gcpListPoints );

      GcpLayer* pProperties = NULL;
      pProperties = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestGcpLayer" ) );
      issea( pProperties != NULL );

      QCoreApplication::instance()->processEvents();

      int pointCount = 0;
      pointCount = pGcpList->getCount();
      issea( pointCount != 0 );
      issea( pointCount == 5 );

      bool ok = false;
      // serialize the GcpList
      FILE *pOutputFile = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }

      pOutputFile = fopen( ( tempHome + "/GcpLayer.gcplayer" ).c_str(), "w" );
      issea( pOutputFile != NULL );

      XMLWriter xwrite( "GcpLayer" );
      ok = pGcpList->toXml( &xwrite );
      issea( ok == true );
      xwrite.writeToFile( pOutputFile );
      fclose( pOutputFile );
      issea( ok == true );

      ModelServicesImp::instance()->setElementName( pGcpList, "TestListSerialized" );

      GcpList* pGcpListCopy = NULL;
      pGcpListCopy = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList", "GcpList", pRasterElement ) );
      issea( pGcpListCopy != NULL );

      GcpLayer* pPropertiesCopy = NULL;
      pPropertiesCopy = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpListCopy, "TestLayerDup" ) );
      issea( pPropertiesCopy != NULL );

      // deserialize the GcpList
      MessageLog *pLog( UtilityServicesImp::instance()->getMessageLog()->getLog( "session" ) );
      XmlReader xml( pLog );

      FilenameImp gcpFile( tempHome + "/GcpLayer.gcplayer" );

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *gcpDoc( NULL );
      try
      {
         gcpDoc = xml.parse( &gcpFile );
      }
      catch( XmlBase::XmlException & )
      {
         // do nothing
      }
      DOMElement *rootelement( NULL );
      if( gcpDoc != NULL )
      {
         rootelement = gcpDoc->getDocumentElement();
      }
      issea( rootelement != NULL );

      if( success )
      {
         unsigned int formatVersion = atoi( A( rootelement->getAttribute( X( "version" ) ) ) );
         try
         {
            ok = pGcpListCopy->fromXml( rootelement, formatVersion );
         }
         catch (...)
         {
            ok = false;
         }
         issea( ok == true );
      }

      pointCount = 0;
      pointCount = pGcpListCopy->getCount();
      issea( pointCount != 0 );
      issea( pointCount == 5 );

      issea( static_cast<GcpList*>( pPropertiesCopy->getDataElement() )->getName() == "TestLayerDup" );
      list<GcpPoint> GcpPointsCopy;
      GcpPointsCopy = static_cast<GcpList*>( pPropertiesCopy->getDataElement() )->getSelectedPoints();
      issea( gcpListPoints.size() == GcpPointsCopy.size() );

      if( success )
      {
         for( int i = 0; i < pointCount; i++ )
         {
            GcpPoint thePoint1 = gcpListPoints.front(); // original list
            GcpPoint thePoint2 = GcpPointsCopy.front(); // deserialized list
            issea( thePoint1.mPixel.mX == thePoint2.mPixel.mX && thePoint1.mPixel.mY == thePoint2.mPixel.mY &&
               thePoint1.mCoordinate.mX == thePoint2.mCoordinate.mX && thePoint1.mCoordinate.mY == thePoint2.mCoordinate.mY );
            gcpListPoints.pop_front();
            GcpPointsCopy.pop_front();
         }
      }

      if( pProperties != NULL )
      {
         pView->deleteLayer( pProperties );
         pProperties = NULL;
      }
      if( pPropertiesCopy != NULL )
      {
         pView->deleteLayer( pPropertiesCopy );
         pPropertiesCopy = NULL;
      }      
      pGcpList = NULL;
      pGcpListCopy = NULL;

      return success;
   }
};


class GcpSerializePrecisionTest : public TestCase
{
public:
   GcpSerializePrecisionTest() : TestCase("SerializePrecision") {}
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

      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList", "GcpList", pRasterElement ) );
      issea( pGcpList != NULL );

      list<GcpPoint> gcpListPoints;

      GcpPoint point1, point2, point3, point4, point5;
      point1.mPixel.mX = 27;
      point1.mPixel.mY = 80;
      point1.mCoordinate.mX = 41.2345678912;
      point1.mCoordinate.mY = 14.2345678912;
      gcpListPoints.push_back( point1 );

      point2.mPixel.mX = 11;
      point2.mPixel.mY = 13;
      point2.mCoordinate.mX = 42.1101987654;
      point2.mCoordinate.mY = 24.1101987654;
      gcpListPoints.push_back( point2 );

      point3.mPixel.mX = 24;
      point3.mPixel.mY = 24;
      point3.mCoordinate.mX = 43.2345678912;
      point3.mCoordinate.mY = 34.2345678912;
      gcpListPoints.push_back( point3 );

      point4.mPixel.mX = 54;
      point4.mPixel.mY = 53;
      point4.mCoordinate.mX = 44.1101987654;
      point4.mCoordinate.mY = 44.1101987654;
      gcpListPoints.push_back( point4 );

      point5.mPixel.mX = 42;
      point5.mPixel.mY = 164;
      point5.mCoordinate.mX = 155.234567891;
      point5.mCoordinate.mY = 551.110198765;
      gcpListPoints.push_back( point5 );
      pGcpList->addPoints( gcpListPoints );

      GcpLayer* pProperties = NULL;
      pProperties = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestGcpLayer" ) );
      issea( pProperties != NULL );

      QCoreApplication::instance()->processEvents();

      int pointCount = 0;
      pointCount = pGcpList->getCount();
      issea( pointCount != 0 );
      issea( pointCount == 5 );

      bool ok = false;
      // serialize the GcpList
      FILE *pOutputFile = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }

      pOutputFile = fopen( ( tempHome + "/GcpPrecisionList.gcp" ).c_str(), "w" );
      issea( pOutputFile != NULL );

      XMLWriter xwrite( "GcpList" );
      ok = pGcpList->toXml( &xwrite );
      issea( ok == true );
      xwrite.writeToFile( pOutputFile );
      fclose( pOutputFile );
      issea( ok == true );

      ModelServicesImp::instance()->setElementName(pGcpList, "TestListSerialized");

      GcpList* pGcpListCopy = NULL;
      pGcpListCopy = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList", "GcpList", pRasterElement ) );
      issea( pGcpListCopy != NULL );

      // deserialize the GcpList
      MessageLog *pLog( UtilityServicesImp::instance()->getMessageLog()->getLog( "session" ) );
      XmlReader xml( pLog );

      FilenameImp gcpFile( tempHome + "/GcpPrecisionList.gcp" );

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *gcpDoc( NULL );
      try
      {
         gcpDoc = xml.parse( &gcpFile );
      }
      catch( XmlBase::XmlException & )
      {
         // do nothing
      }
      DOMElement *rootelement( NULL );
      if( gcpDoc != NULL )
      {
         rootelement = gcpDoc->getDocumentElement();
      }
      issea( rootelement != NULL );

      if( success )
      {
         unsigned int formatVersion = atoi( A( rootelement->getAttribute( X( "version" ) ) ) );
         try
         {
            ok = pGcpListCopy->fromXml( rootelement, formatVersion );
         }
         catch (...)
         {
            ok = false;
         }
         issea( ok == true );

         pointCount = 0;
         pointCount = pGcpListCopy->getCount();
         issea( pointCount != 0 );
         issea( pointCount == 5 );

         list<GcpPoint> GcpPointsCopy;
         GcpPointsCopy = pGcpListCopy->getSelectedPoints();
         issea( gcpListPoints.size() == GcpPointsCopy.size() );

         if( success )
         {
            for( int i = 0; i < pointCount; i++ )
            {
               GcpPoint thePoint1 = gcpListPoints.front(); // original list
               GcpPoint thePoint2 = GcpPointsCopy.front(); // deserialized list
               issea( thePoint1.mPixel.mX == thePoint2.mPixel.mX && thePoint1.mPixel.mY == thePoint2.mPixel.mY &&
                  thePoint1.mCoordinate.mX == thePoint2.mCoordinate.mX && thePoint1.mCoordinate.mY == thePoint2.mCoordinate.mY );
               gcpListPoints.pop_front();
               GcpPointsCopy.pop_front();
            }
         }
      }

      pGcpList = NULL;
      pGcpListCopy = NULL;
      if (pView != NULL)
      {
         success = tst_assert(pView->deleteLayer( pProperties )) && success;
      }
      ModelServicesImp::instance()->destroyElement(pGcpListCopy);

      return success;
   }
};

class GcpLayerRenameTest : public TestCase
{
public:
   GcpLayerRenameTest() : TestCase("LayerRename") {}
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

      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList", "GcpList", pRasterElement ) );
      issea( pGcpList != NULL );

      list<GcpPoint> gcpListPoints;

      // add 20 random GCPs to the list
      for( int i = 0; i < 20; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand() / ( RAND_MAX / 97 + 1 ); // rand between 0 and numRows ??
         newPoint.mPixel.mY = rand() / ( RAND_MAX / 181 + 1 ); // rand between 0 and numCols ??
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }
      pGcpList->addPoints( gcpListPoints );
      QCoreApplication::instance()->processEvents();

      GcpLayer* pProperties = NULL;      
      pProperties = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestList" ) );
      issea( pProperties != NULL );

      unsigned int numLists = 0;
      unsigned int numLayers = 0;
      string name = "";
      vector<Layer*> layerVector;
      vector<string> listVector;
      LayerList *pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );
      numLayers = pList->getNumLayers();
      issea( numLayers == 2 ); // raster layer, TestList

      listVector = ModelServicesImp::instance()->getElementNames( pRasterElement, "GcpList" );
      numLists = listVector.size();
      issea( numLists == 1 );
      issea( listVector.size() == 1 );
      issea( listVector.at( 0 ) == "TestList" );

      pList->getLayers( GCP_LAYER, layerVector );
      issea( layerVector.size() == 1 );
      name = layerVector.at(0)->getName();
      issea( name == "TestList" );
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea( name == pRasterElement->getName() );

      // rename the layer
      layerVector.clear();
      listVector.clear();
      dynamic_cast<GcpLayerImp*>(pProperties)->setName("TestGcpLayerRename");
      ModelServicesImp::instance()->setElementName( pGcpList, "TestGcpLayerRename" );
      pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );
      numLayers = pList->getNumLayers();
      issea( numLayers == 2 ); // raster layer, TestGcpLayerRename

      listVector = ModelServicesImp::instance()->getElementNames( pRasterElement, "GcpList" );
      numLists = listVector.size();
      issea( numLists == 1 );
      issea( listVector.size() == 1 );
      issea( listVector.at( 0 ) == "TestGcpLayerRename" );

      pList->getLayers( GCP_LAYER, layerVector );
      issea( layerVector.size() == 1 );
      name = layerVector.at(0)->getName();
      issea( name == "TestGcpLayerRename" );
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea( name == pRasterElement->getName() );

      pGcpList->clearPoints();
      gcpListPoints.clear();
      issea( ModelServicesImp::instance()->destroyElement( pGcpList ) == true );

      return success;
   }
};

class GcpLayerRenameTest2 : public TestCase
{
public:
   GcpLayerRenameTest2() : TestCase("LayerRename2") {}
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

      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList", "GcpList", pRasterElement ) );
      issea( pGcpList != NULL );

      list<GcpPoint> gcpListPoints;

      // add 20 random GCPs to the list
      for( int i = 0; i < 20; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand() / ( RAND_MAX / 97 + 1 ); // rand between 0 and numRows ??
         newPoint.mPixel.mY = rand() / ( RAND_MAX / 181 + 1 ); // rand between 0 and numCols ??
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }
      pGcpList->addPoints( gcpListPoints );

      GcpLayer* pProperties = NULL;      
      pProperties = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestList" ) );
      issea( pProperties != NULL );

      GcpList* pGcpList2 = NULL;
      pGcpList2 = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "TestList2", "GcpList", pRasterElement ) );
      issea( pGcpList2 != NULL );

      list<GcpPoint> gcpListPoints2;

      // add 25 random GCPs to the list
      for( int i = 0; i < 25; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand() / ( RAND_MAX / 97 + 1 ); // rand between 0 and numRows ??
         newPoint.mPixel.mY = rand() / ( RAND_MAX / 181 + 1 ); // rand between 0 and numCols ??
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints2.push_back( newPoint );
      }
      pGcpList2->addPoints( gcpListPoints2 );
      QCoreApplication::instance()->processEvents();

      GcpLayer* pProperties2 = NULL;      
      pProperties2 = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList2, "TestList2" ) );
      issea( pProperties2 != NULL );

      unsigned int numLists = 0;
      unsigned int numLayers = 0;
      string name = "";
      vector<Layer*> layerVector;
      vector<string> listVector;
      LayerList *pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );
      numLayers = pList->getNumLayers();
      issea( numLayers == 3 ); // raster layer, TestList, TestList2

      listVector = ModelServicesImp::instance()->getElementNames( pRasterElement, "GcpList" );
      numLists = listVector.size();
      issea( numLists == 2 );
      issea( listVector.size() == 2 );
      issea( listVector.at( 0 ) == "TestList" );
      issea( listVector.at( 1 ) == "TestList2" );

      pList->getLayers( GCP_LAYER, layerVector );
      issea( layerVector.size() == 2 );
      name = layerVector.at(0)->getName();
      issea( name == "TestList" );
      name = layerVector.at(1)->getName();
      issea( name == "TestList2" );
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea( name == pRasterElement->getName() );

      // rename the layer
      layerVector.clear();
      listVector.clear();
      dynamic_cast<GcpLayerImp*>(pProperties)->setName("TestGcpLayerRename");
      ModelServicesImp::instance()->setElementName( pGcpList, "TestGcpLayerRename" );
      pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );
      numLayers = pList->getNumLayers();
      issea( numLayers == 3 ); // raster layer, TestGcpLayerRename, TestList2

      listVector = ModelServicesImp::instance()->getElementNames( pRasterElement, "GcpList" );
      numLists = listVector.size();
      issea( numLists == 2 );
      issea( listVector.size() == 2 );
      issea( listVector.at( 0 ) == "TestGcpLayerRename" );
      issea( listVector.at( 1 ) == "TestList2" );

      pList->getLayers( GCP_LAYER, layerVector );
      issea( layerVector.size() == 2 );
      name = layerVector.at(0)->getName();
      issea( name == "TestGcpLayerRename" );
      name = layerVector.at(1)->getName();
      issea( name == "TestList2" );
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea( name == pRasterElement->getName() );

      pGcpList->clearPoints();
      gcpListPoints.clear();
      ModelServicesImp::instance()->destroyElement( pGcpList );
      pGcpList2->clearPoints();
      gcpListPoints2.clear();
      ModelServicesImp::instance()->destroyElement( pGcpList2 );

      return success;
   }
};

class GcpTestSuite : public TestSuiteNewSession
{
public:
   GcpTestSuite() : TestSuiteNewSession( "Gcp" )
   {
      addTestCase( new GcpCreationTest );
      addTestCase( new GcpDeletionTest );
      addTestCase( new GcpListCreationTest );
      addTestCase( new GcpSerializeDeserializeTest );
      addTestCase( new GcpSerializeDeserializeListTest );
      addTestCase( new GcpSerializeDeserializeLayerTest );
      addTestCase( new GcpSerializePrecisionTest );
      addTestCase( new GcpLayerRenameTest );
      addTestCase( new GcpLayerRenameTest2 );
   }
};

REGISTER_SUITE( GcpTestSuite )
