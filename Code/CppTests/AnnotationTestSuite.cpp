/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

#include <math.h>

#include "AnnotationLayerAdapter.h"
#include "AnnotationLayerImp.h"
#include "ArcObject.h"
#include "assert.h"
#include "ConfigurationSettingsImp.h"
#include "DesktopServices.h"
#include "FilenameImp.h"
#include "Font.h"
#include "GraphicGroup.h"
#include "GraphicObjectImp.h"
#include "HighResolutionTimer.h"
#include "LayerList.h"
#include "ModelServicesImp.h"
#include "ObjectResource.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "PolygonObject.h"
#include "PolylineObject.h"
#include "ProductView.h"
#include "ProductWindow.h"
#include "RasterElement.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "UtilityServicesImp.h"

#include <algorithm>
XERCES_CPP_NAMESPACE_USE

using namespace std;

class AnnotationHitTestCase : public TestCase
{
public:
   AnnotationHitTestCase() : TestCase("HitTest") {}
   bool run()
   {
      bool success = true;      
      const double XSIZE = 50.0, YSIZE = 5.0;
      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      success = success && tst_assert( pRasterElement != NULL );

      if( !success )
      {
         return false;
      }

      QCoreApplication::instance()->processEvents();
      SpatialDataWindow* pSdwd = dynamic_cast<SpatialDataWindow*>(
         Service<DesktopServices>()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      success = success && tst_assert( pSdwd != NULL );

      SpatialDataView* pSdview = pSdwd->getSpatialDataView();
      issea( pSdview != NULL );

      AnnotationLayerAdapter *pProp = NULL;
      if( success )
      {
         LayerList *pLayerList = NULL;
         pLayerList = pSdview->getLayerList();
         issea( pLayerList != NULL );

         pProp = dynamic_cast<AnnotationLayerAdapter*>( pLayerList->getLayer( ANNOTATION, NULL, "Annotation 1" ) );
         if( pProp == NULL )
         {
            pProp = dynamic_cast<AnnotationLayerAdapter*>( pSdview->createLayer( ANNOTATION, NULL, "Annotation 1" ) );
         }
      }
      issea( pProp != NULL );

      GraphicObjectImp *pObj = NULL, *pObj2 = NULL;
      GraphicObjectImp *pGroup = NULL;

      if( success )
      {
         pObj = dynamic_cast<GraphicObjectImp*>(pProp->addObject( RECTANGLE_OBJECT ));
      }
      issea( pObj != NULL );

      LocationType corners[4] = {LocationType( 0.0, 0.0 ), LocationType( XSIZE, 0.0 ),
         LocationType( XSIZE, YSIZE ), LocationType( 0.0, YSIZE )};
      issea( pObj->setBoundingBox( LocationType( 0.0, 0.0 ), LocationType( XSIZE, YSIZE ) ) );
      if( success )
      {
         pObj->setLineState( false );
      }
      if( success )
      {
         pObj2 = dynamic_cast<GraphicObjectImp*>(pProp->addObject( RECTANGLE_OBJECT ));
      }
      issea( pObj2 != NULL );
      issea( pObj2->setBoundingBox( LocationType( 1000.0, 1000.0 ), LocationType( 2000.0, 2000.0 ) ) );

      if( success )
      {
         pProp->selectObject( dynamic_cast<GraphicObject*>(pObj) );
      }
      if( success )
      {
         pProp->selectObject( dynamic_cast<GraphicObject*>(pObj2) );
      }
      if( success )
      {
         pProp->groupSelection();
      }
      if( success )
      {
         list<GraphicObject*> allObjects;
         pProp->getObjects( allObjects );
         issea( allObjects.size() > 0 );
         if( success )
         {
            pGroup = dynamic_cast<GraphicObjectImp*>( *allObjects.begin() );
         }
      }

      const double OMAG = 1.0;
      LocationType offsets[4] = {LocationType( OMAG, OMAG ), LocationType( -OMAG, OMAG ),
         LocationType( -OMAG, -OMAG ), LocationType( OMAG, -OMAG )};
      bool results[4][4] = {
         true, false, false, false, 
         false, true, false, false,
         false, false, true, false,
         false, false, false, true};

      for( int i = 0; i < 4; ++i )
      {
         for( int j = 0; j < 4; ++j )
         {
            issea( pGroup->hit( corners[i] + offsets[j] ) == results[i][j] );
         }
      }
      if( success )
      {
         pProp->ungroupSelection();
      }
      if( success ) 
      {
         pProp->deselectAllObjects();
      }
      issea( pObj->setRotation( 90.0 ) );
      if( success )
      {
         pProp->selectObject( dynamic_cast<GraphicObject*>( pObj ) );
      }
      if( success )
      {
         pProp->selectObject( dynamic_cast<GraphicObject*>( pObj2 ) );
      }
      if( success ) 
      {
         pProp->groupSelection();
      }
      if( success )
      {
         list<GraphicObject*> allObjects;
         pProp->getObjects( allObjects );
         issea( allObjects.size() > 0 );
         if( success )
         {
            pGroup = dynamic_cast<GraphicObjectImp*>( *allObjects.begin() );
         }
      }
      const double XCENTER = XSIZE / 2.0;
      const double YCENTER = YSIZE / 2.0;
      LocationType corners2[4] = {
         LocationType( XCENTER+YSIZE / 2.0, YCENTER - XSIZE / 2.0 ), 
         LocationType( XCENTER+YSIZE / 2.0, YCENTER + XSIZE / 2.0 ), 
         LocationType( XCENTER-YSIZE / 2.0, YCENTER + XSIZE / 2.0 ), 
         LocationType( XCENTER-YSIZE / 2.0, YCENTER - XSIZE / 2.0 )};
      bool results2[4][4] = {
         false, true, false, false,
         false, false, true, false,
         false, false, false, true,
         true, false, false, false};
      for( int i = 0; i < 4; ++i )
      {
         for( int j = 0; j < 4; ++j )
         {
            issea( pGroup->hit( corners2[i] + offsets[j] ) == results2[i][j] );
         }
      }
      if( success )
      {
         pProp->removeObject( dynamic_cast<GraphicObject*>( pGroup ), true );
      }

      if( pSdview != NULL )
      {
         success = tst_assert( pSdview->deleteLayer( pProp ) ) && success;
      }

      return success;
   }
};

class AnnotationBoxSelectTestCase : public TestCase
{
public:
   AnnotationBoxSelectTestCase() : TestCase("BoxSelect") {}
   LocationType rotateAroundCenter( LocationType point, LocationType center, double angle )
   {
      LocationType deltaV = point - center;
      double mag = sqrt( deltaV.mX * deltaV.mX + deltaV.mY * deltaV.mY );
      double theta = atan2( deltaV.mY, deltaV.mX );
      theta += angle * PI / 180.0;
      LocationType result;
      result.mX = mag * cos( theta );
      result.mY = mag * sin( theta );
      return result + center;
   }
   bool run()
   {
      bool success = true;
      const double XSIZE = 50.0, YSIZE = 5.0;
      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      success = success && tst_assert( pRasterElement != NULL );

      SpatialDataWindow* pSdwd = dynamic_cast<SpatialDataWindow*>(
         Service<DesktopServices>()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea( pSdwd != NULL );

      SpatialDataView* pSdview = pSdwd->getSpatialDataView();
      issea( pSdview != NULL );

      AnnotationLayerAdapter *pProp = NULL;
      if( success )
      {
         LayerList *pLayerList = NULL;
         pLayerList = pSdview->getLayerList();
         issea( pLayerList != NULL );

         pProp = dynamic_cast<AnnotationLayerAdapter*>( pLayerList->getLayer( ANNOTATION, NULL, "Annotation 1" ) );
         if( pProp == NULL )
         {
            pProp = dynamic_cast<AnnotationLayerAdapter*>( pSdview->createLayer( ANNOTATION, NULL, "Annotation 1" ) );
         }
      }
      issea( pProp != NULL );

      GraphicObjectImp *pObj = NULL;
      GraphicObjectImp *pObj2 = NULL;
      if( success )
      {
         pObj = dynamic_cast<GraphicObjectImp*>( pProp->addObject( RECTANGLE_OBJECT ) );
      }
      issea( pObj != NULL );

      issea( pObj->setBoundingBox( LocationType( 0.0, 0.0 ), LocationType( XSIZE,YSIZE ) ) );
      if( success )
      {
         pObj->setLineState( false );
      }
      if( success )
      {
         pObj2 = dynamic_cast<GraphicObjectImp*>( pProp->addObject( RECTANGLE_OBJECT ) );
      }
      issea( pObj2 != NULL );
      issea( pObj2->setBoundingBox( LocationType( 1000.0, 1000.0 ), LocationType( 2000.0, 2000.0 ) ) );

      const double XCENTER = XSIZE / 2.0;
      const double YCENTER = YSIZE / 2.0;
      issea( pProp->selectObjects( LocationType( -0.5, -0.5 ), LocationType( XSIZE + 0.5, YSIZE + 0.5 ) ) == 1 );
      issea( pProp->selectObjects( LocationType( 0.5, -0.5 ), LocationType( XSIZE + 0.5, YSIZE + 0.5 ) ) == 0 );
      issea( pProp->selectObjects( LocationType( -0.5, -0.5 ), LocationType( XSIZE - 0.5, YSIZE + 0.5 ) ) == 0 );
      issea( pProp->selectObjects( LocationType( -0.5, 0.5 ), LocationType( XSIZE + 0.5, YSIZE + 0.5 ) ) == 0 );
      issea( pProp->selectObjects( LocationType( -0.5, -0.5 ), LocationType( XSIZE + 0.5, YSIZE - 0.5 ) ) == 0 );
      issea( pProp->selectObjects( LocationType( 999.5, 999.5 ), LocationType( 2000.5, 2000.5 ) ) == 1 );
      issea( pProp->selectObjects( LocationType( 1000.5, 999.5 ), LocationType( 2000.5, 2000.5 ) ) == 0 );
      issea( pProp->selectObjects( LocationType( 999.5, 999.5 ), LocationType( 2000.5, 1999.5 ) ) == 0 );
      issea( pProp->selectObjects( LocationType( -0.5, -0.5 ), LocationType( 2000.5, 2000.5 ) ) == 2 );

      if( success )
      {
         pProp->deselectAllObjects();
      }
      issea( pObj->setRotation( 45.0 ) );
      LocationType corners[4];
      corners[0] = rotateAroundCenter( LocationType( 0.0, 0.0 ), LocationType( XCENTER, YCENTER ), 45.0 );
      corners[1] = rotateAroundCenter( LocationType( XSIZE, 0.0 ), LocationType( XCENTER, YCENTER ), 45.0 );
      corners[2] = rotateAroundCenter( LocationType( XSIZE, YSIZE ), LocationType( XCENTER, YCENTER ), 45.0 );
      corners[3] = rotateAroundCenter( LocationType( 0.0, YSIZE ), LocationType( XCENTER, YCENTER ), 45.0 );

      LocationType mins( 1e38, 1e38 );
      LocationType maxs( -1e38, -1e38 );
      for( int i = 0; i < 4; ++i )
      {
         mins.mX = min( mins.mX, corners[i].mX );
         maxs.mX = max( maxs.mX, corners[i].mX );
         mins.mY = min( mins.mY, corners[i].mY );
         maxs.mY = max( maxs.mY, corners[i].mY );
      }
      mins = mins - LocationType( 0.5, 0.5 );
      maxs = maxs + LocationType( 0.5, 0.5 );
      const double OMAG = 1.0;
      LocationType offsets[4] = {LocationType( OMAG, OMAG ), LocationType( -OMAG, OMAG ), LocationType( -OMAG, -OMAG ), LocationType( OMAG, -OMAG )};
      int minResults[4] = {0, 0, 1, 0};
      int maxResults[4] = {1, 0, 0, 0};
      if( success )
      {
         for( int i = 0; i < 4; ++i )
         {			
            success = success && tst_assert( pProp->selectObjects( mins + offsets[i], maxs ) == minResults[i] );
            success = success && tst_assert( pProp->selectObjects( mins, maxs + offsets[i] ) == maxResults[i] );
            if( !success )
            {
               printf( "%d\n", i );
            }
         }
      }

      issea( pProp->selectObjects( LocationType( 999.5, 999.5 ), LocationType( 2000.5, 2000.5 ) ) == 1 );
      issea( pProp->selectObjects( LocationType( 1000.5, 999.5 ), LocationType( 2000.5, 2000.5 ) ) == 0 );
      issea( pProp->selectObjects( LocationType( 999.5, 999.5 ), LocationType( 2000.5, 1999.5 ) ) == 0 );
      issea( pProp->selectObjects( LocationType( -50.0, -50.0 ), LocationType( 2000.5, 2000.5 ) ) == 2 );

      if( success )
      {
         pProp->removeObject( dynamic_cast<GraphicObject*>( pObj ), true );
      }
      if( success )
      {
         pProp->removeObject( dynamic_cast<GraphicObject*>( pObj2 ), true );
      }
      if( pSdview != NULL )
      {
         success = tst_assert( pSdview->deleteLayer( pProp ) ) && success;
      }
      
      return success;
   }
};

class AnnotationBigPolylineTestCase : public TestCase
{
public:
   AnnotationBigPolylineTestCase() : TestCase("BigPolyline") {}
   bool run()
   {
      bool success = true;
      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      success = success && tst_assert( pRasterElement != NULL );

      SpatialDataWindow* pSdwd = dynamic_cast<SpatialDataWindow*>(
         Service<DesktopServices>()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea( pSdwd != NULL );

      SpatialDataView* pSdview = pSdwd->getSpatialDataView();
      issea( pSdview != NULL );

      AnnotationLayerAdapter *pProp = NULL;
      if( success )
      {
         LayerList *pLayerList = NULL;
         pLayerList = pSdview->getLayerList();
         issea( pLayerList != NULL );

         pProp = dynamic_cast<AnnotationLayerAdapter*>( pLayerList->getLayer( ANNOTATION, NULL, "Annotation 1" ) );
         if( pProp == NULL )
         {
            pProp = dynamic_cast<AnnotationLayerAdapter*>( pSdview->createLayer( ANNOTATION, NULL, "Annotation 1" ) );
         }
      }
      issea( pProp != NULL );

      vector<LocationType> vertices;
      for( int i = 0; i < 4096; ++i )
      {
         vertices.push_back( LocationType( i, i * i ) );
      }

      PolylineObject *pObj = dynamic_cast<PolylineObject*>( pProp->addObject( POLYLINE_OBJECT ) );
      issea( pObj != NULL );
      unsigned int startTime = time( NULL );
      if( success )
      {
         issea( pObj->addVertices( vertices ) );
      }
      unsigned int stopTime = time( NULL );
      issea( stopTime - startTime < 10 ); // must happen in < 10 sec

      if( pSdview != NULL )
      {
         success = tst_assert( pSdview->deleteLayer( pProp ) ) && success;
      }

      return success;
   }
};

class AnnotationArcTestCase : public TestCase
{
public:
   AnnotationArcTestCase() : TestCase("Arc") {}
   bool run()
   {
      bool success = true;
      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      success = success && tst_assert( pRasterElement!= NULL );

      SpatialDataWindow* pSdwd = dynamic_cast<SpatialDataWindow*>(
         Service<DesktopServices>()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea( pSdwd != NULL );

      SpatialDataView* pSdview = pSdwd->getSpatialDataView();
      issea( pSdview != NULL );

      AnnotationLayerAdapter *pProp = NULL;
      if( success )
      {
         LayerList *pLayerList = NULL;
         pLayerList = pSdview->getLayerList();
         issea( pLayerList != NULL );

         pProp = dynamic_cast<AnnotationLayerAdapter*>( pLayerList->getLayer( ANNOTATION, NULL, "Annotation 1" ) );
         if( pProp == NULL )
         {
            pProp = dynamic_cast<AnnotationLayerAdapter*>( pSdview->createLayer( ANNOTATION, NULL, "Annotation 1" ) );
         }
      }
      issea( pProp != NULL );

      ArcObject *pObj = dynamic_cast<ArcObject*>( pProp->addObject( ARC_OBJECT ) );
      issea( pObj != NULL );
      pObj->setBoundingBox( LocationType( 10, 10 ), LocationType( 20, 30 ) );
      pObj->setStartAngle( 46 );
      pObj->setStopAngle( 46 );
      QCoreApplication::instance()->processEvents(); // force draw to happen

      if( pSdview != NULL )
      {
         success = tst_assert( pSdview->deleteLayer( pProp ) ) && success;
      }

      return success;
   }
};

class AnnotationSerializeDeserializeObjectTest : public TestCase
{
public:
   AnnotationSerializeDeserializeObjectTest() : TestCase("SerializeObject") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(
         Service<DesktopServices>()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea( pWindow != NULL );

      SpatialDataView* pView = pWindow->getSpatialDataView();
      issea( pView != NULL );

      AnnotationLayer *pAnnotationLayer = NULL;
      if( success )
      {
         LayerList *pLayerList = NULL;
         pLayerList = pView->getLayerList();
         issea( pLayerList != NULL );

         pAnnotationLayer = dynamic_cast<AnnotationLayerAdapter*>( pLayerList->getLayer( ANNOTATION, NULL, "Annotation 1" ) );
         if( pAnnotationLayer == NULL )
         {
            pAnnotationLayer = dynamic_cast<AnnotationLayerAdapter*>( pView->createLayer( ANNOTATION, NULL, "Annotation 1" ) );
         }
      }
      issea( pAnnotationLayer != NULL );

      GraphicObjectImp *pEllipse = NULL;
      const BitMask *pEllipseBitmask = NULL;
      pEllipse = dynamic_cast<GraphicObjectImp*>( pAnnotationLayer->addObject( ELLIPSE_OBJECT ) );
      issea( pEllipse != NULL );

      pEllipse->setBoundingBox( LocationType( 35, 75 ), LocationType( 80, 100 ) );
      pEllipse->setFillColor( ColorType( 0, 0, 255 ) );
      pEllipse->setFillStyle( HATCH );
      pEllipse->setHatchStyle( ASTERISK );
      pEllipseBitmask = pEllipse->getPixels();
      issea( pEllipseBitmask != NULL );

      GraphicObjectImp *pTriangle = NULL;
      const BitMask *pTriangleBitmask = NULL;
      pTriangle = dynamic_cast<GraphicObjectImp*>( pAnnotationLayer->addObject( TRIANGLE_OBJECT ) );
      issea( pTriangle != NULL );

      pTriangle->setBoundingBox( LocationType( 70, 30 ), LocationType( 90, 50 ) );
      pTriangle->setFillColor( ColorType( 0, 255, 255 ) );
      pTriangle->setFillStyle( SOLID_FILL );
      pTriangleBitmask = pTriangle->getPixels();
      issea( pTriangleBitmask != NULL );

      QCoreApplication::instance()->processEvents(); // force draw to happen

      bool ok = false;
      // serialize the ellipse object
      FILE *pOutputFileEllipse = NULL;
      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }

      pOutputFileEllipse = fopen( ( tempHome + "/ellipse.ano" ).c_str(), "w" );
      issea( pOutputFileEllipse != NULL );

      XMLWriter xwrite( "AnnotationElement" );
      ok = pEllipse->toXml( &xwrite );
      issea( ok == true );
      xwrite.writeToFile( pOutputFileEllipse );
      fclose( pOutputFileEllipse );
      issea( ok == true );

      // serialize the triangle object
      FILE *pOutputFileTriangle = NULL;

      pOutputFileTriangle = fopen( ( tempHome + "/triangle.ano" ).c_str(), "w" );
      issea( pOutputFileTriangle != NULL );

      XMLWriter xwrite2( "AnnotationElement" );
      ok = pTriangle->toXml( &xwrite2 );
      issea( ok == true );
      xwrite2.writeToFile( pOutputFileTriangle );
      fclose( pOutputFileTriangle );
      issea( ok == true );

      GraphicObjectImp *pEllipseCopy = NULL;
      pEllipseCopy = dynamic_cast<GraphicObjectImp*>( pAnnotationLayer->addObject( ELLIPSE_OBJECT ) );
      issea( pEllipseCopy != NULL );

      MessageLog *pLog( UtilityServicesImp::instance()->getMessageLog()->getLog( "session" ) );
      XmlReader xml( pLog, false );

      FilenameImp ellipseFile( tempHome + "/ellipse.ano" );

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *ellipseDoc( NULL );
      try
      {
         ellipseDoc = xml.parse( &ellipseFile );
      }
      catch( XmlBase::XmlException & )
      {
         // do nothing
      }
      DOMElement *rootelementEllipse( NULL );
      if( ellipseDoc != NULL )
      {
         rootelementEllipse = ellipseDoc->getDocumentElement();
      }

      // deserialize the ellipse object
      unsigned int formatVersion = atoi( A( rootelementEllipse->getAttribute( X( "version" ) ) ) );
      ok = pEllipseCopy->fromXml( rootelementEllipse, formatVersion );
      issea( ok == true );

      // verify that the contents of the deserialized ellipse match the original
      GraphicObjectType objectType = pEllipseCopy->getGraphicObjectType();
      issea( objectType == ELLIPSE_OBJECT );
      const BitMask *pEllipseCopyBitmask = NULL;
      pEllipseCopyBitmask = pEllipseCopy->getPixels();
      issea( pEllipseCopyBitmask != NULL );
      issea( pEllipseBitmask->compare( *pEllipseCopyBitmask ) == true );
      LocationType LlCorner = pEllipseCopy->getLlCorner();
      LocationType UrCorner = pEllipseCopy->getUrCorner();
      issea( LocationType( 35, 75 ) == LlCorner && LocationType( 80, 100 ) == UrCorner );
      ColorType pFillColor = pEllipseCopy->getFillColor();
      issea( pFillColor == ColorType( 0, 0, 255 ) );
      issea( pEllipseCopy->getFillStyle() == HATCH );
      issea( pEllipseCopy->getHatchStyle() == ASTERISK );

      GraphicObjectImp *pTriangleCopy = NULL;
      pTriangleCopy = dynamic_cast<GraphicObjectImp*>( pAnnotationLayer->addObject( TRIANGLE_OBJECT ) );
      issea( pTriangleCopy != NULL );

      FilenameImp triangleFile( tempHome + "/triangle.ano" );

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *triangleDoc( NULL );
      try
      {
         triangleDoc = xml.parse( &triangleFile );
      }
      catch( XmlBase::XmlException & )
      {
         // do nothing
      }
      DOMElement *rootelementTriangle( NULL );
      if( triangleDoc != NULL )
      {
         rootelementTriangle = triangleDoc->getDocumentElement();
      }

      // deserialize the triangle object
      formatVersion = atoi( A( rootelementTriangle->getAttribute( X( "version" ) ) ) );
      ok = pTriangleCopy->fromXml( rootelementTriangle, formatVersion );
      issea( ok == true );

      // verify that the contents of the deserialized triangle match the original
      objectType = pTriangleCopy->getGraphicObjectType();
      issea( objectType == TRIANGLE_OBJECT );
      const BitMask *pTriangleCopyBitmask = NULL;
      pTriangleCopyBitmask = pTriangleCopy->getPixels();
      issea( pTriangleCopyBitmask != NULL );
      issea( pTriangleBitmask->compare( *pTriangleCopyBitmask ) == true );
      LlCorner = pTriangleCopy->getLlCorner();
      UrCorner = pTriangleCopy->getUrCorner();
      issea( LocationType( 70, 30 ) == LlCorner && LocationType( 90, 50 ) == UrCorner );
      ColorType pTriangleFillColor = pTriangleCopy->getFillColor();
      issea( pTriangleFillColor == ColorType( 0, 255, 255 ) );
      issea( pTriangleCopy->getFillStyle() == SOLID_FILL );

      // clean up
      if( pAnnotationLayer != NULL )
      {
         success = tst_assert( pAnnotationLayer->removeObject( dynamic_cast<GraphicObject*>( pEllipse ), true ) ) && success;
         success = tst_assert( pAnnotationLayer->removeObject( dynamic_cast<GraphicObject*>( pEllipseCopy ), true ) ) && success;
         success = tst_assert( pAnnotationLayer->removeObject( dynamic_cast<GraphicObject*>( pTriangle ), true ) ) && success;
         success = tst_assert( pAnnotationLayer->removeObject( dynamic_cast<GraphicObject*>( pTriangleCopy ), true ) ) && success;
      }
      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pAnnotationLayer ) ) && success;
      }

      return success;
   }
};

class AnnotationSerializeDeserializeLayerTest : public TestCase
{
public:
   AnnotationSerializeDeserializeLayerTest() : TestCase("SerializeLayer") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(
         Service<DesktopServices>()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea( pWindow != NULL );

      SpatialDataView* pView = pWindow->getSpatialDataView();
      issea( pView != NULL );

      AnnotationLayer *pAnnotationLayer = NULL;
      if( success )
      {
         LayerList *pLayerList = NULL;
         pLayerList = pView->getLayerList();
         issea( pLayerList != NULL );

         pAnnotationLayer = dynamic_cast<AnnotationLayerAdapter*>( pLayerList->getLayer( ANNOTATION, NULL, "Annotation 1" ) );
         if( pAnnotationLayer == NULL )
         {
            pAnnotationLayer = dynamic_cast<AnnotationLayerAdapter*>( pView->createLayer( ANNOTATION, NULL, "Annotation 1" ) );
         }
      }
      issea( pAnnotationLayer != NULL );

      GraphicObject *pText = NULL;
      pText = pAnnotationLayer->addObject( TEXT_OBJECT );
      issea( pText != NULL );

      pText->setBoundingBox( LocationType( 25, 75 ), LocationType( 80, 100 ) );
      pText->setTextColor( ColorType( 0, 0, 255 ) );
      pText->setFillColor( ColorType( 128, 0, 255 ) );
      pText->setText( "Testable Text Object" );

      FactoryResource<Font> pFont;
      pFont->setPointSize(8);
      pFont->setItalic(true);

      pText->setFont(pFont.get());

      QCoreApplication::instance()->processEvents(); // force draw to happen
      LocationType LlCorner = pText->getLlCorner(); // these may have been updated after the draw
      LocationType UrCorner = pText->getUrCorner();

      bool ok = false;
      // serialize the annotation layer
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

      pOutputFile = fopen( ( tempHome + "/annontationLayer.anolayer" ).c_str(), "w" );
      issea( pOutputFile != NULL );

      XMLWriter xwrite( "AnnotationLayer" );
      ok = pAnnotationLayer->toXml( &xwrite );
      issea( ok == true );
      xwrite.writeToFile( pOutputFile );
      fclose( pOutputFile );
      issea( ok == true );

      ModelServicesImp::instance()->setElementName(pAnnotationLayer->getDataElement(), "AnnotationSerialized");
      dynamic_cast<AnnotationLayerImp*>(pAnnotationLayer)->setName("AnnotationSerialized");

      // verify the contents of the deserialized layer
      AnnotationLayerAdapter *pAnnotationLayerCopy = NULL;
      pAnnotationLayerCopy = dynamic_cast<AnnotationLayerAdapter*>( pView->createLayer( ANNOTATION, NULL, "Annotation 1" ) );
      issea( pAnnotationLayerCopy != NULL );

      MessageLog *pLog( UtilityServicesImp::instance()->getMessageLog()->getLog( "session" ) );
      XmlReader xml( pLog );

      FilenameImp annotationFile( tempHome + "/annontationLayer.anolayer" );

      XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *annotationDoc( NULL );
      try
      {
         annotationDoc = xml.parse( &annotationFile );
      }
      catch( XmlBase::XmlException & )
      {
         // do nothing
      }
      DOMElement *rootelement( NULL );
      if( annotationDoc != NULL )
      {
         rootelement = annotationDoc->getDocumentElement();
      }
      issea( rootelement != NULL );

      list<GraphicObject*> objectList;
      if( rootelement != NULL )
      {
         // deserialize the layer
         unsigned int formatVersion = atoi( A( rootelement->getAttribute( X( "version" ) ) ) );
         try
         {
            ok = pAnnotationLayerCopy->fromXml( rootelement, formatVersion );
         }
         catch (...)
         {
            ok = false;
         }
         issea( ok == true );

         issea( pAnnotationLayerCopy->getLayerType() == pAnnotationLayer->getLayerType() );
         issea( pAnnotationLayerCopy->getNumObjects() == pAnnotationLayer->getNumObjects() ); // should be 1 each

         pAnnotationLayerCopy->getObjects( objectList );
         issea( objectList.size() == 1 );
      }

      if( success )
      {
         GraphicObjectType objectType = objectList.front()->getGraphicObjectType();
         issea( objectType == TEXT_OBJECT );
         LocationType LlCornerCopy = objectList.front()->getLlCorner();
         LocationType UrCornerCopy = objectList.front()->getUrCorner();
         issea( fabs( LlCorner.mX - LlCornerCopy.mX ) < 1e-8 &&  // check to see if within 8 decimal places
            fabs( LlCorner.mY - LlCornerCopy.mY ) < 1e-8 && 
            fabs( UrCorner.mX - UrCornerCopy.mX ) < 1e-8 && 
            fabs( UrCorner.mY - UrCornerCopy.mY ) < 1e-8 );
         ColorType pFillColor = objectList.front()->getFillColor();
         issea( pFillColor == ColorType( 128, 0, 255 ) );
         ColorType pTextColor = objectList.front()->getTextColor();
         issea( pTextColor == ColorType( 0, 0, 255 ) );
         issea( objectList.front()->getText() == "Testable Text Object");

         const Font* pFont = objectList.front()->getFont();
         issea(pFont != NULL);
         issea(pFont->getPointSize() == 8);
         issea(pFont->getItalic() == true);
      }

      // clean up
      if( pAnnotationLayer != NULL )
      {
         success = tst_assert( pAnnotationLayer->removeObject( pText, true ) ) && success;
      }
      if( pAnnotationLayerCopy != NULL )
      {
         success = tst_assert( pAnnotationLayerCopy->removeObject( objectList.front(), true ) ) && success;
      }
      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pAnnotationLayer ) ) && success;
         success = tst_assert( pView->deleteLayer( pAnnotationLayerCopy ) ) && success;
      }

      return success;
   }
};

class AnnotationRenameLayerTest : public TestCase
{
public:
   AnnotationRenameLayerTest() : TestCase("RenameLayer") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement(true);
      issea( pRasterElement != NULL );

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(
         Service<DesktopServices>()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea( pWindow != NULL );

      SpatialDataView* pView = pWindow->getSpatialDataView();
      issea( pView != NULL );

      AnnotationLayer *pAnnotationLayer = NULL;      
      pAnnotationLayer = dynamic_cast<AnnotationLayer*>( pView->createLayer( ANNOTATION, NULL, "AnnotationLayer" ) );
      issea( pAnnotationLayer != NULL );

      GraphicObject *pEllipse = NULL;
      const BitMask *pEllipseBitmask = NULL;
      pEllipse = pAnnotationLayer->addObject( ELLIPSE_OBJECT );
      issea( pEllipse != NULL );

      pEllipse->setBoundingBox( LocationType( 35, 75 ), LocationType( 80, 100 ) );
      pEllipse->setFillColor( ColorType( 0, 0, 255 ) );
      pEllipse->setFillStyle( HATCH );
      pEllipse->setHatchStyle( ASTERISK );
      pEllipseBitmask = pEllipse->getPixels();
      issea( pEllipseBitmask != NULL );

      GraphicObject *pTriangle = NULL;
      const BitMask *pTriangleBitmask = NULL;
      pTriangle = pAnnotationLayer->addObject( TRIANGLE_OBJECT );
      issea( pTriangle != NULL );

      pTriangle->setBoundingBox( LocationType( 70, 30 ), LocationType( 90, 50 ) );
      pTriangle->setFillColor( ColorType( 0, 255, 255 ) );
      pTriangle->setFillStyle( SOLID_FILL );
      pTriangleBitmask = pTriangle->getPixels();
      issea( pTriangleBitmask != NULL );

      unsigned int numLayers = 0;
      string name = "";
      vector<Layer*> layerVector;
      LayerList *pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );
      numLayers = pList->getNumLayers();
      issea( numLayers == 2 ); // raster layer, AnnotationLayer

      pList->getLayers( ANNOTATION, layerVector );
      issea( layerVector.size() == 1 );
      name = layerVector.at(0)->getName();
      issea( name == "AnnotationLayer" );
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea( name == pRasterElement->getName() );

      // rename the layer
      layerVector.clear();
      dynamic_cast<AnnotationLayerImp*>(pAnnotationLayer)->setName("AnnotationRenamedLayer");
      pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );
      numLayers = pList->getNumLayers();
      issea( numLayers == 2 ); // raster layer, AnnotationRenamedLayer

      pList->getLayers( ANNOTATION, layerVector );
      issea( layerVector.size() == 1 );
      name = layerVector.at(0)->getName();
      issea( name == "AnnotationRenamedLayer" );
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea( name == pRasterElement->getName() );
      QCoreApplication::instance()->processEvents();

      if( pAnnotationLayer != NULL )
      {
         success = tst_assert( pAnnotationLayer->removeObject( pEllipse, true ) ) && success;
         success = tst_assert( pAnnotationLayer->removeObject( pTriangle, true ) ) && success;
      }
      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pAnnotationLayer ) ) && success;
      }

      return success;
   }
};

class AnnotationRenameLayerTest2 : public TestCase
{
public:
   AnnotationRenameLayerTest2() : TestCase("RenameLayer2") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement(true);
      issea( pRasterElement != NULL );

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(
         Service<DesktopServices>()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea( pWindow != NULL );

      SpatialDataView* pView = pWindow->getSpatialDataView();
      issea( pView != NULL );

      AnnotationLayer *pAnnotationLayer = NULL;      
      pAnnotationLayer = dynamic_cast<AnnotationLayer*>( pView->createLayer( ANNOTATION, NULL, "AnnotationLayer" ) );
      issea( pAnnotationLayer != NULL );

      GraphicObject *pEllipse = NULL;
      const BitMask *pEllipseBitmask = NULL;
      pEllipse = pAnnotationLayer->addObject( ELLIPSE_OBJECT );
      issea( pEllipse != NULL );

      pEllipse->setBoundingBox( LocationType( 35, 75 ), LocationType( 80, 100 ) );
      pEllipse->setFillColor( ColorType( 0, 0, 255 ) );
      pEllipse->setFillStyle( HATCH );
      pEllipse->setHatchStyle( ASTERISK );
      pEllipseBitmask = pEllipse->getPixels();
      issea( pEllipseBitmask != NULL );

      AnnotationLayer *pAnnotationLayer2 = NULL;      
      pAnnotationLayer2 = dynamic_cast<AnnotationLayer*>( pView->createLayer( ANNOTATION, NULL, "AnnotationLayer2" ) );
      issea( pAnnotationLayer2 != NULL );

      GraphicObject *pTriangle = NULL;
      const BitMask *pTriangleBitmask = NULL;
      pTriangle = pAnnotationLayer2->addObject( TRIANGLE_OBJECT );
      issea( pTriangle != NULL );

      pTriangle->setBoundingBox( LocationType( 70, 30 ), LocationType( 90, 50 ) );
      pTriangle->setFillColor( ColorType( 0, 255, 255 ) );
      pTriangle->setFillStyle( SOLID_FILL );
      pTriangleBitmask = pTriangle->getPixels();
      issea( pTriangleBitmask != NULL );

      unsigned int numLayers = 0;
      string name = "";
      vector<Layer*> layerVector;
      LayerList *pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );
      numLayers = pList->getNumLayers();
      issea( numLayers == 3 ); // raster layer, AnnotationLayer, AnnotationLayer2

      pList->getLayers( ANNOTATION, layerVector );
      issea( layerVector.size() == 2 );
      name = layerVector.at(0)->getName();
      issea( name == "AnnotationLayer" );
      name = layerVector.at(1)->getName();
      issea( name == "AnnotationLayer2" );
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea( name == pRasterElement->getName() );

      // rename the layer
      layerVector.clear();
      dynamic_cast<AnnotationLayerImp*>(pAnnotationLayer)->setName("AnnotationRenamedLayer");
      pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );
      numLayers = pList->getNumLayers();
      issea( numLayers == 3 ); // raster layer, AnnotationRenamedLayer, AnnotationLayer2

      pList->getLayers( ANNOTATION, layerVector );
      issea( layerVector.size() == 2 );
      name = layerVector.at(0)->getName();
      issea( name == "AnnotationRenamedLayer" );
      name = layerVector.at(1)->getName();
      issea( name == "AnnotationLayer2" );
      name = pList->getLayer(RASTER, pRasterElement)->getName();
      issea( name == pRasterElement->getName() );
      QCoreApplication::instance()->processEvents();

      if( pAnnotationLayer != NULL )
      {
         success = tst_assert( pAnnotationLayer->removeObject( pEllipse, true ) ) && success;
      }
      if( pAnnotationLayer2 != NULL )
      {
         success = tst_assert( pAnnotationLayer2->removeObject( pTriangle, true ) ) && success;
      }
      if( pView != NULL )
      {
         success = tst_assert( pView->deleteLayer( pAnnotationLayer ) ) && success;
         success = tst_assert( pView->deleteLayer( pAnnotationLayer2 ) ) && success;
      }

      return success;
   }
};

class AnnotationCreateLayerTest : public TestCase
{
public:
   AnnotationCreateLayerTest() : TestCase("CreateLayer") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(
         Service<DesktopServices>()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issea( pWindow != NULL );

      SpatialDataView* pView = pWindow->getSpatialDataView();
      issea( pView != NULL );

      string name = "";

      // create an Annotation layer without a specified name
      AnnotationLayer* pAnnotationLayer1 = dynamic_cast<AnnotationLayer*>(pView->createLayer(ANNOTATION, NULL));
      issea(pAnnotationLayer1 != NULL);
      name = pAnnotationLayer1->getName();
      issea(name == "Annotation 1");

      // create an Annotation layer with a specified name
      AnnotationLayer* pAnnotationLayer2 = dynamic_cast<AnnotationLayer*>(pView->createLayer(ANNOTATION, NULL,
         "JustAnotherLayer"));
      issea(pAnnotationLayer2 != NULL);
      name = pAnnotationLayer2->getName();
      issea(name == "JustAnotherLayer");

      // create an Annotation layer without a specified name
      AnnotationLayer* pAnnotationLayer3 = dynamic_cast<AnnotationLayer*>(pView->createLayer(ANNOTATION, NULL));
      issea(pAnnotationLayer3 != NULL);
      name = pAnnotationLayer3->getName();
      issea(name == "Annotation 3");

      // create an Annotation layer with a specified name
      AnnotationLayer* pAnnotationLayer4 = dynamic_cast<AnnotationLayer*>(pView->createLayer(ANNOTATION, NULL,
         "YetAnotherLayer"));
      issea(pAnnotationLayer4 != NULL);
      name = pAnnotationLayer4->getName();
      issea(name == "YetAnotherLayer");

      // create an Annotation layer without a specified name
      AnnotationLayer* pAnnotationLayer5 = dynamic_cast<AnnotationLayer*>(pView->createLayer(ANNOTATION, NULL));
      issea(pAnnotationLayer5 != NULL);
      name = pAnnotationLayer5->getName();
      issea(name == "Annotation 5");

      // create an Annotation layer without a specified name
      AnnotationLayer* pAnnotationLayer6 = dynamic_cast<AnnotationLayer*>(pView->createLayer(ANNOTATION, NULL));
      issea(pAnnotationLayer6 != NULL);
      name = pAnnotationLayer6->getName();
      issea(name == "Annotation 6");

      // create an Annotation layer without a specified name
      AnnotationLayer* pAnnotationLayer7 = dynamic_cast<AnnotationLayer*>(pView->createLayer(ANNOTATION, NULL));
      issea(pAnnotationLayer7 != NULL);
      name = pAnnotationLayer7->getName();
      issea(name == "Annotation 7");

      if( pView != NULL )
      {
         success = tst_assert(pView->deleteLayer(pAnnotationLayer1)) && success;
         success = tst_assert(pView->deleteLayer(pAnnotationLayer2)) && success;
         success = tst_assert(pView->deleteLayer(pAnnotationLayer3)) && success;
         success = tst_assert(pView->deleteLayer(pAnnotationLayer4)) && success;
         success = tst_assert(pView->deleteLayer(pAnnotationLayer5)) && success;
         success = tst_assert(pView->deleteLayer(pAnnotationLayer6)) && success;
         success = tst_assert(pView->deleteLayer(pAnnotationLayer7)) && success;
      }

      return success;
   }
};

class AnnotationLoadEastAndNorthArrow : public TestCase
{
public:
   AnnotationLoadEastAndNorthArrow() : TestCase("LoadEastAndNorthArrow") {}
   bool run()
   {
      bool success = true;

      string fileName = TestUtilities::getTestDataPath() + "tipjul5bands.sio";

      SpatialDataWindow* pWindow = TestUtilities::loadDataSet(fileName, "Auto Importer");
      issea( pWindow != NULL );

      SpatialDataView* pView = pWindow->getSpatialDataView();
      issea( pView != NULL );

      RasterElement* pCube = dynamic_cast<RasterElement*>(pView->getTopMostElement("RasterElement"));
      issea( pCube != NULL );

      issea( TestUtilities::runGeoRef(pCube) );

      Service<ModelServices> pModel;
      DataElement* pAnnotation = pModel->createElement("TestAnnotation", "AnnotationElement", pCube);
      AnnotationLayer* pLayer = dynamic_cast<AnnotationLayer*>(pView->createLayer(ANNOTATION, pAnnotation));

      //note: if the add north arrow object fails, it will do so
      //by displaying a dialog which will hang the TestBed on the autobuild
      //machine.
      GraphicObject* pNorthArrow = pLayer->addObject(NORTHARROW_OBJECT);
      issea( pNorthArrow != NULL );

      GraphicObject* pEastArrow = pLayer->addObject(EASTARROW_OBJECT);
      issea( pEastArrow != NULL );

      issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      return success;
   }
};

class AnnotationTemplateTest : public TestCase
{
public:
   AnnotationTemplateTest() : TestCase("Template") {}
   bool run()
   {
      bool success = true;

      // Create a product window
      Service<DesktopServices> pDesktop;

      ProductWindow* pWindow = static_cast<ProductWindow*>(pDesktop->createWindow("TemplateProduct", PRODUCT_WINDOW));
      issea(pWindow != NULL);

      ProductView* pView = pWindow->getProductView();
      issea(pView != NULL);

      // Get the template directory
      string templatePath;
      const Filename* pTemplatePath = ProductView::getSettingTemplatePath();
      if (pTemplatePath != NULL)
      {
         templatePath = pTemplatePath->getFullPathAndName();
      }

      // For each template file in the directory, load and save the file and reload the saved file
      QDir templateDir(QString::fromStdString(templatePath));

      QStringList templateTypes;
      templateTypes.append("*.spg");

      QFileInfoList templateFiles = templateDir.entryInfoList(templateTypes, QDir::Files);
      for (int i = 0; i < templateFiles.count() && success == true; ++i)
      {
         QFileInfo templateFile = templateFiles[i];

         QString strTemplateFile = templateFile.absoluteFilePath();
         issea(strTemplateFile.isEmpty() == false);

         issea(pView->loadTemplate(strTemplateFile.toStdString()) == true);

         strTemplateFile = templateFile.absolutePath() + "/" + templateFile.completeBaseName() + "_Saved." +
            templateFile.suffix();
         issea(pView->saveTemplate(strTemplateFile.toStdString()) == true);
         issea(pView->loadTemplate(strTemplateFile.toStdString()) == true);

         // Delete the newly saved file
         issea(remove(strTemplateFile.toLatin1()) == 0);
      }

      // Delete the window
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class AnnotationGeoTestCase : public TestCase
{
public:
   AnnotationGeoTestCase() : TestCase("AnnotationGeo") {}
   bool run()
   {
      bool success = true;

      // Load the dataset and georeference it.
      const string filename = TestUtilities::getTestDataPath() + "GeoReference/nine_large-i2.ntf";
      const string georeferencePlugIn = "RPC Georeference";
      SpatialDataWindow* pWindow = TestUtilities::loadDataSet(filename, "Auto Importer");
      issearf(pWindow != NULL);

      SpatialDataView* pView = pWindow->getSpatialDataView();
      issearf(pView != NULL);

      LayerList* pLayerList = pView->getLayerList();
      issearf(pLayerList != NULL);

      RasterElement* pElement = pLayerList->getPrimaryRasterElement();
      issearf(pElement != NULL);
      issearf(TestUtilities::runGeoRef(pElement, georeferencePlugIn));

      // Create an annotation layer.
      AnnotationLayerAdapter* pAnnotationLayer =
         dynamic_cast<AnnotationLayerAdapter*>(pView->createLayer(ANNOTATION, NULL));
      issearf(pAnnotationLayer != NULL);

      GraphicGroup* pGroup = pAnnotationLayer->getGroup();
      issearf(pGroup != NULL);

      // Create a PolygonObject and move it.
      PolygonObject* pPolygon = dynamic_cast<PolygonObject*>(pGroup->addObject(POLYGON_OBJECT));
      issearf(pPolygon != NULL);

      // The vertices are arbitrary.
      vector<LocationType> setVertices(3);
      setVertices[0] = LocationType(1, 1);
      setVertices[1] = LocationType(4, 4);
      setVertices[2] = LocationType(1, 4);
      pPolygon->addVertices(setVertices);
      issearf(pPolygon->getVertices() == setVertices);

      // Move the polygon. Check that the vertices are moved correctly.
      LocationType oldLl = pPolygon->getLlCorner();
      LocationType newLl(2, 2);
      LocationType newUr(5, 5);
      pPolygon->setBoundingBox(newLl, newUr);
      const vector<LocationType>& getVertices = pPolygon->getVertices();
      issearf(getVertices.size() == setVertices.size());
      for (vector<LocationType>::size_type currVertex = 0; currVertex != getVertices.size(); ++currVertex)
      {
         issearf((getVertices[currVertex] - oldLl) == setVertices[currVertex]);
      }

      return success;
   }
};

class AnnotationTestSuite : public TestSuiteNewSession
{
public:
   AnnotationTestSuite() : TestSuiteNewSession( "Annotation" )
   {
      addTestCase( new AnnotationHitTestCase );
      addTestCase( new AnnotationBoxSelectTestCase );
      addTestCase( new AnnotationBigPolylineTestCase );
      addTestCase( new AnnotationArcTestCase );
      addTestCase( new AnnotationSerializeDeserializeObjectTest );
      addTestCase( new AnnotationSerializeDeserializeLayerTest );
      addTestCase( new AnnotationRenameLayerTest );
      addTestCase( new AnnotationRenameLayerTest2 );
      addTestCase( new AnnotationCreateLayerTest );
      addTestCase( new AnnotationLoadEastAndNorthArrow );
      addTestCase( new AnnotationTemplateTest );
      addTestCase( new AnnotationGeoTestCase );
   }
};

REGISTER_SUITE( AnnotationTestSuite )
