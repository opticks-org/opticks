/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnnotationLayerAdapter.h"
#include "ApplicationServices.h"
#include "GraphicObjectImp.h"
#include "AoiElement.h"
#include "AoiLayer.h"
#include "assert.h"
#include "DesktopServicesImp.h"
#include "DimensionDescriptor.h"
#include "DockWindow.h"
#include "GcpLayer.h"
#include "GcpList.h"
#include "HighResolutionTimer.h"
#include "LayerList.h"
#include "MessageLog.h"
#include "MessageLogMgr.h"
#include "ModelServicesImp.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PseudocolorLayer.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataWindowAdapter.h"
#include "SpatialDataViewAdapter.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "UtilityServicesImp.h"
using namespace std;

class ThousandAnnotationPerformanceTestCase : public TestCase
{
public:
   ThousandAnnotationPerformanceTestCase() : TestCase("ThousandAnnotation") {}
   bool run()
   {
      bool success = true;
      bool ok = false;
      int count = 0;
      double timeToRun = 0.0;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataViewAdapter *pView = NULL;
      pView = dynamic_cast<SpatialDataViewAdapter*>( pWindow->getView() );
      issea( pView != NULL );

      AnnotationLayer *pAnnotationLayer = NULL;
      pAnnotationLayer = dynamic_cast<AnnotationLayerAdapter*>( pView->createLayer( ANNOTATION, NULL, "AnnotationTest" ) );
      issea( pAnnotationLayer != NULL );

      GraphicObject *pObj = NULL;

      {
         HrTimer::Resource timer( &timeToRun, false );
         // create 1000 Annotation objects
         for( count = 0; count < 1000; count++ )
         {
            pObj = static_cast<GraphicObject*>( pAnnotationLayer->addObject( RECTANGLE_OBJECT ) );
            issea( pObj != NULL );

            // set the bounding box to random pixels within the scene
            ok = pObj->setBoundingBox( LocationType( ( rand() / ( RAND_MAX / 97 + 1 ) ), ( rand() / ( RAND_MAX / 181 + 1 ) ) ),
               LocationType( ( rand() / ( RAND_MAX / 97 + 1 ) ), ( rand() / ( RAND_MAX / 181 + 1 ) ) ) );
            issea( ok == true );
         }
      }
      printf( "Creating 1000 Graphic Objects takes %f seconds.\n", timeToRun );

      {
         HrTimer::Resource timer( &timeToRun, false );
         issea( pView->deleteLayer( pAnnotationLayer ) );
      }
      printf( "Deleting a layer with 1000 Graphic Objects takes %f seconds.\n", timeToRun );
      
      return success;
   }
};

class TenThousandAnnotationPerformanceTestCase : public TestCase
{
public:
   TenThousandAnnotationPerformanceTestCase() : TestCase("TenThousandAnnotation") {}
   bool run()
   {
      bool success = true;
      bool ok = false;
      int count = 0;
      double timeToRun = 0.0;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataViewAdapter *pView = NULL;
      pView = dynamic_cast<SpatialDataViewAdapter*>( pWindow->getView() );
      issea( pView != NULL );

      AnnotationLayer *pAnnotationLayer = NULL;
      pAnnotationLayer = dynamic_cast<AnnotationLayerAdapter*>( pView->createLayer( ANNOTATION, NULL, "AnnotationTest" ) );
      issea( pAnnotationLayer != NULL );

      GraphicObject *pObj = NULL;

      {
         HrTimer::Resource timer( &timeToRun, false );
         // create 10000 Annotation objects
         for( count = 0; count < 10000; count++ )
         {
            pObj = static_cast<GraphicObject*>( pAnnotationLayer->addObject( RECTANGLE_OBJECT ) );
            issea( pObj != NULL );

            // set the bounding box to random pixels within the scene
            ok = pObj->setBoundingBox( LocationType( ( rand() / ( RAND_MAX / 97 + 1 ) ), ( rand() / ( RAND_MAX / 181 + 1 ) ) ),
               LocationType( ( rand() / ( RAND_MAX / 97 + 1 ) ), ( rand() / ( RAND_MAX / 181 + 1 ) ) ) );
            issea( ok == true );
         }
      }
      printf( "Creating 10000 Graphic Objects takes %f seconds.\n", timeToRun );

      {
         HrTimer::Resource timer( &timeToRun, false );
         issea( pView->deleteLayer( pAnnotationLayer ) );
      }
      printf( "Deleting a layer with 10000 Graphic Objects takes %f seconds.\n", timeToRun );
      
      return success;
   }
};

class OneThousandAoiPerformanceTest : public TestCase
{
public:
   OneThousandAoiPerformanceTest() : TestCase("OneThousandAoi") {}
   bool run()
   {
      bool success = true;
      double timeToExecute = 0.0;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      AoiElement* pAoi = NULL;
      char aoiName[10];
      vector<AoiLayer*> layerVector;
      {
         HrTimer::Resource timer( &timeToExecute, false );
         for( int count = 0; count < 1000; count++ )
         {
            sprintf( aoiName, "myAOI%d", count + 1 );

            pAoi = static_cast<AoiElement*>( ModelServicesImp::instance()->createElement( aoiName, "AoiElement", pRasterElement ) );
            issea( pAoi != NULL );

            GraphicObject *pRect = pAoi->getGroup()->addObject( RECTANGLE_OBJECT );
            pRect->setBoundingBox(LocationType( ( rand() / ( RAND_MAX / 97 + 1 ) ), ( rand() / ( RAND_MAX / 181 + 1 ) )),
               LocationType( ( rand() / ( RAND_MAX / 97 + 1 ) ), ( rand() / ( RAND_MAX / 181 + 1 ) ) ) );

            AoiLayer* pProperties = NULL;
            pProperties = dynamic_cast<AoiLayer*>( pView->createLayer( AOI_LAYER, pAoi, "AoiName" ) );
            issea( pProperties != NULL );
            layerVector.push_back( pProperties );
         }
      }
      printf( "Time taken to create 1000 AOIs is %f seconds.\n", timeToExecute );

      for( int count = 0; count < 1000; count++ )
      {
         HrTimer::Resource timer( &timeToExecute, false );
         pView->deleteLayer( layerVector.at( count ) );
      }
      printf( "Time taken to delete 1000 AOIs is %f seconds.\n", timeToExecute );

      return success;
   }
};

class GcpAddRandomPointsTest : public TestCase
{
public:
   GcpAddRandomPointsTest() : TestCase("GcpAddRandomPoints") {}
   bool run()
   {
      bool success = true;

      double timeFor50 = 0.0;
      double timeFor100 = 0.0;
      double timeFor500 = 0.0;
      double timeFor1000 = 0.0;
      double timeFor10000 = 0.0;
      double timeFor100000 = 0.0;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "HugeGcpList", "GcpList", pRasterElement ) );
      issea( pGcpList != NULL );

      list<GcpPoint> gcpListPoints;

      // try to add 50 random GCPs to the list
      for( int i = 0; i < 50; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand() / ( RAND_MAX / 97 + 1 ); // rand between 0 and numRows ??
         newPoint.mPixel.mY = rand() / ( RAND_MAX / 181 + 1 ); // rand between 0 and numCols ??
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }

      GcpLayer* pProperties = NULL;
      // how long does it take to create the layer?
      {
         HrTimer::Resource timer( &timeFor50, false );
         pGcpList->addPoints( gcpListPoints );

         pProperties = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestGcpLayer" ) );
         issea( pProperties != NULL );
      }

      pGcpList->clearPoints();
      gcpListPoints.clear();

      // try to add 100 random GCPs to the list
      for( int i = 0; i < 100; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand() / ( RAND_MAX / 97 + 1 ); // rand between 0 and numRows ??
         newPoint.mPixel.mY = rand() / ( RAND_MAX / 181 + 1 ); // rand between 0 and numCols ??
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }

      {
         HrTimer::Resource timer( &timeFor100, false );
         pGcpList->addPoints( gcpListPoints );
      }

      pGcpList->clearPoints();
      gcpListPoints.clear();

      // try to add 500 random GCPs to the list
      for( int i = 0; i < 500; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand() / ( RAND_MAX / 97 + 1 ); // rand between 0 and numRows ??
         newPoint.mPixel.mY = rand() / ( RAND_MAX / 181 + 1 ); // rand between 0 and numCols ??
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }

      {
         HrTimer::Resource timer( &timeFor500, false );
         pGcpList->addPoints( gcpListPoints );
      }

      pGcpList->clearPoints();
      gcpListPoints.clear();

      // try to add 1000 random GCPs to the list
      for( int i = 0; i < 1000; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand() / ( RAND_MAX / 97 + 1 ); // rand between 0 and numRows ??
         newPoint.mPixel.mY = rand() / ( RAND_MAX / 181 + 1 ); // rand between 0 and numCols ??
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }

      {
         HrTimer::Resource timer( &timeFor1000, false );
         pGcpList->addPoints( gcpListPoints );
      }

      pGcpList->clearPoints();
      gcpListPoints.clear();

      // try to add 10000 random GCPs to the list
      for( int i = 0; i < 10000; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand() / ( RAND_MAX / 97 + 1 ); // rand between 0 and numRows ??
         newPoint.mPixel.mY = rand() / ( RAND_MAX / 181 + 1 ); // rand between 0 and numCols ??
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }

      {
         HrTimer::Resource timer( &timeFor10000, false );
         pGcpList->addPoints( gcpListPoints );
      }

      pGcpList->clearPoints();
      gcpListPoints.clear();

      // try to add 100000 random GCPs to the list
      for( int i = 0; i < 100000; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand() / ( RAND_MAX / 97 + 1 ); // rand between 0 and numRows ??
         newPoint.mPixel.mY = rand() / ( RAND_MAX / 181 + 1 ); // rand between 0 and numCols ??
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }

      {
         HrTimer::Resource timer( &timeFor100000, false );
         pGcpList->addPoints( gcpListPoints );
      }

      printf( "Results: %f seconds to add 50 points, %f seconds to add 100 points,\n", timeFor50, timeFor100 );
      printf( "%f seconds to add 500 points, %f seconds to add 1000 points,\n", timeFor500, timeFor1000 );
      printf( "%f seconds to add 10000 points, %f seconds to add 100000 points,\n", timeFor10000, timeFor100000 );

      issea( pView->deleteLayer( pProperties ) == true );

      return success;
   }
};

class GcpAddRandomPointsTest2 : public TestCase
{
public:
   GcpAddRandomPointsTest2() : TestCase("GcpAddRandomPoints2") {}
   bool run()
   {
      bool success = true;

      double timeFor50 = 0.0;
      double timeFor100 = 0.0;
      double timeFor500 = 0.0;
      double timeFor1000 = 0.0;
      double timeFor10000 = 0.0;
      double timeFor100000 = 0.0;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement( "HugeGcpList", "GcpList", pRasterElement ) );
      issea( pGcpList != NULL );

      list<GcpPoint> gcpListPoints;

      // try to add 50 random GCPs to the list
      for( int i = 0; i < 50; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand();
         newPoint.mPixel.mY = rand();
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }

      GcpLayer* pProperties = NULL;
      // how long does it take to create the layer?
      {
         HrTimer::Resource timer( &timeFor50, false );
         pGcpList->addPoints( gcpListPoints );

         pProperties = ( GcpLayer* )pView->createLayer( GCP_LAYER, pGcpList, "TestGcpLayer" );
         issea( pProperties != NULL );
      }

      pGcpList->clearPoints();
      gcpListPoints.clear();

      // try to add 100 random GCPs to the list
      for( int i = 0; i < 100; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand();
         newPoint.mPixel.mY = rand();
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }

      {
         HrTimer::Resource timer( &timeFor100, false );
         pGcpList->addPoints( gcpListPoints );
      }

      pGcpList->clearPoints();
      gcpListPoints.clear();

      // try to add 500 random GCPs to the list
      for( int i = 0; i < 500; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand();
         newPoint.mPixel.mY = rand();
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }

      {
         HrTimer::Resource timer( &timeFor500, false );
         pGcpList->addPoints( gcpListPoints );
      }

      pGcpList->clearPoints();
      gcpListPoints.clear();

      // try to add 1000 random GCPs to the list
      for( int i = 0; i < 1000; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand();
         newPoint.mPixel.mY = rand();
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }

      {
         HrTimer::Resource timer( &timeFor1000, false );
         pGcpList->addPoints( gcpListPoints );
      }

      pGcpList->clearPoints();
      gcpListPoints.clear();

      // try to add 10000 random GCPs to the list
      for( int i = 0; i < 10000; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand();
         newPoint.mPixel.mY = rand();
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }

      {
         HrTimer::Resource timer( &timeFor10000, false );
         pGcpList->addPoints( gcpListPoints );
      }

      pGcpList->clearPoints();
      gcpListPoints.clear();

      // try to add 100000 random GCPs to the list
      for( int i = 0; i < 100000; i++ )
      {
         GcpPoint newPoint;
         newPoint.mPixel.mX = rand();
         newPoint.mPixel.mY = rand();
         newPoint.mCoordinate.mX = 0.0;
         newPoint.mCoordinate.mY = 0.0;
         gcpListPoints.push_back( newPoint );
      }

      {
         HrTimer::Resource timer( &timeFor100000, false );
         pGcpList->addPoints( gcpListPoints );
      }

      printf( "Results: %f seconds to add 50 points, %f seconds to add 100 points,\n", timeFor50, timeFor100 );
      printf( "%f seconds to add 500 points, %f seconds to add 1000 points,\n", timeFor500, timeFor1000 );
      printf( "%f seconds to add 10000 points, %f seconds to add 100000 points,\n", timeFor10000, timeFor100000 );

      issea( pView->deleteLayer( pProperties ) == true );

      return success;
   }
};

class MessageLogPerformanceTest : public TestCase
{
public:
   MessageLogPerformanceTest() : TestCase("MessageLog") {}
   bool run()
   {
      bool success = true;
      string logName = "cppPerformanceTestLog";
      double timeTaken = 0.0;
      bool ok = false;

      MessageLogMgr *pLogMgr = NULL;
      pLogMgr = UtilityServicesImp::instance()->getMessageLog();
      issea( pLogMgr != NULL );

      MessageLog *pLog = NULL;
      pLog = pLogMgr->getLog( logName );
      issea( pLog != NULL );

      DockWindow *pDockWindow = NULL;
      pDockWindow = dynamic_cast<DockWindow*>( DesktopServicesImp::instance()->getWindow( "Message Log Window", DOCK_WINDOW ) );
      issea( pDockWindow != NULL );

      Step *pStep = NULL;

      // how long does it take to add a single step?
      {
         HrTimer::Resource timer( &timeTaken, false );

         pStep = pLog->createStep( "This is a performance test.", "cppTests", "FC73DB73-D416-45F0-AC85-A09B9989E3EC" );
         issea( pStep != NULL );

         ok = pStep->addProperty( "firstStep", 1 );
         issea( ok == true );
         ok = pStep->finalize( Message::Success );
         issea( ok == true );
      }
      printf( "\tAdding the first Step takes %f seconds.\n", timeTaken );


      // create 10,000 steps to see if the system slows down
      for( int count = 2; count < 10002; count++ )
      {
         pStep = pLog->createStep( "This is a performance test.", "cppTests", "42A60F91-C53B-495D-8036-CAFB78B5567D" );
         issea( pStep != NULL );

         ok = pStep->addProperty( "loopIteration", count );
         issea( ok == true );
         ok = pStep->finalize( Message::Success );
         issea( ok == true );
      }

      // how long does it take to add a single step after 10001 have already been added?
      {
         HrTimer::Resource timer( &timeTaken, false );

         pStep = pLog->createStep( "This is a performance test.", "cppTests", "09137733-ACA5-4570-98A1-6649E5F2A526" );
         issea( pStep != NULL );

         ok = pStep->addProperty( "lastStep", 10002 );
         issea( ok == true );
         ok = pStep->finalize( Message::Success );
         issea( ok == true );
      }
      printf( "\tAdding the 10002th Step takes %f seconds.\n", timeTaken );

      return success;
   }
};

class MessageLogRecursivePerformanceTest : public TestCase
{
public:
   MessageLogRecursivePerformanceTest() : TestCase("MessageLogRecursive") {}
   bool run()
   {
      bool success = true;
      string logName = "cppRecursivePerformanceTestLog";
      double timeTaken = 0.0;
      bool ok = false;

      MessageLogMgr *pLogMgr = NULL;
      pLogMgr = UtilityServicesImp::instance()->getMessageLog();
      issea( pLogMgr != NULL );

      MessageLog *pLog = NULL;
      pLog = pLogMgr->getLog( logName );
      issea( pLog != NULL );

      DockWindow *pDockWindow = NULL;
      pDockWindow = dynamic_cast<DockWindow*>( DesktopServicesImp::instance()->getWindow( "Message Log Window", DOCK_WINDOW ) );
      issea( pDockWindow != NULL );

      Step *pStep = NULL;
      Step *pTopStep = NULL;
      Step *pSubStep = NULL;

      // how long does it take to add a single step?
      {
         HrTimer::Resource timer( &timeTaken, false );

         pStep = pLog->createStep( "This is a performance test.", "cppTests", "F23277E7-4D67-4021-8687-0FAD318A5099" );
         issea( pStep != NULL );

         ok = pStep->addProperty( "firstStep", 1 );
         issea( ok == true );

         ok = pStep->finalize( Message::Success );
         issea( ok == true );
      }
      printf( "\tAdding the first Step takes %f seconds.\n", timeTaken );


      // create 10,000 recursive steps with 3 sub-steps each to see if the system slows down
      for( int count = 2; count < 10002; count++ )
      {
         pTopStep = pLog->createStep( "This is a performance test.", "cppTests", "3E6C18F1-BAA9-453B-AA63-BCA075A512B8" );
         issea( pTopStep != NULL );

         ok = pTopStep->addProperty( "loopIteration", count );
         issea( ok == true );

         for( int i = 0; i < 3; i++ )
         {
            pSubStep = pLog->createStep( "This is a sub step for the performance test.", "cppTests",
               "F6ABA5A3-1C99-468D-8F25-8C8C56F20FDA" );
            issea( pSubStep != NULL );

            ok = pSubStep->addProperty( "subloopIteration", i );
            issea( ok == true );
         }
         ok = pTopStep->finalize( Message::Success );
         issea( ok == true );
      }

      // how long does it take to add a single step after 10001 have already been added?
      {
         HrTimer::Resource timer( &timeTaken, false );

         pStep = pLog->createStep( "This is a performance test.", "cppTests", "333E35ED-5EE9-4834-956A-39AF37A4D8CD" );
         issea( pStep != NULL );

         ok = pStep->addProperty( "lastStep", 10002 );
         issea( ok == true );
      }
      printf( "\tAdding the 10002th Step takes %f seconds.\n", timeTaken );

      ok = pStep->finalize( Message::Success );
      issea( ok == true );

      return success;
   }
};

class Pseudocolor1000ClassPerformanceTest : public TestCase
{
public:
   Pseudocolor1000ClassPerformanceTest() : TestCase("Pseudocolor1000") {}
   bool run()
   {
      bool success = true;
      double timeToExecute = 0.0;
      unsigned int count = 0;
      Service<ApplicationServices> pApplication;
      issea( pApplication.get() != NULL );
      ObjectFactory *pFact = NULL;
      pFact = pApplication->getObjectFactory();
      issea( pFact != NULL );

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      RasterDataDescriptor *pDescriptor = NULL;
      pDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDescriptor != NULL );

      unsigned int numRows = pDescriptor->getRowCount();
      unsigned int numCols = pDescriptor->getColumnCount();

      RasterDataDescriptor *pRMDescriptor = RasterUtilities::generateRasterDataDescriptor(
         "testMatrix", pRasterElement, numRows, numCols, 1, BSQ, INT1UBYTE, IN_MEMORY);
      issea(pRMDescriptor != NULL);
      ModelResource<RasterElement> pRMData(pRMDescriptor);
      issea(pRMData.get() != NULL);
      pRMDescriptor = NULL;

      // now add the Results Matrix to the viewer as a pseudocolor layer
      PseudocolorLayer *pLayer = NULL;
      pLayer = dynamic_cast<PseudocolorLayer*>( pView->createLayer( PSEUDOCOLOR, pRMData.release(), "resultsMatrix" ) );
      issea( pLayer != NULL );

      // now add 1000 classes to the pseudocolor layer
      char className[15];
      {
         HrTimer::Resource timer( &timeToExecute, false );
         for( int count = 0; count < 1000; count++ )
         {
            sprintf( className, "class_%d", count );
            issea( pLayer->addInitializedClass( className, count, ColorType( ( rand() / ( RAND_MAX / 255 + 1 ) ), ( rand() / ( RAND_MAX / 255 + 1 ) ), ( rand() / ( RAND_MAX / 255 + 1 ) ) ), true ) == count );
         }
      }
      printf( "To create 1000 pseudocolor classes takes %f seconds.\n", timeToExecute );

      pLayer->clear();
      issea( pLayer->getClassCount() == 0 );

      issea( pView->deleteLayer( pLayer ) );

      return success;
   }
};

class Pseudocolor2000ClassPerformanceTest : public TestCase
{
public:
   Pseudocolor2000ClassPerformanceTest() : TestCase("Pseudocolor2000") {}
   bool run()
   {
      bool success = true;
      double timeToExecute = 0.0;
      unsigned int count = 0;
      Service<ApplicationServices> pApplication;
      issea( pApplication.get() != NULL );
      ObjectFactory *pFact = NULL;
      pFact = pApplication->getObjectFactory();
      issea( pFact != NULL );

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindowAdapter *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindowAdapter*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      RasterDataDescriptor *pDescriptor = NULL;
      pDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDescriptor != NULL );

      unsigned int numRows = pDescriptor->getRowCount();
      unsigned int numCols = pDescriptor->getColumnCount();

      RasterDataDescriptor *pRMDescriptor = RasterUtilities::generateRasterDataDescriptor("testMatrix", pRasterElement,
         numRows, numCols, 1, BSQ, INT1UBYTE, IN_MEMORY);
      issea(pRMDescriptor != NULL);
      ModelResource<RasterElement> pRMData(pRMDescriptor);
      issea(pRMData.get() != NULL);
      pRMDescriptor = NULL;

      // now add the Results Matrix to the viewer as a pseudocolor layer
      PseudocolorLayer *pLayer = NULL;
      pLayer = dynamic_cast<PseudocolorLayer*>( pView->createLayer( PSEUDOCOLOR, pRMData.release(), "resultsMatrix" ) );
      issea( pLayer != NULL );

      // now add 2000 classes to the pseudocolor layer
      char className[15];
      {
         HrTimer::Resource timer( &timeToExecute, false );
         for( int count = 0; count < 2000; count++ )
         {
            sprintf( className, "class_%d", count );
            issea( pLayer->addInitializedClass( className, count, ColorType( ( rand() / ( RAND_MAX / 255 + 1 ) ), ( rand() / ( RAND_MAX / 255 + 1 ) ), ( rand() / ( RAND_MAX / 255 + 1 ) ) ), true ) == count );
         }
      }
      printf( "To create 2000 pseudocolor classes takes %f seconds.\n", timeToExecute );

      pLayer->clear();
      issea( pLayer->getClassCount() == 0 );

      issea( pView->deleteLayer( pLayer ) );

      return success;
   }
};

class View100LayerPerformanceTest : public TestCase
{
public:
   View100LayerPerformanceTest() : TestCase("View100Layer") {}
   bool run()
   {
      bool success = true;
      double timeToExecute = 0.0;
      unsigned int count = 0;
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

      RasterDataDescriptor *pRMDescriptor = RasterUtilities::generateRasterDataDescriptor(
         "testMatrix", pRasterElement, numRows, numCols, 1, BSQ, INT1UBYTE, IN_MEMORY);
      issea(pRMDescriptor != NULL);
      ModelResource<RasterElement> pRMData(pRMDescriptor);
      issea(pRMData.get() != NULL);
      pRMDescriptor = NULL;

      {
         HrTimer::Resource timer( &timeToExecute, false );
         for( count = 0; count < 100; count++ )
         {
            char name[15];
            sprintf( name, "Layer %d", count );
            pView->createLayer( THRESHOLD, pRMData.release(), name );
         }
      }
      printf( "Creating 100 Threshold Layers takes %f seconds.\n", timeToExecute );

      LayerList *pLayerList = NULL;
      pLayerList = pView->getLayerList();
      unsigned int numLayers = 0;
      numLayers = pLayerList->getNumLayers();

      {
         HrTimer::Resource timer( &timeToExecute, false );
         for( count = 0; count < 100; count++ )
         {
            char name[15];
            sprintf( name, "Layer %d", count );
            issea( pView->deleteLayer( pLayerList->getLayer( THRESHOLD, pRMData.get(), name ) ) == true );
         }
      }

      printf( "Deleting 100 Threshold Layers takes %f seconds.\n", timeToExecute );

      return success;
   }
};

class View150LayerPerformanceTest : public TestCase
{
public:
   View150LayerPerformanceTest() : TestCase("View150Layer") {}
   bool run()
   {
      bool success = true;
      double timeToExecute = 0.0;
      unsigned int count = 0;
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

      RasterDataDescriptor *pRMDescriptor = RasterUtilities::generateRasterDataDescriptor(
         "testMatrix", pRasterElement, numRows, numCols, 1, BSQ, INT1UBYTE, IN_MEMORY);
      issea(pRMDescriptor != NULL);
      ModelResource<RasterElement> pRMData(pRMDescriptor);
      issea(pRMData.get() != NULL);
      pRMDescriptor = NULL;

      {
         HrTimer::Resource timer( &timeToExecute, false );
         for( count = 0; count < 150; count++ )
         {
            char name[15];
            sprintf( name, "Layer %d", count );
            pView->createLayer( THRESHOLD, pRMData.release(), name );
         }
      }
      printf( "Creating 150 Threshold Layers takes %f seconds.\n", timeToExecute );

      LayerList *pLayerList = NULL;
      pLayerList = pView->getLayerList();
      unsigned int numLayers = 0;
      numLayers = pLayerList->getNumLayers();

      {
         HrTimer::Resource timer( &timeToExecute, false );
         for( count = 0; count < 150; count++ )
         {
            char name[15];
            sprintf( name, "Layer %d", count );
            issea( pView->deleteLayer( pLayerList->getLayer( THRESHOLD, pRMData.get(), name ) ) == true );
         }
      }

      printf( "Deleting 150 Threshold Layers takes %f seconds.\n", timeToExecute );

      return success;
   }
};

class PerformanceTestSuite : public TestSuiteNewSession
{
public:
   PerformanceTestSuite() : TestSuiteNewSession( "Performance" )
   {
      addTestCase( new ThousandAnnotationPerformanceTestCase );
      addTestCase( new TenThousandAnnotationPerformanceTestCase );
      addTestCase( new OneThousandAoiPerformanceTest );
      addTestCase( new GcpAddRandomPointsTest );
      addTestCase( new GcpAddRandomPointsTest2 );
      addTestCase( new MessageLogPerformanceTest );
      addTestCase( new MessageLogRecursivePerformanceTest );
      addTestCase( new Pseudocolor1000ClassPerformanceTest );
      addTestCase( new Pseudocolor2000ClassPerformanceTest );
      addTestCase( new View100LayerPerformanceTest );
      addTestCase( new View150LayerPerformanceTest );
   }
};

REGISTER_SUITE( PerformanceTestSuite )
