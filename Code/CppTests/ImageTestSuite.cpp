/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "assert.h"
#include "AppConfig.h"
#include "DataAccessorImpl.h"
#include "DataElement.h"
#include "DataRequest.h"
#include "DesktopServices.h"
#include "Executable.h"
#include "Image.h"
#include "Layer.h"
#include "LayerList.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterLayer.h"
#include "RasterLayerImp.h"
#include "RasterUtilities.h"
#include "SpatialDataViewImp.h"
#include "SpatialDataWindow.h"
#include "Statistics.h"
#include "TestSuiteNewSession.h"
#include "TestUtilities.h"

#if defined(CG_SUPPORTED)
#include "GpuImage.h"
#endif

#include <fstream>
#include <limits>
#include <vector>

using namespace std;

class FilterRedrawTestCase : public TestCase
{
public:
   FilterRedrawTestCase() : TestCase("FilterRedraw") {}
   bool run()
   {
      // Tests COMETIV-294
      bool success = true;

      ImporterResource pImporter("ENVI Importer", TestUtilities::getTestDataPath() +
         "CreateChip/cube512x512x10x4flsb.bip.hdr", NULL, false);
      issearf(pImporter->execute());
      SpatialDataView *pSDView = 
         pImporter->getOutArgList().getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
      SpatialDataViewImp *pSDViewImp = dynamic_cast<SpatialDataViewImp*>(pSDView);
      issearf(pSDView != NULL && pSDViewImp != NULL);

      RasterLayerImp *pLayer = dynamic_cast<RasterLayerImp*>(pSDView->getTopMostLayer(RASTER));
      issearf(pLayer != NULL);
      DataElement *pElement = pLayer->getDataElement();
      issearf(pElement != NULL);
      RasterDataDescriptor *pDescriptor = dynamic_cast<RasterDataDescriptor*>(pElement->getDataDescriptor());
      issearf(pDescriptor != NULL);

#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : What to do if GPU unavailable? (leckels)")
      issearf(pLayer->isGpuImageSupported());
      issearf(pLayer->isFilterSupported("ByPass"));

      pLayer->enableGpuImage(true);
      pLayer->enableFilter("ByPass");

      issearf(pLayer->isGpuImageEnabled());
      issearf(pLayer->isFilterEnabled("ByPass"));

      pSDViewImp->repaint();

      Image *pImage = pLayer->getImage();
      issearf(pImage != NULL);

      // Make sure tiles don't need to be updated, since nothing has changed
      std::vector<unsigned int> tileZoomIndicies;
      std::vector<Tile*> tiles = pImage->getTilesToUpdate(pImage->getTilesToDraw(), tileZoomIndicies);
      issearf(tiles.empty());

      // change something to make tile updates required
      pLayer->setDisplayedBand(GRAY, pDescriptor->getActiveBand(1));
      pLayer->generateImage();
      tiles = pImage->getTilesToUpdate(pImage->getTilesToDraw(), tileZoomIndicies);
      issearf(!tiles.empty());

      // Draw to update tiles
      pSDViewImp->repaint();

      // make sure they don't need to be updated again
      tiles = pImage->getTilesToUpdate(pImage->getTilesToDraw(), tileZoomIndicies);
      issearf(tiles.empty());

      Service<DesktopServices>()->deleteView(pSDView);
      
      return success;
   }
};

class HistogramEqualizationTestCase : public TestCase
{
public:
   HistogramEqualizationTestCase() : TestCase("HistogramEqualization") {}
   bool run()
   {
      // Tests COMETIV-921
      bool success = true;

      ImporterResource pImporter("ENVI Importer", TestUtilities::getTestDataPath() + "test_black_one_band.hdr",
         NULL, false);
      issearf(pImporter->execute());
      SpatialDataView *pSDView = 
         pImporter->getOutArgList().getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
      SpatialDataViewImp *pSDViewImp = dynamic_cast<SpatialDataViewImp*>(pSDView);
      issearf(pSDView != NULL && pSDViewImp != NULL);
      LayerList* pList = pSDView->getLayerList();
      issearf(pList != NULL);
      DataElement* pElem = static_cast<DataElement*>(pList->getPrimaryRasterElement());
      issearf(pElem != NULL);
      RasterLayer* pLayer = dynamic_cast<RasterLayer*>(pList->getLayer(RASTER, pElem));
      issearf(pLayer != NULL);
      pLayer->setStretchType(GRAYSCALE_MODE, EQUALIZATION);

      // force redraw to test divide by zero check in Image::prepareScale
      pSDViewImp->repaint();
      if (pLayer->getStretchType(GRAYSCALE_MODE) != EQUALIZATION)
      {
         success = false;
      }

      Service<DesktopServices>()->deleteView(pSDView);

      return success;
   }
};


class FeedbackBufferTestCase : public TestCase
{
public:
   FeedbackBufferTestCase() : TestCase("FeedbackBuffer") {}
   bool run()
   {
      bool success = true;

#if defined(CG_SUPPORTED)
      bool oldAlwaysAlpha = GpuImage::mAlwaysAlpha;
      GpuImage::mAlwaysAlpha = true;

      // GpuImage::mAlwaysAlpha should force alpha to be on
      success = runWithAlpha(false, true);
      success = success && runWithAlpha(true, true);

      // Allow alpha to be off as appropriate
      GpuImage::mAlwaysAlpha = false;

      success = runWithAlpha(true, true);

      // Restore alwaysAlpha setting
      GpuImage::mAlwaysAlpha = oldAlwaysAlpha;
#else
      success = assertProc(false, "CG_SUPPORTED == true", __FILE__, __LINE__);
#endif

      return success;
   }

   bool runWithAlpha(bool alphaOn, bool alphaShouldBeOn)
   {
      // Tests OPTICKS-176
      bool success = true;

#if defined(CG_SUPPORTED)
      // Set small texture size to allow multi-tile feedback on small dataset
      GpuImage::setMaxTextureSize(64);

      // Load dataset
      ImporterResource pImporter("GeoTIFF Importer", TestUtilities::getTestDataPath() + "landsat6band.tif",
         NULL, false);
      issearf(pImporter->execute());

      // Get primary raster layer
      SpatialDataView *pSDView = 
         pImporter->getOutArgList().getPlugInArgValue<SpatialDataView>(Executable::ViewArg());
      SpatialDataViewImp *pSDViewImp = dynamic_cast<SpatialDataViewImp*>(pSDView);
      issearf(pSDView != NULL && pSDViewImp != NULL);
      LayerList* pList = pSDView->getLayerList();
      issearf(pList != NULL);
      RasterElement* pElem = dynamic_cast<RasterElement*>(pList->getPrimaryRasterElement());
      issearf(pElem != NULL);
      RasterLayer* pLayer = dynamic_cast<RasterLayer*>(pList->getLayer(RASTER, pElem));
      issearf(pLayer != NULL);

      // Set/clear bad values to enable/disable alpha in the image
      RasterDataDescriptor* pDesc = dynamic_cast<RasterDataDescriptor*>(pElem->getDataDescriptor());
      issearf(pDesc != NULL);
      unsigned int bandCount = pDesc->getBandCount();
      vector<DimensionDescriptor> bands = pDesc->getBands();
      vector<int> badValues;
      if (alphaOn == true)
      {
         badValues.push_back(-1); // -1 will not change stretch
      }
      for (vector<DimensionDescriptor>::iterator pBand=bands.begin(); 
         pBand!=bands.end(); 
         ++pBand)
      {
         Statistics *pStat = pElem->getStatistics(*pBand);
         issearf(pStat != NULL);
         pStat->setBadValues(badValues);
      }

      pElem->updateData();

      // Turn on GPU and the bypass filter
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : What to do if GPU unavailable? (tjohnson)")
      issearf(pLayer->isGpuImageSupported());
      issearf(pLayer->isFilterSupported("ByPass"));
      pLayer->enableGpuImage(true);
      pLayer->enableFilter("ByPass");

      // Force redraw to populate the feedback buffer
      pSDViewImp->repaint();

      // Read a big portion of the buffer
      const int bigSize = 140;
      std::vector<float> bigValues;
      bool bigHasAlphas = false;
      pLayer->readFilterBuffer(118, 120, bigSize, bigSize, bigValues, bigHasAlphas);
      int bigStep = (bigHasAlphas?2:1);
      issearf(bigHasAlphas == alphaShouldBeOn);

      // Read a small portion of the buffer
      const int smallSize = 6;
      std::vector<float> smallValues;
      bool smallHasAlphas = false;
      pLayer->readFilterBuffer(118, 120, smallSize, smallSize, smallValues, smallHasAlphas);
      int smallStep = (smallHasAlphas?2:1);
      issearf(smallHasAlphas == alphaShouldBeOn);

      Service<DesktopServices>()->deleteView(pSDView);

      // Reset max texture size to the HW limit
      GpuImage::setMaxTextureSize(0);

      // Read truth file
      const int size = bigSize*bigSize;
      float properValues[size];
      FILE *pFile = fopen((TestUtilities::getTestDataPath() + "FeedbackBufferTestResults.txt").c_str(), "r");
      issearf(pFile != NULL);
      int count = 0;
      for (int i=0; i<size; ++i)
      {
         count += fscanf(pFile, "%g", &properValues[i]);
      }
      fclose(pFile);
      issearf(count == size);

      // Compare truth with values read from the buffer
      for (int i=0; i<size; ++i)
      {
         issearf_ae(6, bigValues[bigStep*i], properValues[i]);
      }

      // Compare truth with values read in from a single tile
      for (int i=0; i<smallSize; ++i)
      {
         for (int j=0; j<smallSize; ++j)
         {
            float truth = properValues[i*bigSize + j];
            float value = smallValues[smallStep*(i*smallSize + j)];
            issearf_ae(6, value, truth);
         }
      }
#else
      success = assertProc(false, "CG_SUPPORTED == true", __FILE__, __LINE__);
#endif

      return success;
   }
};

class GpuStretchTestCase : public TestCase
{
public:
   GpuStretchTestCase() : TestCase("GpuStretch") {}
   bool run()
   {
      // Tests readFilterBuffer to ensure that data passed to it returns 
      // unchanged when the filter does no math on the texture.
      bool success = true;

#if defined(CG_SUPPORTED)
      // Set small texture size to allow multi-tile feedback on small dataset
      GpuImage::setMaxTextureSize(64);

      success = success && testEncodingType<unsigned char>();
      success = success && testEncodingType<signed char>();
      success = success && testEncodingType<unsigned short>();
      success = success && testEncodingType<short>();
      success = success && testEncodingType<unsigned int>();
      success = success && testEncodingType<int>();
      success = success && testEncodingType<float>();
      success = success && testEncodingType<double>();

      // Reset max texture size to the HW limit
      GpuImage::setMaxTextureSize(0);
#else
      success = assertProc(false, "CG_SUPPORTED == true", __FILE__, __LINE__);
#endif

      return success;
   }

#if defined(CG_SUPPORTED)
private:
   static const int mRows = 256;
   static const int mColumns = 256;
   static const int mBands = 8;
   template<typename T> EncodingType getEncodingType() { return UNKNOWN; }
   template<> EncodingType getEncodingType<unsigned char>() { return INT1UBYTE; }
   template<> EncodingType getEncodingType<signed char>() { return INT1SBYTE; }
   template<> EncodingType getEncodingType<unsigned short>() { return INT2UBYTES; }
   template<> EncodingType getEncodingType<short>() { return INT2SBYTES; }
   template<> EncodingType getEncodingType<unsigned int>() { return INT4UBYTES; }
   template<> EncodingType getEncodingType<int>() { return INT4SBYTES; }
   template<> EncodingType getEncodingType<float>() { return FLT4BYTES; }
   template<> EncodingType getEncodingType<double>() { return FLT8BYTES; }
   template<typename T> T pixelValue(int band, int row, int col)
   {
      int value = (row + col) * band;
      if (value > 255)
      {
         value = 255;
      }
      if (std::numeric_limits<T>::min() < 0)
      {
         value -= 128;
      }
      return static_cast<T>(value);
   }
   template<typename T> bool testEncodingType()
   {
      bool success = true;
      cout << "  " << this->getName() << ": type = " << typeid(T()).name() << endl;
      ModelResource<RasterElement> pRaster (createRaster<T>());
      issearf(pRaster.get() != NULL);
      RasterLayer *pLayer = createView(pRaster.get());
      success = verifyRaster<T>(pLayer, pRaster.get());
      return success;
   }
   template<typename T>
   ModelResource<RasterElement> createRaster()
   {
      bool success = true;
      ModelResource<RasterElement> pNullRaster(static_cast<RasterElement*>(NULL));
      ModelResource<RasterElement> pRaster (RasterUtilities::createRasterElement("GpuStretchTestDataSet", mRows, mColumns,
         mBands, getEncodingType<T>(), BSQ));
      issear(pRaster.get() != NULL, pNullRaster);
      RasterDataDescriptor *pDesc = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
      issear(pDesc != NULL, pNullRaster);
      for (int band=0; band<mBands; ++band)
      {
         FactoryResource<DataRequest> pRequest;
         pRequest->setBands(pDesc->getActiveBand(band), pDesc->getActiveBand(band));
         DataAccessor da = pRaster->getDataAccessor(pRequest.release());
         for (int row=0; row<mRows; ++row)
         {
            issear(da.isValid(), pNullRaster);
            T* pData = static_cast<T*>(da->getRow());
            for (int col=0; col<mColumns; ++col)
            {
               *pData++ = pixelValue<T>(band, row, col);
            }
            da->nextRow();
         }
      }
      return pRaster;
   }
   RasterLayer *createView(RasterElement *pRaster)
   {
      bool success = true;
      SpatialDataWindow *pWindow = dynamic_cast<SpatialDataWindow*>(Service<DesktopServices>()->createWindow(
         "GpuStretchTestCase", SPATIAL_DATA_WINDOW));
      issear(pWindow != NULL, NULL);
      SpatialDataView *pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      issear(pView != NULL, NULL);
      RasterLayer *pLayer = dynamic_cast<RasterLayer*>(pView->createLayer(RASTER, pRaster));
      issear(pLayer != NULL, NULL);
      pView->setPrimaryRasterElement(pRaster);
      pLayer->enableGpuImage(true);
      // readFilterBuffer needs to have a filter enabled to work. The ByPass 
      // filter is used because it does not modify the data passed to it.
      pLayer->enableFilter("ByPass");
      return pLayer;
   }
   template<typename T>
   bool verifyRaster(RasterLayer *pLayer, RasterElement *pRaster)
   {
      bool success = true;
      RasterDataDescriptor *pDesc = dynamic_cast<RasterDataDescriptor*>(pRaster->getDataDescriptor());
      issearf(pDesc != NULL);

      for (int band=0; band<mBands; ++band)
      {
         pLayer->setDisplayedBand(GRAY, pDesc->getActiveBand(band));
         vector<float> values;
         bool hasAlphas = false;
         pLayer->readFilterBuffer(0, 0, mRows, mColumns, values, hasAlphas);
         int stepSize = (hasAlphas?2:1);
         for (int row=0; row<mRows; ++row)
         {
            for (int col=0; col<mColumns; ++col)
            {
               int index = (row*mColumns + col)*stepSize;
               float value = values[index];
               bool closeEnough = (fabs(value - pixelValue<T>(band, row, col)) <= 0.002) ||
                  (ApproxDouble(value, 5) == ApproxDouble(pixelValue<T>(band, row, col), 5));
               if (!closeEnough)
               {
                  std::cout << "value = " << value << ", pixelValue = " << 
                     static_cast<int>(pixelValue<T>(band, row, col)) << std::endl;
                  // the static_cast<int> is necessary to make the cout display
                  // integer values for character data types
               }
               issearf(closeEnough);
            }
         }
      }
      return success;
   }
#endif
};

class ImageTestSuite : public TestSuiteNewSession
{
public:
   ImageTestSuite() : TestSuiteNewSession( "Image" )
   {
   #ifdef CG_SUPPORTED
      addTestCase( new FilterRedrawTestCase );
      addTestCase( new HistogramEqualizationTestCase );
      addTestCase( new FeedbackBufferTestCase );
      addTestCase( new GpuStretchTestCase );
   #endif
   }
};

REGISTER_SUITE( ImageTestSuite )
