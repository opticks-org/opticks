/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include "AoiElement.h"
#include "AppConfig.h"
#include "assert.h"
#include "Classification.h"
#include "ConfigurationSettingsImp.h"
#include "DateTimeImp.h"
#include "DesktopServices.h"
#include "Endian.h"
#include "GcpList.h"
#include "ModelServicesImp.h"
#include "ObjectResource.h"
#include "Observer.h"
#include "PlugInDescriptor.h"
#include "PlugInManagerServicesImp.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterFileDescriptor.h"
#include "RasterLayer.h"
#include "RasterUtilities.h"
#include "SafeSlot.h"
#include "SignatureDataDescriptor.h"
#include "SignatureFileDescriptor.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "SignalBlocker.h"
#include "StringUtilities.h"
#include "SubjectAdapter.h"
#include "TestBedTestUtilities.h"
#include "TestCase.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"
#include "TiePointList.h"
#include "TimeUtilities.h"
#include "Units.h"
#include "xmlbase.h"

#include <algorithm>
#include <iostream>
using namespace std;

vector<string> descriptorUpdates;
vector<string> modelUpdates;
vector<string> dataElementUpdates;
vector<string> desktopUpdates;

class ConfigurationSettingsLookupTest : public TestCase
{
public:
   ConfigurationSettingsLookupTest() : TestCase( "Lookup" ) {}
   bool run()
   {
      bool success = true;

      DataVariant var;
      var = Service<ConfigurationSettings>()->getSetting( "Progress/AutoClose" );
      issea( var.getTypeName() == "bool" );
      issea( var.isValid() ); // may be set differently depending on the user, so I can't compare real values
      
      var = Service<ConfigurationSettings>()->getSetting( "SessionManager/QueryForSave" );
      issea( var.getTypeName() == "SessionSaveType" );
      issea( var.isValid() ); // may be set differently depending on the user, so I can't compare real values

      var = Service<ConfigurationSettings>()->getSetting( "group/key" );
      issea( var.getTypeName() == "void" );
      issea( var.isValid() == false );

      return success;
   }
};

class PluginCheckTest : public TestCase
{
public:
   PluginCheckTest() : TestCase("PlugInCheck") {}
   bool run()
   {
      bool success = true;
      
      // populate a map with the names of all the required plugins
      // name, whether its been visited
      map<string, bool> requiredNames;
      requiredNames["ACFTA"] = false;
      requiredNames["ACFTA"] = false;
      requiredNames["ACFTB"] = false;
      requiredNames["AIMIDA"] = false;
      requiredNames["AIMIDB"] = false;
      requiredNames["Animation Options"] = false;
      requiredNames["Annotation Image Palette"] = false;
      requiredNames["Annotation Image Palette Options"] = false;
      requiredNames["Annotation Layer Exporter"] = false;
      requiredNames["Annotation Layer Options"] = false;
      requiredNames["Annotation Layer Properties"] = false;
      requiredNames["Annotation Model Exporter"] = false;
      requiredNames["AOI Layer Exporter"] = false;
      requiredNames["AOI Layer Options"] = false;
      requiredNames["AOI Layer Properties"] = false;
      requiredNames["AOI Model Exporter"] = false;
      requiredNames["ASPAM Importer"] = false;
      requiredNames["ASPAM Manager"] = false;
      requiredNames["ASPAM Viewer"] = false;
      requiredNames["Auto Importer"] = false;
      requiredNames["AVI Product Exporter"] = false;
      requiredNames["Band Binning"] = false;
      requiredNames["Band Math"] = false;
      requiredNames["BANDSA"] = false;
      requiredNames["BANDSB"] = false;
      requiredNames["Batch Wizard Executor"] = false;
      requiredNames["Bitmap Plot Widget Exporter"] = false;
      requiredNames["Bitmap View Exporter"] = false;
      requiredNames["BLOCKA"] = false;
      requiredNames["Bmp Exporter Options"] = false;
      requiredNames["CGM Exporter"] = false;
      requiredNames["CGM Importer"] = false;
      requiredNames["Change Up Direction"] = false;
      requiredNames["Classification Properties"] = false;
      requiredNames["CMETAA"] = false;
#if defined(OPENCOLLADA_SUPPORT)
      requiredNames["COLLADA Exporter"] = false;
      requiredNames["COLLADA Importer"] = false;
#endif
      requiredNames["Connected Components"] = false;
      requiredNames["Convolution Matrix Editor"] = false;
      requiredNames["Copyright Information"] = false;
      requiredNames["Covariance"] = false;
      requiredNames["Create Animation"] = false;
      requiredNames["Create Export File Descriptor"] = false;
      requiredNames["Create File Descriptor"] = false;
      requiredNames["Create Raster File Descriptor"] = false;
      requiredNames["Create Signature File Descriptor"] = false;
      requiredNames["Data Descriptor Properties"] = false;
      requiredNames["Data Fusion"] = false;
      requiredNames["Derive Layer"] = false;
      requiredNames["Derive Product"] = false;
      requiredNames["Dock Window Options"] = false;
      requiredNames["DTED Importer"] = false;
      requiredNames["Edit Data Descriptor"] = false;
      requiredNames["ENGRDA"] = false;
      requiredNames["ENVI Exporter"] = false;
      requiredNames["ENVI Importer"] = false;
      requiredNames["ENVI Library Exporter"] = false;
      requiredNames["ENVI Library Importer"] = false;
      requiredNames["Export Data Set"] = false;
      requiredNames["EXOPTA"] = false;
      requiredNames["EXPLTA"] = false;
      requiredNames["EXPLTB"] = false;
      requiredNames["Feature Class Properties"] = false;
      requiredNames["Feature Layer Exporter"] = false;
      requiredNames["Feature Manager"] = false;
      requiredNames["File Descriptor Properties"] = false;
      requiredNames["File Location Options"] = false;
      requiredNames["FITS Importer"] = false;
      requiredNames["FITS Signature Importer"] = false;
      requiredNames["FitsRasterPager"] = false;
      requiredNames["Flatten Annotation Layer"] = false;
      requiredNames["Flicker Controls"] = false;
      requiredNames["Full Screen Options"] = false;
      requiredNames["GCP Georeference"] = false;
      requiredNames["Gcp Layer Options"] = false;
      requiredNames["GCP Layer Properties"] = false;
      requiredNames["GCP List Layer Exporter"] = false;
      requiredNames["GCP List Model Exporter"] = false;
      requiredNames["GDAL Raster Pager"] = false;
      requiredNames["General Options"] = false;
      requiredNames["Generic Convolution"] = false;
      requiredNames["Generic GDAL Importer"] = false;
      requiredNames["Generic Importer"] = false;
      requiredNames["Generic HDF5 Importer"] = false;
      requiredNames["Geographic Feature Layer Importer"] = false;
      requiredNames["Geographic Features Options"] = false;
      requiredNames["Geographic Features Window"] = false;
      requiredNames["GeoMosaic"] = false;
      requiredNames["Georeference"] = false;
      requiredNames["Georeference Options"] = false;
      requiredNames["Georeference Properties"] = false;
      requiredNames["GeoTIFF Exporter"] = false;
      requiredNames["GeoTIFF Importer"] = false;
      requiredNames["GeoTIFF Plot Widget Exporter"] = false;
      requiredNames["GeoTIFF View Exporter"] = false;
      requiredNames["GeoTiffPager"] = false;
      requiredNames["Get AnnotationElement"] = false;
      requiredNames["Get AnnotationLayer"] = false;
      requiredNames["Get Any"] = false;
      requiredNames["Get AoiElement"] = false;
      requiredNames["Get AoiLayer"] = false;
      requiredNames["Get CartesianPlot"] = false;
      requiredNames["Get ClassificationLayer"] = false;
      requiredNames["Get CustomLayer"] = false;
      requiredNames["Get Data Descriptor"] = false;
      requiredNames["Get DataElement"] = false;
      requiredNames["Get DataElementGroup"] = false;
      requiredNames["Get Data Set"] = false;
      requiredNames["Get Data Set Wavelengths"] = false;
      requiredNames["Get Existing Filename"] = false;
      requiredNames["Get Existing Filenames"] = false;
      requiredNames["Get GcpLayer"] = false;
      requiredNames["Get GcpList"] = false;
      requiredNames["Get GraphicElement"] = false;
      requiredNames["Get GraphicLayer"] = false;
      requiredNames["Get HistogramPlot"] = false;
      requiredNames["Get LatLonLayer"] = false;
      requiredNames["Get Layer"] = false;
      requiredNames["Get MeasurementLayer"] = false;
      requiredNames["Get New Filename"] = false;
      requiredNames["Get OrthographicView"] = false;
      requiredNames["Get PerspectiveView"] = false;
      requiredNames["Get PlotView"] = false;
      requiredNames["Get PlotWidget"] = false;
      requiredNames["Get PointCloudElement"] = false;
      requiredNames["Get PolarPlot"] = false;
      requiredNames["Get Primary Raster Layer"] = false;
      requiredNames["Get ProductView"] = false;
      requiredNames["Get PseudocolorLayer"] = false;
      requiredNames["Get RasterElement"] = false;
      requiredNames["Get RasterLayer"] = false;
      requiredNames["Get Signature"] = false;
      requiredNames["Get SignatureLibrary"] = false;
      requiredNames["Get SignaturePlot"] = false;
      requiredNames["Get SignatureSet"] = false;
      requiredNames["Get SpatialDataView"] = false;
      requiredNames["Get ThresholdLayer"] = false;
      requiredNames["Get TiePointLayer"] = false;
      requiredNames["Get TiePointList"] = false;
      requiredNames["Get View"] = false;
      requiredNames["Graphic Object Properties"] = false;
      requiredNames["Hdf4Pager"] = false;
      requiredNames["Hdf5Pager"] = false;
      requiredNames["Histogram Plot Properties"] = false;
      requiredNames["Hyperion Importer"] = false;
      requiredNames["ICHIPB"] = false;
      requiredNames["Ice Exporter"] = false;
      requiredNames["Ice Exporter Options"] = false;
      requiredNames["Ice Importer"] = false;
      requiredNames["Ice PseudocolorLayer Exporter"] = false;
      requiredNames["Ice PseudocolorLayer Importer"] = false;
      requiredNames["Ice Threshold Layer Exporter"] = false;
      requiredNames["Ice Threshold Layer Importer"] = false;
      requiredNames["IGM Georeference"] = false;
      requiredNames["Import Data Set"] = false;
      requiredNames["In Memory Pager"] = false;
      requiredNames["J2KLRA"] = false;
      requiredNames["JPEG Plot Widget Exporter"] = false;
      requiredNames["JPEG View Exporter"] = false;
      requiredNames["Jpeg Exporter Options"] = false;
      requiredNames["JPEG2000 Exporter"] = false;
      requiredNames["JPEG2000 Importer"] = false;
      requiredNames["JPEG2000 Pager"] = false;
      requiredNames["Keyboard Shortcut Options"] = false;
      requiredNames["KML Exporter"] = false;
      requiredNames["KML HTTP Server"] = false;
      requiredNames["KML Layer Exporter"] = false;
      requiredNames["LAS Importer"] = false;
      requiredNames["Latitude/Longitude Layer Properties"] = false;
      requiredNames["Lat/Lon Layer Options"] = false;
      requiredNames["Layer Importer"] = false;
      requiredNames["Layer Options"] = false;
      requiredNames["Load Annotation"] = false;
      requiredNames["Load AOI"] = false;
      requiredNames["Load GCP List"] = false;
      requiredNames["Load Template"] = false;
      requiredNames["Measurement Layer Options"] = false;
      requiredNames["Measurement Layer Properties"] = false;
      requiredNames["Measurement Object Properties"] = false;
      requiredNames["MemoryMappedPager"] = false;
      requiredNames["MENSRA"] = false;
      requiredNames["MENSRB"] = false;
      requiredNames["Metadata Properties"] = false;
      requiredNames["MOD26A"] = false;
      requiredNames["MODIS Georeference"] = false;
      requiredNames["MODIS L1B Importer"] = false;
      requiredNames["MODIS L1B Importer Options"] = false;
      requiredNames["MODIS Pager"] = false;
      requiredNames["Morphological Close"] = false;
      requiredNames["Morphological Dilation"] = false;
      requiredNames["Morphological Erosion"] = false;
      requiredNames["Morphological Open"] = false;
      requiredNames["Mosaic Chip"] = false;
      requiredNames["Mosaic Manager"] = false;
      requiredNames["MPD26A"] = false;
      requiredNames["Metadata Exporter"] = false;
      requiredNames["Model Importer"] = false;
      requiredNames["Module Properties"] = false;
      requiredNames["Movie Product Exporter Options"] = false;
      requiredNames["MJPEG Product Exporter"] = false;
      requiredNames["MPEG-1 Product Exporter"] = false;
      requiredNames["NITF DES Exporter"] = false;
      requiredNames["NITF Exporter"] = false;
      requiredNames["NITF Importer"] = false;
      requiredNames["NITF Properties Manager"] = false;
      requiredNames["NitfPager"] = false;
      requiredNames["OSSIM Services"] = false;
      requiredNames["Overview Window Options"] = false;
      requiredNames["PATCHA"] = false;
      requiredNames["PATCHB"] = false;
      requiredNames["Plot View Properties"] = false;
      requiredNames["Plug-In Descriptor Properties"] = false;
      requiredNames["Png Exporter Options"] = false;
      requiredNames["PNG Plot Widget Exporter"] = false;
      requiredNames["PNG View Exporter"] = false;
      requiredNames["Point Cloud View Properties"] = false;
      requiredNames["PointCloud In Memory Pager"] = false;
      requiredNames["PointCloud MemoryMappedPager"] = false;
      requiredNames["PostScript Exporter"] = false;
      requiredNames["Principal Component Analysis"] = false;
      requiredNames["Print View"] = false;
      requiredNames["Product View Properties"] = false;
      requiredNames["Properties Dialog"] = false;
      requiredNames["Properties ENGRDA"] = false;
      requiredNames["Pseudocolor Layer Options"] = false;
      requiredNames["Pseudocolor Layer Properties"] = false;
      requiredNames["QT Cluster"] = false;
      requiredNames["RADSDA"] = false;
      requiredNames["Raster Layer Options"] = false;
      requiredNames["Raster Layer Properties"] = false;
      requiredNames["REFLNA"] = false;
      requiredNames["Results Exporter"] = false;
      requiredNames["RPC00A"] = false;
      requiredNames["RPC00B"] = false;
      requiredNames["RPC Georeference"] = false;
      requiredNames["Run Interpreter Commands"] = false;
      requiredNames["RunScript"] = false;
      requiredNames["Save Annotation"] = false;
      requiredNames["Save AOI"] = false;
      requiredNames["Save AOI From Data Set"] = false;
      requiredNames["Save GCP List"] = false;
      requiredNames["Save GCP List From Data Set"] = false;
      requiredNames["Save Template"] = false;
      requiredNames["Scripting Window Properties"] = false;
      requiredNames["Second Moment"] = false;
      requiredNames["SECTGA"] = false;
      requiredNames["Select Plug-In"] = false;
      requiredNames["SENSRA"] = false;
      requiredNames["SENSRB"] = false;
      requiredNames["Session Options"] = false;
      requiredNames["Set Data Set Wavelengths"] = false;
      requiredNames["Set Displayed Band"] = false;
      requiredNames["Set Display Mode"] = false;
      requiredNames["Set Threshold Options"] = false;
      requiredNames["Set View Display Area"] = false;
      requiredNames["Shape File Exporter"] = false;
      requiredNames["Shape File Importer"] = false;
      requiredNames["SIO Importer"] = false;
      requiredNames["Spatial Data View Options"] = false;
      requiredNames["Spatial Data View Properties"] = false;
      requiredNames["Spatial Resampler"] = false;
      requiredNames["Spatial Resampler Options"] = false;
      requiredNames["Status Bar Options"] = false;
      requiredNames["Suppressible Dialog Options"] = false;
      requiredNames["STDIDB"] = false;
      requiredNames["STDIDC"] = false;
      requiredNames["Threshold Data"] = false;
      requiredNames["Threshold Layer Options"] = false;
      requiredNames["Threshold Layer Properties"] = false;
      requiredNames["Tie Point Layer Options"] = false;
      requiredNames["Tie Point Layer Properties"] = false;
      requiredNames["Tie Point List Layer Exporter"] = false;
      requiredNames["Tie Point List Model Exporter"] = false;
      requiredNames["TIFF/GeoTIFF Exporter Options"] = false;
      requiredNames["Tool Bar Options"] = false;
      requiredNames["Unknown Tre Parser"] = false;
      requiredNames["USE00A"] = false;
      requiredNames["USE26A"] = false;
      requiredNames["View Options"] = false;
      requiredNames["View Properties"] = false;
      requiredNames["Wavelength Metadata Exporter"] = false;
      requiredNames["Wavelength Metadata Importer"] = false;
      requiredNames["Wavelength Properties"] = false;
      requiredNames["Wavelength Text Exporter"] = false;
      requiredNames["Wavelength Text Importer"] = false;
      requiredNames["Wizard Executor"] = false;

      // Build a list of all optional plugins (i.e., plugins from the sampler)
      set<string> optionalNames;
      optionalNames.insert("AnimationTest Dialog");
      optionalNames.insert("Animation Timing Test Dialog");
      optionalNames.insert("Any Plug-In");
      optionalNames.insert("Background Execution Test");
      optionalNames.insert("Close Notification Test");
      optionalNames.insert("Custom Data Element Plug-In");
      optionalNames.insert("CustomLayerPlugIn");
      optionalNames.insert("Custom Mouse Mode Plug-In");
      optionalNames.insert("Data Plotter");
      optionalNames.insert("Demo");
      optionalNames.insert("Desktop API Test");
      optionalNames.insert("Desktop API Test Properties");
      optionalNames.insert("Dummy Custom Algorithm");
      optionalNames.insert("Dummy Custom Importer");
      optionalNames.insert("Log Context Menu Actions");
      optionalNames.insert("Menu And ToolBar Test");
      optionalNames.insert("Message Log Test");
      optionalNames.insert("Modeless Dialog");
      optionalNames.insert("Mouse Mode Test");
      optionalNames.insert("Multi Movie");
      optionalNames.insert("Multi-Layer Movie");
      optionalNames.insert("Passthrough PlugIn");
      optionalNames.insert("Pixel Aspect Ratio Test");
      optionalNames.insert("Plot Manager");
      optionalNames.insert("Plug-In Tester");
      optionalNames.insert("Plug-in Sampler example options");
      optionalNames.insert("Point Cloud Histogram");
      optionalNames.insert("Example Suppressible Dialog Options");
      optionalNames.insert("Raster Timing Test");
      optionalNames.insert("Sample MODIS HDF4-EOS Importer");
      optionalNames.insert("Sample RasterElement Importer");
      optionalNames.insert("Sample Scriptor");
      optionalNames.insert("SampleGeoref");
      optionalNames.insert("Start Scriptor Menu Item");
      optionalNames.insert("Tie Point Tester");
      optionalNames.insert("Tutorial 1");
      optionalNames.insert("Tutorial 2");
      optionalNames.insert("Tutorial 3");
      optionalNames.insert("Tutorial 4");
      optionalNames.insert("Tutorial 5");
      optionalNames.insert("XML-RPC Server");

      vector<PlugInDescriptor*> foundPlugIns = Service<PlugInManagerServices>()->getPlugInDescriptors();

      for (vector<PlugInDescriptor*>::const_iterator iter = foundPlugIns.begin();
         iter != foundPlugIns.end(); ++iter)
      {
         PlugInDescriptor *pDescriptor = *iter;
         if (pDescriptor == NULL) // allow loop to continue even if success is false.
         {
            isseac(false);
         }

         map<string, bool>::iterator nameIter = requiredNames.find(pDescriptor->getName());
         if (nameIter == requiredNames.end())
         {
            if (optionalNames.find(pDescriptor->getName()) == optionalNames.end())
            {
               printf("The plugin %s was found but not allowed.\n", pDescriptor->getName().c_str());
               isseac(false);
            }
         }
         else
         {
            if (nameIter->second == true)
            {
               printf("The plugin %s was a duplicate.\n", pDescriptor->getName().c_str());
               isseac(false);
            }

            nameIter->second = true;
         }
      }

      for (map<string, bool>::const_iterator iter = requiredNames.begin();
         iter != requiredNames.end(); ++iter)
      {
         if (iter->second == false)
         {
            printf("The plugin %s was not found.\n", iter->first.c_str());
            isseac(false);
         }
      }

      return success;
   }
};


class TimeUtilitiesTestCase : public TestCase
{
public:
   TimeUtilitiesTestCase() :
      TestCase("TimeUtilities")
   {}

   bool run();
};

bool TimeUtilitiesTestCase::run()
{
   bool success = true;
   struct tm timeStruct;
   struct tm timeStruct2;

   // Test 1: out of range dates
   timeStruct.tm_year = 38;
   success = success && tst_assert(TimeUtilities::timeStructToSecondsFrom1940(timeStruct) == 0);
   timeStruct.tm_year = 183;
   success = success && tst_assert(TimeUtilities::timeStructToSecondsFrom1940(timeStruct) == 0);

   // Test 2: 3:35:08pm, March 5, 1984 = 1,394,120,108
   timeStruct.tm_hour = 15;
   timeStruct.tm_min = 35;
   timeStruct.tm_sec = 8;
   timeStruct.tm_mon = 2;
   timeStruct.tm_mday = 5;
   timeStruct.tm_year = 84;
   unsigned long ulResult = TimeUtilities::timeStructToSecondsFrom1940(timeStruct);
   success = success && tst_assert(ulResult == 1394120108);

   // Test 3: 1394120108 = 3:35:08pm, Monday March 5, 1984
   timeStruct2 = TimeUtilities::secondsFrom1940ToTimeStruct(ulResult);
   success = success && tst_assert(timeStruct2.tm_hour == 15);
   success = success && tst_assert(timeStruct2.tm_min == 35);
   success = success && tst_assert(timeStruct2.tm_sec == 8);
   success = success && tst_assert(timeStruct2.tm_year == 84);
   success = success && tst_assert(timeStruct2.tm_yday == 64);
   success = success && tst_assert(timeStruct2.tm_isdst == 0);
   success = success && tst_assert(timeStruct2.tm_mday == 5);
   success = success && tst_assert(timeStruct2.tm_mon == 2);
   success = success && tst_assert(timeStruct2.tm_wday == 1); // Monday

   // Test 4: 3:35:08pm, Feb 29, 1984 = 1,393,688,108
   timeStruct.tm_hour = 15;
   timeStruct.tm_min = 35;
   timeStruct.tm_sec = 8;
   timeStruct.tm_mon = 1;
   timeStruct.tm_mday = 29;
   timeStruct.tm_year = 84;
   ulResult = TimeUtilities::timeStructToSecondsFrom1940(timeStruct);
   success = success && tst_assert(ulResult == 1393688108);

   // Test 5: 1393688108 = 3:35:08pm, Wednesday Feb 29, 1984
   timeStruct2 = TimeUtilities::secondsFrom1940ToTimeStruct(ulResult);
   success = success && tst_assert(timeStruct2.tm_hour == 15);
   success = success && tst_assert(timeStruct2.tm_min == 35);
   success = success && tst_assert(timeStruct2.tm_sec == 8);
   success = success && tst_assert(timeStruct2.tm_year == 84);
   success = success && tst_assert(timeStruct2.tm_yday == 59);
   success = success && tst_assert(timeStruct2.tm_isdst == 0);
   success = success && tst_assert(timeStruct2.tm_mday == 29);
   success = success && tst_assert(timeStruct2.tm_mon == 1);
   success = success && tst_assert(timeStruct2.tm_wday == 3);

   // Test 6: 3:35:08pm, Mar 1, 1984 = 1,393,688,108
   timeStruct.tm_hour = 15;
   timeStruct.tm_min = 35;
   timeStruct.tm_sec = 8;
   timeStruct.tm_mon = 2;
   timeStruct.tm_mday = 1;
   timeStruct.tm_year = 84;
   ulResult = TimeUtilities::timeStructToSecondsFrom1940(timeStruct);
   success = success && tst_assert(ulResult == 1393774508);

   // Test 7: 1393774508 = 3:35:08pm, Thursday Mar 1, 1984
   timeStruct2 = TimeUtilities::secondsFrom1940ToTimeStruct(ulResult);
   success = success && tst_assert(timeStruct2.tm_hour == 15);
   success = success && tst_assert(timeStruct2.tm_min == 35);
   success = success && tst_assert(timeStruct2.tm_sec == 8);
   success = success && tst_assert(timeStruct2.tm_year == 84);
   success = success && tst_assert(timeStruct2.tm_yday == 60);
   success = success && tst_assert(timeStruct2.tm_isdst == 0);
   success = success && tst_assert(timeStruct2.tm_mday == 1);
   success = success && tst_assert(timeStruct2.tm_mon == 2);
   success = success && tst_assert(timeStruct2.tm_wday == 4);

   // Test 8: 3:35:08pm, Apr 1, 1984 = 1,396,452,908
   timeStruct.tm_hour = 15;
   timeStruct.tm_min = 35;
   timeStruct.tm_sec = 8;
   timeStruct.tm_mon = 3;
   timeStruct.tm_mday = 1;
   timeStruct.tm_year = 84;
   ulResult = TimeUtilities::timeStructToSecondsFrom1940(timeStruct);
   success = success && tst_assert(ulResult == 1396452908);

   // Test 9: 1396452908 = 3:35:08pm, Thursday Apr 1, 1984
   timeStruct2 = TimeUtilities::secondsFrom1940ToTimeStruct(ulResult);
   success = success && tst_assert(timeStruct2.tm_hour == 15);
   success = success && tst_assert(timeStruct2.tm_min == 35);
   success = success && tst_assert(timeStruct2.tm_sec == 8);
   success = success && tst_assert(timeStruct2.tm_year == 84);
   success = success && tst_assert(timeStruct2.tm_yday == 91);
   success = success && tst_assert(timeStruct2.tm_isdst == 0);
   success = success && tst_assert(timeStruct2.tm_mday == 1);
   success = success && tst_assert(timeStruct2.tm_mon == 3);
   success = success && tst_assert(timeStruct2.tm_wday == 0);

   // Test 10: Ensure consistency of results with mktime
   unsigned long ulResult1970 = ulResult - TimeUtilities::TimeScaleOffset;
   time_t mkResult = mktime(&timeStruct2) - 5 * 3600; // EST to GMT difference = 5 hours
   success = success && tst_assert(ulResult1970 == mkResult);

   timeStruct2.tm_year = 50;
   ulResult = TimeUtilities::timeStructToSecondsFrom1940(timeStruct2);
   timeStruct2 = TimeUtilities::secondsFrom1940ToTimeStruct(ulResult); // updates the mday and wday fields
   char buffer[256];
   strftime(buffer, 256, "%A, day %d of %B in the year %Y.", &timeStruct2);
   success = success && tst_assert(strcmp(buffer, "Saturday, day 01 of April in the year 1950.") == 0);

   return success;
}

class DateTimeTestCase : public TestCase
{
public:
   DateTimeTestCase() : TestCase("DateTime") {}
   bool run();
};

bool DateTimeTestCase::run()
{
   bool success = true;
   {
      DateTimeImp dt1, dt2;

      success = success && tst_assert( dt1.isValid() == false );
      success = success && tst_assert( dt2.isValid() == false );

      dt1.set( 1984, 3, 5, 15, 35, 8 );
      success = success && tst_assert( dt1.getStructured() == 447348908 );

      dt2.set( 1984, 3, 6, 15, 35, 8 );
      success = success && tst_assert( dt2.getSecondsSince( dt1 ) == 86400.0 );

      string format = "%m/%d/%Y";      
      dt1.set( 1944, 3, 5, 15, 35, 8 );
      string buffer = dt1.getFormattedUtc( format);
      success = success && tst_assert( buffer == "03/05/1944" );

      dt2 = dt1;
      success = success && tst_assert( dt1.getStructured() == dt2.getStructured() );
      DateTimeImp dt3( dt1 );
      success = success && tst_assert( dt1.getStructured() == dt3.getStructured() );
      DateTimeImp dt4( dt1.getStructured() );
      success = success && tst_assert( dt1.getStructured() == dt4.getStructured() );
      success = success && tst_assert( dt1 == dt4 );

      DateTimeImp dt5, dt6, dt7;
      dt5.set( 1984, 3, 5 );
      dt6 = dt5;
      success = success && tst_assert( dt5.operator==( dt6 ) );
      success = success && tst_assert( dt5.isTimeValid() == false );
      success = success && tst_assert( dt1.isTimeValid() == true );
      success = success && tst_assert( dt2.isTimeValid() == true );
      success = success && tst_assert( dt6.isTimeValid() == false );
      dt7.set( 1984, 3, 5, 0, 0, 0 );
      success = success && tst_assert( dt5.operator!=( dt7 ) );
      success = success && tst_assert( !dt5.operator==( dt7 ) );

      unsigned long sam = static_cast<unsigned long>( time( NULL ) );
      dt1.setToCurrentTime();
      unsigned long structured = static_cast<unsigned long>( dt1.getStructured() );
      success = success && tst_assert( structured == sam );
   } // dt1-dt4 get destructed here to test the dtor

   return success;
}

class CustomSubject : public SubjectAdapter
{
public:
   void notify(const string &signal, const boost::any &v=boost::any())
   {
      SubjectAdapter::notify(signal, v);
   }
   const list<SafeSlot>& getSlots(const string &signal)
   {
      return SubjectImp::getSlots(signal);
   }
   ~CustomSubject()
   {
      notify(Subject::signalDeleted());
   }
};

class CustomObserver1 : public Observer
{
public:
   CustomObserver1(vector<string> & actions) : mActions(actions)
   {
   }

   void update(Subject &subject, const string &signal, const boost::any &v)
   {
      mActions.push_back(signal);
      if (signal == Subject::signalModified())
      {
         bool success = true;

         // leaves an empty slot in the subject
         issea(subject.detach(Subject::signalModified(), Slot(this, &CustomObserver1::update)) == true);

         // verifies that we don't detect the slot on a subsequent attempt to detach
         issea (subject.detach(Subject::signalModified(), Slot(this, &CustomObserver1::update)) == false);
      }
   }

   void attached(Subject &subject, const string &signal, const Slot &slot)
   {
      update(subject, "Attached", boost::any());
   }
   void detached(Subject &subject, const string &signal, const Slot &slot)
   {
      update(subject, "Detached", boost::any());
   }
private:
   vector<string> &mActions;
};

class CustomObserver2 : public Observer
{
public:
   CustomObserver2(CustomObserver1 *pObs, vector<string> & actions) : mpObserver1(pObs), mActions(actions)
   {
   }

   void update(Subject &subject, const string &signal, const boost::any &v)
   {
      mActions.push_back(signal);
      if (signal == Subject::signalModified())
      {
         if (mpObserver1)
         {
            mActions.push_back("Deleting mpObserver1");
            delete mpObserver1;
            mpObserver1 = NULL;
         }
      }
   }

   void attached(Subject &subject, const string &signal, const Slot &slot)
   {
      update(subject, "Attached", boost::any());
   }
   void detached(Subject &subject, const string &signal, const Slot &slot)
   {
      update(subject, "Detached", boost::any());
   }
private:
   CustomObserver1 *mpObserver1;
   vector<string> &mActions;
};

class CustomObserver3
{
public:
   CustomObserver3(Subject *pSubject) : mNotifyCount(0)
   {
      pSubject->attach(Subject::signalModified(), Slot(this, &CustomObserver3::update));
   }
   virtual void virtualUpdate(Subject &subject, const string &signal, const boost::any &v)
   {
      ++mNotifyCount;
   }
   void update(Subject &subject, const string &signal, const boost::any &v)
   {
      virtualUpdate(subject, signal, v);
   }

   int getNotifyCount() const
   {
      return mNotifyCount;
   }

private:
   int mNotifyCount;
};

class CustomObserver4: public CustomObserver3
{
public:
   CustomObserver4(Subject *pSubject) : CustomObserver3(pSubject)
   {
   }
   void virtualUpdate(Subject &subject, const string &signal, const boost::any &v)
   {
      CustomObserver3::virtualUpdate(subject, signal, v);
      static bool first = true;
      subject.detach(Subject::signalModified(), 
         Slot(static_cast<CustomObserver3*>(this), &CustomObserver3::update));
      if (first)
      {
         subject.attach(Subject::signalModified(), 
            Slot(static_cast<CustomObserver3*>(this), &CustomObserver3::update));
      }
      first = false;
   }
};

class SubjectObserverTest  : public TestCase, public Observer
{
public:
   SubjectObserverTest() : TestCase("SubjectObserver") {}
   bool run()
   {
      bool success = true;

      //TEST1 - Test a detach while in the Observer update method
      mNotifiedCount = 0;
      CustomSubject* pSubj = new CustomSubject();
      pSubj->attach(Subject::signalModified(), Slot(this, &SubjectObserverTest::update));
      issea( mNotifiedCount == 1 ); //attach
      pSubj->notify(Subject::signalModified());
      pSubj->detach(Subject::signalModified(), Slot(this, &SubjectObserverTest::update));
      issea( mNotifiedCount == 3 ); //notify and detach

      //since, we detached during last notify this 2nd notify
      //should do nothing and should not update mNotifiedCount
      pSubj->notify(Subject::signalDeleted()); 
      issea( mNotifiedCount == 3 ); //already detached
      delete pSubj;

      //TEST2 - Do a normal detach and make sure no longer notified.
      mNotifiedCount = 0;
      pSubj = new CustomSubject();
      pSubj->attach(Subject::signalModified(), Slot(this, &SubjectObserverTest::update));
      pSubj->attach(Subject::signalDeleted(), Slot(this, &SubjectObserverTest::update));
      issea( mNotifiedCount == 2 ); //attaches
      pSubj->notify(Subject::signalDeleted());
      issea( mNotifiedCount == 3 ); //deleted
      pSubj->detach(Subject::signalDeleted(), Slot(this, &SubjectObserverTest::update));
      issea( mNotifiedCount == 4 ); //detach

      //since, we have detached this should do nothing.
      pSubj->notify(Subject::signalDeleted());
      issea( mNotifiedCount == 4 ); //already detached
      delete pSubj;

      //TEST3 - Attempt to attach twice, make sure only attached once
      mNotifiedCount = 0;
      pSubj = new CustomSubject();
      pSubj->attach(Subject::signalModified(), Slot(this, &SubjectObserverTest::update));
      issea( mNotifiedCount == 1 ); //attach
      pSubj->attach(Subject::signalModified(), Slot(this, &SubjectObserverTest::update));
      issea( mNotifiedCount == 1 ); //already attached
      pSubj->notify(Subject::signalDeleted());
      issea( mNotifiedCount == 1 ); //not attached to deleted
      pSubj->detach(Subject::signalModified(), Slot(this, &SubjectObserverTest::update));
      issea( mNotifiedCount == 2 ); //detach, but only once
      delete pSubj;

      //TEST4 - Make sure not notified during Subject destruction
      mNotifiedCount = 0;
      pSubj = new CustomSubject();
      pSubj->attach(Subject::signalModified(), Slot(this, &SubjectObserverTest::update));
      issea( mNotifiedCount == 1 ); //attach
      delete pSubj;
      issea( mNotifiedCount == 1 ); //no notification on delete

      //TEST5 - Test a detach & delete while in the Observer update method
      mNotifiedCount = 0;
      vector<string> actions;
      pSubj = new CustomSubject();
      CustomObserver1 *pObs1 = new CustomObserver1(actions);
      CustomObserver2 *pObs2 = new CustomObserver2(pObs1, actions);
      pSubj->attach(Subject::signalModified(), Slot(pObs1, &CustomObserver1::update));
      pSubj->attach(Subject::signalModified(), Slot(pObs2, &CustomObserver2::update));
      issea(pSubj->getSlots(Subject::signalModified()).size() == 2);
      pSubj->notify(Subject::signalModified()); // detaches & then deletes pObs1
      issea(pSubj->getSlots(Subject::signalModified()).size() == 1);
      pSubj->detach(Subject::signalModified(), Slot(pObs2, &CustomObserver2::update));
      pSubj->notify(Subject::signalModified()); // should do nothing
      issea(pSubj->getSlots(Subject::signalModified()).size() == 0);
      delete pSubj;
      delete pObs2;
      issea(actions.size() == 7);
      if (success)
      {
         issea(actions[0] == "Attached");
         issea(actions[1] == "Attached");
         issea(actions[2] == SIGNAL_NAME(Subject, Modified));
         issea(actions[3] == "Detached");
         issea(actions[4] == SIGNAL_NAME(Subject, Modified));
         issea(actions[5] == "Deleting mpObserver1");
         issea(actions[6] == "Detached");
      }

      //TEST6 - Detach and re-attach during notification
      pSubj = new CustomSubject();
      CustomObserver4 *pObs4 = new CustomObserver4(pSubj);
      issea(pSubj->getSlots(Subject::signalModified()).size() == 1);
      issea(pObs4->getNotifyCount() == 0);
      pSubj->notify(Subject::signalModified()); // will detach/reattach
      issea_ext1(pObs4->getNotifyCount(), ==, 1);
      issea(pSubj->getSlots(Subject::signalModified()).size() == 1);
      pSubj->notify(Subject::signalModified()); // will detach
      issea(pObs4->getNotifyCount() == 2);
      issea(pSubj->getSlots(Subject::signalModified()).size() == 0);
      pSubj->notify(Subject::signalModified()); // will not get to pObs4->update
      issea(pObs4->getNotifyCount() == 2);
      issea(false == pSubj->detach(Subject::signalModified(), 
         Slot(static_cast<CustomObserver3*>(pObs4), &CustomObserver3::update)));
      delete pObs4;
      pSubj->notify(Subject::signalModified());
      delete pSubj;

      //TEST7 - Disable and renable notification
      pSubj = new CustomSubject();
      CustomObserver3* pObs3 = new CustomObserver3(pSubj);
      issea(pSubj->signalsEnabled() == true);
      pSubj->notify(Subject::signalModified());
      issea_ext1(pObs3->getNotifyCount(), ==, 1);

      dynamic_cast<Subject*>(pSubj)->enableSignals(false);
      issea(pSubj->signalsEnabled() == false);
      pSubj->notify(Subject::signalModified());
      issea_ext1(pObs3->getNotifyCount(), ==, 1);

      dynamic_cast<Subject*>(pSubj)->enableSignals(true);
      issea(pSubj->signalsEnabled() == true);
      pSubj->notify(Subject::signalModified());
      issea_ext1(pObs3->getNotifyCount(), ==, 2);

      {
         SignalEnabler enabler(*pSubj, true);
         issea(pSubj->signalsEnabled() == true);
         pSubj->notify(Subject::signalModified());
         issea_ext1(pObs3->getNotifyCount(), ==, 3);
      }
      issea(pSubj->signalsEnabled() == true);

      {
         SignalEnabler enabler(*pSubj, false);
         issea(pSubj->signalsEnabled() == false);
         pSubj->notify(Subject::signalModified());
         issea_ext1(pObs3->getNotifyCount(), ==, 3);
      }
      issea(pSubj->signalsEnabled() == true);

      {
         SignalBlocker blocker(*pSubj);
         issea(pSubj->signalsEnabled() == false);
         pSubj->notify(Subject::signalModified());
         issea_ext1(pObs3->getNotifyCount(), ==, 3);
      }
      issea(pSubj->signalsEnabled() == true);

      {
         SignalEnabler enabler(*pSubj);
         issea(pSubj->signalsEnabled() == true);
         dynamic_cast<Subject*>(pSubj)->enableSignals(false);
      }
      issea(pSubj->signalsEnabled() == true);

      issea(pSubj->attach(Subject::signalDeleted(), Slot(pObs3, &CustomObserver3::update)));
      dynamic_cast<Subject*>(pSubj)->enableSignals(false);
      delete pSubj;
      issea_ext1(pObs3->getNotifyCount(), ==, 4); // deletion should be notified even when disabled

      delete pObs3;

      //TEST8 - Make sure SignalBlocker can outlive Subject
      pSubj = new CustomSubject();
      SignalBlocker* pBlocker = new SignalBlocker(*pSubj);
      delete pSubj;
      delete pBlocker; //blocker outlives pSubj, this line will cause a crash in the TestBed if the test fails

      return success;
   }

   void update(Subject &subject, const string &signal, const boost::any &v)
   {
      mNotifiedCount++;
      if (signal == Subject::signalModified())
      {
         subject.detach(Subject::signalModified(), Slot(this, &SubjectObserverTest::update));
      }
   }

   void attached(Subject &subject, const string &signal, const Slot &slot)
   {
      update(subject, "Attached", boost::any());
   }
   void detached(Subject &subject, const string &signal, const Slot &slot)
   {
      update(subject, "Detached", boost::any());
   }

private:
   int mNotifiedCount;
};

class DescriptorObserver
{
public:
   virtual ~DescriptorObserver()
   {}

   void update(Subject& subject, const string& signal, const boost::any& data)
   {
      cout << "\tNotified of " << subject.getObjectType() << " with signal " << signal << endl;
      descriptorUpdates.push_back(signal);
   }
};

class ModelObserver
{
public:
   virtual ~ModelObserver() {} 
   void update( Subject &subject, const string &signal, const boost::any &data )
   {  
      DataElement *pElement = boost::any_cast<DataElement*>( data );
      if( pElement != NULL )
      {
         if( signal == SIGNAL_NAME( ModelServices, ElementCreated ) ||
             signal == SIGNAL_NAME( ModelServices, ElementDestroyed ) )
         {
            cout << "\tNotified of " << pElement->getType() << " with signal " << signal << endl;
            modelUpdates.push_back( signal );
         }
      }
   }
};

class DesktopObserver
{
public:
   virtual ~DesktopObserver() {} 
   void update( Subject &subject, const string &signal, const boost::any &data )
   {  
      cout << "\tNotified of " << subject.getObjectType() << " with signal " << signal << endl;
      desktopUpdates.push_back( signal );
   }
};

class ElementObserver
{
public:
   virtual ~ElementObserver() {} 
   void update( Subject &subject, const string &signal, const boost::any &data )
   {  
      if( signal == SIGNAL_NAME( AoiElement, Modified ) ||
          signal == SIGNAL_NAME( AoiElement, Deleted ) ||
          signal == SIGNAL_NAME( GcpList, PointAdded ) ||
          signal == SIGNAL_NAME( GcpList, PointsAdded ) ||
          signal == SIGNAL_NAME( GcpList, PointRemoved ) ||
          signal == SIGNAL_NAME( GcpList, PointsRemoved ) ||
          signal == SIGNAL_NAME( GcpList, Deleted ) ||
          signal == SIGNAL_NAME( TiePointList, Modified ) ||
          signal == SIGNAL_NAME( TiePointList, Deleted ) )
      {
         cout << "\tNotified of " << subject.getObjectType() << " with signal " << signal << endl;
         dataElementUpdates.push_back( signal );
      }
   }
};

class SubjectNotifyTest : public TestCase
{
public:
   SubjectNotifyTest() : TestCase("SubjectNotify") {}
   bool run()
   {
      bool success = true;

      RasterElement* pRasterElement = TestUtilities::getStandardRasterElement();
      issea( pRasterElement != NULL );

      Service<ModelServices> pModel;
      Service<DesktopServices> pDesktop;

      DescriptorObserver* pDescriptorObserver = new DescriptorObserver();
      issearf(pDescriptorObserver != NULL);

      ModelObserver *pModelObserver = new ModelObserver();
      issea( pModelObserver != NULL );

      ElementObserver *pElementObserver = new ElementObserver();
      issea( pElementObserver != NULL );

      DesktopObserver *pDesktopObserver = new DesktopObserver();
      issea( pDesktopObserver != NULL );

      DataDescriptorResource<RasterDataDescriptor> pRasterDescriptor("TestRasterDescriptor",
         TypeConverter::toString<RasterElement>(), NULL);
      issea(pRasterDescriptor->attach(SIGNAL_NAME(DataDescriptor, ParentChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterDescriptor->attach(SIGNAL_NAME(DataDescriptor, ProcessingLocationChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterDescriptor->attach(SIGNAL_NAME(RasterDataDescriptor, DataTypeChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterDescriptor->attach(SIGNAL_NAME(RasterDataDescriptor, ValidDataTypesChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterDescriptor->attach(SIGNAL_NAME(RasterDataDescriptor, BadValuesChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterDescriptor->attach(SIGNAL_NAME(RasterDataDescriptor, InterleaveFormatChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterDescriptor->attach(SIGNAL_NAME(RasterDataDescriptor, RowsChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterDescriptor->attach(SIGNAL_NAME(RasterDataDescriptor, ColumnsChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterDescriptor->attach(SIGNAL_NAME(RasterDataDescriptor, BandsChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterDescriptor->attach(SIGNAL_NAME(RasterDataDescriptor, PixelSizeChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterDescriptor->attach(SIGNAL_NAME(RasterDataDescriptor, DisplayBandChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterDescriptor->attach(SIGNAL_NAME(RasterDataDescriptor, DisplayModeChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));

      Units* pUnits = pRasterDescriptor->getUnits();
      issearf(pUnits != NULL);

      issea(pUnits->attach(SIGNAL_NAME(Units, Renamed), Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pUnits->attach(SIGNAL_NAME(Units, TypeChanged), Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pUnits->attach(SIGNAL_NAME(Units, RangeChanged), Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pUnits->attach(SIGNAL_NAME(Units, ScaleChanged), Slot(pDescriptorObserver, &DescriptorObserver::update)));

      vector<EncodingType> validDataTypes;
      validDataTypes.push_back(FLT4BYTES);
      validDataTypes.push_back(FLT8BYTES);

      vector<int> badValues;
      badValues.push_back(0);

      vector<DimensionDescriptor> rows = RasterUtilities::generateDimensionVector(100, true, false, true);
      vector<DimensionDescriptor> columns = RasterUtilities::generateDimensionVector(200, true, false, true);
      vector<DimensionDescriptor> bands = RasterUtilities::generateDimensionVector(300, true, false, true);

      pRasterDescriptor->setProcessingLocation(ON_DISK_READ_ONLY);
      pRasterDescriptor->setValidDataTypes(validDataTypes);
      pRasterDescriptor->setDataType(FLT8BYTES);
      pRasterDescriptor->setBadValues(badValues);
      pRasterDescriptor->setInterleaveFormat(BSQ);
      pRasterDescriptor->setRows(rows);
      pRasterDescriptor->setColumns(columns);
      pRasterDescriptor->setBands(bands);
      pRasterDescriptor->setXPixelSize(2.0);
      pRasterDescriptor->setYPixelSize(3.0);
      pRasterDescriptor->setDisplayBand(GRAY, bands.at(10));
      pRasterDescriptor->setDisplayBand(RED, bands.at(20));
      pRasterDescriptor->setDisplayBand(GREEN, bands.at(30));
      pRasterDescriptor->setDisplayBand(BLUE, bands.at(40));
      pRasterDescriptor->setDisplayMode(RGB_MODE);

      pUnits->setUnitName("MyCustomUnits");
      pUnits->setUnitType(CUSTOM_UNIT);
      pUnits->setRangeMin(123.4);
      pUnits->setRangeMax(567.8);
      pUnits->setScaleFromStandard(9.01);

      issea(descriptorUpdates.size() == 20);
      issea(descriptorUpdates[0] == pRasterDescriptor->signalProcessingLocationChanged());
      issea(descriptorUpdates[1] == pRasterDescriptor->signalValidDataTypesChanged());
      issea(descriptorUpdates[2] == pRasterDescriptor->signalDataTypeChanged());
      issea(descriptorUpdates[3] == pRasterDescriptor->signalBadValuesChanged());
      issea(descriptorUpdates[4] == pRasterDescriptor->signalInterleaveFormatChanged());
      issea(descriptorUpdates[5] == pRasterDescriptor->signalRowsChanged());
      issea(descriptorUpdates[6] == pRasterDescriptor->signalColumnsChanged());
      issea(descriptorUpdates[7] == pRasterDescriptor->signalBandsChanged());
      issea(descriptorUpdates[8] == pRasterDescriptor->signalPixelSizeChanged());
      issea(descriptorUpdates[9] == pRasterDescriptor->signalPixelSizeChanged());
      issea(descriptorUpdates[10] == pRasterDescriptor->signalDisplayBandChanged());
      issea(descriptorUpdates[11] == pRasterDescriptor->signalDisplayBandChanged());
      issea(descriptorUpdates[12] == pRasterDescriptor->signalDisplayBandChanged());
      issea(descriptorUpdates[13] == pRasterDescriptor->signalDisplayBandChanged());
      issea(descriptorUpdates[14] == pRasterDescriptor->signalDisplayModeChanged());
      issea(descriptorUpdates[15] == pUnits->signalRenamed());
      issea(descriptorUpdates[16] == pUnits->signalTypeChanged());
      issea(descriptorUpdates[17] == pUnits->signalRangeChanged());
      issea(descriptorUpdates[18] == pUnits->signalRangeChanged());
      issea(descriptorUpdates[19] == pUnits->signalScaleChanged());

      FactoryResource<RasterFileDescriptor> pRasterFileDescriptor;
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(FileDescriptor, FilenameChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(FileDescriptor, DatasetLocationChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(FileDescriptor, EndianChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, RowsChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, ColumnsChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, BandsChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, BitsPerElementChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, HeaderBytesChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, TrailerBytesChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, PrelineBytesChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, PostlineBytesChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, PrebandBytesChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, PostbandBytesChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, InterleaveFormatChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, BandFilesChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, PixelSizeChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));
      issea(pRasterFileDescriptor->attach(SIGNAL_NAME(RasterFileDescriptor, GcpsChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));

      string testDataPath = TestUtilities::getTestDataPath();
      string filename = testDataPath + "MyFile.ext";

      EndianType endian = Endian::getSystemEndian();
      if (endian == BIG_ENDIAN_ORDER)
      {
         endian = LITTLE_ENDIAN_ORDER;
      }
      else if (endian == LITTLE_ENDIAN_ORDER)
      {
         endian = BIG_ENDIAN_ORDER;
      }

      vector<string> bandFiles;
      bandFiles.push_back(testDataPath + "MyBandFile1.ext");
      bandFiles.push_back(testDataPath + "MyBandFile2.ext");
      bandFiles.push_back(testDataPath + "MyBandFile3.ext");
      bandFiles.push_back(testDataPath + "MyBandFile4.ext");
      bandFiles.push_back(testDataPath + "MyBandFile5.ext");

      GcpPoint point1;
      point1.mPixel.mX = 21;
      point1.mPixel.mY = 22;
      GcpPoint point2;
      point2.mPixel.mX = 10;
      point2.mPixel.mY = 10;
      GcpPoint point3;
      point3.mPixel.mX = 2;
      point3.mPixel.mY = 5;
      GcpPoint point4;
      point4.mPixel.mX = 34;
      point4.mPixel.mY = 65;
      GcpPoint point5;
      point5.mPixel.mX = 12;
      point5.mPixel.mY = 3;
      GcpPoint point6;
      point6.mPixel.mX = 4;
      point6.mPixel.mY = 55;

      list<GcpPoint> newGcps;
      newGcps.push_back(point2);
      newGcps.push_back(point3);
      newGcps.push_back(point4);
      newGcps.push_back(point5);
      newGcps.push_back(point6);

      pRasterFileDescriptor->setFilename(filename);
      pRasterFileDescriptor->setDatasetLocation("MyDatasetLocation");
      pRasterFileDescriptor->setEndian(endian);
      pRasterFileDescriptor->setRows(rows);
      pRasterFileDescriptor->setColumns(columns);
      pRasterFileDescriptor->setBands(bands);
      pRasterFileDescriptor->setBitsPerElement(10);
      pRasterFileDescriptor->setHeaderBytes(20);
      pRasterFileDescriptor->setTrailerBytes(30);
      pRasterFileDescriptor->setPrelineBytes(40);
      pRasterFileDescriptor->setPostlineBytes(50);
      pRasterFileDescriptor->setPrebandBytes(60);
      pRasterFileDescriptor->setPostbandBytes(70);
      pRasterFileDescriptor->setInterleaveFormat(BSQ);
      pRasterFileDescriptor->setBandFiles(bandFiles);
      pRasterFileDescriptor->setXPixelSize(2.0);
      pRasterFileDescriptor->setYPixelSize(3.0);
      pRasterFileDescriptor->setGcps(newGcps);

      issea(descriptorUpdates.size() == 38);
      issea(descriptorUpdates[20] == pRasterFileDescriptor->signalFilenameChanged());
      issea(descriptorUpdates[21] == pRasterFileDescriptor->signalDatasetLocationChanged());
      issea(descriptorUpdates[22] == pRasterFileDescriptor->signalEndianChanged());
      issea(descriptorUpdates[23] == pRasterFileDescriptor->signalRowsChanged());
      issea(descriptorUpdates[24] == pRasterFileDescriptor->signalColumnsChanged());
      issea(descriptorUpdates[25] == pRasterFileDescriptor->signalBandsChanged());
      issea(descriptorUpdates[26] == pRasterFileDescriptor->signalBitsPerElementChanged());
      issea(descriptorUpdates[27] == pRasterFileDescriptor->signalHeaderBytesChanged());
      issea(descriptorUpdates[28] == pRasterFileDescriptor->signalTrailerBytesChanged());
      issea(descriptorUpdates[29] == pRasterFileDescriptor->signalPrelineBytesChanged());
      issea(descriptorUpdates[30] == pRasterFileDescriptor->signalPostlineBytesChanged());
      issea(descriptorUpdates[31] == pRasterFileDescriptor->signalPrebandBytesChanged());
      issea(descriptorUpdates[32] == pRasterFileDescriptor->signalPostbandBytesChanged());
      issea(descriptorUpdates[33] == pRasterFileDescriptor->signalInterleaveFormatChanged());
      issea(descriptorUpdates[34] == pRasterFileDescriptor->signalBandFilesChanged());
      issea(descriptorUpdates[35] == pRasterFileDescriptor->signalPixelSizeChanged());
      issea(descriptorUpdates[36] == pRasterFileDescriptor->signalPixelSizeChanged());
      issea(descriptorUpdates[37] == pRasterFileDescriptor->signalGcpsChanged());

      DataDescriptorResource<SignatureDataDescriptor> pSignatureDescriptor("TestSignatureDescriptor",
         TypeConverter::toString<Signature>(), NULL);
      issea(pSignatureDescriptor->attach(SIGNAL_NAME(SignatureDataDescriptor, UnitsChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));

      pSignatureDescriptor->setUnits("MyUnits", pUnits);

      issea(descriptorUpdates.size() == 39);
      issea(descriptorUpdates[38] == pSignatureDescriptor->signalUnitsChanged());

      FactoryResource<SignatureFileDescriptor> pSignatureFileDescriptor;
      issea(pSignatureFileDescriptor->attach(SIGNAL_NAME(SignatureFileDescriptor, UnitsChanged),
         Slot(pDescriptorObserver, &DescriptorObserver::update)));

      pSignatureFileDescriptor->setUnits("MyUnits", pUnits);

      issea(descriptorUpdates.size() == 40);
      issea(descriptorUpdates[39] == pSignatureFileDescriptor->signalUnitsChanged());

      issea( pModel->attach( SIGNAL_NAME( ModelServices, ElementCreated ),
         Slot( pModelObserver, &ModelObserver::update ) ) );

      issea( pModel->attach( SIGNAL_NAME( ModelServices, ElementDestroyed ),
         Slot( pModelObserver, &ModelObserver::update ) ) );

      AoiElement *pAoi = dynamic_cast<AoiElement*>( pModel->createElement( 
         "notifyTestAoi", "AoiElement", NULL ) );
      issea( pAoi != NULL );
      issea( modelUpdates.size() == 1 );
      issea( modelUpdates.at( 0 ) == ModelServices::signalElementCreated() );

      issea( pAoi->attach( SIGNAL_NAME( AoiElement, Modified ),
         Slot( pElementObserver, &ElementObserver::update ) ) );

      issea( pAoi->attach( SIGNAL_NAME( AoiElement, Deleted ),
         Slot( pElementObserver, &ElementObserver::update ) ) );

      pAoi->addPoint( LocationType( 50, 50 ) );
      issea( dataElementUpdates.size() == 8 );
      issea( dataElementUpdates.at( 0 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 1 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 2 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 3 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 4 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 5 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 6 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 7 ) == pAoi->signalModified() );

      vector<LocationType> newPoints;
      newPoints.push_back( LocationType( 2, 5 ) );
      newPoints.push_back( LocationType( 34, 65 ) );
      newPoints.push_back( LocationType( 12, 3 ) );
      newPoints.push_back( LocationType( 4, 55 ) );
      pAoi->addPoints( newPoints );
      issea( dataElementUpdates.size() == 16 );
      issea( dataElementUpdates.at( 6 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 7 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 8 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 9 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 10 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 11 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 15 ) == pAoi->signalModified() );

      pAoi->removePoint( LocationType( 34, 65 ) );
      issea( dataElementUpdates.size() == 26 );
      issea( dataElementUpdates.at( 12 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 13 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 14 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 15 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 16 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 17 ) == pAoi->signalModified() );
      issea( dataElementUpdates.at( 25 ) == pAoi->signalModified() );

      pModel->destroyElement( pAoi );
      issea( modelUpdates.size() == 2 );
      issea( modelUpdates.at( 1 ) == ModelServices::signalElementDestroyed() );
      issea( dataElementUpdates.size() == 27 );
      issea( dataElementUpdates.at( 26 ) == pAoi->signalDeleted() );

      GcpList *pList = dynamic_cast<GcpList*>( pModel->createElement( "notifyTestGcp", "GcpList", NULL ) );
      issea( pList != NULL );
      issea( modelUpdates.size() == 3 );
      issea( modelUpdates.at( 2 ) == ModelServices::signalElementCreated() );

      issea( pList->attach( SIGNAL_NAME( GcpList, PointAdded ),
         Slot( pElementObserver, &ElementObserver::update ) ) );

      issea( pList->attach( SIGNAL_NAME( GcpList, PointRemoved ),
         Slot( pElementObserver, &ElementObserver::update ) ) );

      issea( pList->attach( SIGNAL_NAME( GcpList, PointsAdded ),
         Slot( pElementObserver, &ElementObserver::update ) ) );

      issea( pList->attach( SIGNAL_NAME( GcpList, PointsRemoved ),
         Slot( pElementObserver, &ElementObserver::update ) ) );

      issea( pList->attach( SIGNAL_NAME( GcpList, Deleted ),
         Slot( pElementObserver, &ElementObserver::update ) ) );

      pList->addPoint( point1 );
      issea( dataElementUpdates.size() == 28 );
      issea( dataElementUpdates.at( 27 ) == pList->signalPointAdded() );

      pList->addPoints( newGcps );
      issea( dataElementUpdates.size() == 29 );
      issea( dataElementUpdates.at( 28 ) == pList->signalPointsAdded() );

      newGcps.clear();
      newGcps.push_back( point4 );
      newGcps.push_back( point5 );
      pList->removePoints( newGcps );
      issea( dataElementUpdates.size() == 30 );
      issea( dataElementUpdates.at( 29 ) == pList->signalPointsRemoved() );

      pList->removePoint( point1 );
      issea( dataElementUpdates.size() == 31 );
      issea( dataElementUpdates.at( 30 ) == pList->signalPointRemoved() );

      //this should not notify because I am not attached to Cleared
      pList->clearPoints();
      issea( dataElementUpdates.size() == 31 );

      pModel->destroyElement( pList );
      issea( modelUpdates.size() == 4 );
      issea( modelUpdates.at( 3 ) == ModelServices::signalElementDestroyed() );
      issea( dataElementUpdates.size() == 32 );
      issea( dataElementUpdates.at( 31 ) == pList->signalDeleted() );

      TiePointList *pTiePointList = dynamic_cast<TiePointList*>( pModel->createElement(
         "notifyTestTieList", "TiePointList", NULL ) );
      issea( pTiePointList != NULL );
      issea( modelUpdates.size() == 5 );
      issea( modelUpdates.at( 4 ) == ModelServices::signalElementCreated() );

      issea( pTiePointList->attach( SIGNAL_NAME( TiePointList, Modified ),
         Slot( pElementObserver, &ElementObserver::update ) ) );

      issea( pTiePointList->attach( SIGNAL_NAME( TiePointList, Deleted ),
         Slot( pElementObserver, &ElementObserver::update ) ) );

      TiePoint tiepoint;
      tiepoint.mReferencePoint.mX = 31;
      tiepoint.mReferencePoint.mY = 32;
      vector<TiePoint> points;
      points.push_back( tiepoint );
      pTiePointList->adoptTiePoints( points );
      issea( dataElementUpdates.size() == 33 );
      issea( dataElementUpdates.at( 32 ) == pTiePointList->signalModified() );

      pModel->destroyElement( pTiePointList );
      issea( modelUpdates.size() == 6 );
      issea( modelUpdates.at( 5 ) == ModelServices::signalElementDestroyed() );
      issea( dataElementUpdates.size() == 34 );
      issea( dataElementUpdates.at( 33 ) == pTiePointList->signalDeleted() );

      RasterElement *pRasterCopy = dynamic_cast<RasterElement*>( pRasterElement->copy(
         "notifySensorData", pRasterElement ) );
      issea( pRasterCopy != NULL );
      issea( modelUpdates.size() == 7 );
      issea( modelUpdates.at( 6 ) == ModelServices::signalElementCreated() );

      SpatialDataWindow *pWindow = dynamic_cast<SpatialDataWindow*>( pDesktop->createWindow(
         "notifyTestWindow", SPATIAL_DATA_WINDOW ) );
      issea( pWindow != NULL );

      issea( pWindow->attach( SIGNAL_NAME( SpatialDataWindow, Modified ),
         Slot( pDesktopObserver, &DesktopObserver::update ) ) );

      issea( pWindow->attach( SIGNAL_NAME( SpatialDataWindow, Deleted ),
         Slot( pDesktopObserver, &DesktopObserver::update ) ) );

      SpatialDataView *pView = pWindow->getSpatialDataView();
      issea( pView != NULL );

      RasterLayer *pLayer = dynamic_cast<RasterLayer*>( pView->createLayer( RASTER, pRasterElement ) );
      issea( pLayer != NULL );

      issea( pView->attach( SIGNAL_NAME( SpatialDataView, BackgroundColorChanged ),
         Slot( pDesktopObserver, &DesktopObserver::update ) ) );

      issea( pView->attach( SIGNAL_NAME( SpatialDataView, ClassificationChanged ),
         Slot( pDesktopObserver, &DesktopObserver::update ) ) );

      issea( pView->attach( SIGNAL_NAME( SpatialDataView, Deleted ),
         Slot( pDesktopObserver, &DesktopObserver::update ) ) );

      issea( pView->attach( SIGNAL_NAME( SpatialDataView, ExtentsChanged ),
         Slot( pDesktopObserver, &DesktopObserver::update ) ) );

      issea( pView->attach( SIGNAL_NAME( SpatialDataView, LayerHidden ),
         Slot( pDesktopObserver, &DesktopObserver::update ) ) );

      issea( pView->attach( SIGNAL_NAME( SpatialDataView, LayerShown ),
         Slot( pDesktopObserver, &DesktopObserver::update ) ) );

      issea( pView->attach( SIGNAL_NAME( SpatialDataView, MouseModeChanged ),
         Slot( pDesktopObserver, &DesktopObserver::update ) ) );

      issea( pView->attach( SIGNAL_NAME( SpatialDataView, Renamed ),
         Slot( pDesktopObserver, &DesktopObserver::update ) ) );

      issea( pView->attach( SIGNAL_NAME( SpatialDataView, RotationChanged ),
         Slot( pDesktopObserver, &DesktopObserver::update ) ) );

      issea( pView->attach( SIGNAL_NAME( SpatialDataView, ZoomChanged ),
         Slot( pDesktopObserver, &DesktopObserver::update ) ) );

      issea( pLayer->attach( SIGNAL_NAME( Layer, ExtentsModified ),
         Slot( pDesktopObserver, &DesktopObserver::update ) ) );

      pView->setBackgroundColor( ColorType( 255, 0, 0 ) );
      issea( desktopUpdates.size() == 1 );
      issea( desktopUpdates.at( 0 ) == pView->signalBackgroundColorChanged() );

      FactoryResource<Classification> pClassification;
      pClassification->setCodewords("Test");
      pView->setClassification(pClassification.get());
      issea( desktopUpdates.size() == 2 );
      issea( desktopUpdates.at( 1 ) == pView->signalClassificationChanged() );

      pLayer->setXScaleFactor( 3.5 );
      issea( desktopUpdates.size() == 3 );
      issea( desktopUpdates.at( 2 ) == pLayer->signalExtentsModified() );

      pLayer->setYScaleFactor( 3.5 );
      issea( desktopUpdates.size() == 4 );
      issea( desktopUpdates.at( 3 ) == pLayer->signalExtentsModified() );

      issea( pView->hideLayer( pLayer ) );
      issea( desktopUpdates.size() == 5 );
      issea( desktopUpdates.at( 4 ) == pView->signalLayerHidden() );

      issea( pView->showLayer( pLayer ) );
      issea( desktopUpdates.size() == 6 );
      issea( desktopUpdates.at( 5 ) == pView->signalLayerShown() );

      pView->setMouseMode( "RotateMode" );
      issea( desktopUpdates.size() == 7 );
      issea( desktopUpdates.at( 6 ) == pView->signalMouseModeChanged() );

      pView->setName( "newName" );
      issea( desktopUpdates.size() == 8 );
      issea( desktopUpdates.at( 7 ) == pView->signalRenamed() );

      pView->rotateBy( 30 );
      issea( desktopUpdates.size() == 9 );
      issea( desktopUpdates.at( 8 ) == pView->signalRotationChanged() );

      pView->zoomTo( 350 );
      issea( desktopUpdates.size() == 10 );
      issea( desktopUpdates.at( 9 ) == pView->signalZoomChanged() );

      pView->zoomExtents();
      issea( desktopUpdates.size() == 11 );
      issea( desktopUpdates.at( 10 ) == pView->signalZoomChanged() );

      issea(TestUtilities::destroyWorkspaceWindow(pWindow));
      issea( desktopUpdates.size() == 13 );
      issea( desktopUpdates.at( 11 ) == pWindow->signalDeleted() );
      issea( desktopUpdates.at( 12 ) == pView->signalDeleted() );

      pModel->destroyElement( pRasterCopy );
      issea( modelUpdates.size() == 8 );
      issea( modelUpdates.at( 7 ) == ModelServices::signalElementDestroyed() );

      issea( pModel->detach( SIGNAL_NAME( ModelServices, ElementCreated ),
         Slot( pModelObserver, &ModelObserver::update ) ) );

      issea( pModel->detach( SIGNAL_NAME( ModelServices, ElementDestroyed ),
         Slot( pModelObserver, &ModelObserver::update ) ) );

      return success;
   }
};

class EnumWrapperTest : public TestCase
{
public:
   EnumWrapperTest() : TestCase("EnumWrapper") {}
   bool run()
   {
      bool success = true;

      InterleaveFormatType t1;
      InterleaveFormatType t2;
      InterleaveFormatType t3 = BIP;
      InterleaveFormatType t4 = BIL;
      InterleaveFormatType t5 = BIP;

      issea(t1.isValid() == false);
      issea(t3.isValid() == true);

      switch (t1)
      {
      case BSQ:
         issea(false); //t1 is unitialized, shouldn't match
         break;
      case BIP:
         issea(false); //t1 is unitialized, shouldn't match
         break;
      case BIL:
         issea(false); //t1 is unitialized, shouldn't match
         break;
      default:
         issea(true); //t1 is unitialized, should match the default
         break;
      };

      switch (t3)
      {
      case BSQ:
         issea(false); //t3 is set to BIP, shouldn't match
         break;
      case BIP:
         issea(true); //t3 is set to BIP, should match
         break;
      case BIL:
         issea(false); //t3 is set to BIP, shouldn't match
         break;
      default:
         issea(false); //t3 is set to BIP, shouldn't match
         break;
      };

      issea((t1 == t2) == true); //both uninitialized
      issea((t3 == BSQ) == false);
      issea((BSQ == t3) == false);
      issea((t3 == BIP) == true); 
      issea((t3 == t4) == false); 
      issea((BIP == BSQ) == false); 
      issea((t1 == BSQ) == false); //t1 is uninitialized
      issea((BSQ == t1) == false); //t1 is uninitialized

      issea((t1 != t2) == false); //both uninitialized
      issea((t3 != BSQ) == true);
      issea((t3 != BIP) == false);
      issea((t3 != t4) == true); 
      issea((BIP != BSQ) == true); 
      issea((t1 != BSQ) == true); //t1 is uninitialized
      issea((BSQ != t1) == true); //t1 is unitialized

      issea((t1 < t2) == false); //both uninitialized
      issea((t2 < t1) == false); //both uninitialized
      issea((t3 < t4) == true); 
      issea((t4 < t3) == false); 
      issea((t3 < t5) == false); 
      issea((t5 < t3) == false); 
      issea((t3 < BIL) == true); 
      issea((BIL < t3) == false); 
      issea((t3 < BSQ) == false); 
      issea((BSQ < t3) == true);
      issea((t1 < BSQ) == false); //t1 is unitialized 
      issea((BSQ < t1) == true); //t1 is unitialized 
      issea((t1 < t3) == false); //t1 is unitialized 
      issea((t3 < t1) == true); //t1 is unitialized 

      issea((t1 <= t2) == true); //both uninitialized
      issea((t2 <= t1) == true); //both uninitialized
      issea((t3 <= t4) == true); 
      issea((t4 <= t3) == false); 
      issea((t3 <= t5) == true); 
      issea((t5 <= t3) == true); 
      issea((t3 <= BIL) == true); 
      issea((BIL <= t3) == false); 
      issea((t3 <= BSQ) == false); 
      issea((BSQ <= t3) == true);
      issea((t1 <= BSQ) == false); //t1 is unitialized 
      issea((BSQ <= t1) == true); //t1 is unitialized 
      issea((t1 <= t3) == false); //t1 is unitialized 
      issea((t3 <= t1) == true); //t1 is unitialized 

      issea((t1 > t2) == false); //both uninitialized
      issea((t2 > t1) == false); //both uninitialized
      issea((t3 > t4) == false); 
      issea((t4 > t3) == true); 
      issea((t3 > t5) == false); 
      issea((t5 > t3) == false); 
      issea((t3 > BIL) == false); 
      issea((BIL > t3) == true); 
      issea((t3 > BSQ) == true); 
      issea((BSQ > t3) == false);
      issea((t1 > BSQ) == true); //t1 is unitialized 
      issea((BSQ > t1) == false); //t1 is unitialized 
      issea((t1 > t3) == true); //t1 is unitialized 
      issea((t3 > t1) == false); //t1 is unitialized 

      issea((t1 >= t2) == true); //both uninitialized
      issea((t2 >= t1) == true); //both uninitialized
      issea((t3 >= t4) == false); 
      issea((t4 >= t3) == true);
      issea((t3 >= t5) == true); 
      issea((t5 >= t3) == true); 
      issea((t3 >= BIL) == false); 
      issea((BIL >= t3) == true); 
      issea((t3 >= BSQ) == true); 
      issea((BSQ >= t3) == false);
      issea((t1 >= BSQ) == true); //t1 is unitialized 
      issea((BSQ >= t1) == false); //t1 is unitialized 
      issea((t1 >= t3) == true); //t1 is unitialized 
      issea((t3 >= t1) == false); //t1 is unitialized 

      return success;
   }
};

class StringUtilitiesTest : public TestCase
{
public:
   StringUtilitiesTest() : TestCase("StringUtilities") {}
   bool run()
   {
      bool success = true;

      //Test enum conversion to a string
      issea(StringUtilities::toDisplayString(PLAY_ONCE) == "Play Once");
      issea(StringUtilities::toXmlString(PLAY_ONCE) == "play_once");

      //make sure error reset to false
      bool error = true;
      issea(StringUtilities::toDisplayString(REPEAT, &error) == "Repeat");
      issea(error == false);
      error = true;
      issea(StringUtilities::toXmlString(REPEAT, &error) == "repeat");
      issea(error == false);

      //make sure error reset to false
      error = true;
      issea(StringUtilities::toDisplayString(AnimationCycle(), &error) == "");
      issea(error == false);
      error = true;
      issea(StringUtilities::toXmlString(AnimationCycle(), &error) == "");
      issea(error == false);

      //Test conversion to an enum
      issea(StringUtilities::fromDisplayString<AnimationCycle>("Play Once") == PLAY_ONCE);
      issea(StringUtilities::fromXmlString<AnimationCycle>("play_once") == PLAY_ONCE);

      //make sure error reset to false
      error = true;
      issea(StringUtilities::fromDisplayString<AnimationCycle>("Repeat", &error) == REPEAT);
      issea(error == false);
      error = true;
      issea(StringUtilities::fromXmlString<AnimationCycle>("repeat", &error) == REPEAT);
      issea(error == false);

      //make sure error reset to false
      error = true;
      issea(StringUtilities::fromDisplayString<AnimationCycle>("", &error) == AnimationCycle());
      issea(error == false);
      error = true;
      issea(StringUtilities::fromXmlString<AnimationCycle>("", &error) == AnimationCycle());
      issea(error == false);

      //make sure error reset to true
      error = false;
      issea(StringUtilities::fromDisplayString<AnimationCycle>("FooBar", &error) == AnimationCycle());
      issea(error == true);
      error = false;
      issea(StringUtilities::fromXmlString<AnimationCycle>("FooBar", &error) == AnimationCycle());
      issea(error == true);

      //check vector conversion to a string
      vector<AnimationCycle> enumSingleVec;
      enumSingleVec.push_back(REPEAT);

      vector<AnimationCycle> enumMultiVec;
      enumMultiVec.push_back(REPEAT);
      enumMultiVec.push_back(PLAY_ONCE);
      enumMultiVec.push_back(BOUNCE);

      vector<AnimationCycle> enumMultiVec2;
      enumMultiVec2.push_back(REPEAT);
      enumMultiVec2.push_back(AnimationCycle());
      enumMultiVec2.push_back(BOUNCE);

      //make sure error reset to false
      error = true;
      issea(StringUtilities::toDisplayString(enumSingleVec, &error) == "Repeat");
      issea(error == false);
      error = true;
      issea(StringUtilities::toXmlString(enumSingleVec, &error) == "repeat");
      issea(error == false);
      error = true;
      issea(StringUtilities::toDisplayString(enumMultiVec, &error) == "Repeat, Play Once, Bounce");
      issea(error == false);
      error = true;
      issea(StringUtilities::toXmlString(enumMultiVec, &error) == "repeat, play_once, bounce");
      issea(error == false);

      //make sure error reset to false
      error = true;
      issea(StringUtilities::toDisplayString(enumMultiVec2, &error) == "Repeat, , Bounce");
      issea(error == false);
      error = true;
      issea(StringUtilities::toXmlString(enumMultiVec2, &error) == "repeat, , bounce");
      issea(error == false);

      //check string conversion to a vector
      error = true;
      vector<AnimationCycle> parsedSingleVec = StringUtilities::fromDisplayString<vector<AnimationCycle> >("Repeat", &error);
      issea(parsedSingleVec.size() == 1);
      issea(parsedSingleVec[0] == REPEAT);
      issea(error == false);

      error = true;
      parsedSingleVec = StringUtilities::fromXmlString<vector<AnimationCycle> >("repeat", &error);
      issea(parsedSingleVec.size() == 1);
      issea(parsedSingleVec[0] == REPEAT);
      issea(error == false);

      error = true;
      vector<AnimationCycle> parsedMultiVec = StringUtilities::fromDisplayString<vector<AnimationCycle> >("Repeat, Bounce, Play Once, Repeat", &error);
      issea(parsedMultiVec.size() == 4);
      issea(parsedMultiVec[0] == REPEAT);
      issea(parsedMultiVec[1] == BOUNCE);
      issea(parsedMultiVec[2] == PLAY_ONCE);
      issea(parsedMultiVec[3] == REPEAT);
      issea(error == false);

      error = true;
      parsedMultiVec = StringUtilities::fromXmlString<vector<AnimationCycle> >("repeat, bounce, play_once, repeat", &error);
      issea(parsedMultiVec.size() == 4);
      issea(parsedMultiVec[0] == REPEAT);
      issea(parsedMultiVec[1] == BOUNCE);
      issea(parsedMultiVec[2] == PLAY_ONCE);
      issea(parsedMultiVec[3] == REPEAT);
      issea(error == false);

      error = true;
      parsedMultiVec = StringUtilities::fromXmlString<vector<AnimationCycle> >("repeat, , play_once, repeat", &error);
      issea(parsedMultiVec.size() == 4);
      issea(parsedMultiVec[0] == REPEAT);
      issea(parsedMultiVec[1] == AnimationCycle());
      issea(parsedMultiVec[2] == PLAY_ONCE);
      issea(parsedMultiVec[3] == REPEAT);
      issea(error == false);

      error = false;
      parsedMultiVec = StringUtilities::fromDisplayString<vector<AnimationCycle> >("Repeat, Bounce, Play Once, RePEAT", &error);
      issea(parsedMultiVec.size() == 0);
      issea(error == true);

      error = false;
      parsedMultiVec = StringUtilities::fromXmlString<vector<AnimationCycle> >("repeat, bounce, play_once, rePEAT", &error);
      issea(parsedMultiVec.size() == 0);
      issea(error == true);

      error = false;
      parsedMultiVec = StringUtilities::fromDisplayString<vector<AnimationCycle> >("Repeat,Bounce,Play Once,Repeat", &error);
      issea(parsedMultiVec.size() == 0);
      issea(error == true);

      error = false;
      parsedMultiVec = StringUtilities::fromXmlString<vector<AnimationCycle> >("repeat,bounce,play_once,repeat", &error);
      issea(parsedMultiVec.size() == 0);
      issea(error == true);
     
      error = true;
      issea(StringUtilities::toDisplayString(1, &error) == "1");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(50, &error) == "50");
      issea(error == false);

      error = true;
      issea(StringUtilities::toDisplayString(-100, &error) == "-100");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(-100, &error) == "-100");
      issea(error == false);

      error = true;
      issea(StringUtilities::toDisplayString(1.089, &error) == "1.089");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(50.2398, &error) == "50.2398");
      issea(error == false);

      error = true;
      issea(StringUtilities::toDisplayString(-100.3290, &error) == "-100.329");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(-100.8765, &error) == "-100.8765"); //correct - precision is currently 3
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString<char>(0, &error) == "0");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString<char>(-1, &error) == "-1");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString<char>(127, &error) == "127");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString<char>(-128, &error) == "-128");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString('A', &error) == "65");
      issea(error == false);

      error = true;
      issea(StringUtilities::toDisplayString('A', &error) == "65");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString('?', &error) == "63");
      issea(error == false);

      error = true;
      issea(StringUtilities::toDisplayString('?', &error) == "63");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString<unsigned char>(0, &error) == "0");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString<unsigned char>(100, &error) == "100");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString<unsigned char>(255, &error) == "255");
      issea(error == false);

      error = true;
      issea(StringUtilities::fromDisplayString<char>("1", &error) == 1);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromDisplayString<char>("127", &error) == 127);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromXmlString<char>("127", &error) == 127);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromDisplayString<char>("-128", &error) == -128);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromXmlString<char>("-128", &error) == -128);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromDisplayString<unsigned char>("255", &error) == 255);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromXmlString<unsigned char>("255", &error) == 255);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromDisplayString<unsigned char>("42", &error) == 42);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromXmlString<unsigned char>("42", &error) == 42);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromDisplayString<int>("1", &error) == 1);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromXmlString<int>("50", &error) == 50);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromDisplayString<int>("-100", &error) == -100);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromXmlString<int>("-100", &error) == -100);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromDisplayString<double>("1.089", &error) == 1.089);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromXmlString<double>("50.2398", &error) == 50.2398);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromDisplayString<double>("-100.3290", &error) == -100.329);
      issea(error == false);

      error = true;
      issea(StringUtilities::fromXmlString<double>("-100.8765", &error) == -100.8765);
      issea(error == false);

      vector<char> charValues;
      charValues.push_back(10);
      charValues.push_back(-20);
      charValues.push_back(30);
      error = true;
      issea(StringUtilities::toDisplayString(charValues, &error) == "10, -20, 30");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(charValues, &error) == "10, -20, 30");
      issea(error == false);

      error = true;
      vector<char> parsedCharValues = StringUtilities::fromDisplayString<vector<char> >("10, -20, 30", &error);
      issea(parsedCharValues.size() == 3);
      issea(parsedCharValues[0] == 10);
      issea(parsedCharValues[1] == -20);
      issea(parsedCharValues[2] == 30);
      issea(error == false);

      error = true;
      parsedCharValues = StringUtilities::fromXmlString<vector<char> >("10, -20, 30", &error);
      issea(parsedCharValues.size() == 3);
      issea(parsedCharValues[0] == 10);
      issea(parsedCharValues[1] == -20);
      issea(parsedCharValues[2] == 30);
      issea(error == false);

      vector<unsigned char> ucharValues;
      ucharValues.push_back(10);
      ucharValues.push_back(200);
      ucharValues.push_back(30);
      error = true;
      issea(StringUtilities::toDisplayString(ucharValues, &error) == "10, 200, 30");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(ucharValues, &error) == "10, 200, 30");
      issea(error == false);

      error = true;
      vector<unsigned char> parsedUcharValues =
         StringUtilities::fromDisplayString<vector<unsigned char> >("10, 200, 30", &error);
      issea(parsedUcharValues.size() == 3);
      issea(parsedUcharValues[0] == 10);
      issea(parsedUcharValues[1] == 200);
      issea(parsedUcharValues[2] == 30);
      issea(error == false);

      error = true;
      parsedUcharValues = StringUtilities::fromXmlString<vector<unsigned char> >("10, 200, 30", &error);
      issea(parsedUcharValues.size() == 3);
      issea(parsedUcharValues[0] == 10);
      issea(parsedUcharValues[1] == 200);
      issea(parsedUcharValues[2] == 30);
      issea(error == false);

      vector<int> intValues;
      intValues.push_back(10000);
      intValues.push_back(-20000);
      intValues.push_back(30000);
      error = true;
      issea(StringUtilities::toDisplayString(intValues, &error) == "10000, -20000, 30000");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(intValues, &error) == "10000, -20000, 30000");
      issea(error == false);

      error = true;
      vector<int> parsedIntValues = StringUtilities::fromDisplayString<vector<int> >("10000, -20000, 30000", &error);
      issea(parsedIntValues.size() == 3);
      issea(parsedIntValues[0] == 10000);
      issea(parsedIntValues[1] == -20000);
      issea(parsedIntValues[2] == 30000);
      issea(error == false);

      error = true;
      parsedIntValues = StringUtilities::fromXmlString<vector<int> >("10000, -20000, 30000", &error);
      issea(parsedIntValues.size() == 3);
      issea(parsedIntValues[0] == 10000);
      issea(parsedIntValues[1] == -20000);
      issea(parsedIntValues[2] == 30000);
      issea(error == false);

      error = true;
      parsedIntValues = StringUtilities::fromDisplayString<vector<int> >("   10000,    -20000   ,   30000", &error);
      issea(parsedIntValues.size() == 3);
      issea(parsedIntValues[0] == 10000);
      issea(parsedIntValues[1] == -20000);
      issea(parsedIntValues[2] == 30000);
      issea(error == false);

      error = true;
      parsedIntValues = StringUtilities::fromXmlString<vector<int> >("   10000,    -20000   ,   30000", &error);
      issea(parsedIntValues.size() == 3);
      issea(parsedIntValues[0] == 10000);
      issea(parsedIntValues[1] == -20000);
      issea(parsedIntValues[2] == 30000);
      issea(error == false);

      vector<double> doubleValues;
      doubleValues.push_back(10001.782398543);
      doubleValues.push_back(-20000.88329);
      doubleValues.push_back(030000.00);
      error = true;
      issea(StringUtilities::toDisplayString(doubleValues, &error) == "10001.782398543, -20000.88329, 30000");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(doubleValues, &error) == "10001.782398543, -20000.88329, 30000");
      issea(error == false);

      error = true;
      vector<double> parsedDoubleValues = StringUtilities::fromDisplayString<vector<double> >("10000.98531, -20000.765, 30000.939", &error);
      issea(parsedDoubleValues.size() == 3);
      issea(parsedDoubleValues[0] == 10000.98531);
      issea(parsedDoubleValues[1] == -20000.765);
      issea(parsedDoubleValues[2] == 30000.939);
      issea(error == false);

      error = true;
      parsedDoubleValues = StringUtilities::fromXmlString<vector<double> >("10000.98531, -20000.765, 30000.939", &error);
      issea(parsedDoubleValues.size() == 3);
      issea(parsedDoubleValues[0] == 10000.98531);
      issea(parsedDoubleValues[1] == -20000.765);
      issea(parsedDoubleValues[2] == 30000.939);
      issea(error == false);

      error = true;
      parsedDoubleValues = StringUtilities::fromDisplayString<vector<double> >("   10000.98531,    -20000.765   ,   30000.939", &error);
      issea(parsedDoubleValues.size() == 3);
      issea(parsedDoubleValues[0] == 10000.98531);
      issea(parsedDoubleValues[1] == -20000.765);
      issea(parsedDoubleValues[2] == 30000.939);
      issea(error == false);

      error = true;
      parsedDoubleValues = StringUtilities::fromXmlString<vector<double> >("   10000.98531,    -20000.765   ,   30000.939", &error);
      issea(parsedDoubleValues.size() == 3);
      issea(parsedDoubleValues[0] == 10000.98531);
      issea(parsedDoubleValues[1] == -20000.765);
      issea(parsedDoubleValues[2] == 30000.939);
      issea(error == false);

      ColorType color(54, 32, 28, 74);
      error = true;
      issea(StringUtilities::toDisplayString(color, &error) == "#4a36201c");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(color, &error) == "#4a36201c");
      issea(error == false);

      error = true;
      ColorType newColor = StringUtilities::fromDisplayString<ColorType>("#4a36201c", &error);
      issea(newColor == color);
      issea(error == false);

      error = true;
      newColor = StringUtilities::fromXmlString<ColorType>("#4a36201c", &error);
      issea(newColor == color);
      issea(error == false);

      ColorType opaqueColor = color;
      opaqueColor.mAlpha = 255;
      error = true;
      newColor = StringUtilities::fromDisplayString<ColorType>("#36201c", &error);
      issea(newColor == opaqueColor);
      issea(error == false);

      error = true;
      newColor = StringUtilities::fromXmlString<ColorType>("#36201c", &error);
      issea(newColor == opaqueColor);
      issea(error == false);

      ColorType invalidColor;
      error = true;
      issea(StringUtilities::toDisplayString(invalidColor, &error) == "InvalidColor");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(invalidColor, &error) == "InvalidColor");
      issea(error == false);

      error = true;
      ColorType newInvalidColor = StringUtilities::fromXmlString<ColorType>("InvalidColor", &error);
      issea(newInvalidColor == invalidColor);
      issea(error == false);
      
      error = true;
      newInvalidColor = StringUtilities::fromDisplayString<ColorType>("InvalidColor", &error);
      issea(newInvalidColor == invalidColor);
      issea(error == false);


      FactoryResource<DateTime> pDateTime;
      pDateTime->set(2004, 1, 20, 5, 30, 54);
      error = true;
      issea(StringUtilities::toDisplayString(pDateTime.get(), &error) == "January 20, 2004, 05:30:54");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(pDateTime.get(), &error) == "2004-01-20T05:30:54Z");
      issea(error == false);

      error = false;
      issea(StringUtilities::toDisplayString(static_cast<DateTime*>(NULL), &error) == "");
      issea(error == true);

      error = false;
      issea(StringUtilities::toXmlString(static_cast<DateTime*>(NULL), &error) == "");
      issea(error == true);

      FactoryResource<DateTime> pDateTime2;
      issea(pDateTime2->isValid() == false);
      error = true;
      issea(StringUtilities::toDisplayString(pDateTime2.get(), &error) == "Invalid DateTime");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(pDateTime2.get(), &error) == "InvalidDateTime");
      issea(error == false);

      FactoryResource<DateTime> pDateTime3;
      pDateTime3->set(2004, 1, 20);
      error = true;
      issea(StringUtilities::toDisplayString(pDateTime3.get(), &error) == "January 20, 2004");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(pDateTime3.get(), &error) == "2004-01-20");
      issea(error == false);

      error = true;
      FactoryResource<DateTime> pParsedDate(StringUtilities::fromDisplayString<DateTime*>("January 20, 2004, 05:30:54", &error));
      issea(pParsedDate->isValid() == true);
      issea(pParsedDate->getStructured() == 1074576654);
      issea(error == false);

      error = true;
      FactoryResource<DateTime> pParsedDate2(StringUtilities::fromXmlString<DateTime*>("2004-01-20T05:30:54Z", &error));
      issea(pParsedDate2->isValid() == true);
      issea(pParsedDate2->getStructured() == 1074576654);
      issea(error == false);

      error = false;
      FactoryResource<DateTime> pParsedDate3(StringUtilities::fromDisplayString<DateTime*>("", &error));
      issea(pParsedDate3.get() == NULL);
      issea(error == true);

      error = false;
      FactoryResource<DateTime> pParsedDate4(StringUtilities::fromXmlString<DateTime*>("", &error));
      issea(pParsedDate4.get() == NULL);
      issea(error == true);

      error = true;
      FactoryResource<DateTime> pParsedDate5(StringUtilities::fromDisplayString<DateTime*>("Invalid DateTime", &error));
      issea(pParsedDate5.get() != NULL);
      issea(pParsedDate5->isValid() == false);
      issea(error == false);

      error = true;
      FactoryResource<DateTime> pParsedDate6(StringUtilities::fromXmlString<DateTime*>("InvalidDateTime", &error));
      issea(pParsedDate6.get() != NULL);
      issea(pParsedDate6->isValid() == false);
      issea(error == false);

      error = true;
      FactoryResource<DateTime> pParsedDate7(StringUtilities::fromDisplayString<DateTime*>("January 20, 2004", &error));
      issea(pParsedDate7.get() != NULL);
      issea(pParsedDate7->isValid() == true);
      issea(pParsedDate7->isTimeValid() == false);
      issea(pParsedDate7->getStructured() == 1074556800);
      issea(error == false);

      error = true;
      FactoryResource<DateTime> pParsedDate8(StringUtilities::fromXmlString<DateTime*>("2004-01-20", &error));
      issea(pParsedDate8.get() != NULL);
      issea(pParsedDate8->isValid() == true);
      issea(pParsedDate8->isTimeValid() == false);
      issea(pParsedDate8->getStructured() == 1074556800);
      issea(error == false);

#if defined(WIN_API)
      string originalFilename = "T:\\Foo/Bar Baz.txt";
      string displayVersion = "T:/Foo/Bar Baz.txt";
      string xmlVersion = "file:///T:/Foo/Bar%20Baz.txt";
#else
      string originalFilename = "/Foo/Bar Baz.txt";
      string displayVersion = "/Foo/Bar Baz.txt";
      string xmlVersion = "file:///Foo/Bar%20Baz.txt";
#endif

      FactoryResource<Filename> pFilename;
      pFilename->setFullPathAndName(originalFilename); //this path doesn't need to exist for this test to work
      error = true;
      issea(StringUtilities::toDisplayString(pFilename.get(), &error) == displayVersion);
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(pFilename.get(), &error) == xmlVersion);
      issea(error == false);

      error = false;
      issea(StringUtilities::toDisplayString(static_cast<Filename*>(NULL), &error) == "");
      issea(error == true);

      error = false;
      issea(StringUtilities::toXmlString(static_cast<Filename*>(NULL), &error) == "");
      issea(error == true);

      error = true;
      FactoryResource<Filename> pParsedFilename(StringUtilities::fromDisplayString<Filename*>(displayVersion, &error));
      issea(pParsedFilename->getFullPathAndName() == displayVersion);
      issea(error == false);

      error = true;
      FactoryResource<Filename> pParsedFilename2(StringUtilities::fromXmlString<Filename*>(xmlVersion, &error));
      issea(pParsedFilename2->getFullPathAndName() == displayVersion);
      issea(error == false);

      error = true;
      FactoryResource<Filename> pParsedFilename3(StringUtilities::fromXmlString<Filename*>(displayVersion, &error));
      issea(pParsedFilename3->getFullPathAndName() == displayVersion);
      issea(error == false);

      error = true;
      FactoryResource<Filename> pParsedFilename4(StringUtilities::fromXmlString<Filename*>("", &error));
      issea(pParsedFilename4.get() != NULL);
      issea(pParsedFilename4->getFullPathAndName() == "");
      issea(error == false);
      
      vector<Filename*> origFilenames;
      string displayFilenameVecStr;
      string xmlFilenameVecStr;
      vector<string> displayFilenames;
      vector<string> xmlFilenames;
#if defined(WIN_API)
      FactoryResource<Filename> pOrgFile1;
      pOrgFile1->setFullPathAndName("T:/Foo/Bar Baz.txt");
      origFilenames.push_back(pOrgFile1.get());
      FactoryResource<Filename> pOrgFile2;
      pOrgFile2->setFullPathAndName("T:/My Test Path/With Spaces/File.foo.bar");
      origFilenames.push_back(pOrgFile2.get());
      displayFilenameVecStr = "T:/Foo/Bar Baz.txt, T:/My Test Path/With Spaces/File.foo.bar";
      xmlFilenameVecStr = "file:///T:/Foo/Bar%20Baz.txt, file:///T:/My%20Test%20Path/With%20Spaces/File.foo.bar";
#else
      FactoryResource<Filename> pOrgFile1;
      pOrgFile1->setFullPathAndName("/Foo/Bar Baz.txt");
      origFilenames.push_back(pOrgFile1.get());
      FactoryResource<Filename> pOrgFile2;
      pOrgFile2->setFullPathAndName("/My Test Path/With Spaces/File.foo.bar");
      origFilenames.push_back(pOrgFile2.get());
      displayFilenameVecStr = "/Foo/Bar Baz.txt, /My Test Path/With Spaces/File.foo.bar";
      xmlFilenameVecStr = "file:///Foo/Bar%20Baz.txt, file:///My%20Test%20Path/With%20Spaces/File.foo.bar";
#endif

      error = true;
      issea(StringUtilities::toDisplayString(origFilenames, &error) == displayFilenameVecStr);
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(origFilenames, &error) == xmlFilenameVecStr);
      issea(error == false);

      error = true;
      issea(StringUtilities::toDisplayString(vector<Filename*>(), &error) == "");
      issea(error == false);

      error = true;
      issea(StringUtilities::toXmlString(vector<Filename*>(), &error) == "");
      issea(error == false);

      error = true;
      vector<Filename*> displayFilenameVec = StringUtilities::fromDisplayString<vector<Filename*> >(displayFilenameVecStr, &error);
      issea(displayFilenameVec.size() == 2);
      issea(*(displayFilenameVec[0]) == *(origFilenames[0]));
      FactoryResource<Filename> pCleanup1(displayFilenameVec[0]);
      issea(*(displayFilenameVec[1]) == *(origFilenames[1]));
      FactoryResource<Filename> pCleanup2(displayFilenameVec[1]);
      issea(error == false);

      error = true;
      vector<Filename*> xmlFilenameVec = StringUtilities::fromXmlString<vector<Filename*> >(xmlFilenameVecStr, &error);
      issea(xmlFilenameVec.size() == 2);
      issea(*(xmlFilenameVec[0]) == *(origFilenames[0]));
      FactoryResource<Filename> pCleanup3(xmlFilenameVec[0]);
      issea(*(xmlFilenameVec[1]) == *(origFilenames[1]));
      FactoryResource<Filename> pCleanup4(xmlFilenameVec[1]);
      issea(error == false);

      error = true;
      vector<Filename*> emptyFilenameVec = StringUtilities::fromXmlString<vector<Filename*> >("", &error);
      issea(emptyFilenameVec.size() == 0);
      issea(error == false);

      return success;
   }
};

class SafeSlotObserver
{
public:
   SafeSlotObserver(Subject *pSubject)
   {
      pSubject->attach(Subject::signalModified(), SafeSlot(this, &SafeSlotObserver::update, &mInvalidator));
   }
   void update(Subject &subject, const string &signal, const boost::any &v)
   {
      // do nothing
   }
   // Slot requires that the Observer have at least one virtual method
   virtual ~SafeSlotObserver()
   {
   }
private:
   SlotInvalidator mInvalidator;
};

class SafeSlotObserver2
{
public:
   SafeSlotObserver2()
   {
   }
   void update(Subject &subject, const string &signal, const boost::any &v)
   {
      // do nothing
   }
   // Slot requires that the Observer have at least one virtual method
   virtual ~SafeSlotObserver2()
   {
   }
};

class SafeSlotObserver3
{
public:
   SafeSlotObserver3(Subject *pSubject) : mpInvalidator(new SlotInvalidator), mNotifyCount(0)
   {
      pSubject->attach(Subject::signalModified(), SafeSlot(this, &SafeSlotObserver3::update, mpInvalidator));
   }
   void update(Subject &subject, const string &signal, const boost::any &v)
   {
      static bool first = true;
      ++mNotifyCount;
      delete mpInvalidator;
      if (first)
      {
         mpInvalidator = new SlotInvalidator;
         subject.attach(Subject::signalModified(), SafeSlot(this, &SafeSlotObserver3::update, mpInvalidator));
      }
      else
      {
         mpInvalidator = NULL;
      }
      first = false;
   }
   // Slot requires that the Observer have at least one virtual method
   virtual ~SafeSlotObserver3()
   {
   }

   int getNotifyCount() const
   {
      return mNotifyCount;
   }

private:
   SlotInvalidator *mpInvalidator;
   int mNotifyCount;
};

class AutoObserver : public SlotInvalidator
{
public:
   AutoObserver(Subject *pSubject)
   {
      pSubject->attach(Subject::signalModified(), AutoSlot(this, &AutoObserver::update));
   }
   void update(Subject &subject, const string &signal, const boost::any &v)
   {
      // do nothing
   }
   // Slot requires that the Observer have at least one virtual method
   virtual ~AutoObserver()
   {
   }
};

class SafeSlotTest  : public TestCase
{
public:
   SafeSlotTest() : TestCase("SafeSlot") {}
   bool run()
   {
      bool success = true;

      //Delete an observer while attached
      CustomSubject* pSubj = new CustomSubject();
      SafeSlotObserver *pObs = new SafeSlotObserver(pSubj);
      issea(false == pSubj->detach(Subject::signalModified(), SafeSlot(pObs, &SafeSlotObserver::update, NULL)));
      issea(pSubj->getSlots(Subject::signalModified()).size() == 1);
      pSubj->notify(Subject::signalModified());
      delete pObs;
      pSubj->notify(Subject::signalModified());
      issea(pSubj->getSlots(Subject::signalModified()).size() == 0);
      delete pSubj;

      //Delete an auto-observer while attached
      pSubj = new CustomSubject();
      AutoObserver *pObs2 = new AutoObserver(pSubj);
      issea(false == pSubj->detach(Subject::signalModified(), SafeSlot(pObs2, &AutoObserver::update, NULL)));
      issea(pSubj->getSlots(Subject::signalModified()).size() == 1);
      pSubj->notify(Subject::signalModified());
      delete pObs2;
      pSubj->notify(Subject::signalModified());
      issea(pSubj->getSlots(Subject::signalModified()).size() == 0);
      delete pSubj;

      //Delete invalidator while observer still exists
      pSubj = new CustomSubject();
      SafeSlotObserver2 *pObs3 = new SafeSlotObserver2;
      {
         SlotInvalidator invalidator;
         issea(true == pSubj->attach(Subject::signalModified(), SafeSlot(pObs3, &SafeSlotObserver2::update, &invalidator)));
         issea(pSubj->getSlots(Subject::signalModified()).size() == 1);
         pSubj->notify(Subject::signalModified());
      } // invalidator destroyed, and the slot is detached here
      pSubj->notify(Subject::signalModified());
      issea(pSubj->getSlots(Subject::signalModified()).size() == 0);
      delete pObs3;
      pSubj->notify(Subject::signalModified());
      delete pSubj;

      //Delete invalidator during notification
      pSubj = new CustomSubject();
      SafeSlotObserver3 *pObs4 = new SafeSlotObserver3(pSubj);
      issea(pSubj->getSlots(Subject::signalModified()).size() == 1);
      issea(pObs4->getNotifyCount() == 0);
      pSubj->notify(Subject::signalModified()); // will delete the invalidator causing detach/reattach
      issea(pObs4->getNotifyCount() == 1);
      issea(pSubj->getSlots(Subject::signalModified()).size() == 1);
      pSubj->notify(Subject::signalModified()); // will delete the invalidator causing detach
      issea(pObs4->getNotifyCount() == 2);
      issea(pSubj->getSlots(Subject::signalModified()).size() == 0);
      pSubj->notify(Subject::signalModified()); // will not get to pObs4->update
      issea(pObs4->getNotifyCount() == 2);
      issea(false == pSubj->detach(Subject::signalModified(), SafeSlot(pObs4, &SafeSlotObserver3::update, NULL)));
      delete pObs4;
      pSubj->notify(Subject::signalModified());
      delete pSubj;

      //Delete invalidator after manually detaching
      pSubj = new CustomSubject();
      pObs3 = new SafeSlotObserver2;
      SlotInvalidator *pInvalidator = NULL;
      {
         SlotInvalidator invalidator;
         pInvalidator = &invalidator;
         issea(true == pSubj->attach(Subject::signalModified(), SafeSlot(pObs3, &SafeSlotObserver2::update, &invalidator)));
         issea(pSubj->getSlots(Subject::signalModified()).size() == 1);
         pSubj->notify(Subject::signalModified());
         issea(true == pSubj->detach(Subject::signalModified(), SafeSlot(pObs3, &SafeSlotObserver2::update, &invalidator)));
         issea(pSubj->getSlots(Subject::signalModified()).size() == 0);
         pSubj->notify(Subject::signalModified());
      } // invalidator destroyed here
      issea(false == pSubj->detach(Subject::signalModified(), SafeSlot(pObs3, &SafeSlotObserver2::update, pInvalidator)));
      delete pObs3;
      pSubj->notify(Subject::signalModified());
      delete pSubj;

      return success;
   }
};

class UtilityTestSuite : public TestSuiteNewSession
{
public:
   UtilityTestSuite() : TestSuiteNewSession( "Utility" )
   {
      addTestCase( new ConfigurationSettingsLookupTest );
      addTestCase( new PluginCheckTest );
      addTestCase( new TimeUtilitiesTestCase );
      addTestCase( new DateTimeTestCase );
      addTestCase( new SubjectObserverTest );
      addTestCase( new SubjectNotifyTest );
      addTestCase( new EnumWrapperTest );
      addTestCase( new StringUtilitiesTest );
      addTestCase( new SafeSlotTest );
   }
};

REGISTER_SUITE( UtilityTestSuite )
