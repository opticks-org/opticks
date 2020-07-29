/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */
#include "ApplicationServices.h"
#include "AnnotationLayer.h"
#include "AnnotationLayerImp.h"
#include "AoiElement.h"
#include "AoiLayer.h"
#include "AoiLayerImp.h"
#include "assert.h"
#include "ConnectionManager.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "RasterUtilities.h"
#include "DesktopServicesImp.h"
#include "DimensionDescriptor.h"
#include "DockWindow.h"
#include "GcpLayer.h"
#include "GcpLayerImp.h"
#include "GcpList.h"
#include "GraphicObject.h"
#include "GraphicGroup.h"
#include "HighResolutionTimer.h"
#include "ImportDescriptor.h"
#include "LayerList.h"
#include "ModelServicesImp.h"
#include "ObjectFactory.h"
#include "ObjectResource.h"
#include "PlugInManagerServices.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "RasterLayerImp.h"
#include "SpatialDataWindow.h"
#include "SpatialDataView.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "ThresholdLayer.h"
#include "ThresholdLayerImp.h"
#include "ViewImp.h"

#ifdef UNIX_API
#include <unistd.h>
#endif

#include <iostream>
using namespace std;

#include <QtWidgets/QDockWidget>
#include <QtWidgets/QMainWindow>

vector<string> updates;

class DockWindowObserver
{
public:
   virtual ~DockWindowObserver() {} 
   void update( Subject &subject, const string &signal, const boost::any &data )
   {  
      if( signal == SIGNAL_NAME( DockWindow, Docked ) || signal == SIGNAL_NAME( DockWindow, Undocked ) ||
          signal == SIGNAL_NAME( DockWindow, Shown ) ||  signal == SIGNAL_NAME( DockWindow, Hidden ) )
      {
         cout << "\tNotified of " << subject.getObjectType() << " with signal " << signal << endl;
         updates.push_back( signal );
      }
   }
};

class ViewPanToTest : public TestCase
{
public:
   ViewPanToTest() : TestCase("PanTo") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      ViewImp* pView = dynamic_cast<ViewImp*>(pWindow->getView());
      issea( pView != NULL );

      LocationType visibleCenter;
      pView->zoomExtents();

      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 48.5, 90.5 ), 1e-3 ) == true );

      pView->zoomInsetTo( 500 );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 48.5, 90.5 ), 1e-3 ) == true );

      pView->panTo( LocationType( 10, 153 ) );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 10, 153 ), 1e-3 ) == true );

      pView->zoomInsetTo( 250 );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 10, 153 ), 1e-3 ) == true );

      pView->panTo( LocationType( 83, 22 ) );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 83, 22 ), 1e-3 ) == true );

      pView->zoomInsetTo( 612 );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 83, 22 ), 1e-3 ) == true );

      pView->panTo( LocationType( 43, 102 ) );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 43, 102 ), 1e-3 ) == true );

      //attempt to pan off the cube extents and make
      //sure that pan is clipped to the cube extents.
      pView->panTo( LocationType( 100, 300 ) );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 97, 181 ), 1e-3 ) == true );

      pView->panTo( LocationType( 0, 0 ) );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 0, 0 ), 1e-3 ) == true );

      pView->zoomExtents();
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 48.5, 90.5 ), 1e-3 ) == true );

      return success;
   }


   bool ViewPanToTest::isWithinTolerance( LocationType exactCenter, LocationType approxCenter, double tolerance )
   {
      if( ( fabs( exactCenter.mX - approxCenter.mX ) < tolerance ) &&
         ( fabs( exactCenter.mY - approxCenter.mY ) < tolerance ) )
      {
         return true;
      }
      else
      {
         return false;
      }
   }

};

class ViewSetLayerDisplayIndexTest : public TestCase
{
public:
   ViewSetLayerDisplayIndexTest() : TestCase("SetLayerDisplayIndex") {}
   bool run()
   {
      bool success = true;
      double timeToExecute = 0.0;
      {
         HrTimer::Resource timer( &timeToExecute, false );

         RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
         issea( pRasterElement != NULL );

         SpatialDataWindow *pWindow = NULL;
         pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
         issea( pWindow != NULL );

         SpatialDataView *pView = NULL;
         pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
         issea( pView != NULL );

         RasterDataDescriptor *pDataDescriptor = NULL;
         pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
         issea( pDataDescriptor != NULL );

         unsigned int numRows = pDataDescriptor->getRowCount();
         unsigned int numCols = pDataDescriptor->getColumnCount();
         int count = 0;

         // create a GCP layer
         GcpList* pGcpList = NULL;
         pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement(
            "TestList", "GcpList", NULL ) );
         issea( pGcpList != NULL );

         // put GCP points on several pixels
         list<GcpPoint> gcpListPoints;
         for( unsigned int row = 0; row < numRows; row += 20 )
         {
            for( unsigned int col = 0; col < numCols; col += 20 )
            {
               GcpPoint newPoint;
               newPoint.mPixel.mX = col;
               newPoint.mPixel.mY = row;
               newPoint.mCoordinate.mX = 0;
               newPoint.mCoordinate.mY = 0;
               gcpListPoints.push_back( newPoint );
            }
         }
         pGcpList->addPoints( gcpListPoints );

         GcpLayer* pGCPLayer = NULL;
         pGCPLayer = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestGcpLayer" ) );
         issea( pGCPLayer != NULL );

         // create an AOI layer
         AoiElement* pAoi = NULL;
         pAoi = dynamic_cast<AoiElement*>( ModelServicesImp::instance()->createElement(
            "AoiName", "AoiElement", pRasterElement ) );
         issea( pAoi != NULL );

         AoiLayer* pAOILayer = NULL;
         pAOILayer = dynamic_cast<AoiLayer*>( pView->createLayer( AOI_LAYER, pAoi, "AoiName" ) );
         issea( pAOILayer != NULL );

         GraphicObject *pRect = pAOILayer->addObject(RECTANGLE_OBJECT);
         issea( pRect != NULL );
         pRect->setBoundingBox(LocationType(5, 5), LocationType(numCols-5, numRows-5));

         // create an annotation layer
         AnnotationLayer *pAnnotationLayer = NULL;
         pAnnotationLayer = dynamic_cast<AnnotationLayer*>( pView->createLayer( ANNOTATION, NULL, "AnnotationTest" ) );
         issea( pAnnotationLayer != NULL );

         GraphicObject *pEllipse = NULL;
         pEllipse = pAnnotationLayer->addObject( ELLIPSE_OBJECT );
         issea( pEllipse != NULL );

         pEllipse->setBoundingBox( LocationType( 10, 10 ), LocationType( 90, 170 ) );
         pEllipse->setFillColor( ColorType( 0, 0, 255 ) );
         pEllipse->setFillStyle( HATCH );
         pEllipse->setHatchStyle( ASTERISK );

         // create a threshold layer
         Layer *pThreshProperties = NULL;
         if( success )
         {
            pThreshProperties = TestUtilities::createThresholdLayer( pRasterElement, 0, -1 );
         }
         issea( pThreshProperties != NULL );

         string layerName = pThreshProperties->getName();
         issea( layerName != "" );

         LayerList *pLayerList = NULL;
         pLayerList = pView->getLayerList();
         issea( pLayerList != NULL );

         ThresholdLayer *pThresholdLayer = NULL;
         if( success )
         {
            pThresholdLayer = dynamic_cast<ThresholdLayer*>( pLayerList->getLayer( pThreshProperties->getLayerType(),
               pThreshProperties->getDataElement(), layerName ) );
         }
         issea( pThresholdLayer != NULL );

         RasterLayer *pRasterLayer = NULL;
         pRasterLayer = dynamic_cast<RasterLayer*>( pLayerList->getLayer( RASTER, pRasterElement ) );
         issea( pRasterLayer != NULL );

         // first cycle through the layers, setting each layer to the front layer one after another, for 100 iterations
         for( count = 0; count < 100; count++ )
         {         
            issea( pView->setFrontLayer( pGCPLayer ) == true );
            issea( pView->setFrontLayer( pAOILayer ) == true );
            issea( pView->setFrontLayer( pAnnotationLayer ) == true );
            issea( pView->setFrontLayer( pThresholdLayer ) == true );
            issea( pView->setFrontLayer( pRasterLayer ) == true );
         }

         // now call setFrontLayer() 100 times on each layer individually
         for( count = 0; count < 100; count++ )
         {
            issea( pView->setFrontLayer( pGCPLayer ) == true );
         }
         for( count = 0; count < 100; count++ )
         {
            issea( pView->setFrontLayer( pAOILayer ) == true );
         }
         for( count = 0; count < 100; count++ )
         {
            issea( pView->setFrontLayer( pRasterLayer ) == true );
         }
         for( count = 0; count < 100; count++ )
         {
            issea( pView->setFrontLayer( pAnnotationLayer ) == true );
         }
         for( count = 0; count < 100; count++ )
         {
            issea( pView->setFrontLayer( pThresholdLayer ) == true );
         }

         // try setting the layer display index 100 times for each layer
         for( count = 0; count < 100; count++ )
         {
            issea( pView->setLayerDisplayIndex( pGCPLayer, 4 ) == true );
            issea( pView->setLayerDisplayIndex( pGCPLayer, 1 ) == true );
            issea( pView->setLayerDisplayIndex( pGCPLayer, 3 ) == true );
            issea( pView->setLayerDisplayIndex( pGCPLayer, 0 ) == true );
            issea( pView->setLayerDisplayIndex( pGCPLayer, 2 ) == true );
         }

         for( count = 0; count < 100; count++ )
         {
            issea( pView->setLayerDisplayIndex( pAOILayer, 1 ) == true );
            issea( pView->setLayerDisplayIndex( pAOILayer, 4 ) == true );
            issea( pView->setLayerDisplayIndex( pAOILayer, 2 ) == true );
            issea( pView->setLayerDisplayIndex( pAOILayer, 0 ) == true );
            issea( pView->setLayerDisplayIndex( pAOILayer, 3 ) == true );
         }

         for( count = 0; count < 100; count++ )
         {
            issea( pView->setLayerDisplayIndex( pRasterLayer, 2 ) == true );
            issea( pView->setLayerDisplayIndex( pRasterLayer, 0 ) == true );
            issea( pView->setLayerDisplayIndex( pRasterLayer, 4 ) == true );
            issea( pView->setLayerDisplayIndex( pRasterLayer, 3 ) == true );
            issea( pView->setLayerDisplayIndex( pRasterLayer, 1 ) == true );
         }

         for( count = 0; count < 100; count++ )
         {
            issea( pView->setLayerDisplayIndex( pAnnotationLayer, 0 ) == true );
            issea( pView->setLayerDisplayIndex( pAnnotationLayer, 3 ) == true );
            issea( pView->setLayerDisplayIndex( pAnnotationLayer, 1 ) == true );
            issea( pView->setLayerDisplayIndex( pAnnotationLayer, 2 ) == true );
            issea( pView->setLayerDisplayIndex( pAnnotationLayer, 4 ) == true );
         }

         for( count = 0; count < 100; count++ )
         {
            issea( pView->setLayerDisplayIndex( pThresholdLayer, 3 ) == true );
            issea( pView->setLayerDisplayIndex( pThresholdLayer, 1 ) == true );
            issea( pView->setLayerDisplayIndex( pThresholdLayer, 0 ) == true );
            issea( pView->setLayerDisplayIndex( pThresholdLayer, 4 ) == true );
            issea( pView->setLayerDisplayIndex( pThresholdLayer, 2 ) == true );
         }

         issea( pView->deleteLayer( pGCPLayer ) == true ); 

         issea( pAnnotationLayer->removeObject( pEllipse, true ) );
         issea( pView->deleteLayer( pAnnotationLayer ) == true );
         issea( pView->deleteLayer( pAOILayer ) == true ); 
         issea( pView->deleteLayer( pThresholdLayer ) == true );
      }

      printf( "Time to execute ViewSetLayerDisplayIndexTest is %f seconds\n", timeToExecute );

      return success;
   }
};

class ViewPanAndZoomTest : public TestCase
{
public:
   ViewPanAndZoomTest() : TestCase("PanAndZoom") {}
   bool run()
   {
      bool success = true;
      double minX = 0.0;
      double minY = 0.0;
      double maxX = 0.0;
      double maxY = 0.0;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      ViewImp* pView = dynamic_cast<ViewImp*>(pWindow->getView());
      issea( pView != NULL );

      LocationType visibleCenter;

      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 48.5, 90.5 ), 1e-3 ) == true );

      pView->getExtents( minX, minY, maxX, maxY );
      issea( minX == 0 && minY == 0 && maxX == 97 && maxY == 181 );

      // test the pan functions
      pView->panBy( -15.0, -7.0 );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 33.5, 83.5 ), 1e-3 ) == true );

      pView->panBy( 23.0, -17.2 );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 56.5, 66.3 ), 1e-3 ) == true );

      pView->panTo( LocationType( 91.3, 49.8 ) );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 91.3, 49.8 ), 1e-3 ) == true );

      pView->panBy( -15.1, -7.4 );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 76.2, 42.4 ), 1e-3 ) == true );

      pView->getExtents( minX, minY, maxX, maxY );
      issea( minX == 0 && minY == 0 && maxX == 97 && maxY == 181 );

      // test the zoom functions
      pView->zoomExtents();

      pView->getExtents( minX, minY, maxX, maxY );
      issea( minX == 0 && minY == 0 && maxX == 97 && maxY == 181 );

      pView->zoomToBox( LocationType( 15, 154 ), LocationType( 70, 30 ) );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 42.5, 92.0 ), 1e-3 ) == true );

      pView->zoomInsetTo( 1234 );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 42.5, 92.0 ), 1e-3 ) == true );

      pView->zoomToBox( LocationType( 81, 144 ), LocationType( 93, 54 ) );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 87.0, 99.0 ), 1e-3 ) == true );

      pView->zoomInsetTo( 24 );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 87.0, 99.0 ), 1e-3 ) == true );

      pView->zoomInsetTo( 0 );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 87.0, 99.0 ), 1e-3 ) == true );

      pView->zoomInsetTo( 3000 );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 87.0, 99.0 ), 1e-3 ) == true );

      // test pan functions while zoomed in
      pView->panBy( -19.6, 17.4 );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 67.4, 116.4 ), 1e-3 ) == true );

      pView->panTo( LocationType( 59.7, 99.8 ) );
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 59.7, 99.8 ), 1e-3 ) == true );

      pView->zoomExtents();
      visibleCenter = pView->getVisibleCenter();
      issea( isWithinTolerance( visibleCenter, LocationType( 48.5, 90.5 ), 1e-3 ) == true );      

      return success;
   }


   bool ViewPanAndZoomTest::isWithinTolerance( LocationType exactCenter, LocationType approxCenter, double tolerance )
   {
      if( ( fabs( exactCenter.mX - approxCenter.mX ) < tolerance ) &&
         ( fabs( exactCenter.mY - approxCenter.mY ) < tolerance ) )
      {
         return true;
      }
      else
      {
         return false;
      }
   }
};

class ViewLayerListTest : public TestCase
{
public:
   ViewLayerListTest() : TestCase("LayerList") {}
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

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      unsigned int numRows = pDataDescriptor->getRowCount();
      unsigned int numCols = pDataDescriptor->getColumnCount();
      unsigned int count = 0;

      RasterDataDescriptor *pRMDescriptor = RasterUtilities::generateRasterDataDescriptor(
         "testMatrix", pRasterElement, numRows, numCols, 1, BSQ, INT1UBYTE, IN_MEMORY);
      issea(pRMDescriptor != NULL);
      ModelResource<RasterElement> pRMData(pRMDescriptor);
      issea(pRMData.get() != NULL);
      pRMDescriptor = NULL;

      // now add the Results Matrix to the viewer as a Raster layer
      RasterLayer *pLayer = NULL;
      pLayer = dynamic_cast<RasterLayer*>( pView->createLayer( RASTER, pRMData.release(), "resultsMatrix" ) );
      issea( pLayer != NULL );

      // check to see if the layer is there
      LayerList *pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );
      issea( pList->getLayer( RASTER, pRMData.get(), "resultsMatrix" ) != NULL );

      RasterElement *pRMDataPtr = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement( "testMatrix", "RasterElement", pRasterElement ) );
      issea( pRMDataPtr != NULL );

      // delete the ResultsMatrix
      ModelServicesImp::instance()->destroyElement( pRMDataPtr );

      // check to see if the raster layer is gone
      pList = pView->getLayerList();
      issea( pList != NULL );
      issea( pList->getLayer( RASTER, pRMData.get(), "resultsMatrix" ) == NULL );

      // recreate the ResultsMatrix
      pRMDescriptor = RasterUtilities::generateRasterDataDescriptor(
         "testMatrix", pRasterElement, 1, 1, 1, BSQ, INT1UBYTE, IN_MEMORY);
      issea(pRMDescriptor != NULL);
      ModelResource<RasterElement> pRMData2(pRMDescriptor);
      issea(pRMData2.get() != NULL);
      pRMDescriptor = NULL;

      // now add the Results Matrix to the viewer as a Raster layer again
      RasterLayer *pLayer2 = NULL;
      pLayer2 = dynamic_cast<RasterLayer*>( pView->createLayer( RASTER, pRMData2.release(), "resultsMatrix" ) );
      issea( pLayer2 != NULL );
      pView->refresh();

      pList = pView->getLayerList();
      issea( pList != NULL );
      issea( pList->getLayer( RASTER, pRMData2.get(), "resultsMatrix" ) != NULL );

      // there should be only one Raster layer besides the cube itself
      unsigned int numLayers = pList->getNumLayers();
      issea(numLayers == 2);

      // delete the ResultsMatrix
      ModelServicesImp::instance()->destroyElement( pRMData2.get() );
      issea( pList->getLayer( RASTER, pRMData2.get(), "resultsMatrix" ) == NULL );

      // only the original cube layer itself remains
      numLayers = pList->getNumLayers();
      issea(numLayers == 1);

      return success;
   }
};


class AlphaChangeTest : public TestCase
{
public:
   AlphaChangeTest() : TestCase("Alpha") {}
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

      LayerList *pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );

      RasterLayer *pRaster = NULL;
      pRaster = dynamic_cast<RasterLayer*>( pList->getLayer( RASTER, pRasterElement, pRasterElement->getName() ) );
      issea( pRaster != NULL );

      for( unsigned int x = 0; x < 256; x++ )
      {
         pRaster->setAlpha( x );
         issea( pRaster->getAlpha() == x );
         pView->refresh();
      }

      return success;
   }
};


class ResultsMatrixAsDisplayedBandTest : public TestCase
{
public:
   ResultsMatrixAsDisplayedBandTest() : TestCase("RMBand") {}
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

      ThresholdLayer *pProperties = NULL;
      if( success )
      {
         pProperties = TestUtilities::createThresholdLayer( pRasterElement, 0, -1 );
      }
      issea( pProperties != NULL );
      if( pProperties == NULL )
      {
         return false;
      }
      pProperties->setFirstThreshold( 8.0 );
      pView->setLayerDisplayIndex( pProperties, 2 ); // put the layer under the cube layer
      RasterElement *pThresholdElement = NULL;
      pThresholdElement = dynamic_cast<RasterElement*>( pProperties->getDataElement() );
      issea( pThresholdElement != NULL );

      LayerList *pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );

      RasterLayer *pRaster = NULL;
      pRaster = dynamic_cast<RasterLayer*>( pList->getLayer( RASTER, pRasterElement, pRasterElement->getName() ) );
      issea( pRaster != NULL );

      DimensionDescriptor redBand;
      DimensionDescriptor greenBand;
      DimensionDescriptor blueBand;

      DimensionDescriptor zeroBand;
      zeroBand.setOriginalNumber( 0 );
      zeroBand.setOnDiskNumber( 0 );
      zeroBand.setActiveNumber( 0 );

      redBand = pRaster->getDisplayedBand( RED );
      greenBand = pRaster->getDisplayedBand( GREEN );
      blueBand = pRaster->getDisplayedBand( BLUE );

      pRaster->setDisplayedBand( RED, zeroBand, pThresholdElement );
      pRaster->setDisplayedBand( GREEN, greenBand );
      pRaster->setDisplayedBand( BLUE, blueBand );

      pRaster->setDisplayedBand( RED, redBand );
      pRaster->setDisplayedBand( GREEN, zeroBand, pThresholdElement );
      pRaster->setDisplayedBand( BLUE, blueBand );

      pRaster->setDisplayedBand( RED, redBand );
      pRaster->setDisplayedBand( GREEN, greenBand );
      pRaster->setDisplayedBand( BLUE, zeroBand, pThresholdElement );

      pRaster->setDisplayedBand( RED, redBand );
      pRaster->setDisplayedBand( GREEN, greenBand );
      pRaster->setDisplayedBand( BLUE, blueBand );

      pRaster->setDisplayedBand( RED, zeroBand, pThresholdElement );
      pRaster->setDisplayedBand( GREEN, greenBand );
      pRaster->setDisplayedBand( BLUE, blueBand );
      pView->refresh();

      // remove the Threshold layer
      issea( pView->deleteLayer( pProperties ) );

      return success;
   }
};


class ViewAndSpectralElementDeleteTest : public TestCase
{
public:
   ViewAndSpectralElementDeleteTest() : TestCase("ViewDelete") {}
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

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      unsigned int numCols = pDataDescriptor->getColumnCount();
      unsigned int numRows = pDataDescriptor->getRowCount();
      unsigned int count = 0;

      ModelResource<RasterElement> pRMData(RasterUtilities::createRasterElement(
         "testMatrix", numRows, numCols, INT1UBYTE, true, pRasterElement));
      issea(pRMData.get() != NULL);

      DataAccessor daMatrix = pRMData->getDataAccessor();
      issea( daMatrix.isValid() );

      unsigned char *pPtr = NULL;

      // fill the Results Matrix with random data from 0 - 255
      for( unsigned int row = 0; row < numRows; row++ )
      {
         pPtr = static_cast<unsigned char*>( daMatrix->getRow() );
         for( unsigned int col = 0; col < numCols; col++ )
         {
            *pPtr = rand() / ( RAND_MAX / 256 + 1 );
            pPtr++;
         }
         daMatrix->nextRow();
      }

      SpatialDataWindow *pNewWindow = NULL;
      pNewWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->createWindow( "SensorDataWindow", SPATIAL_DATA_WINDOW ) );
      issea( pNewWindow != NULL );

      SpatialDataView *pNewView = NULL;
      pNewView = dynamic_cast<SpatialDataView*>( pNewWindow->getView() );
      issea( pNewView != NULL );

      SpatialDataWindow *pNewWindow2 = NULL;
      pNewWindow2 = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->createWindow( "ResultsMatrixWindow", SPATIAL_DATA_WINDOW ) );
      issea( pNewWindow2 != NULL );

      SpatialDataView *pNewView2 = NULL;
      pNewView2 = dynamic_cast<SpatialDataView*>( pNewWindow2->getView() );
      issea( pNewView2 != NULL );

      RasterElement* pSD = dynamic_cast<RasterElement*>(pRasterElement->copy("testCube", NULL));
      issea( pSD != NULL );

      RasterLayer *pRasterLayer = NULL;
      pRasterLayer = dynamic_cast<RasterLayer*>( pNewView->createLayer( RASTER, pSD, "testCube" ) );
      issea( pRasterLayer != NULL );

      // now add the Results Matrix as a threshold layer
      ThresholdLayer *pThresholdLayer = NULL;
      pThresholdLayer = dynamic_cast<ThresholdLayer*>( pNewView2->createLayer( THRESHOLD, pRMData.release(), "resultsMatrix" ) );
      issea( pThresholdLayer != NULL );
      pThresholdLayer->setRegionUnits( RAW_VALUE );
      pThresholdLayer->setPassArea( UPPER );
      pThresholdLayer->setFirstThreshold( 0 );

      ModelServicesImp::instance()->destroyElement( pRMData.get() );
      ModelServicesImp::instance()->destroyElement( pSD );

      // wait for 5 seconds so visual inspection can be done
#if defined(WIN_API)
      Sleep( 5000 ); // milliseconds
#else
      sleep( 5 ); // seconds
#endif

      issea( DesktopServicesImp::instance()->getWindow( "SensorDataWindow", SPATIAL_DATA_WINDOW ) != NULL );
      issea( DesktopServicesImp::instance()->getWindow( "ResultsMatrixWindow", SPATIAL_DATA_WINDOW ) != NULL );

      return success;
   }
};

class LayerRenameTest : public TestCase
{
public:
   LayerRenameTest() : TestCase("Rename") {}
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

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      // create a GCP layer
      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement(
         "TestList", "GcpList", pRasterElement ) );
      issea( pGcpList != NULL );

      // put GCP points on several pixels
      list<GcpPoint> gcpListPoints;
      for( unsigned int row = 0; row < pDataDescriptor->getRowCount(); row += 20 )
      {
         for( unsigned int col = 0; col < pDataDescriptor->getColumnCount(); col += 20 )
         {
            GcpPoint newPoint;
            newPoint.mPixel.mX = col;
            newPoint.mPixel.mY = row;
            newPoint.mCoordinate.mX = 0;
            newPoint.mCoordinate.mY = 0;
            gcpListPoints.push_back( newPoint );
         }
      }
      pGcpList->addPoints( gcpListPoints );

      GcpLayer* pGCPLayer = NULL;
      pGCPLayer = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestGcpLayer" ) );
      issea( pGCPLayer != NULL );

      // create an AOI layer
      AoiElement* pAoi = NULL;
      pAoi = dynamic_cast<AoiElement*>( ModelServicesImp::instance()->createElement(
         "AoiName", "AoiElement", pRasterElement ) );
      issea( pAoi != NULL );

      AoiLayer* pAOILayer = NULL;
      pAOILayer = dynamic_cast<AoiLayer*>( pView->createLayer( AOI_LAYER, pAoi, "AoiName" ) );
      issea( pAOILayer != NULL );

      GraphicObject *pRect = pAoi->getGroup()->addObject( RECTANGLE_OBJECT );
      issea( pRect != NULL );
      pRect->setBoundingBox( LocationType( 5, 5 ), LocationType( pDataDescriptor->getColumnCount() - 5, pDataDescriptor->getRowCount() -5 ) );

      // create an annotation layer
      AnnotationLayer *pAnnotationLayer = NULL;
      pAnnotationLayer = dynamic_cast<AnnotationLayer*>( pView->createLayer( ANNOTATION, NULL, "AnnotationTest" ) );
      issea( pAnnotationLayer != NULL );

      GraphicObject *pEllipse = NULL;
      pEllipse = pAnnotationLayer->addObject( ELLIPSE_OBJECT );
      issea( pEllipse != NULL );

      pEllipse->setBoundingBox( LocationType( 10, 10 ), LocationType( 90, 170 ) );
      pEllipse->setFillColor( ColorType( 0, 0, 255 ) );
      pEllipse->setFillStyle( HATCH );
      pEllipse->setHatchStyle( ASTERISK );

      // create a threshold layer
      ThresholdLayer *pThreshProperties = NULL;
      if( success )
      {
         pThreshProperties = TestUtilities::createThresholdLayer( pRasterElement, 0, -1 );
      }
      issea( pThreshProperties != NULL );

      string layerName = pThreshProperties->getName();
      issea( layerName != "" );

      LayerList *pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );

      if( success )
      {
         RasterLayer *pRasterLayer = NULL;
         pRasterLayer = dynamic_cast<RasterLayer*>( pList->getLayer( RASTER, pRasterElement, pRasterElement->getName() ) );
         string originalElementName = pRasterElement->getName();
         issea( pRasterLayer != NULL );

         ThresholdLayer *pThresholdLayer = NULL;      
         pThresholdLayer = dynamic_cast<ThresholdLayer*>( pList->getLayer( pThreshProperties->getLayerType(),
            pThreshProperties->getDataElement(), layerName ) );      
         issea( pThresholdLayer != NULL );

         pList->renameLayer( pGCPLayer, "newGcpLayerName" );
         pList->renameLayer( pAOILayer, "newAoiLayerName" );
         pList->renameLayer( pAnnotationLayer, "newAnnotationLayerName" );
         pList->renameLayer( pRasterLayer, "newCubeLayerName" );
         pList->renameLayer( pThresholdLayer, "newThresholdLayerName" );

         pList->renameLayer( pGCPLayer, "newGcpLayerName2" );
         pList->renameLayer( pAOILayer, "newAoiLayerName2" );
         pList->renameLayer( pAnnotationLayer, "newAnnotationLayerName2" );
         pList->renameLayer( pRasterLayer, "newCubeLayerName2" );
         pList->renameLayer( pThresholdLayer, "newThresholdLayerName2" );

         pList->renameLayer( pGCPLayer, "newGcpLayerName3" );
         pList->renameLayer( pAOILayer, "newAoiLayerName3" );
         pList->renameLayer( pAnnotationLayer, "newAnnotationLayerName3" );
         pList->renameLayer( pRasterLayer, "newCubeLayerName3" );
         pList->renameLayer( pThresholdLayer, "newThresholdLayerName3" );

         pList->renameLayer( pGCPLayer, "newGcpLayerName" );
         pList->renameLayer( pAOILayer, "newAoiLayerName" );
         pList->renameLayer( pAnnotationLayer, "newAnnotationLayerName" );
         pList->renameLayer( pRasterLayer, "newCubeLayerName" );
         pList->renameLayer( pThresholdLayer, "newThresholdLayerName" );

         // rename the cube layer to its original name
         pList->renameLayer( pRasterLayer, originalElementName );
         issea( pView->deleteLayer( pGCPLayer ) );
         issea( pView->deleteLayer( pAOILayer ) );
         issea( pView->deleteLayer( pAnnotationLayer ) );
         issea( pView->deleteLayer( pThresholdLayer ) );
      }

      return success;
   }
};

class LayerNameChangeTest : public TestCase
{
public:
   LayerNameChangeTest() : TestCase("NameChange") {}
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

      RasterDataDescriptor *pDataDescriptor = NULL;
      pDataDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDataDescriptor != NULL );

      // create a GCP layer
      GcpList* pGcpList = NULL;
      pGcpList = dynamic_cast<GcpList*>( ModelServicesImp::instance()->createElement(
         "TestList", "GcpList", pRasterElement ) );
      issea( pGcpList != NULL );

      // put GCP points on several pixels
      list<GcpPoint> gcpListPoints;
      for( unsigned int row = 0; row < pDataDescriptor->getRowCount(); row += 20 )
      {
         for( unsigned int col = 0; col < pDataDescriptor->getColumnCount(); col += 20 )
         {
            GcpPoint newPoint;
            newPoint.mPixel.mX = col;
            newPoint.mPixel.mY = row;
            newPoint.mCoordinate.mX = 0;
            newPoint.mCoordinate.mY = 0;
            gcpListPoints.push_back( newPoint );
         }
      }
      pGcpList->addPoints( gcpListPoints );

      GcpLayer* pGCPLayer = NULL;
      pGCPLayer = dynamic_cast<GcpLayer*>( pView->createLayer( GCP_LAYER, pGcpList, "TestGcpLayer" ) );
      issea( pGCPLayer != NULL );

      // create an AOI layer
      AoiElement* pAoi = NULL;
      pAoi = dynamic_cast<AoiElement*>( ModelServicesImp::instance()->createElement(
         "AoiName", "AoiElement", pRasterElement ) );
      issea( pAoi != NULL );

      AoiLayer* pAOILayer = NULL;
      pAOILayer = dynamic_cast<AoiLayer*>( pView->createLayer( AOI_LAYER, pAoi, "AoiName" ) );
      issea( pAOILayer != NULL );

      GraphicObject *pRect = pAoi->getGroup()->addObject( RECTANGLE_OBJECT );
      issea( pRect != NULL );
      pRect->setBoundingBox( LocationType( 5, 5 ), LocationType( pDataDescriptor->getColumnCount() - 5, pDataDescriptor->getRowCount() - 5 ) );

      // create an annotation layer
      AnnotationLayer *pAnnotationLayer = NULL;
      pAnnotationLayer = dynamic_cast<AnnotationLayer*>( pView->createLayer( ANNOTATION, NULL, "AnnotationTest" ) );
      issea( pAnnotationLayer != NULL );

      GraphicObject *pEllipse = NULL;
      pEllipse = pAnnotationLayer->addObject( ELLIPSE_OBJECT );
      issea( pEllipse != NULL );

      pEllipse->setBoundingBox( LocationType( 10, 10 ), LocationType( 90, 170 ) );
      pEllipse->setFillColor( ColorType( 0, 0, 255 ) );
      pEllipse->setFillStyle( HATCH );
      pEllipse->setHatchStyle( ASTERISK );

      // create a threshold layer
      ThresholdLayer *pThreshProperties = NULL;
      if( success )
      {
         pThreshProperties = TestUtilities::createThresholdLayer( pRasterElement, 0, -1 );
      }
      issea( pThreshProperties != NULL );

      string layerName = pThreshProperties->getName();
      issea( layerName != "" );

      LayerList *pList = NULL;
      pList = pView->getLayerList();
      issea( pList != NULL );

      if( success )
      {
         RasterLayer *pRasterLayer = NULL;
         pRasterLayer = dynamic_cast<RasterLayer*>( pList->getLayer( RASTER, pRasterElement, pRasterElement->getName() ) );
         issea( pRasterLayer != NULL );

         ThresholdLayer *pThresholdLayer = NULL;      
         pThresholdLayer = dynamic_cast<ThresholdLayer*>( pList->getLayer( pThreshProperties->getLayerType(),
            pThreshProperties->getDataElement(), layerName ) );      
         issea( pThresholdLayer != NULL );

         dynamic_cast<GcpLayerImp*>(pGCPLayer)->setName("newGcpLayerName");
         dynamic_cast<AoiLayerImp*>(pAOILayer)->setName("newAoiLayerName");
         dynamic_cast<AnnotationLayerImp*>(pAnnotationLayer)->setName("newAnnotationLayerName");
         dynamic_cast<RasterLayerImp*>(pRasterLayer)->setName("newCubeLayerName");
         dynamic_cast<ThresholdLayerImp*>(pThresholdLayer)->setName("newThresholdLayerName");

         // don't delete anything because this test if for COMETDEV-149
      }

      return success;
   }
};

class DockWindowLocationTest : public TestCase
{
public:
   DockWindowLocationTest() : TestCase("DockWindowLocation") {}
   bool run()
   {
      bool success = true;
      DesktopServices *pDesktop = DesktopServicesImp::instance();
      DockWindow *pDockWindow = static_cast<DockWindow*>(
         pDesktop->createWindow( "DockWindowLocationTest", DOCK_WINDOW ) );
      QDockWidget *pDockWidget = dynamic_cast<QDockWidget*>( pDockWindow );
      issea( pDockWidget != NULL );

      QMainWindow *pMainWindow = dynamic_cast<QMainWindow*>(
         pDesktop->getMainWidget() );
      issea( pMainWindow != NULL );

      pMainWindow->addDockWidget( Qt::LeftDockWidgetArea, pDockWidget );
      issea( pDesktop->getDockWindowArea( *pDockWindow ) == DOCK_LEFT );

      pMainWindow->addDockWidget( Qt::RightDockWidgetArea, pDockWidget );
      issea( pDesktop->getDockWindowArea( *pDockWindow ) == DOCK_RIGHT );

      pMainWindow->addDockWidget( Qt::TopDockWidgetArea, pDockWidget );
      issea( pDesktop->getDockWindowArea( *pDockWindow ) == DOCK_TOP );

      pMainWindow->addDockWidget( Qt::BottomDockWidgetArea, pDockWidget );
      issea( pDesktop->getDockWindowArea( *pDockWindow ) == DOCK_BOTTOM );

      pDockWidget->setFloating( true );
      issea( pDesktop->getDockWindowArea( *pDockWindow ) == DOCK_FLOATING );

      pDockWidget->setFloating( false );
      issea( pDesktop->getDockWindowArea( *pDockWindow ) == DOCK_BOTTOM );

      issea(pDesktop->deleteWindow(pDockWindow));

      return success;
   }
};

class DockWindowNotifyTest : public TestCase
{
public:
   DockWindowNotifyTest() : TestCase("DockWindowNotify") {}
   bool run()
   {
      bool success = true;
      Service<DesktopServices> pDesktop;
      DockWindowObserver *pObserver = NULL;
      pObserver = new DockWindowObserver();
      issea( pObserver != NULL );

      DockWindow *pDockWindow = NULL;
      pDockWindow = static_cast<DockWindow*>( pDesktop->createWindow( "DockWindowNotifyTest", DOCK_WINDOW ) );
      issea( pDockWindow != NULL );

      issea( pDockWindow->attach( SIGNAL_NAME( DockWindow, Docked ),
         Slot( pObserver, &DockWindowObserver::update ) ) );

      issea( pDockWindow->attach( SIGNAL_NAME( DockWindow, Undocked ),
         Slot( pObserver, &DockWindowObserver::update ) ) );

      issea( pDockWindow->attach( SIGNAL_NAME( DockWindow, Shown ),
         Slot( pObserver, &DockWindowObserver::update ) ) );

      issea( pDockWindow->attach( SIGNAL_NAME( DockWindow, Hidden ),
         Slot( pObserver, &DockWindowObserver::update ) ) );

      issea( pDesktop->getDockWindowArea( *pDockWindow ) == DOCK_BOTTOM );

      issea( updates.size() == 0 );

      pDockWindow->undock();
      issea( updates.size() == 1 );
      issea( updates.at( 0 ) == SIGNAL_NAME( DockWindow, Undocked ) );

      pDockWindow->dock();
      issea( updates.size() == 2 );
      issea( updates.at( 1 ) == SIGNAL_NAME( DockWindow, Docked ) );

      pDockWindow->hide();
      issea( updates.size() == 3 );
      issea( updates.at( 2 ) == SIGNAL_NAME( DockWindow, Hidden ) );

      pDockWindow->undock();
      issea( updates.size() == 4 );
      issea( updates.at( 3 ) == SIGNAL_NAME( DockWindow, Undocked ) );

      pDockWindow->dock();
      issea( updates.size() == 5 );
      issea( updates.at( 4 ) == SIGNAL_NAME( DockWindow, Docked ) );

      pDockWindow->show();
      issea( updates.size() == 6 );
      issea( updates.at( 5 ) == SIGNAL_NAME( DockWindow, Shown ) );

      pDockWindow->undock();
      issea( updates.size() == 7 );
      issea( updates.at( 6 ) == SIGNAL_NAME( DockWindow, Undocked ) );

      issea( pDockWindow->detach( SIGNAL_NAME( DockWindow, Docked ),
         Slot( pObserver, &DockWindowObserver::update ) ) );

      issea( pDockWindow->detach( SIGNAL_NAME( DockWindow, Undocked ),
         Slot( pObserver, &DockWindowObserver::update ) ) );

      issea( pDockWindow->detach( SIGNAL_NAME( DockWindow, Shown ),
         Slot( pObserver, &DockWindowObserver::update ) ) );

      issea( pDockWindow->detach( SIGNAL_NAME( DockWindow, Hidden ),
         Slot( pObserver, &DockWindowObserver::update ) ) );

      issea(pDesktop->deleteWindow(pDockWindow));
      return success;
   }
};

class LayerOffsetAndScalingTest : public TestCase
{
public:
   LayerOffsetAndScalingTest() : TestCase("OffsetAndScaling") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issearf(pRasterElement != NULL);

      Service<DesktopServices> pDesktop;

      SpatialDataWindow* pWindow =
         dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
      issearf(pWindow != NULL);

      SpatialDataView* pView = pWindow->getSpatialDataView();
      issearf(pView != NULL);

      RasterLayer* pLayer = dynamic_cast<RasterLayer*>(pView->getTopMostLayer(RASTER));
      issearf(pLayer != NULL);

      double xCoord = 0.0;
      double yCoord = 0.0;

      issea( pLayer->getXScaleFactor() == 1.0 );
      issea( pLayer->getYScaleFactor() == 1.0 );
      issea( pLayer->getXOffset() == 0.0 );
      issea( pLayer->getYOffset() == 0.0 );

      pLayer->translateWorldToData( 1, 1, xCoord, yCoord );
      issea( xCoord == 1.0 && yCoord == 1.0 );
      pLayer->translateDataToWorld( 27, 33, xCoord, yCoord );
      issea( xCoord == 27.0 && yCoord == 33.0 );

      pLayer->setXScaleFactor( 7.0 );
      pLayer->setYScaleFactor( 3.0 );

      pLayer->translateWorldToData( 56, 27, xCoord, yCoord );
      issea( xCoord == 8.0 && yCoord == 9.0 );
      pLayer->translateDataToWorld( 27, 33, xCoord, yCoord );
      issea( xCoord == 189.0 && yCoord == 99.0 );

      pLayer->setXScaleFactor( 0.5 );
      pLayer->setYScaleFactor( 0.8 );

      pLayer->translateWorldToData( 60, 50, xCoord, yCoord );
      issea( xCoord == 120.0 && yCoord == 62.5 );
      pLayer->translateDataToWorld( 80, 70, xCoord, yCoord );
      issea( xCoord == 40.0 && yCoord == 56.0 );

      pLayer->setXScaleFactor(0.0);
      pLayer->setYScaleFactor(0.0);

      pLayer->translateWorldToData(60, 50, xCoord, yCoord);
      issea(xCoord == 120.0 && yCoord == 62.5);
      pLayer->translateDataToWorld(80, 70, xCoord, yCoord);
      issea(xCoord == 40.0 && yCoord == 56.0);

      pLayer->setXScaleFactor(-7.0);
      pLayer->setYScaleFactor(-3.0);

      pLayer->translateWorldToData(60, 50, xCoord, yCoord);
      issea(xCoord == 120.0 && yCoord == 62.5);
      pLayer->translateDataToWorld(80, 70, xCoord, yCoord);
      issea(xCoord == 40.0 && yCoord == 56.0);

      // reset the scale
      pLayer->setXScaleFactor( 1.0 );
      pLayer->setYScaleFactor( 1.0 );
      pLayer->translateWorldToData( 60, 50, xCoord, yCoord );
      issea( xCoord == 60.0 && yCoord == 50.0 );
      pLayer->translateDataToWorld( 80, 70, xCoord, yCoord );
      issea( xCoord == 80.0 && yCoord == 70.0 );

      pLayer->setXOffset( 20 );
      pLayer->setYOffset( 40 );

      pLayer->translateWorldToData( 20, 40, xCoord, yCoord );
      issea( xCoord == 0.0 && yCoord == 0.0 );
      pLayer->translateDataToWorld( 80, 70, xCoord, yCoord );
      issea( xCoord == 100.0 && yCoord == 110.0 );

      pLayer->setXOffset( 10.3 );
      pLayer->setYOffset( 20.4 );

      pLayer->translateWorldToData( 20, 40, xCoord, yCoord );
      issea( xCoord == 9.7 && yCoord == 19.6 );
      pLayer->translateDataToWorld( 80, 70, xCoord, yCoord );
      issea( xCoord == 90.3 && yCoord == 90.4 );

      pLayer->setXOffset( 0.3 );
      pLayer->setYOffset( 0.4 );

      pLayer->translateWorldToData( 20, 40, xCoord, yCoord );
      issea( xCoord == 19.7 && yCoord == 39.6 );
      pLayer->translateDataToWorld( 80, 70, xCoord, yCoord );
      issea( xCoord == 80.3 && yCoord == 70.4 );

      pLayer->setXOffset( -20 );
      pLayer->setYOffset( -40 );

      pLayer->translateWorldToData( 20, 40, xCoord, yCoord );
      issea( xCoord == 40.0 && yCoord == 80.0 );
      pLayer->translateDataToWorld( 80, 70, xCoord, yCoord );
      issea( xCoord == 60.0 && yCoord == 30.0 );

      pLayer->setXOffset( -10.3 );
      pLayer->setYOffset( -20.4 );

      pLayer->translateWorldToData( 20, 40, xCoord, yCoord );
      issea( xCoord == 30.3 && yCoord == 60.4 );
      pLayer->translateDataToWorld( 80, 70, xCoord, yCoord );
      issea( xCoord == 69.7 && yCoord == 49.6 );

      pLayer->setXOffset( -0.3 );
      pLayer->setYOffset( -0.4 );

      pLayer->translateWorldToData( 20, 40, xCoord, yCoord );
      issea( xCoord == 20.3 && yCoord == 40.4 );
      pLayer->translateDataToWorld( 80, 70, xCoord, yCoord );
      issea( xCoord == 79.7 && yCoord == 69.6 );

      // reset the offset
      pLayer->setXOffset( 0.0 );
      pLayer->setYOffset( 0.0 );
      pLayer->translateWorldToData( 60, 50, xCoord, yCoord );
      issea( xCoord == 60.0 && yCoord == 50.0 );
      pLayer->translateDataToWorld( 80, 70, xCoord, yCoord );
      issea( xCoord == 80.0 && yCoord == 70.0 );

      return success;
   }
};

class LayerCreateWithOffsetAndScalingTest : public TestCase
{
public:
   LayerCreateWithOffsetAndScalingTest() : TestCase("LayerCreateWithOffsetAndScaling") {}
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

      RasterLayer* pLayer = dynamic_cast<RasterLayer*>(pView->getTopMostLayer(RASTER));
      issea( pLayer != NULL );

      double xCoord = 0.0;
      double yCoord = 0.0;

      issea( pLayer->getXScaleFactor() == 1.0 );
      issea( pLayer->getYScaleFactor() == 1.0 );
      issea( pLayer->getXOffset() == 0.0 );
      issea( pLayer->getYOffset() == 0.0 );

      pLayer->setXScaleFactor( 5.0 );
      pLayer->setYScaleFactor( 4.0 );
      pLayer->setXOffset( 15.0 );
      pLayer->setYOffset( 22.0 );

      issea( pLayer->getXScaleFactor() == 5.0 );
      issea( pLayer->getYScaleFactor() == 4.0 );
      issea( pLayer->getXOffset() == 15.0 );
      issea( pLayer->getYOffset() == 22.0 );

      pLayer->translateWorldToData( 30, 30, xCoord, yCoord );
      issea( xCoord == 3.0 && yCoord == 2.0 );
      pLayer->translateDataToWorld( 75, 75, xCoord, yCoord );
      issea( xCoord == 390.0 && yCoord == 322.0 );

      // create an AOI that will made into an AOI Layer
      AoiElement* pAoi = NULL;
      pAoi = dynamic_cast<AoiElement*>( ModelServicesImp::instance()->createElement(
         "testAOI", "AoiElement", pRasterElement ) );
      issea( pAoi != NULL );

      // create the AOI Layer
      AoiLayer *pAoiLayer = NULL;
      pAoiLayer = dynamic_cast<AoiLayer*>( pView->createLayer( AOI_LAYER, pAoi ) );
      issea( pAoiLayer != NULL );

      // The AOI Layer should have taken on the offset and scale of the Cube Layer
      issea( pAoiLayer->getXScaleFactor() == 5.0 );
      issea( pAoiLayer->getYScaleFactor() == 4.0 );
      issea( pAoiLayer->getXOffset() == 15.0 );
      issea( pAoiLayer->getYOffset() == 22.0 );

      ModelServicesImp::instance()->destroyElement( pAoi );

      return success;
   }
};

class ImportWithOffsetAndScalingTest : public TestCase
{
public:
   ImportWithOffsetAndScalingTest() : TestCase("ImportWithOffsetAndScalingTest") {}
   bool run()
   {
      bool success = true;

      string filename = TestUtilities::getTestDataPath() + "daytonchip.sio";

      ImporterResource sioImporter( "SIO Importer", filename, NULL, false );

      vector<ImportDescriptor*> importDescriptors = sioImporter->getImportDescriptors();
      issea(!importDescriptors.empty());

      ImportDescriptor* pImportDescriptor = importDescriptors.front();
      issea(pImportDescriptor != NULL);

      RasterDataDescriptor* pDataDescriptor =
         dynamic_cast<RasterDataDescriptor*>(pImportDescriptor->getDataDescriptor());
      issea( pDataDescriptor != NULL );

      issea( pDataDescriptor->getXPixelSize() == 1.0 );
      issea( pDataDescriptor->getYPixelSize() == 1.0 );

      // change the pixel size before importing
      pDataDescriptor->setXPixelSize( 3.5 );
      pDataDescriptor->setYPixelSize( 4.0 );

      // import the cube
      issea( sioImporter->execute() );

      RasterElement* pRasterElement = NULL;
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement(
         filename, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( DesktopServicesImp::instance()->getWindow(
         pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      RasterLayer* pLayer = dynamic_cast<RasterLayer*>(pView->getTopMostLayer(RASTER));
      issea( pLayer != NULL );

      issea( pLayer->getXOffset() == 0.0 );
      issea( pLayer->getYOffset() == 0.0 );
      issea( pLayer->getXScaleFactor() == 3.5 );
      issea( pLayer->getYScaleFactor() == 4.0 );

      // create an AOI that will made into an AOI Layer
      AoiElement* pAoi = NULL;
      pAoi = dynamic_cast<AoiElement*>( ModelServicesImp::instance()->createElement(
         "testAOI", "AoiElement", pRasterElement ) );
      issea( pAoi != NULL );

      // create the AOI Layer
      AoiLayer *pAoiLayer = NULL;
      pAoiLayer = dynamic_cast<AoiLayer*>( pView->createLayer( AOI_LAYER, pAoi ) );
      issea( pAoiLayer != NULL );

      // The AOI Layer should have taken on the offset and scale of the Cube Layer
      issea( pAoiLayer->getXOffset() == 0.0 );
      issea( pAoiLayer->getYOffset() == 0.0 );
      issea( pAoiLayer->getXScaleFactor() == 3.5 );
      issea( pAoiLayer->getYScaleFactor() == 4.0 );

      issea(Service<ModelServices>()->destroyElement(pAoi));
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class TranslateCoordinateTest : public TestCase
{
public:
   TranslateCoordinateTest() : TestCase("TranslateCoordinateTest") {}
   bool run()
   {
      bool success = true;

      Service<DesktopServices> pDesktop;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      SpatialDataWindow *pWindow = NULL;
      pWindow = dynamic_cast<SpatialDataWindow*>( pDesktop->getWindow( pRasterElement->getName(), SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      RasterDataDescriptor *pDescriptor = NULL;
      pDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDescriptor != NULL );

      double xWorldCoord = 0.0;
      double yWorldCoord = 0.0;
      double xScreenCoord = 0.0;
      double yScreenCoord = 0.0;
      unsigned int row = 0;
      unsigned int col = 0;

      for( row = 0; row < pDescriptor->getRowCount(); row++ )
      {
         for( col = 0; col < pDescriptor->getColumnCount(); col++ )
         {
            pView->translateWorldToScreen( col, row, xScreenCoord, yScreenCoord ); // change to screen coordinates
            pView->translateScreenToWorld( xScreenCoord, yScreenCoord, xWorldCoord, yWorldCoord ); // change back
            issea( fabs( static_cast<double>( col ) - xWorldCoord ) < 1e-10 ); // were the values retained?
            issea( fabs( static_cast<double>( row ) - yWorldCoord ) < 1e-10 );
         }
      }

      for( row = 0; row < pDescriptor->getRowCount(); row++ )
      {
         for( col = 0; col < pDescriptor->getColumnCount(); col++ )
         {
            pView->translateScreenToWorld( col, row, xWorldCoord, yWorldCoord ); // change to world coordinates
            pView->translateWorldToScreen( xWorldCoord, yWorldCoord, xScreenCoord, yScreenCoord ); // change back
            issea( fabs( static_cast<double>( col ) - xScreenCoord ) < 1e-10 ); // were the values retained?
            issea( fabs( static_cast<double>( row ) - yScreenCoord ) < 1e-10 );
         }
      }

      return success;
   }
};

class TerrainDeleteTest : public TestCase
{
public:
   TerrainDeleteTest() : TestCase("TerrainDeleteTest") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea(pRasterElement != NULL);

      RasterElement* pTerrain = dynamic_cast<RasterElement*>
         (pRasterElement->copy("Terrain", NULL));
      issea(pTerrain != NULL);

      issea(pRasterElement->getTerrain() == NULL);
      pRasterElement->setTerrain(pTerrain);
      issea(pRasterElement->getTerrain() != NULL);

      Service<ModelServices> pModel;
      issea(pModel->destroyElement(pTerrain));
      issea(pRasterElement->getTerrain() == NULL);

      return success;
   }
};

class ViewTestSuite : public TestSuiteNewSession
{
public:
   ViewTestSuite() : TestSuiteNewSession( "View" )
   {
      addTestCase( new ViewPanToTest );
      addTestCase( new ViewSetLayerDisplayIndexTest );
      addTestCase( new ViewPanAndZoomTest );
      addTestCase( new ViewLayerListTest );
      addTestCase( new AlphaChangeTest );
      addTestCase( new ResultsMatrixAsDisplayedBandTest );
      addTestCase( new ViewAndSpectralElementDeleteTest );
      addTestCase( new LayerRenameTest );
      addTestCase( new LayerNameChangeTest );
      addTestCase( new DockWindowLocationTest );
      addTestCase( new DockWindowNotifyTest );
      addTestCase( new LayerOffsetAndScalingTest );
      addTestCase( new LayerCreateWithOffsetAndScalingTest );
      addTestCase( new ImportWithOffsetAndScalingTest );
      addTestCase( new TranslateCoordinateTest );
      addTestCase( new TerrainDeleteTest );
   }
};

REGISTER_SUITE( ViewTestSuite )
