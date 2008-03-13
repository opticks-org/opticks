/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GpuImage.h"
#include "CgContext.h"
#include "ColorBuffer.h"
#include "glCommon.h"
#include "GpuTile.h"
#include "ImageFilter.h"
#include "ImageUtilities.h"
#include "MathUtil.h"
#include "MultiThreadedAlgorithm.h"
#include "RasterDataDescriptor.h"
#include "RasterUtilities.h"
#include "UtilityServicesImp.h"

#include <limits>
using namespace mta;
using namespace std;

GpuImage::GpuImage() :
   mGrayscaleProgram(0),
   mColormapProgram(0),
   mColormapTexture(0),
   mRgbProgram(0),
   mpCgProgram(NULL),
   mFragmentProfile(CG_PROFILE_UNKNOWN),
   mInputTexture(0),
   mPreviousBand(0)
{
   // Load the Cg programs and fragment profile
   CgContext* pCgContext = CgContext::instance();
   if (pCgContext != NULL)
   {
      mGrayscaleProgram = pCgContext->loadFragmentProgram("GrayscaleDisplay.cg");
      mColormapProgram = pCgContext->loadFragmentProgram("ColormapDisplay.cg");
      mRgbProgram = pCgContext->loadFragmentProgram("RgbDisplay.cg");
      mFragmentProfile = pCgContext->getFragmentProfile();
   }
}

GpuImage::~GpuImage()
{
   // Delete the colormap texture
   if (mColormapTexture != 0)
   {
      glDeleteTextures(1, &mColormapTexture);
   }

   // Unload and destroy Cg programs
   CgContext* pCgContext = CgContext::instance();
   if (pCgContext != NULL)
   {
      pCgContext->destroyCgProgram("GrayscaleDisplay.cg");
      pCgContext->destroyCgProgram("ColormapDisplay.cg");
      pCgContext->destroyCgProgram("RgbDisplay.cg");
   }
}

// Grayscale
void GpuImage::initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX,
                          unsigned int imageSizeY, unsigned int channels, GLenum format, EncodingType type,
                          void* pData, StretchType stretchType, vector<double>& stretchPoints,
                          RasterElement *pRasterElement, const vector<int>& badValues)
{
   int tileSizeX = 0;
   int tileSizeY = 0;
   calculateTileSize(type, imageSizeX, imageSizeY, tileSizeX, tileSizeY);

   initializeGrayscale();

   GLenum texFormat = ((format == GL_RGBA || format == GL_LUMINANCE_ALPHA)? GL_LUMINANCE_ALPHA : GL_LUMINANCE);
   Image::initialize(tileSizeX, tileSizeY, channel, imageSizeX, imageSizeY, channels, texFormat, type, pData,
      stretchType, stretchPoints, pRasterElement, badValues);
}

void GpuImage::initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX,
                          unsigned int imageSizeY, unsigned int channels, GLenum format, EncodingType type,
                          ComplexComponent component, void* pData, StretchType stretchType,
                          vector<double>& stretchPoints, RasterElement* pRasterElement, const vector<int>& badValues)
{
   int tileSizeX = 0;
   int tileSizeY = 0;
   calculateTileSize(type, imageSizeX, imageSizeY, tileSizeX, tileSizeY);

   initializeGrayscale();

   GLenum texFormat = ((format == GL_RGBA || format == GL_LUMINANCE_ALPHA)? GL_LUMINANCE_ALPHA : GL_LUMINANCE);
   Image::initialize(tileSizeX, tileSizeY, channel, imageSizeX, imageSizeY, channels, texFormat, type, component,
      pData, stretchType, stretchPoints, pRasterElement, badValues);
}

// Colormap
void GpuImage::initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX,
                          unsigned int imageSizeY, unsigned int channels, GLenum format, EncodingType type,
                          void* pData, StretchType stretchType, vector<double>& stretchPoints,
                          RasterElement* pRasterElement, const vector<ColorType>& colorMap, const vector<int>& badValues)
{
   int tileSizeX = 0;
   int tileSizeY = 0;
   calculateTileSize(type, imageSizeX, imageSizeY, tileSizeX, tileSizeY);

   initializeColormap(colorMap);

   GLenum texFormat = ((format == GL_RGBA || format == GL_LUMINANCE_ALPHA)? GL_LUMINANCE_ALPHA : GL_LUMINANCE);
   Image::initialize(tileSizeX, tileSizeY, channel, imageSizeX, imageSizeY, channels, texFormat, type, pData,
      stretchType, stretchPoints, pRasterElement, colorMap, badValues);
}

void GpuImage::initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX,
                          unsigned int imageSizeY, unsigned int channels, GLenum format, EncodingType type,
                          ComplexComponent component, void* pData, StretchType stretchType,
                          vector<double>& stretchPoints, RasterElement* pRasterElement,
                          const vector<ColorType>& colorMap, const vector<int>& badValues)
{
   int tileSizeX = 0;
   int tileSizeY = 0;
   calculateTileSize(type, imageSizeX, imageSizeY, tileSizeX, tileSizeY);

   initializeColormap(colorMap);

   GLenum texFormat = ((format == GL_RGBA || format == GL_LUMINANCE_ALPHA)? GL_LUMINANCE_ALPHA : GL_LUMINANCE);
   Image::initialize(tileSizeX, tileSizeY, channel, imageSizeX, imageSizeY, channels, texFormat, type, component,
      pData, stretchType, stretchPoints, pRasterElement, colorMap, badValues);
}

// RGB
void GpuImage::initialize(int sizeX, int sizeY, DimensionDescriptor band1, DimensionDescriptor band2, DimensionDescriptor band3,
                          unsigned int imageSizeX, unsigned int imageSizeY, unsigned int channels, GLenum format,
                          EncodingType type, void* pData, StretchType stretchType, vector<double>& stretchPointsRed,
                          vector<double>& stretchPointsGreen, vector<double>& stretchPointsBlue,
                          RasterElement* pRasterElement)
{
   initializeRgb();

   int tileSizeX = 0;
   int tileSizeY = 0;
   calculateTileSize(type, imageSizeX, imageSizeY, tileSizeX, tileSizeY);

   Image::initialize(tileSizeX, tileSizeY, band1, band2, band3, imageSizeX, imageSizeY, channels, format, type,
      pData, stretchType, stretchPointsRed, stretchPointsGreen, stretchPointsBlue, pRasterElement);
}

void GpuImage::initialize(int sizeX, int sizeY, DimensionDescriptor band1, DimensionDescriptor band2, DimensionDescriptor band3,
                          unsigned int imageSizeX, unsigned int imageSizeY, unsigned int channels, GLenum format,
                          EncodingType type, ComplexComponent component, void* pData, StretchType stretchType,
                          vector<double>& stretchPointsRed, vector<double>& stretchPointsGreen,
                          vector<double>& stretchPointsBlue, RasterElement* pRasterElement)
{
   initializeRgb();

   int tileSizeX = 0;
   int tileSizeY = 0;
   calculateTileSize(type, imageSizeX, imageSizeY, tileSizeX, tileSizeY);

   Image::initialize(tileSizeX, tileSizeY, band1, band2, band3, imageSizeX, imageSizeY, channels, format, type,
      component, pData, stretchType, stretchPointsRed, stretchPointsGreen, stretchPointsBlue, pRasterElement);
}

void GpuImage::initialize(int sizeX, int sizeY, DimensionDescriptor band1, DimensionDescriptor band2, DimensionDescriptor band3,
                          unsigned int imageSizeX, unsigned int imageSizeY, unsigned int channels, GLenum format,
                          EncodingType type1, EncodingType type2, EncodingType type3, ComplexComponent component,
                          void* pData, StretchType stretchType, vector<double>& stretchPointsRed,
                          vector<double>& stretchPointsGreen, vector<double>& stretchPointsBlue,
                          RasterElement* pRasterElement1, RasterElement* pRasterElement2, RasterElement* pRasterElement3)
{
   initializeRgb();

   // Calculate the tile size
   int tileSizeX = 0;
   int tileSizeY = 0;

   if ((type1 == type2) && (type1 == type3))
   {
      calculateTileSize(type1, imageSizeX, imageSizeY, tileSizeX, tileSizeY);
   }
   else
   {
      // Since the encoding types are different, float textures are created
      calculateTileSize(FLT4BYTES, imageSizeX, imageSizeY, tileSizeX, tileSizeY);
   }

   Image::initialize(tileSizeX, tileSizeY, band1, band2, band3, imageSizeX, imageSizeY, channels, format, type1,
      type2, type3, component, pData, stretchType, stretchPointsRed, stretchPointsGreen, stretchPointsBlue,
      pRasterElement1, pRasterElement2, pRasterElement3);
}

class GpuTileThread;
class GpuTileInput
{
public:
   GpuTileInput(const vector<GpuTile*>& tiles, vector<int>& tileZoomIndices, const Image::ImageData& info) :
      mTiles(tiles), mTileZoomIndices(tileZoomIndices), mInfo(info)
   {
   }

   const vector<GpuTile*>& mTiles;
   vector<int>& mTileZoomIndices;
   const Image::ImageData& mInfo;
};

class GpuTileOutput
{
public:
   bool compileOverallResults(const vector<GpuTileThread*>& threads) { return true; }
};

void GpuImage::updateTiles(vector<Tile*>& tilesToUpdate, vector<int>& tileZoomIndices)
{
   vector<GpuTile*> tiles;
   for (unsigned int i = 0; i < tilesToUpdate.size(); ++i)
   {
      GpuTile* pTile = dynamic_cast<GpuTile*> (tilesToUpdate[i]);
      if (pTile != NULL)
      {
         tiles.push_back(pTile);
      }
   }

   const ImageData& info = getImageData();
   GpuTileInput tileInput(tiles, tileZoomIndices, info);
   GpuTileOutput tileOutput;

   StatusBarReporter barReporter("Generating Image", "app", "6860C9DA-5643-4e32-B8F1-4CD5B01350E9");
   StatusBarReporter *pReporter = NULL;
   if (tiles.size() > 1)
   {
      pReporter = &barReporter;
   }

   unsigned int threadCount = ConfigurationSettings::getSettingThreadCount();
   MultiThreadedAlgorithm<GpuTileInput, GpuTileOutput, GpuTileThread> imageTileAlgorithm
      (threadCount, tileInput, tileOutput, pReporter);
   imageTileAlgorithm.run();
}

class SetGpuTile : public ThreadCommand
{
public:
   SetGpuTile(GpuTile* pTile, void* pData, EncodingType encodingType, int zoomIndex) :
      mpTile(pTile), mpData(pData), mEncodingType(encodingType), mZoomIndex(zoomIndex)
   {
   }

   void run()
   {
      if (mpTile != NULL)
      {
         mpTile->setupTile(mpData, mEncodingType, mZoomIndex);
      }
   }

private:
   GpuTile* mpTile;
   void* mpData;
   EncodingType mEncodingType;
   int mZoomIndex;
};

class GpuTileThread : public AlgorithmThread
{
public:
   GpuTileThread(const GpuTileInput& input, int threadCount, int threadIndex, ThreadReporter& reporter) :
      AlgorithmThread(threadIndex, reporter),
      mTiles(input.mTiles),
      mTileZoomIndices(input.mTileZoomIndices),
      mInfo(input.mInfo),
      mTileRange(getThreadRange(threadCount, mTiles.size()))
      {
      }

   void run();

private:
   const vector<GpuTile*>& mTiles;
   vector<int>& mTileZoomIndices;
   const Image::ImageData& mInfo;
   Range mTileRange;

   template <typename In, typename Out>
   void populateTextureData(vector<Out>& texData, unsigned int tileSizeX, unsigned int tileSizeY,
      DataAccessor da, int currentChannel, int totalChannels, EncodingType outputType)
   {
      std::vector<int>::const_iterator badBegin = mInfo.mKey.mBadValues.begin();
      std::vector<int>::const_iterator badEnd = mInfo.mKey.mBadValues.end();
      bool bBadValues = (badBegin != badEnd);
      bool bHas1BadValue = mInfo.mKey.mBadValues.size() == 1;
      int singleBadValue = 0;
      if (bHas1BadValue)
      {
         singleBadValue = mInfo.mKey.mBadValues.front();
      }

      vector<Out>::iterator targetBase = texData.begin();
      for (unsigned int y1 = 0; y1 < tileSizeY; y1++, targetBase += (mInfo.mTileSizeX * totalChannels))
      {
         vector<Out>::iterator target = targetBase;
         target += (currentChannel - 1);     // Subtract one since currentChannel is a one-based value

         for (unsigned int x1 = 0; x1 < tileSizeX; x1++)
         {
            In* source = reinterpret_cast<In*>(da->getColumn());
            if ((outputType == FLT4BYTES) || (outputType == FLT8BYTES))
            {
               *target = static_cast<Out> (*source);
            }
            else
            {
               *target = static_cast<Out> (*source - numeric_limits<In>::min());
            }

            if (bBadValues)
            {
               double dValue = static_cast<double> (*target);
               int tempInt = roundDouble(dValue);

               target += (totalChannels - currentChannel);
               if (bHas1BadValue)
               {
                  if (tempInt == singleBadValue)
                  {
                     *target = static_cast<Out> (0);
                  }
                  else
                  {
                     *target = numeric_limits<Out>::max();
                  }
               }
               else if (binary_search(badBegin, badEnd, tempInt))
               {
                  *target = static_cast<Out> (0);
               }
               else
               {
                  *target = numeric_limits<Out>::max();
               }

               ++target;
            }
            else
            {
               if (totalChannels == 2)
               {
                  ++target;
                  *target = numeric_limits<Out>::max();
                  ++target;
               }
               else if (static_cast<int>(x1) < mInfo.mTileSizeX - 1)
               {
                  target += totalChannels;
               }
            }

            da->nextColumn();
         }

         da->nextRow();
      }
   }

   template <typename Out>
   void populateTextureData(vector<Out>& texData, EncodingType inputType, unsigned int tileSizeX,
      unsigned int tileSizeY, DataAccessor da, int currentChannel, int totalChannels, EncodingType outputType)
   {
      switch (inputType)
      {
         case INT1UBYTE:
            populateTextureData<unsigned char>(texData, tileSizeX, tileSizeY, da,
               currentChannel, totalChannels, outputType);
            break;

         case INT1SBYTE:
            populateTextureData<signed char>(texData, tileSizeX, tileSizeY, da,
               currentChannel, totalChannels, outputType);
            break;

         case INT2UBYTES:
            populateTextureData<unsigned short>(texData, tileSizeX, tileSizeY, da,
               currentChannel, totalChannels, outputType);
            break;

         case INT2SBYTES:
            populateTextureData<signed short>(texData, tileSizeX, tileSizeY, da,
               currentChannel, totalChannels, outputType);
            break;

         case INT4UBYTES:
            populateTextureData<unsigned int>(texData, tileSizeX, tileSizeY, da,
               currentChannel, totalChannels, outputType);
            break;

         case INT4SBYTES:
            populateTextureData<signed int>(texData, tileSizeX, tileSizeY, da,
               currentChannel, totalChannels, outputType);
            break;

         case FLT4BYTES:
            populateTextureData<float>(texData, tileSizeX, tileSizeY, da,
               currentChannel, totalChannels, outputType);
            break;

         case FLT8BYTES:
            populateTextureData<double>(texData, tileSizeX, tileSizeY, da,
               currentChannel, totalChannels, outputType);
            break;

         case UNKNOWN:        // Fall through to the next case statement
         case INT4SCOMPLEX:   // Fall through to the next case statement
         case FLT8COMPLEX:    // Fall through to the next case statement
         default:
            break;
      }
   }

   template <typename In, typename Out>
   void populateEmptyTextureData(vector<Out>& texData, unsigned int tileSizeX, unsigned int tileSizeY,
      int currentChannel, int totalChannels, EncodingType outputType)
   {
      std::vector<int>::const_iterator badBegin = mInfo.mKey.mBadValues.begin();
      std::vector<int>::const_iterator badEnd = mInfo.mKey.mBadValues.end();
      bool bBadValues = (badBegin != badEnd);

      vector<Out>::iterator targetBase = texData.begin();
      for (unsigned int y1 = 0; y1 < tileSizeY; y1++, targetBase += (mInfo.mTileSizeX * totalChannels))
      {
         vector<Out>::iterator target = targetBase;
         target += (currentChannel - 1);     // Subtract one since currentChannel is a one-based value

         for (unsigned int x1 = 0; x1 < tileSizeX; x1++)
         {
            *target = static_cast<Out> (0);

            if (bBadValues)
            {
               target += (totalChannels - currentChannel);
               *target = static_cast<Out> (0);
               ++target;
            }
            else
            {
               if (totalChannels == 2)
               {
                  ++target;
                  *target = static_cast<Out> (0);
                  ++target;
               }
               else if (static_cast<int>(x1) < mInfo.mTileSizeX - 1)
               {
                  target += totalChannels;
               }
            }
         }
      }
   }

   template <typename Out>
   void populateEmptyTextureData(vector<Out>& texData, EncodingType inputType, unsigned int tileSizeX,
      unsigned int tileSizeY, int currentChannel, int totalChannels, EncodingType outputType)
   {
      switch (inputType)
      {
         case INT1UBYTE:
            populateEmptyTextureData<unsigned char>(texData, tileSizeX, tileSizeY,
               currentChannel, totalChannels, outputType);
            break;

         case INT1SBYTE:
            populateEmptyTextureData<signed char>(texData, tileSizeX, tileSizeY,
               currentChannel, totalChannels, outputType);
            break;

         case INT2UBYTES:
            populateEmptyTextureData<unsigned short>(texData, tileSizeX, tileSizeY,
               currentChannel, totalChannels, outputType);
            break;

         case INT2SBYTES:
            populateEmptyTextureData<signed short>(texData, tileSizeX, tileSizeY, 
               currentChannel, totalChannels, outputType);
            break;

         case INT4UBYTES:
            populateEmptyTextureData<unsigned int>(texData, tileSizeX, tileSizeY,
               currentChannel, totalChannels, outputType);
            break;

         case INT4SBYTES:
            populateEmptyTextureData<signed int>(texData, tileSizeX, tileSizeY, 
               currentChannel, totalChannels, outputType);
            break;

         case FLT4BYTES:
            populateEmptyTextureData<float>(texData, tileSizeX, tileSizeY, 
               currentChannel, totalChannels, outputType);
            break;

         case FLT8BYTES:
            populateEmptyTextureData<double>(texData, tileSizeX, tileSizeY,
               currentChannel, totalChannels, outputType);
            break;

         case UNKNOWN:        // Fall through to the next case statement
         case INT4SCOMPLEX:   // Fall through to the next case statement
         case FLT8COMPLEX:    // Fall through to the next case statement
         default:
            break;
      }
   }

   template <typename Out>
   void createGrayscale(EncodingType outputType)
   {
      if (mTileRange.mLast < mTileRange.mFirst)
      {
         return;
      }

      int bufSize = mInfo.mTileSizeX * mInfo.mTileSizeY;
      int channels = 1;

      bool badValues = !mInfo.mKey.mBadValues.empty();
      if (mInfo.mFormat == GL_LUMINANCE_ALPHA)
      {
         bufSize *= 2;
         channels++;
      }

      vector<Out> texData(bufSize);

      int oldPercentDone = -1;
      for (int i = mTileRange.mFirst; i <= mTileRange.mLast; ++i)
      {
         GpuTile* pTile = mTiles[i];
         if (pTile != NULL)
         {
            unsigned int posX = static_cast<unsigned int>(pTile->getPos().mX);
            unsigned int posY = static_cast<unsigned int>(pTile->getPos().mY);
            unsigned int geomSizeX = static_cast<unsigned int>(pTile->getGeomSize().mX);
            unsigned int geomSizeY = static_cast<unsigned int>(pTile->getGeomSize().mY);
            RasterElement* pRasterElement = mInfo.mKey.mpRasterElement[0];
            VERIFYNRV(pRasterElement != NULL);

            const RasterDataDescriptor *pDescriptor = dynamic_cast<const RasterDataDescriptor*>(
               pRasterElement->getDataDescriptor());
            VERIFYNRV(pDescriptor != NULL);

            FactoryResource<DataRequest> pRequest;
            pRequest->setRows(pDescriptor->getActiveRow(posY), 
               pDescriptor->getActiveRow(posY+geomSizeY-1), geomSizeY);
            pRequest->setColumns(pDescriptor->getActiveColumn(posX), 
               pDescriptor->getActiveColumn(posX+geomSizeX-1), geomSizeX);
            pRequest->setBands(mInfo.mKey.mBand1,
               mInfo.mKey.mBand1, 1);
            
            DataAccessor da = pRasterElement->getDataAccessor(pRequest.release());
            if (!da.isValid())
            {
               return;
            }

            populateTextureData(texData, mInfo.mRawType[0], geomSizeX, geomSizeY, da, 1, channels, outputType);
            runInMainThread(SetGpuTile(pTile, &texData[0], outputType, mTileZoomIndices[i]));
         }

         int percentDone = 100 * (i - mTileRange.mFirst + 1) / (mTileRange.mLast - mTileRange.mFirst + 1);
         if (percentDone >= oldPercentDone + 10)
         {
            oldPercentDone = percentDone;
            getReporter().reportProgress(getThreadIndex(), percentDone);
         }
      }
   }

   template <typename Out>
   void createRgb(EncodingType outputType)
   {
      if (mTileRange.mLast < mTileRange.mFirst)
      {
         return;
      }

      int bufSize = mInfo.mTileSizeX * mInfo.mTileSizeY * 3;
      vector<Out> texData(bufSize);

      int oldPercentDone = -1;
      for (int i = mTileRange.mFirst; i <= mTileRange.mLast; ++i)
      {
         GpuTile* pTile = mTiles[i];
         if (pTile != NULL)
         {
            // Create a data accessor for each band
            unsigned int posX = static_cast<unsigned int>(pTile->getPos().mX);
            unsigned int posY = static_cast<unsigned int>(pTile->getPos().mY);
            unsigned int geomSizeX = static_cast<unsigned int>(pTile->getGeomSize().mX);
            unsigned int geomSizeY = static_cast<unsigned int>(pTile->getGeomSize().mY);

            bool haveRedData = false, haveGreenData = false, haveBlueData = false;

            RasterElement* pRedRasterElement = mInfo.mKey.mpRasterElement[0];
            DimensionDescriptor redBand = mInfo.mKey.mBand1;
            haveRedData = ((pRedRasterElement != NULL) && (redBand.isActiveNumberValid()));

            RasterElement* pGreenRasterElement = mInfo.mKey.mpRasterElement[1];
            DimensionDescriptor greenBand = mInfo.mKey.mBand2;
            haveGreenData = ((pGreenRasterElement != NULL) && (greenBand.isActiveNumberValid()));

            RasterElement* pBlueRasterElement = mInfo.mKey.mpRasterElement[2];
            DimensionDescriptor blueBand = mInfo.mKey.mBand3;
            haveBlueData = ((pBlueRasterElement != NULL) && (blueBand.isActiveNumberValid()));

            if (haveRedData)
            {
               RasterDataDescriptor *pRedRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(pRedRasterElement->getDataDescriptor());
               VERIFYNRV(pRedRasterDescriptor != NULL);
               FactoryResource<DataRequest> pRedRequest;
               pRedRequest->setRows(pRedRasterDescriptor->getActiveRow(posY), 
                  pRedRasterDescriptor->getActiveRow(posY+geomSizeY-1), geomSizeY);
               pRedRequest->setColumns(pRedRasterDescriptor->getActiveColumn(posX), 
                  pRedRasterDescriptor->getActiveColumn(posX+geomSizeX-1), geomSizeX);
               pRedRequest->setBands(mInfo.mKey.mBand1,
                  mInfo.mKey.mBand1, 1);

               DataAccessor daRed = pRedRasterElement->getDataAccessor(pRedRequest.release());
               if (!daRed.isValid())
               {
                  return;
               }
               populateTextureData(texData, mInfo.mRawType[0], geomSizeX, geomSizeY, daRed, 1, 3, outputType);
            }
            else
            {
               populateEmptyTextureData(texData, mInfo.mRawType[0], geomSizeX, geomSizeY, 1, 3, outputType);
            }

            if (haveGreenData)
            {
               RasterDataDescriptor *pGreenRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(pGreenRasterElement->getDataDescriptor());
               VERIFYNRV(pGreenRasterDescriptor != NULL);

               FactoryResource<DataRequest> pGreenRequest;
               pGreenRequest->setRows(pGreenRasterDescriptor->getActiveRow(posY), 
                  pGreenRasterDescriptor->getActiveRow(posY+geomSizeY-1), geomSizeY);
               pGreenRequest->setColumns(pGreenRasterDescriptor->getActiveColumn(posX), 
                  pGreenRasterDescriptor->getActiveColumn(posX+geomSizeX-1), geomSizeX);
               pGreenRequest->setBands(mInfo.mKey.mBand2,
                  mInfo.mKey.mBand2, 1);
            
               DataAccessor daGreen = pGreenRasterElement->getDataAccessor(pGreenRequest.release());
               if (!daGreen.isValid())
               {
                  return;
               }

               populateTextureData(texData, mInfo.mRawType[1], geomSizeX, geomSizeY, daGreen, 2, 3, outputType);
            }
            else
            {
               populateEmptyTextureData(texData, mInfo.mRawType[1], geomSizeX, geomSizeY, 2, 3, outputType);
            }

            if (haveBlueData)
            {
               RasterDataDescriptor *pBlueRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(pBlueRasterElement->getDataDescriptor());
               VERIFYNRV(pBlueRasterDescriptor != NULL);

               FactoryResource<DataRequest> pBlueRequest;
               pBlueRequest->setRows(pBlueRasterDescriptor->getActiveRow(posY), 
                  pBlueRasterDescriptor->getActiveRow(posY+geomSizeY-1), geomSizeY);
               pBlueRequest->setColumns(pBlueRasterDescriptor->getActiveColumn(posX), 
                  pBlueRasterDescriptor->getActiveColumn(posX+geomSizeX-1), geomSizeX);
               pBlueRequest->setBands(mInfo.mKey.mBand3,
                  mInfo.mKey.mBand3, 1);

               DataAccessor daBlue = pBlueRasterElement->getDataAccessor(pBlueRequest.release());
               if (!daBlue.isValid())
               {
                  return;
               }

               populateTextureData(texData, mInfo.mRawType[2], geomSizeX, geomSizeY, daBlue, 3, 3, outputType);
            }
            else
            {
               populateEmptyTextureData(texData, mInfo.mRawType[2], geomSizeX, geomSizeY, 3, 3, outputType);
            }

            runInMainThread(SetGpuTile(pTile, &texData[0], outputType, mTileZoomIndices[i]));
         }

         int percentDone = 100 * (i - mTileRange.mFirst + 1) / (mTileRange.mLast - mTileRange.mFirst + 1);
         if (percentDone >= oldPercentDone + 10)
         {
            oldPercentDone = percentDone;
            getReporter().reportProgress(getThreadIndex(), percentDone);
         }
      }
   }
};    // End of GpuTileThread class declaration

void GpuTileThread::run()
{
   if (mInfo.mKey.mStretchPoints2.empty() == true)    // Grayscale or colormap
   {
      if ((mInfo.mRawType[0] == INT1UBYTE) || (mInfo.mRawType[0] == INT1SBYTE))
      {
         createGrayscale<unsigned char>(INT1UBYTE);
      }
      else if ((mInfo.mRawType[0] == INT2UBYTES) || (mInfo.mRawType[0] == INT2SBYTES))
      {
         createGrayscale<unsigned short>(INT2UBYTES);
      }
      else if ((mInfo.mRawType[0] == INT4UBYTES) || (mInfo.mRawType[0] == INT4SBYTES) ||
         (mInfo.mRawType[0] == FLT4BYTES) || (mInfo.mRawType[0] == FLT8BYTES))
      {
         createGrayscale<float>(FLT4BYTES);
      }
   }
   else     // RGB
   {
      if ((mInfo.mRawType[0] == mInfo.mRawType[1]) && (mInfo.mRawType[0] == mInfo.mRawType[2]))
      {
         if ((mInfo.mRawType[0] == INT1UBYTE) || (mInfo.mRawType[0] == INT1SBYTE))
         {
            createRgb<unsigned char>(INT1UBYTE);
         }
         else if ((mInfo.mRawType[0] == INT2UBYTES) || (mInfo.mRawType[0] == INT2SBYTES))
         {
            createRgb<unsigned short>(INT2UBYTES);
         }
         else if ((mInfo.mRawType[0] == INT4UBYTES) || (mInfo.mRawType[0] == INT4SBYTES) ||
            (mInfo.mRawType[0] == FLT4BYTES) || (mInfo.mRawType[0] == FLT8BYTES))
         {
            createRgb<float>(FLT4BYTES);
         }
      }
      else
      {
         createRgb<float>(FLT4BYTES);
      }
   }
}

void GpuImage::enableFilter(ImageFilterDescriptor *pDescriptor)
{
   if (isFilterEnabled(pDescriptor) == true)
   {
      return;
   }

   initializeFilter(pDescriptor);
}

void GpuImage::initializeFilter(ImageFilterDescriptor *pDescriptor)
{
   VERIFYNRV(pDescriptor != NULL);

   const vector<Tile*>* pTiles = getActiveTiles();
   if (pTiles == NULL)
   {
      return;
   }

   unsigned int numTiles = pTiles->size();
   vector<int> tileZoomIndices;
   vector<Tile*> tilesToUpdate;

   tileZoomIndices.reserve(numTiles);
   tilesToUpdate.reserve(numTiles);

   vector<Tile*>::const_iterator iter;
   for (iter = pTiles->begin(); iter != pTiles->end(); ++iter)
   {
      GpuTile* pTile = dynamic_cast<GpuTile*>(*iter);
      if (pTile != NULL)
      {
         pTile->createFilter(pDescriptor);
         tilesToUpdate.push_back(pTile);
         tileZoomIndices.push_back(pTile->getTextureIndex());
      }
   }

   updateTiles(tilesToUpdate, tileZoomIndices);

   // Initialize tile filters
   if (pDescriptor->getType() == ImageFilterDescriptor::FEEDBACK_FILTER)
   {
      // number of iterations to use to initialize the feedback buffer
      // filters can override this value through the gic file
      unsigned int numInitFrames = 20;
      ImageFilterDescriptorExt1 *pDescriptorExt = dynamic_cast<ImageFilterDescriptorExt1*>(pDescriptor);
      if (NN(pDescriptorExt))
      {
         const unsigned int *pInitFrames = dv_cast<unsigned int>(&pDescriptorExt->getParameter("initializationIterations"));
         if (pInitFrames != NULL)
         {
            numInitFrames = *pInitFrames;
         }
      }

      for (unsigned int j=0; j<tilesToUpdate.size(); ++j)
      {
         GpuTile *pGpuTile = static_cast<GpuTile*>(tilesToUpdate[j]);
         bool freeze = pGpuTile->getFilterFreezeFlag(pDescriptor);
         pGpuTile->freezeFilter(pDescriptor, false);
         for (unsigned int i = 0; i<numInitFrames; ++i)
         {
            // Apply the filter repeatedly on the current frame on each tile
            // to initialize the feedback buffer. This all happens in the GPU.
            pGpuTile->applyFilters();
         }
         pGpuTile->freezeFilter(pDescriptor, freeze);
      }
   }

   // Set the member variable, mPreviousBand, to 0 to allow the filter to be applied
   // to the image when it is enabled
   mPreviousBand = 0;
}

void GpuImage::enableFilters(const vector<ImageFilterDescriptor*>& descriptors)
{
   const vector<Tile*>* pTiles = getActiveTiles();
   if (pTiles == NULL)
   {
      return;
   }

   if (pTiles->empty() == true)
   {
      return;
   }

   // All tiles have the same filters so just get the current filters from the first tile
   vector<ImageFilterDescriptor*> currentFilters;
   vector<ImageFilterDescriptor*> newFilters = descriptors;

   GpuTile* pTile = dynamic_cast<GpuTile*>(pTiles->front());
   if (pTile != NULL)
   {
      currentFilters = pTile->getFilters();
   }

   // Delete currently enabled filters that are not in the new enabled vector
   vector<ImageFilterDescriptor*>::iterator currentIter;
   for (currentIter = currentFilters.begin(); currentIter != currentFilters.end(); ++currentIter)
   {
      ImageFilterDescriptor* pDescriptor = *currentIter;
      if (pDescriptor != NULL)
      {
         vector<ImageFilterDescriptor*>::iterator newIter =
            find(newFilters.begin(), newFilters.end(), pDescriptor);
         if (newIter == newFilters.end())
         {
            disableFilter(pDescriptor);
         }
         else
         {
            // Remove the filter from the vector of filters to enable
            newFilters.erase(newIter);
         }
      }
   }

   // Enable the remaining image filters
   vector<ImageFilterDescriptor*>::iterator newIter;
   for (newIter = newFilters.begin(); newIter != newFilters.end(); ++newIter)
   {
      ImageFilterDescriptor* pDescriptor = *newIter;
      if (pDescriptor != NULL)
      {
         enableFilter(pDescriptor);
      }
   }
}

void GpuImage::resetFilter(ImageFilterDescriptor *pDescriptor)
{
   if (!isFilterEnabled(pDescriptor))
   {
      return;
   }

   const vector<Tile*>* pTiles = getActiveTiles();
   if (pTiles == NULL)
   {
      return;
   }

   vector<Tile*>::const_iterator tileIter = pTiles->begin();
   while (tileIter != pTiles->end())
   {
      GpuTile* pTile = dynamic_cast<GpuTile*>(*tileIter);
      if (pTile != NULL)
      {
         pTile->resetFilter(pDescriptor);
      }
      ++tileIter;
   }

   initializeFilter(pDescriptor);
}

void GpuImage::freezeFilter(ImageFilterDescriptor *pDescriptor, bool toggle)
{
   if (!isFilterEnabled(pDescriptor))
   {
      return;
   }

   const vector<Tile*>* pTiles = getActiveTiles();
   if (pTiles == NULL)
   {
      return;
   }

  vector<Tile*>::const_iterator tileIter = pTiles->begin();
   while (tileIter != pTiles->end())
   {
      GpuTile* pTile = dynamic_cast<GpuTile*>(*tileIter);
      if (pTile != NULL)
      {
         pTile->freezeFilter(pDescriptor, toggle);
      }
      ++tileIter;
   }
}

void GpuImage::disableFilter(ImageFilterDescriptor *pDescriptor)
{
   if (isFilterEnabled(pDescriptor) == false)
   {
      return;
   }

   const vector<Tile*>* pTiles = getActiveTiles();
   if (pTiles == NULL)
   {
      return;
   }

   unsigned int numTiles = pTiles->size();
   for (unsigned int i = 0; i < numTiles; ++i)
   {
      GpuTile* pTile = dynamic_cast<GpuTile*> (pTiles->at(i));
      if (pTile != NULL)
      {
         pTile->destroyFilter(pDescriptor);
      }
   }
}

bool GpuImage::isFilterEnabled(ImageFilterDescriptor *pDescriptor) const
{
   const vector<Tile*>* pTiles = getActiveTiles();
   if (pTiles == NULL)
   {
      return false;
   }

   if (pTiles->empty() == true)
   {
      return false;
   }

   unsigned int numTiles = pTiles->size();
   for (unsigned int i = 0; i < numTiles; ++i)
   {
      GpuTile* pTile = dynamic_cast<GpuTile*> (pTiles->at(i));
      if (pTile != NULL)
      {
         bool bEnabled = pTile->hasFilter(pDescriptor);
         if (bEnabled == false)
         {
            return false;
         }
      }
   }

   return true;
}

void GpuImage::initializeGrayscale()
{
   // Update the active Cg program
   if (mpCgProgram != &mGrayscaleProgram)
   {
      // Cg program
      mpCgProgram = &mGrayscaleProgram;

      // Cg parameters
      CgContext* pCgContext = CgContext::instance();
      if (pCgContext != NULL)
      {
         mCgParameters = pCgContext->getParameters(*mpCgProgram);
      }
   }
}

void GpuImage::initializeColormap(const vector<ColorType>& colorMap)
{
   // Update the active Cg program
   if (mpCgProgram != &mColormapProgram)
   {
      // Cg program
      mpCgProgram = &mColormapProgram;

      // Cg parameters
      CgContext* pCgContext = CgContext::instance();
      if (pCgContext != NULL)
      {
         mCgParameters = pCgContext->getParameters(*mpCgProgram);
      }
   }

   // Update the colormap texture
   if (mColormapTexture != 0)
   {
      glDeleteTextures(1, &mColormapTexture);
   }

   unsigned int numColors = colorMap.size();

   vector<unsigned char> colors(numColors * 4);
   for (unsigned int i = 0, textureIndex = 0; i < numColors; ++i)
   {
      ColorType color = colorMap[i];
      colors[textureIndex++] = color.mRed;
      colors[textureIndex++] = color.mGreen;
      colors[textureIndex++] = color.mBlue;
      colors[textureIndex++] = color.mAlpha;
   }

   glGenTextures(1, &mColormapTexture);
   if (mColormapTexture != 0)
   {
      glBindTexture(GL_TEXTURE_RECTANGLE_NV, mColormapTexture);
      glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA, numColors, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &colors[0]);   
   }
}

void GpuImage::initializeRgb()
{
   // Update the active Cg program
   if (mpCgProgram != &mRgbProgram)
   {
      // Cg program
      mpCgProgram = &mRgbProgram;

      // Cg parameters
      CgContext* pCgContext = CgContext::instance();
      if (pCgContext != NULL)
      {
         mCgParameters = pCgContext->getParameters(*mpCgProgram);
      }
   }
}

void GpuImage::calculateTileSize(EncodingType dataType, const unsigned int imageWidth, const unsigned int imageHeight,
                                 int& tileWidth, int& tileHeight) const
{
   // Initially the tile size equal to the image size
   tileWidth = imageWidth;
   tileHeight = imageHeight;

   // Retrieve the current maximum texture size allowed for the system
   GLint maxTextureSize = 2048;
   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

   // Update the maximum texture size to include the internal data format
   unsigned int bytesPerElement = RasterUtilities::bytesInEncoding(dataType);
   if (bytesPerElement == 0)
   {
      bytesPerElement = 1;
   }

   maxTextureSize = maxTextureSize / bytesPerElement;

   // Compute tile height
   while (tileHeight > maxTextureSize)
   {
      if (tileHeight % 2 == 1)
      {
         tileHeight = (tileHeight + 1) / 2;
      }
      else
      {
         tileHeight /= 2;
      }
   }

   // Compute tile width
   while (tileWidth > maxTextureSize)
   {
      if (tileWidth % 2 == 1)
      {
         tileWidth = (tileWidth + 1) / 2;
      }
      else
      {
         tileWidth /= 2;
      }
   }
}

Tile* GpuImage::createTile() const
{
   return (new GpuTile());
}

void GpuImage::drawTiles(const vector<Tile*>& tiles, GLint textureMode)
{
   if (tiles.empty() == true)
   {
      return;
   }

   // Enable the fragment profile and bind the display program
   cgGLEnableProfile(mFragmentProfile);
   cgGLBindProgram(*mpCgProgram);
   setCgParameterValues();

   // Draw the tiles
   for (unsigned int i = 0; i < tiles.size(); ++i)
   {
      GpuTile* pTile = dynamic_cast<GpuTile*> (tiles[i]);
      if (pTile != NULL)
      {
         pTile->draw(mInputTexture, textureMode);
      }
   }

   // Disable the profile
   cgGLDisableProfile(mFragmentProfile);
}

void GpuImage::setCgParameterValues()
{
   const ImageData& imageInfo = getImageData();

   unsigned int numParameters = mCgParameters.size();
   for (unsigned int i = 0; i < numParameters; ++i)
   {
      const char* pParameterName = cgGetParameterName(mCgParameters.at(i));
      if (pParameterName != NULL)
      {
         if ((strcmp(pParameterName, "texCoord") != 0) && (strcmp(pParameterName, "outputColor") != 0))
         {
            if (strcmp(pParameterName, "inputImage") == 0)
            {
               mInputTexture = mCgParameters.at(i);
            }
            else if (strcmp(pParameterName, "colorMap") == 0)
            {
               cgGLSetTextureParameter(mCgParameters.at(i), mColormapTexture);
               cgGLEnableTextureParameter(mCgParameters.at(i));
            }
            else if (strcmp(pParameterName, "numColors") == 0)
            {
               GLint numColors = 0;
               glBindTexture(GL_TEXTURE_RECTANGLE_NV, mColormapTexture);
               glGetTexLevelParameteriv(GL_TEXTURE_RECTANGLE_NV, 0, GL_TEXTURE_WIDTH, &numColors);

               cgGLSetParameter1f(mCgParameters.at(i), static_cast<float>(numColors));
            }
            else if ((strcmp(pParameterName, "dataMax") == 0) || (strcmp(pParameterName, "redDataMax") == 0) ||
               (strcmp(pParameterName, "greenDataMax") == 0) || (strcmp(pParameterName, "blueDataMax") == 0))
            {
               EncodingType dataType = FLT4BYTES;
               if ((imageInfo.mRawType[0] == imageInfo.mRawType[1]) &&
                  (imageInfo.mRawType[0] == imageInfo.mRawType[2]))
               {
                  if ((strcmp(pParameterName, "dataMax") == 0) || (strcmp(pParameterName, "redDataMax") == 0))
                  {
                     dataType = imageInfo.mRawType[0];
                  }
                  else if (strcmp(pParameterName, "greenDataMax") == 0)
                  {
                     dataType = imageInfo.mRawType[1];
                  }
                  else if (strcmp(pParameterName, "blueDataMax") == 0)
                  {
                     dataType = imageInfo.mRawType[2];
                  }
               }

               if ((dataType == INT1UBYTE) || (dataType == INT1SBYTE))
               {
                  cgGLSetParameter1f(mCgParameters.at(i), static_cast<float>(numeric_limits<unsigned char>::max()));
               }
               else if ((dataType == INT2UBYTES) || (dataType == INT2SBYTES))
               {
                  cgGLSetParameter1f(mCgParameters.at(i), static_cast<float>(numeric_limits<unsigned short>::max()));
               }
               else
               {
                  cgGLSetParameter1f(mCgParameters.at(i), static_cast<float>(1.0));
               }
            }
            else if ((strcmp(pParameterName, "lowerValue") == 0) || (strcmp(pParameterName, "redLowerValue") == 0))
            {
               float lowerValue = getTextureStretchValue(static_cast<float>(imageInfo.mKey.mStretchPoints1[0]), imageInfo.mRawType[0]);
               cgGLSetParameter1f(mCgParameters.at(i), lowerValue);
            }
            else if ((strcmp(pParameterName, "upperValue") == 0) || (strcmp(pParameterName, "redUpperValue") == 0))
            {
               float upperValue = getTextureStretchValue(static_cast<float>(imageInfo.mKey.mStretchPoints1[1]), imageInfo.mRawType[0]);
               cgGLSetParameter1f(mCgParameters.at(i), upperValue);
            }
            else if (strcmp(pParameterName, "greenLowerValue") == 0)
            {
               float lowerValue = getTextureStretchValue(static_cast<float>(imageInfo.mKey.mStretchPoints2[0]), imageInfo.mRawType[1]);
               cgGLSetParameter1f(mCgParameters.at(i), lowerValue);
            }
            else if (strcmp(pParameterName, "greenUpperValue") == 0)
            {
               float upperValue = getTextureStretchValue(static_cast<float>(imageInfo.mKey.mStretchPoints2[1]), imageInfo.mRawType[1]);
               cgGLSetParameter1f(mCgParameters.at(i), upperValue);
            }
            else if (strcmp(pParameterName, "blueLowerValue") == 0)
            {
               float lowerValue = getTextureStretchValue(static_cast<float>(imageInfo.mKey.mStretchPoints3[0]), imageInfo.mRawType[2]);
               cgGLSetParameter1f(mCgParameters.at(i), lowerValue);
            }
            else if (strcmp(pParameterName, "blueUpperValue") == 0)
            {
               float upperValue = getTextureStretchValue(static_cast<float>(imageInfo.mKey.mStretchPoints3[1]), imageInfo.mRawType[2]);
               cgGLSetParameter1f(mCgParameters.at(i), upperValue);
            }
            else if (strcmp(pParameterName, "alpha") == 0)
            {
               float alpha = static_cast<float>(getAlpha());
               cgGLSetParameter1f(mCgParameters.at(i), alpha);
            }
         }
      }
   }
}

float GpuImage::getTextureStretchValue(float rawValue, EncodingType dataType) const
{
   const ImageData& imageInfo = getImageData();

   float stretchValue = rawValue;
   if ((imageInfo.mRawType[0] == imageInfo.mRawType[1]) && (imageInfo.mRawType[0] == imageInfo.mRawType[2]))
   {
      if (dataType == INT1SBYTE)
      {
         stretchValue -= numeric_limits<signed char>::min();
      }
      else if (dataType == INT2SBYTES)
      {
         stretchValue -= numeric_limits<signed short>::min();
      }
   }

   return stretchValue;
}

void GpuImage::setActiveTileSet(const ImageKey &key)
{
   ImageKey defaultImageKey;
   if (mpCgProgram == &mGrayscaleProgram)
   {
      if (mGrayscaleImageKey == defaultImageKey)
      {
         mGrayscaleImageKey = key;
      }

      Image::setActiveTileSet(mGrayscaleImageKey);
   }
   else if (mpCgProgram == &mColormapProgram)
   {
      if (mColormapImageKey == defaultImageKey)
      {
         mColormapImageKey = key;
      }

      Image::setActiveTileSet(mColormapImageKey);
   }
   else if (mpCgProgram == &mRgbProgram)
   {
      if (mRgbImageKey == defaultImageKey)
      {
         mRgbImageKey = key;
      }

      Image::setActiveTileSet(mRgbImageKey);
   }
}

unsigned int GpuImage::getMaxNumTileSets() const
{
   // One tile set for grayscale, one for RGB, and one for the color map
   return 3;
}

vector<Tile*> GpuImage::getTilesToUpdate(const vector<Tile*>& tilesToDraw, vector<int>& tileZoomIndices)
{
   const Image::ImageData imageInfo = Image::getImageData();

   // check for a change in the band number by comparing a one-based band
   // number to allow for an invalid default value of zero
   if (mPreviousBand != (imageInfo.mKey.mBand1.getActiveNumber() + 1))
   {
      // band change has occurred and ALL the tiles need to be updated
      // not just the ones that are visable and will be drawn to the screen
      const vector<Tile*> *pTileSet = getActiveTiles();
      if (pTileSet != NULL)
      {
         int numTiles = pTileSet->size();

         vector<Tile*> tilesToUpdate;
         tilesToUpdate.reserve(numTiles);

         tileZoomIndices.clear();
         tileZoomIndices.reserve(numTiles);

         vector<Tile*>::const_iterator iter;
         for (iter = pTileSet->begin(); iter != pTileSet->end(); ++iter)
         {
            Tile* pTile = *iter;
            if (pTile != NULL)
            {
               tilesToUpdate.push_back(pTile);
               tileZoomIndices.push_back(pTile->getTextureIndex());
            }
         }

         mPreviousBand = imageInfo.mKey.mBand1.getActiveNumber() + 1;
         return tilesToUpdate;
      }
   }

   return Image::getTilesToUpdate(tilesToDraw, tileZoomIndices);
}

unsigned int GpuImage::readTiles(double xCoord, double yCoord, GLsizei width, GLsizei height, vector<float>& values)
{
   if ((width == 0) || (height == 0))
   {
      return 0;
   }

   // get the active tile set
   const vector<Tile*> *pTileSet = getActiveTiles();
   VERIFY(pTileSet != NULL);

   const ImageData& imageData = getImageData();
   int imageWidth = imageData.mImageSizeX;
   int imageHeight = imageData.mImageSizeY;
   
   int x1Coord = static_cast<int>(xCoord);
   int y1Coord = static_cast<int>(yCoord);
   int x2Coord = x1Coord + width;
   int y2Coord = y1Coord + height;

   GLsizei calculatedWidth = width;
   GLsizei calculatedHeight = height;

   vector<Tile*> tiles;
   vector<LocationType> tileLocations;

   // get the tiles and their locations within the image that have the requested data
   getTilesToRead(x1Coord, y1Coord, width, height, tiles, tileLocations);

   vector<GLsizei> tileWidths;
   vector<GLsizei> sourceOffsets;
   vector<GLsizei> destinationOffsets;
   vector<float> dataVector;

   unsigned int counter = 0;
   unsigned int numElements = 0;
   size_t tileNum = 0;
   size_t numTiles = tiles.size();

   if (numTiles == 0)
   {
      return 0;
   }

   GpuTile *pTile = dynamic_cast<GpuTile*>(tiles.front());
   if (pTile == NULL)
   {
      return 0;
   }

   std::vector<ImageFilterDescriptor*> filters = pTile->getFilters();
   if (filters.empty())
   {
      return 0;
   }

   ImageFilter *pFilter = pTile->getFilter(filters.front());
   if (pFilter == NULL)
   {
      return 0;
   }

   // need to get results filter buffer in order to determine how many channels there are in the texture
   ColorBuffer *pColorBuffer = pFilter->getResultsBuffer();
   if (pColorBuffer == NULL)
   {
      return 0;
   }

   unsigned int numChannels = ImageUtilities::getNumColorChannels(pColorBuffer->getTextureFormat());
   if (values.size() < width * height * numChannels)
   {
      values.resize(width * height * numChannels);
   }

   if (values.empty() == true)
   {
      return 0;
   }

   float *pValueData = &values[0];

   tileWidths.reserve(numTiles);
   sourceOffsets.reserve(numTiles);
   destinationOffsets.reserve(numTiles);

   if (numTiles > 2)
   {
      dataVector.resize(width * height * numChannels);
   }
   else
   {
      // The values in the vector are ordered according to rows, so if the tiles are side-by-side,
      // we will need to reorder the values later
      if (tileLocations.front().mY == tileLocations.back().mY)
      {
         dataVector.resize(width * height * numChannels);
      }
   }
   
   if (dataVector.capacity() != 0)
   {
      pValueData = &dataVector[0];
   }

   vector<Tile*>::iterator tileIter = tiles.begin();

   // read the tiles
   while (tileIter != tiles.end())
   {
      numElements += readTile(*tileIter, tileLocations.at(tileNum), x1Coord, y1Coord, 
                              calculatedWidth, calculatedHeight, pValueData);
      
      sourceOffsets.push_back(counter);

      // move the pointer to the next position in the array
      counter += (calculatedWidth * calculatedHeight);
      pValueData += counter;

      // set the initial offsets for the tiles
      if (tileNum == 0)
      {
         destinationOffsets.push_back(0);
      }
      else
      {
         // check to see if current tile is on the same row as the previous tile
         if (tileLocations.at(tileNum).mX == tileLocations.at(tileNum-1).mX)
         {
            destinationOffsets.push_back(tileWidths.at(tileNum-1) + destinationOffsets.at(tileNum-1));
         }
         else
         {
            destinationOffsets.push_back(counter);
         }
      }
      tileWidths.push_back(calculatedWidth);
      tileNum++;
      tileIter++;
   }

    // Reorder the data into the destination vector to be row order
   if (dataVector.empty() == false)
   {
      pValueData = &values[0];
      unsigned int numElementsCopied = 0;
      while (numElementsCopied < numElements)
      {
         for (tileNum = 0; tileNum < numTiles; tileNum++)
         {
            memcpy((pValueData + destinationOffsets[tileNum]), &dataVector[sourceOffsets[tileNum]], 
                     (sizeof(float) * tileWidths[tileNum]));

            // set source and destination offsets for next pass
            sourceOffsets[tileNum] += tileWidths[tileNum];
            destinationOffsets[tileNum] += tileWidths[tileNum];

            numElementsCopied += tileWidths[tileNum];
         }
      }
   }

   // get scale factor
   if (mInfo.mRawType[0] != FLT4BYTES)
   {
      float scaleFactor = 1.0;
      switch(mInfo.mRawType[0])
      {
      case INT1SBYTE:
         scaleFactor = static_cast<float>(numeric_limits<char>::max());
         break;
      case INT1UBYTE:
         scaleFactor = static_cast<float>(numeric_limits<unsigned char>::max());
         break;
      case INT2SBYTES:
         scaleFactor = static_cast<float>(numeric_limits<short>::max());
         break;
      case INT2UBYTES:
         scaleFactor = static_cast<float>(numeric_limits<unsigned short>::max());
         break;
      case INT4SBYTES:
         scaleFactor = static_cast<float>(numeric_limits<int>::max());
         break;
      case INT4UBYTES:
         scaleFactor = static_cast<float>(numeric_limits<unsigned int>::max());
         break;
      default:
         scaleFactor = 1.0;
         break;
      }

      // scale the filtered results
      for (unsigned int ii = 0; ii < numElements; ii++)
      {
         pValueData[ii] *= scaleFactor;
      }
   }

   return numElements;
}

void GpuImage::getTilesToRead(int xCoord, int yCoord, GLsizei width, GLsizei height, 
                              vector<Tile*> &tiles, vector<LocationType> &tileLocations)
{
   tiles.clear();
   tileLocations.clear();

   const vector<Tile*> *pTileSet = getActiveTiles();
   VERIFYNRV(pTileSet != NULL);

   const ImageData& imageData = getImageData();
   int imageWidth = imageData.mImageSizeX;

   LocationType tileGeomSize;
   bool inTile = false;
   vector<Tile*>::const_iterator tileIter = pTileSet->begin();
   LocationType tileLocation;
   int currentWidth = 0;
   int rowNum = 0;
   int colNum = 0;
   int x2Coord = xCoord + width;
   int y2Coord = yCoord + height;
   while (tileIter != pTileSet->end())
   {
      tileGeomSize = (*tileIter)->getGeomSize();

      inTile = ((xCoord >= colNum*tileGeomSize.mX) && (xCoord < (colNum+1)*tileGeomSize.mX) &&
                (yCoord >= rowNum*tileGeomSize.mY) && (yCoord < (rowNum+1)*tileGeomSize.mY)) ||
               ((x2Coord >= colNum*tileGeomSize.mX) && (x2Coord < (colNum+1)*tileGeomSize.mX) &&
                (y2Coord >= rowNum*tileGeomSize.mY) && (y2Coord < (rowNum+1)*tileGeomSize.mY));

      if (inTile)
      {
         tileLocation.mX = colNum;
         tileLocation.mY = rowNum;
         tileLocations.push_back(tileLocation);
         tiles.push_back(*tileIter);
      }

      currentWidth += static_cast<int>(tileGeomSize.mX);
      if (currentWidth >= imageWidth)
      {
         rowNum++;
         colNum = 0;
         currentWidth = 0;
      }
      else
      {
         colNum++;
      }
      tileIter++;
   }
}

unsigned int GpuImage::readTile(Tile *pTile, const LocationType &tileLocation, int x1Coord, int y1Coord, 
                                GLsizei &calculatedWidth, GLsizei &calculatedHeight, GLvoid *pValues)
{
   LocationType geomSize = pTile->getGeomSize();
   int x1TileCoord = static_cast<int>(tileLocation.mX * geomSize.mX);
   int y1TileCoord = static_cast<int>(tileLocation.mY * geomSize.mY);
   int x2TileCoord = static_cast<int>((tileLocation.mX + 1.0) * geomSize.mX);
   int y2TileCoord = static_cast<int>((tileLocation.mY + 1.0) * geomSize.mY);

   int x2Coord = x1Coord + calculatedWidth;
   int y2Coord = y1Coord + calculatedHeight;

   // convert coordinates since each tile's image buffer has coordinates
   // starting at (0,0)
   int calculatedXCoord = 0;
   int calculatedYCoord = 0;

   // compute the width and x coordinate of the chip of data to be read
   if (x1TileCoord <= x1Coord && x2TileCoord < x2Coord)
   {
      calculatedWidth = x2TileCoord - x1Coord;
      calculatedXCoord = x1Coord - x1TileCoord;
   }
   else if (x1TileCoord > x1Coord && x2TileCoord >= x2Coord)
   {
      calculatedWidth = x2Coord - x1TileCoord;
   }
   else
   {
      calculatedXCoord = x1Coord - x1TileCoord;
   }

   // compute the height and y coorddinate of the chip of data to be read
   if (y1TileCoord <= y1Coord && y2TileCoord < y2Coord)
   {
      calculatedHeight = y2TileCoord - y1Coord;
      calculatedYCoord = y1Coord - y1TileCoord;
   }
   else if (y1TileCoord > y1Coord && y2TileCoord >= y2Coord)
   {
      calculatedHeight = y2Coord - y1TileCoord;
   }
   else
   {
      calculatedYCoord = y1Coord - y1TileCoord;
   }

   GpuTile *pGpuTile = static_cast<GpuTile*>(pTile);
   return (pGpuTile->readFilterBuffer(calculatedXCoord, calculatedYCoord, calculatedWidth, calculatedHeight, pValues));
}
