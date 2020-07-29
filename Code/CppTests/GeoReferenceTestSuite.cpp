/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "ConfigurationSettings.h"
#include "DesktopServices.h"
#include "GcpList.h"
#include "GeoreferenceDescriptor.h"
#include "LatLonLayer.h"
#include "Layer.h"
#include "LayerList.h"
#include "ModelServicesImp.h"
#include "ObjectResource.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterUtilities.h"
#include "SpatialDataWindow.h"
#include "SpatialDataView.h"
#include "TestBedTestUtilities.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "TypesFile.h"

#include <QtCore/QString>

using namespace std;

class GeoReferenceCornerAndCenterTest : public TestCase
{
public:
   GeoReferenceCornerAndCenterTest() : TestCase("CornerAndCenter") {}
   bool run()
   {
      bool success = true;
      bool ok = false;
      LocationType origin, zeroXmaxY, maxXzeroY, maxXmaxY, center;

      string filename = TestUtilities::getTestDataPath() + "tipjul5bands.sio";

      SpatialDataWindow *pWindow = NULL;
      pWindow = TestUtilities::loadDataSet( filename, "SIO Importer" );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      RasterElement *pRasterElement = NULL;
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement(
         filename, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      RasterDataDescriptor *pDescriptor = NULL;
      pDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDescriptor != NULL );

      RasterFileDescriptor *pFileDescriptor = NULL;
      pFileDescriptor = dynamic_cast<RasterFileDescriptor*>( pDescriptor->getFileDescriptor() );
      issea( pFileDescriptor != NULL );

      list<GcpPoint> gcpList = pFileDescriptor->getGcps();
      issea( gcpList.size() == 5 );
      origin = gcpList.front().mCoordinate;
      gcpList.pop_front();
      maxXzeroY = gcpList.front().mCoordinate;
      gcpList.pop_front();
      zeroXmaxY = gcpList.front().mCoordinate;
      gcpList.pop_front();
      maxXmaxY = gcpList.front().mCoordinate;
      gcpList.pop_front();
      center = gcpList.front().mCoordinate;

      issea( TestUtilities::runGeoRef( pRasterElement ) );
      issea( pRasterElement->isGeoreferenced() );
      LocationType geo_origin, geo_zeroXmaxY, geo_maxXzeroY, geo_maxXmaxY, geo_center;

      geo_origin = pRasterElement->convertPixelToGeocoord( LocationType( 0.0, 0.0 ) );
      geo_zeroXmaxY = pRasterElement->convertPixelToGeocoord( LocationType( 0.0, 168.0 ) );
      geo_maxXzeroY = pRasterElement->convertPixelToGeocoord( LocationType( 168.0, 0.0 ) );
      geo_maxXmaxY = pRasterElement->convertPixelToGeocoord( LocationType( 168.0, 168.0 ) );
      geo_center = pRasterElement->convertPixelToGeocoord( LocationType( 84.0, 84.0 ) );

      issea( fabs( geo_origin.mX - origin.mX ) < 1e-10 && fabs( geo_origin.mY - origin.mY ) < 1e-10 );
      issea( fabs( geo_zeroXmaxY.mX - zeroXmaxY.mX ) < 1e-10 && fabs( geo_zeroXmaxY.mY - zeroXmaxY.mY ) < 1e-10 );
      issea( fabs( geo_maxXzeroY.mX - maxXzeroY.mX ) < 1e-10 && fabs( geo_maxXzeroY.mY - maxXzeroY.mY ) < 1e-10 );
      //issea( fabs( geo_maxXmaxY.mX - maxXmaxY.mX ) < 1e-10 && fabs( geo_maxXmaxY.mY - maxXmaxY.mY ) < 1e-10 );
      //issea( fabs( geo_center.mX - center.mX ) < 1e-10 && fabs( geo_center.mY - center.mY ) < 1e-10 );

      issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      return success;
   }
};

class GeoReferenceTest : public TestCase
{
public:
   GeoReferenceTest() : TestCase("Chip") {}
   bool run()
   {
      bool success = true;
      bool ok = false;

      string filename = TestUtilities::getTestDataPath() + "afghan.sio";

      SpatialDataWindow *pWindow = NULL;
      pWindow = TestUtilities::loadDataSet( filename, "SIO Importer" );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      RasterElement *pRasterElement = NULL;
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement(
         filename, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      issea( TestUtilities::runGeoRef( pRasterElement ) );
      issea( pRasterElement->isGeoreferenced() );

      string tempHome;
      const Filename* pTempPath = ConfigurationSettings::getSettingTempPath();
      if (pTempPath != NULL)
      {
         tempHome = pTempPath->getFullPathAndName();
      }
      QString qTempHome = QString::fromStdString( tempHome );
      qTempHome.replace( "\\", "/" );
      tempHome = qTempHome.toStdString();

      RasterDataDescriptor *pDescriptor = NULL;
      pDescriptor = dynamic_cast<RasterDataDescriptor*>( pRasterElement->getDataDescriptor() );
      issea( pDescriptor != NULL );

      { //scope to immediately destroy resources
         FactoryResource<RasterFileDescriptor> pFileDescriptor1( 
            RasterUtilities::generateRasterFileDescriptorForExport(pDescriptor, tempHome + "/afghan_subset.tif", 
            pDescriptor->getActiveRow(1000), pDescriptor->getActiveRow(1300), 0,
            pDescriptor->getActiveColumn(1000), pDescriptor->getActiveColumn(1300), 0,
            pDescriptor->getActiveBand(0), pDescriptor->getActiveBand(6), 0));
         ExporterResource exporter1("GeoTIFF Exporter", pRasterElement, pFileDescriptor1.get());
         issea(exporter1->execute());
      }

      { //scope to immediately destroy resources
         FactoryResource<RasterFileDescriptor> pFileDescriptor2( 
            RasterUtilities::generateRasterFileDescriptorForExport(pDescriptor, tempHome + "/afghan_subset.ice.h5", 
            pDescriptor->getActiveRow(1000), pDescriptor->getActiveRow(1300), 0,
            pDescriptor->getActiveColumn(1000), pDescriptor->getActiveColumn(1300), 0,
            pDescriptor->getActiveBand(0), pDescriptor->getActiveBand(6), 0));
         ExporterResource exporter2("Ice Exporter", pRasterElement, pFileDescriptor2.get());
         issea(exporter2->execute());
      }

      { //scope to immediately destroy resources
         FactoryResource<RasterFileDescriptor> pFileDescriptor3(
            RasterUtilities::generateRasterFileDescriptorForExport(pDescriptor, tempHome + "/afghan_subset2.tif", 
            pDescriptor->getActiveRow(5000), pDescriptor->getActiveRow(5500), 0,
            pDescriptor->getActiveColumn(5000), pDescriptor->getActiveColumn(5500), 0,
            pDescriptor->getActiveBand(0), pDescriptor->getActiveBand(6), 0));
         ExporterResource exporter3("GeoTIFF Exporter", pRasterElement, pFileDescriptor3.get());
         issea(exporter3->execute());
      }

      { //scope to immediately destroy resources
         FactoryResource<RasterFileDescriptor> pFileDescriptor4(
            RasterUtilities::generateRasterFileDescriptorForExport(pDescriptor, tempHome + "/afghan_subset2.ice.h5", 
            pDescriptor->getActiveRow(5000), pDescriptor->getActiveRow(5500), 0,
            pDescriptor->getActiveColumn(5000), pDescriptor->getActiveColumn(5500), 0,
            pDescriptor->getActiveBand(0), pDescriptor->getActiveBand(6), 0));
         ExporterResource exporter4("Ice Exporter", pRasterElement, pFileDescriptor4.get());
         issea(exporter4->execute());
      }

      pDescriptor = NULL;

      // VERIFY THE Ice CHIPS
      string chipFilename1 = tempHome + "/afghan_subset.ice.h5";
      SpatialDataWindow *pWindowChip1 = NULL;
      pWindowChip1 = TestUtilities::loadDataSet( chipFilename1, "Ice Importer" );
      issea( pWindowChip1 != NULL );

      SpatialDataView *pViewChip1 = NULL;
      pViewChip1 = dynamic_cast<SpatialDataView*>( pWindowChip1->getView() );
      issea( pViewChip1 != NULL );

      RasterElement *pRasterElementChip1 = NULL;
      pRasterElementChip1 = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement(
         chipFilename1, "RasterElement", NULL ) );
      issea( pRasterElementChip1 != NULL );

      issea( TestUtilities::runGeoRef( pRasterElementChip1 ) );
      issea( pRasterElementChip1->isGeoreferenced() );


      string chipFilename2 = tempHome + "/afghan_subset2.ice.h5";
      SpatialDataWindow *pWindowChip2 = NULL;
      pWindowChip2 = TestUtilities::loadDataSet( chipFilename2, "Ice Importer" );
      issea( pWindowChip2 != NULL );

      SpatialDataView *pViewChip2 = NULL;
      pViewChip2 = dynamic_cast<SpatialDataView*>( pWindowChip2->getView() );
      issea( pViewChip2 != NULL );

      RasterElement *pRasterElementChip2 = NULL;
      pRasterElementChip2 = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement(
         chipFilename2, "RasterElement", NULL ) );
      issea( pRasterElementChip2 != NULL );

      issea( TestUtilities::runGeoRef( pRasterElementChip2 ) );
      issea( pRasterElementChip2->isGeoreferenced());
     
      const double NITF_GCP_TOLERANCE = 0.0005; // NITF GCPs only go with 0.001 precision

      // compare the geocoordinates of the chipped scenes with the corresponding pixels of the original
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 0, 0 ) ).mX 
         - pRasterElement->convertPixelToGeocoord( LocationType( 1000, 1000 ) ).mX )
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 0, 0 ) ).mY 
         - pRasterElement->convertPixelToGeocoord( LocationType( 1000, 1000 ) ).mY ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 0, 300 ) ).mX 
         - pRasterElement->convertPixelToGeocoord( LocationType( 1000, 1300 ) ).mX ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 0, 300 ) ).mY 
         - pRasterElement->convertPixelToGeocoord( LocationType( 1000, 1300 ) ).mY ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 300, 0 ) ).mX 
         - pRasterElement->convertPixelToGeocoord( LocationType( 1300, 1000 ) ).mX ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 300, 0 ) ).mY 
         - pRasterElement->convertPixelToGeocoord( LocationType( 1300, 1000 ) ).mY ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 300, 300 ) ).mX 
         - pRasterElement->convertPixelToGeocoord( LocationType( 1300, 1300 ) ).mX ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 300, 300 ) ).mY 
         - pRasterElement->convertPixelToGeocoord( LocationType( 1300, 1300 ) ).mY ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 150, 150 ) ).mX 
         - pRasterElement->convertPixelToGeocoord( LocationType( 1150, 1150 ) ).mX ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 150, 150 ) ).mY 
         - pRasterElement->convertPixelToGeocoord( LocationType( 1150, 1150 ) ).mY ) 
         < NITF_GCP_TOLERANCE );
      issea(TestUtilities::destroyWorkspaceWindow(pWindowChip1));

      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 0, 0 ) ).mX 
         - pRasterElement->convertPixelToGeocoord( LocationType( 5000, 5000 ) ).mX ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 0, 0 ) ).mY 
         - pRasterElement->convertPixelToGeocoord( LocationType( 5000, 5000 ) ).mY ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 0, 500 ) ).mX 
         - pRasterElement->convertPixelToGeocoord( LocationType( 5000, 5500 ) ).mX ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 0, 500 ) ).mY 
         - pRasterElement->convertPixelToGeocoord( LocationType( 5000, 5500 ) ).mY ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 500, 0 ) ).mX 
         - pRasterElement->convertPixelToGeocoord( LocationType( 5500, 5000 ) ).mX ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 500, 0 ) ).mY 
         - pRasterElement->convertPixelToGeocoord( LocationType( 5500, 5000 ) ).mY ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 500, 500 ) ).mX 
         - pRasterElement->convertPixelToGeocoord( LocationType( 5500, 5500 ) ).mX ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 500, 500 ) ).mY 
         - pRasterElement->convertPixelToGeocoord( LocationType( 5500, 5500 ) ).mY ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 250, 250 ) ).mX 
         - pRasterElement->convertPixelToGeocoord( LocationType( 5250, 5250 ) ).mX ) 
         < NITF_GCP_TOLERANCE );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 250, 250 ) ).mY
         - pRasterElement->convertPixelToGeocoord( LocationType( 5250, 5250 ) ).mY ) 
         < NITF_GCP_TOLERANCE );
      issea(TestUtilities::destroyWorkspaceWindow(pWindowChip2));

      // VERIFY THE TIFF CHIPS
      chipFilename1 = tempHome + "/afghan_subset.tif";
      pWindowChip1 = NULL;
      pWindowChip1 = TestUtilities::loadDataSet( chipFilename1, "GeoTIFF Importer" );
      issea( pWindowChip1 != NULL );

      pViewChip1 = NULL;
      pViewChip1 = dynamic_cast<SpatialDataView*>( pWindowChip1->getView() );
      issea( pViewChip1 != NULL );

      pRasterElementChip1 = NULL;
      pRasterElementChip1 = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement(
         chipFilename1, "RasterElement", NULL ) );
      issea( pRasterElementChip1 != NULL );

      issea( TestUtilities::runGeoRef( pRasterElementChip1 ) );
      issea( pRasterElementChip1->isGeoreferenced() );

      chipFilename2 = tempHome + "/afghan_subset2.tif";
      pWindowChip2 = NULL;
      pWindowChip2 = TestUtilities::loadDataSet( chipFilename2, "GeoTIFF Importer" );
      issea( pWindowChip2 != NULL );

      pViewChip2 = NULL;
      pViewChip2 = dynamic_cast<SpatialDataView*>( pWindowChip2->getView() );
      issea( pViewChip2 != NULL );

      pRasterElementChip2 = NULL;
      pRasterElementChip2 = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement(
         chipFilename2, "RasterElement", NULL ) );
      issea( pRasterElementChip2 != NULL );

      issea( TestUtilities::runGeoRef( pRasterElementChip2 ) );
      issea( pRasterElementChip2->isGeoreferenced() );

      // compare the geocoordinates of the chipped scenes with the corresponding pixels of the original
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 0, 0 ) ).mX - pRasterElement->convertPixelToGeocoord( LocationType( 1000, 1000 ) ).mX ) < 1e-8 );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 0, 0 ) ).mY - pRasterElement->convertPixelToGeocoord( LocationType( 1000, 1000 ) ).mY ) < 1e-8 );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 0, 300 ) ).mX - pRasterElement->convertPixelToGeocoord( LocationType( 1000, 1300 ) ).mX ) < 1e-8 );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 0, 300 ) ).mY - pRasterElement->convertPixelToGeocoord( LocationType( 1000, 1300 ) ).mY ) < 1e-8 );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 300, 0 ) ).mX - pRasterElement->convertPixelToGeocoord( LocationType( 1300, 1000 ) ).mX ) < 1e-8 );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 300, 0 ) ).mY - pRasterElement->convertPixelToGeocoord( LocationType( 1300, 1000 ) ).mY ) < 1e-8 );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 300, 300 ) ).mX - pRasterElement->convertPixelToGeocoord( LocationType( 1300, 1300 ) ).mX ) < 1e-8 );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 300, 300 ) ).mY - pRasterElement->convertPixelToGeocoord( LocationType( 1300, 1300 ) ).mY ) < 1e-8 );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 150, 150 ) ).mX - pRasterElement->convertPixelToGeocoord( LocationType( 1150, 1150 ) ).mX ) < 1e-8 );
      issea( fabs( pRasterElementChip1->convertPixelToGeocoord( LocationType( 150, 150 ) ).mY - pRasterElement->convertPixelToGeocoord( LocationType( 1150, 1150 ) ).mY ) < 1e-8 );
      issea(TestUtilities::destroyWorkspaceWindow(pWindowChip1));

      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 0, 0 ) ).mX - pRasterElement->convertPixelToGeocoord( LocationType( 5000, 5000 ) ).mX ) < 1e-8 );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 0, 0 ) ).mY - pRasterElement->convertPixelToGeocoord( LocationType( 5000, 5000 ) ).mY ) < 1e-8 );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 0, 500 ) ).mX - pRasterElement->convertPixelToGeocoord( LocationType( 5000, 5500 ) ).mX ) < 1e-8 );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 0, 500 ) ).mY - pRasterElement->convertPixelToGeocoord( LocationType( 5000, 5500 ) ).mY ) < 1e-8 );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 500, 0 ) ).mX - pRasterElement->convertPixelToGeocoord( LocationType( 5500, 5000 ) ).mX ) < 1e-8 );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 500, 0 ) ).mY - pRasterElement->convertPixelToGeocoord( LocationType( 5500, 5000 ) ).mY ) < 1e-8 );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 500, 500 ) ).mX - pRasterElement->convertPixelToGeocoord( LocationType( 5500, 5500 ) ).mX ) < 1e-8 );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 500, 500 ) ).mY - pRasterElement->convertPixelToGeocoord( LocationType( 5500, 5500 ) ).mY ) < 1e-8 );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 250, 250 ) ).mX - pRasterElement->convertPixelToGeocoord( LocationType( 5250, 5250 ) ).mX ) < 1e-8 );
      issea( fabs( pRasterElementChip2->convertPixelToGeocoord( LocationType( 250, 250 ) ).mY - pRasterElement->convertPixelToGeocoord( LocationType( 5250, 5250 ) ).mY ) < 1e-8 );
      issea(TestUtilities::destroyWorkspaceWindow(pWindowChip2));

      issea(TestUtilities::destroyWorkspaceWindow(pWindow)); // close original cube
      return success;
   }
};


class GeoReferenceOrientationTest : public TestCase
{
public:
   GeoReferenceOrientationTest() : TestCase("Orientation") {}
   bool run()
   {
      bool success = true;
      bool ok = false;
      // wpafb.tif values
      double lat1[24] = {39.877558, 39.877558, 39.877558, 39.877558, 39.877558, 39.877558, 39.877550, 39.877550, 39.877550, 39.877550, 39.877550, 39.877550, 39.750732, 39.750732, 39.750731, 39.750731, 39.750731, 39.750731, 39.750723, 39.750723, 39.750723, 39.750722, 39.750722, 39.750722};
      double lon1[24] = {-84.097658, -84.097647, -84.097636, -84.097622, -84.097611, -84.097600, -84.097658, -84.097647, -84.097636, -84.097625, -84.097614, -84.097600, -83.998033, -83.998022, -83.998011, -83.998000, -83.997986, -83.997975, -83.998033, -83.998022, -83.998011, -83.998000, -83.997986, -83.997978};
      // landsat6band.tif values
      double lat2[24] = {44.283371, 44.283376, 44.283387, 44.283393, 44.283399, 44.283409, 44.283117, 44.283117, 44.283131, 44.283136, 44.283147, 44.283144, 44.156906, 44.156900, 44.156911, 44.156919, 44.156928, 44.156939, 44.156636, 44.156658, 44.156664, 44.156661, 44.156686, 44.156686};
      double lon2[24] = {-107.783333, -107.782914, -107.782547, -107.782225, -107.781847, -107.781525, -107.783333, -107.782900, -107.782531, -107.782194, -107.781825, -107.781475, -107.596997, -107.596622, -107.596225, -107.595917, -107.595547, -107.595200, -107.596972, -107.596617, -107.596244, -107.595908, -107.595536, -107.595189};
      // wpafb_geotif.tif values
      double lat3[24] = {39.369339, 39.369339, 39.369339, 39.369343, 39.369339, 39.369336, 39.369519, 39.369522, 39.369522, 39.369522, 39.369511, 39.369522, 40.015131, 40.015133, 40.015133, 40.015133, 40.015136, 40.015136, 40.015314, 40.015314, 40.015314, 40.015314, 40.015317, 40.015314};
      double lon3[24] = {-84.671192, -84.670972, -84.670742, -84.670511, -84.670267, -84.670031, -84.671203, -84.670972, -84.670742, -84.670511, -84.670264, -84.670047, -83.666011, -83.665781, -83.665553, -83.665317, -83.665089, -83.664858, -83.666014, -83.665781, -83.665550, -83.665322, -83.665089, -83.664856};
      // 041078a5.tif values
      double lat4[24] = {40.983125, 40.983125, 40.983125, 40.983122, 40.983122, 40.983122, 40.983147, 40.983147, 40.983144, 40.983144, 40.983144, 40.983144, 41.128439, 41.128439, 41.128439, 41.128439, 41.128436, 41.128436, 41.128461, 41.128458, 41.128461, 41.128458, 41.128458, 41.128458};
      double lon4[24] = {-78.637528, -78.637500, -78.637472, -78.637442, -78.637414, -78.637383, -78.637528, -78.637497, -78.637469, -78.637442, -78.637411, -78.637383, -78.487497, -78.487467, -78.487439, -78.487408, -78.487381, -78.487350, -78.487497, -78.487464, -78.487436, -78.487408, -78.487381, -78.487350};

      string filename1 = TestUtilities::getTestDataPath() + "GeoReference/wpafb.tif";
      string filename2 = TestUtilities::getTestDataPath() + "GeoReference/landsat6band.tif";
      string windowName = "";

      SpatialDataWindow *pWindow = NULL;
      pWindow = TestUtilities::loadDataSet( filename1, "GeoTIFF Importer" );
      issea( pWindow != NULL );

      SpatialDataView *pView = NULL;
      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      windowName = pWindow->getName();
      issea( windowName != "" );

      RasterElement *pRasterElement = NULL;
      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement(
         windowName, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      issea( TestUtilities::runGeoRef( pRasterElement ) );
      issea( pRasterElement->isGeoreferenced() );

      // wpafb.tif is a known good data set
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 0 ) ).mX - lat1[0] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 0 ) ).mY - lon1[0] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 1 ) ).mX - lat1[1] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 1 ) ).mY - lon1[1] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 2 ) ).mX - lat1[2] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 2 ) ).mY - lon1[2] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 3 ) ).mX - lat1[3] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 3 ) ).mY - lon1[3] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 4 ) ).mX - lat1[4] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 4 ) ).mY - lon1[4] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 5 ) ).mX - lat1[5] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 5 ) ).mY - lon1[5] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 0 ) ).mX - lat1[6] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 0 ) ).mY - lon1[6] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 1 ) ).mX - lat1[7] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 1 ) ).mY - lon1[7] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 2 ) ).mX - lat1[8] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 2 ) ).mY - lon1[8] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 3 ) ).mX - lat1[9] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 3 ) ).mY - lon1[9] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 4 ) ).mX - lat1[10] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 4 ) ).mY - lon1[10] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 5 ) ).mX - lat1[11] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 5 ) ).mY - lon1[11] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8994, 13798 ) ).mX - lat1[12] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8994, 13798 ) ).mY - lon1[12] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8995, 13798 ) ).mX - lat1[13] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8995, 13798 ) ).mY - lon1[13] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8996, 13798 ) ).mX - lat1[14] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8996, 13798 ) ).mY - lon1[14] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8997, 13798 ) ).mX - lat1[15] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8997, 13798 ) ).mY - lon1[15] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8998, 13798 ) ).mX - lat1[16] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8998, 13798 ) ).mY - lon1[16] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8999, 13798 ) ).mX - lat1[17] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8999, 13798 ) ).mY - lon1[17] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8994, 13799 ) ).mX - lat1[18] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8994, 13799 ) ).mY - lon1[18] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8995, 13799 ) ).mX - lat1[19] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8995, 13799 ) ).mY - lon1[19] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8996, 13799 ) ).mX - lat1[20] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8996, 13799 ) ).mY - lon1[20] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8997, 13799 ) ).mX - lat1[21] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8997, 13799 ) ).mY - lon1[21] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8998, 13799 ) ).mX - lat1[22] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8998, 13799 ) ).mY - lon1[22] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8999, 13799 ) ).mX - lat1[23] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 8999, 13799 ) ).mY - lon1[23] ) < 1e-3 );

      // close the data set
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      pWindow = NULL;
      pView = NULL;
      pRasterElement = NULL;
      windowName = "";

      pWindow = TestUtilities::loadDataSet( filename2, "GeoTIFF Importer" );
      issea( pWindow != NULL );

      pView = dynamic_cast<SpatialDataView*>( pWindow->getView() );
      issea( pView != NULL );

      windowName = pWindow->getName();
      issea( windowName != "" );

      pRasterElement = dynamic_cast<RasterElement*>( ModelServicesImp::instance()->getElement(
         windowName, "RasterElement", NULL ) );
      issea( pRasterElement != NULL );

      issea( TestUtilities::runGeoRef( pRasterElement ) );
      issea( pRasterElement->isGeoreferenced() );

      // landsat6band.tif is known good data set
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 0 ) ).mX - lat2[0] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 0 ) ).mY - lon2[0] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 1 ) ).mX - lat2[1] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 1 ) ).mY - lon2[1] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 2 ) ).mX - lat2[2] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 2 ) ).mY - lon2[2] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 3 ) ).mX - lat2[3] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 3 ) ).mY - lon2[3] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 4 ) ).mX - lat2[4] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 4 ) ).mY - lon2[4] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 5 ) ).mX - lat2[5] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 0, 5 ) ).mY - lon2[5] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 0 ) ).mX - lat2[6] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 0 ) ).mY - lon2[6] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 1 ) ).mX - lat2[7] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 1 ) ).mY - lon2[7] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 2 ) ).mX - lat2[8] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 2 ) ).mY - lon2[8] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 3 ) ).mX - lat2[9] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 3 ) ).mY - lon2[9] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 4 ) ).mX - lat2[10] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 4 ) ).mY - lon2[10] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 5 ) ).mX - lat2[11] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 1, 5 ) ).mY - lon2[11] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 506, 510 ) ).mX - lat2[12] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 506, 510 ) ).mY - lon2[12] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 507, 510 ) ).mX - lat2[13] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 507, 510 ) ).mY - lon2[13] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 508, 510 ) ).mX - lat2[14] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 508, 510 ) ).mY - lon2[14] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 509, 510 ) ).mX - lat2[15] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 509, 510 ) ).mY - lon2[15] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 510, 510 ) ).mX - lat2[16] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 510, 510 ) ).mY - lon2[16] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 511, 510 ) ).mX - lat2[17] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 511, 510 ) ).mY - lon2[17] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 506, 511 ) ).mX - lat2[18] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 506, 511 ) ).mY - lon2[18] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 507, 511 ) ).mX - lat2[19] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 507, 511 ) ).mY - lon2[19] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 508, 511 ) ).mX - lat2[20] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 508, 511 ) ).mY - lon2[20] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 509, 511 ) ).mX - lat2[21] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 509, 511 ) ).mY - lon2[21] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 510, 511 ) ).mX - lat2[22] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 510, 511 ) ).mY - lon2[22] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 511, 511 ) ).mX - lat2[23] ) < 1e-3 );
      issea( fabs( pRasterElement->convertPixelToGeocoord( LocationType( 511, 511 ) ).mY - lon2[23] ) < 1e-3 );

      // close the data set
      issea(TestUtilities::destroyWorkspaceWindow(pWindow));

      return success;
   }
};

class GeoReferenceInverseTest : public TestCase
{
public:
   GeoReferenceInverseTest() : TestCase("GeoReferenceInverseTest")
   {
   }

   struct DataSpec
   {
      std::string mFilename;
      std::string mGeorefPlugin;
   };

   bool run()
   {      
      bool success = true;
      bool innersuccess = true;

      GeoReferenceInverseTest::DataSpec specs[] =
      {
         {TestUtilities::getTestDataPath() + "GeoReference/landsat6band.tif", "GCP Georeference"},
         {TestUtilities::getTestDataPath() + "GeoReference/nine_large-i2.ntf", "RPC Georeference"},
         { "", "" }
      };
      for (int i = 0; specs[i].mFilename.empty() == false; ++i)
      {
         printf("Loading %s\n", specs[i].mFilename.c_str());
         ImporterResource impResource("Auto Importer", specs[i].mFilename, NULL, false);
         impResource->execute();
         vector<DataElement*> elements = impResource->getImportedElements();
         isseab(elements.size() == 1);

         RasterElement *pRasterElement = dynamic_cast<RasterElement*>(elements.front());

         isseab(pRasterElement != NULL);

         issea( TestUtilities::runGeoRef( pRasterElement, specs[i].mGeorefPlugin ) );
         issea( pRasterElement->isGeoreferenced() );

         RasterDataDescriptor *pDd = 
            dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
         isseab(pDd != NULL);

         int xMax = pDd->getColumnCount() - 1;
         int yMax = pDd->getRowCount() - 1;

         // x is columns, y is rows
         float xPortions[] = {0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 1.0, -1.0}; // negatives to terminate
         float yPortions[] = {0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 1.0, -1.0}; // negative to terminate

         for (int i = 0; xPortions[i] >= 0 && yPortions[i] >= 0; ++i)
         {
            LocationType pix(xPortions[i]*xMax, yPortions[i]*yMax);
            LocationType geo = pRasterElement->convertPixelToGeocoord(pix);
            LocationType pix2 = pRasterElement->convertGeocoordToPixel(geo);

            // RPC seems to terminate upon finding a point within 0.1 pixels, but the
            // nine_large dataset gives up to 5e-6.
            innersuccess = ((fabs(pix.mX - pix2.mX) < 5e-6) && (fabs(pix.mY - pix2.mY) < 5e-6));
            if (!innersuccess)
            {
               double distx = pix.mX - pix2.mX;
               double disty = pix.mY - pix2.mY;
               double distance = sqrt(distx*distx+disty*disty);

               printf("  Point (%f,%f)->(%f,%f)\n    %f apart.\n", pix.mX, pix.mY, pix2.mX, pix2.mY, distance);
            }
         }
         WorkspaceWindow* pWindow = dynamic_cast<WorkspaceWindow*>(
            Service<DesktopServices>()->getWindow(pRasterElement->getName(), SPATIAL_DATA_WINDOW));
         issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      }
      issea(innersuccess);

      return success;
   }

};

class AutoGeoreferenceTest : public TestCase
{
public:
   AutoGeoreferenceTest() :
      TestCase("AutoGeoreference")
   {}

   bool run()
   {
      bool success = true;

      // Get the current settings
      bool autoGeoreference = GeoreferenceDescriptor::getSettingAutoGeoreference();
      bool createLayer = GeoreferenceDescriptor::getSettingCreateLayer();
      bool displayLayer = GeoreferenceDescriptor::getSettingDisplayLayer();
      GeocoordType geocoordType = GeoreferenceDescriptor::getSettingGeocoordType();
      DmsFormatType latLonFormat = GeoreferenceDescriptor::getSettingLatLonFormat();

      // Auto-georeference, create the lat/long layer, display the lat/long layer, lat/long display, DMS
      GeoreferenceDescriptor::setSettingAutoGeoreference(true);
      GeoreferenceDescriptor::setSettingCreateLayer(true);
      GeoreferenceDescriptor::setSettingDisplayLayer(true);
      GeoreferenceDescriptor::setSettingGeocoordType(GEOCOORD_LATLON);
      GeoreferenceDescriptor::setSettingLatLonFormat(DMS_FULL);
      issea(testGeoreference(true, true, true, GEOCOORD_LATLON, DMS_FULL));

      // Auto-georeference, create the lat/long layer, display the lat/long layer, lat/long display, decimal degrees
      GeoreferenceDescriptor::setSettingLatLonFormat(DMS_FULL_DECIMAL);
      issea(testGeoreference(true, true, true, GEOCOORD_LATLON, DMS_FULL_DECIMAL));

      // Auto-georeference, create the lat/long layer, display the lat/long layer, UTM display, decimal degrees
      GeoreferenceDescriptor::setSettingGeocoordType(GEOCOORD_UTM);
      issea(testGeoreference(true, true, true, GEOCOORD_UTM, DMS_FULL_DECIMAL));

      // Auto-georeference, create the lat/long layer, do not display the lat/long layer, UTM display, decimal degrees
      GeoreferenceDescriptor::setSettingDisplayLayer(false);
      issea(testGeoreference(true, true, false, GEOCOORD_UTM, DMS_FULL_DECIMAL));

      // Auto-georeference, do not create the lat/long layer, do not display the lat/long layer, UTM display,
      // decimal degrees
      GeoreferenceDescriptor::setSettingCreateLayer(false);
      issea(testGeoreference(true, false, false, GEOCOORD_UTM, DMS_FULL_DECIMAL));

      // Do not auto-georeference, create the lat/long layer, display the lat/long layer, UTM display, decimal degrees
      GeoreferenceDescriptor::setSettingAutoGeoreference(false);
      GeoreferenceDescriptor::setSettingCreateLayer(true);
      GeoreferenceDescriptor::setSettingDisplayLayer(true);
      issea(testGeoreference(false, false, false, GEOCOORD_UTM, DMS_FULL_DECIMAL));

      // Auto-georeference, create the lat/long layer, display the lat/long layer, lat/long display, DMS
      GeoreferenceDescriptor::setSettingAutoGeoreference(true);
      GeoreferenceDescriptor::setSettingDisplayLayer(false);
      GeoreferenceDescriptor::setSettingGeocoordType(GEOCOORD_LATLON);
      GeoreferenceDescriptor::setSettingLatLonFormat(DMS_FULL);

      // The settings now should be at their default values when entering this test
      issea(GeoreferenceDescriptor::getSettingAutoGeoreference() == true);
      issea(GeoreferenceDescriptor::getSettingCreateLayer() == true);
      issea(GeoreferenceDescriptor::getSettingDisplayLayer() == false);
      issea(GeoreferenceDescriptor::getSettingGeocoordType() == GEOCOORD_LATLON);
      issea(GeoreferenceDescriptor::getSettingLatLonFormat() == DMS_FULL);

      // Reset the settings back to the values before the test began
      GeoreferenceDescriptor::setSettingAutoGeoreference(autoGeoreference);
      GeoreferenceDescriptor::setSettingCreateLayer(createLayer);
      GeoreferenceDescriptor::setSettingDisplayLayer(displayLayer);
      GeoreferenceDescriptor::setSettingGeocoordType(geocoordType);
      GeoreferenceDescriptor::setSettingLatLonFormat(latLonFormat);

      return success;
   }

protected:
   bool testGeoreference(bool expectedGeorefResult, bool expectedLayerCreatedResult, bool expectedLayerDisplayResult,
      GeocoordType expectedGeocoordType, DmsFormatType expectedLatLonFormat)
   {
      bool success = true;
      string filename = TestUtilities::getTestDataPath() + "GeoReference/landsat6band.tif";

      SpatialDataWindow* pWindow = TestUtilities::loadDataSet(filename, "Auto Importer");
      issearf(pWindow != NULL);

      SpatialDataView* pView = pWindow->getSpatialDataView();
      issearf(pView != NULL);

      LayerList* pLayerList = pView->getLayerList();
      issearf(pLayerList != NULL);

      RasterElement* pRasterElement = pLayerList->getPrimaryRasterElement();
      issearf(pRasterElement != NULL);

      issea(pRasterElement->isGeoreferenced() == expectedGeorefResult);

      LatLonLayer* pLatLonLayer = dynamic_cast<LatLonLayer*>(pLayerList->getLayer(LAT_LONG, pRasterElement,
         "GEO_RESULTS"));
      issea((pLatLonLayer != NULL) == expectedLayerCreatedResult);
      if (pLatLonLayer != NULL)
      {
         issea(pView->isLayerDisplayed(pLatLonLayer) == expectedLayerDisplayResult);
         issea(pLatLonLayer->getGeocoordType() == expectedGeocoordType);
         issea(pLatLonLayer->getLatLonFormat() == expectedLatLonFormat);
      }

      issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      return success;
   }
};

class GeoReferenceTestSuite : public TestSuiteNewSession
{
public:
   GeoReferenceTestSuite() : TestSuiteNewSession( "GeoReference" )
   {
      addTestCase( new GeoReferenceInverseTest );
      addTestCase( new GeoReferenceCornerAndCenterTest );
      addTestCase( new GeoReferenceTest );
      addTestCase( new GeoReferenceOrientationTest );
      addTestCase( new AutoGeoreferenceTest );
   }
};

REGISTER_SUITE( GeoReferenceTestSuite )
