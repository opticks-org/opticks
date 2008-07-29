/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <limits>
#include <math.h>

#include "AppConfig.h"
#include "AppVerify.h"
#include "DataAccessorImpl.h"
#include "DrawUtil.h"
#include "Image.h"
#include "MathUtil.h"
#include "ModelServices.h"
#include "MultiThreadedAlgorithm.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "Statistics.h"
#include "switchOnEncoding.h"
#include "Tile.h"
#include "UtilityServicesImp.h"

using namespace std;
using namespace mta;

vector<ColorType> Image::sDefaultColorMap;
unsigned int Image::TileSet::sNextId = 0;

Image::Image() :
mpTiles(NULL),
mAlpha(255),
mInfo(0, DimensionDescriptor(), DimensionDescriptor(), DimensionDescriptor(), LINEAR, std::vector<double>(), std::vector<double>(), std::vector<double>(), sDefaultColorMap, COMPLEX_MAGNITUDE, GL_LUMINANCE, NULL, NULL, NULL)
{
   mNumTilesX = 0;
   mNumTilesY = 0;
   mInfo.mTileSizeX = 0;
   mInfo.mTileSizeY = 0;
   mInfo.mImageSizeX = 0;
   mInfo.mImageSizeY = 0;
   mInfo.mpExponentialMultipliers = NULL;
   mInfo.mpLogarithmicMultipliers = NULL;
   mInfo.mpEqualizationValues[0] = NULL;
   mInfo.mpEqualizationValues[1] = NULL;
   mInfo.mpEqualizationValues[2] = NULL;
}

// Grayscale
void Image::initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX, unsigned int imageSizeY,
             unsigned int channels, GLenum format, EncodingType type, void *data, StretchType stretchType,
             vector<double>& stretchPoints, RasterElement *pRasterElement, const std::vector<int> &badValues)
{
   mInfo = ImageData(channels, channel, DimensionDescriptor(), DimensionDescriptor(), stretchType, stretchPoints, std::vector<double>(), std::vector<double>(), 
      sDefaultColorMap, COMPLEX_MAGNITUDE, GL_LUMINANCE, pRasterElement, pRasterElement, pRasterElement, badValues);
   mInfo.mTileSizeX = sizeX;
   mInfo.mTileSizeY = sizeY;
   mInfo.mImageSizeX = imageSizeX;
   mInfo.mImageSizeY = imageSizeY;
   mNumTilesX = (imageSizeX + sizeX - 1) / sizeX;
   mNumTilesY = (imageSizeY + sizeY - 1) / sizeY;
   mInfo.mRawType[0] = type;
   mInfo.mRawType[1] = type;
   mInfo.mRawType[2] = type;
   mInfo.mFormat = format;
   mInfo.mpData = data;
   if(mInfo.mpExponentialMultipliers != NULL)
   {
      delete mInfo.mpExponentialMultipliers;
      mInfo.mpExponentialMultipliers = NULL;
   }
   if(mInfo.mpLogarithmicMultipliers != NULL)
   {
      delete mInfo.mpLogarithmicMultipliers;
      mInfo.mpLogarithmicMultipliers = NULL;
   }
   for (int i=0; i<3; ++i)
   {
      if (mInfo.mpEqualizationValues[i] != NULL)
      {
         delete mInfo.mpEqualizationValues[i];
         mInfo.mpEqualizationValues[i] = NULL;
      }
   }

   setActiveTileSet(mInfo.mKey);
   createTiles();
}

void Image::initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX, unsigned int imageSizeY,
             unsigned int channels, GLenum format, EncodingType type, ComplexComponent component, void *data,
             StretchType stretchType, vector<double>& stretchPoints, RasterElement *pRasterElement,
             const std::vector<int> &badValues)
{
   mInfo = ImageData(channels, channel, DimensionDescriptor(), DimensionDescriptor(), stretchType, stretchPoints, std::vector<double>(), std::vector<double>(), 
      sDefaultColorMap, component, GL_LUMINANCE, pRasterElement, pRasterElement, pRasterElement, badValues);
   mInfo.mTileSizeX = sizeX;
   mInfo.mTileSizeY = sizeY;
   mInfo.mImageSizeX = imageSizeX;
   mInfo.mImageSizeY = imageSizeY;
   mNumTilesX = (imageSizeX + sizeX - 1) / sizeX;
   mNumTilesY = (imageSizeY + sizeY - 1) / sizeY;
   mInfo.mRawType[0] = type;
   mInfo.mRawType[1] = type;
   mInfo.mRawType[2] = type;
   mInfo.mFormat = format;
   mInfo.mpData = data;
   if(mInfo.mpExponentialMultipliers != NULL)
   {
      delete mInfo.mpExponentialMultipliers;
      mInfo.mpExponentialMultipliers = NULL;
   }
   if(mInfo.mpLogarithmicMultipliers != NULL)
   {
      delete mInfo.mpLogarithmicMultipliers;
      mInfo.mpLogarithmicMultipliers = NULL;
   }
   for (int i=0; i<3; ++i)
   {
      if (mInfo.mpEqualizationValues[i] != NULL)
      {
         delete mInfo.mpEqualizationValues[i];
         mInfo.mpEqualizationValues[i] = NULL;
      }
   }

   setActiveTileSet(mInfo.mKey);
   createTiles();
}

// Colormap
void Image::initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX, unsigned int imageSizeY,
             unsigned int channels, GLenum format, EncodingType type, void *data, StretchType stretchType,
             vector<double>& stretchPoints, RasterElement *pRasterElement, const vector<ColorType>& colorMap,
             const std::vector<int> &badValues)
{
   mInfo = ImageData(channels, channel, DimensionDescriptor(), DimensionDescriptor(), stretchType, stretchPoints, std::vector<double>(), std::vector<double>(), 
      colorMap, COMPLEX_MAGNITUDE, GL_LUMINANCE, pRasterElement, pRasterElement, pRasterElement, badValues);
   mInfo.mTileSizeX = sizeX;
   mInfo.mTileSizeY = sizeY;
   mInfo.mImageSizeX = imageSizeX;
   mInfo.mImageSizeY = imageSizeY;
   mNumTilesX = (imageSizeX + sizeX - 1) / sizeX;
   mNumTilesY = (imageSizeY + sizeY - 1) / sizeY;
   mInfo.mRawType[0] = type;
   mInfo.mRawType[1] = type;
   mInfo.mRawType[2] = type;
   mInfo.mFormat = format;
   mInfo.mpData = data;
   if(mInfo.mpExponentialMultipliers != NULL)
   {
      delete mInfo.mpExponentialMultipliers;
      mInfo.mpExponentialMultipliers = NULL;
   }
   if(mInfo.mpLogarithmicMultipliers != NULL)
   {
      delete mInfo.mpLogarithmicMultipliers;
      mInfo.mpLogarithmicMultipliers = NULL;
   }
   for (int i=0; i<3; ++i)
   {
      if (mInfo.mpEqualizationValues[i] != NULL)
      {
         delete mInfo.mpEqualizationValues[i];
         mInfo.mpEqualizationValues[i] = NULL;
      }
   }
   setActiveTileSet(mInfo.mKey);
   createTiles();
}

void Image::initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX, unsigned int imageSizeY,
             unsigned int channels, GLenum format, EncodingType type, ComplexComponent component, void *data,
             StretchType stretchType, vector<double>& stretchPoints, RasterElement *pRasterElement,
             const vector<ColorType>& colorMap, const std::vector<int> &badValues)
{
   mInfo = ImageData(channels, channel, DimensionDescriptor(), DimensionDescriptor(), stretchType, stretchPoints, std::vector<double>(), std::vector<double>(), 
      colorMap, component, GL_LUMINANCE, pRasterElement, pRasterElement, pRasterElement, badValues);
   mInfo.mTileSizeX = sizeX;
   mInfo.mTileSizeY = sizeY;
   mInfo.mImageSizeX = imageSizeX;
   mInfo.mImageSizeY = imageSizeY;
   mNumTilesX = (imageSizeX + sizeX - 1) / sizeX;
   mNumTilesY = (imageSizeY + sizeY - 1) / sizeY;
   mInfo.mRawType[0] = type;
   mInfo.mRawType[1] = type;
   mInfo.mRawType[2] = type;
   mInfo.mFormat = format;
   mInfo.mpData = data;
   if(mInfo.mpExponentialMultipliers != NULL)
   {
      delete mInfo.mpExponentialMultipliers;
      mInfo.mpExponentialMultipliers = NULL;
   }
   if(mInfo.mpLogarithmicMultipliers != NULL)
   {
      delete mInfo.mpLogarithmicMultipliers;
      mInfo.mpLogarithmicMultipliers = NULL;
   }
   for (int i=0; i<3; ++i)
   {
      if (mInfo.mpEqualizationValues[i] != NULL)
      {
         delete mInfo.mpEqualizationValues[i];
         mInfo.mpEqualizationValues[i] = NULL;
      }
   }
   setActiveTileSet(mInfo.mKey);
   createTiles();
}

// RGB
void Image::initialize(int sizeX, int sizeY, DimensionDescriptor band1, DimensionDescriptor band2, DimensionDescriptor band3,
             unsigned int imageSizeX, unsigned int imageSizeY, unsigned int channels, GLenum format,
             EncodingType type, void *data, StretchType stretchType, vector<double>& stretchPointsRed,
             vector<double>& stretchPointsGreen, vector<double>& stretchPointsBlue,
             RasterElement *pRasterElement)
{
   mInfo = ImageData(channels, band1, band2, band3, stretchType, stretchPointsRed, stretchPointsGreen, stretchPointsBlue, sDefaultColorMap, COMPLEX_MAGNITUDE, GL_RGB, pRasterElement, pRasterElement, pRasterElement);
   mInfo.mTileSizeX = sizeX;
   mInfo.mTileSizeY = sizeY;
   mInfo.mImageSizeX = imageSizeX;
   mInfo.mImageSizeY = imageSizeY;
   mNumTilesX = (imageSizeX + sizeX - 1) / sizeX;
   mNumTilesY = (imageSizeY + sizeY - 1) / sizeY;
   mInfo.mRawType[0] = type;
   mInfo.mRawType[1] = type;
   mInfo.mRawType[2] = type;
   mInfo.mFormat = format;
   mInfo.mpData = data;
   if(mInfo.mpExponentialMultipliers != NULL)
   {
      delete mInfo.mpExponentialMultipliers;
      mInfo.mpExponentialMultipliers = NULL;
   }
   if(mInfo.mpLogarithmicMultipliers != NULL)
   {
      delete mInfo.mpLogarithmicMultipliers;
      mInfo.mpLogarithmicMultipliers = NULL;
   }
   for (int i=0; i<3; ++i)
   {
      if (mInfo.mpEqualizationValues[i] != NULL)
      {
         delete mInfo.mpEqualizationValues[i];
         mInfo.mpEqualizationValues[i] = NULL;
      }
   }
   setActiveTileSet(mInfo.mKey);
   createTiles();
}

void Image::initialize(int sizeX, int sizeY, DimensionDescriptor band1, DimensionDescriptor band2, DimensionDescriptor band3,
             unsigned int imageSizeX, unsigned int imageSizeY, unsigned int channels, GLenum format,
             EncodingType type, ComplexComponent component, void *data, StretchType stretchType,
             vector<double>& stretchPointsRed, vector<double>& stretchPointsGreen,
             vector<double>& stretchPointsBlue, RasterElement *pRasterElement)
{
   mInfo = ImageData(channels, band1, band2, band3, stretchType, stretchPointsRed, stretchPointsGreen, stretchPointsBlue, sDefaultColorMap, component, GL_RGB, pRasterElement, pRasterElement, pRasterElement);
   mInfo.mTileSizeX = sizeX;
   mInfo.mTileSizeY = sizeY;
   mInfo.mImageSizeX = imageSizeX;
   mInfo.mImageSizeY = imageSizeY;
   mNumTilesX = (imageSizeX + sizeX - 1) / sizeX;
   mNumTilesY = (imageSizeY + sizeY - 1) / sizeY;
   mInfo.mRawType[0] = type;
   mInfo.mRawType[1] = type;
   mInfo.mRawType[2] = type;
   mInfo.mFormat = format;
   mInfo.mpData = data;
   if(mInfo.mpExponentialMultipliers != NULL)
   {
      delete mInfo.mpExponentialMultipliers;
      mInfo.mpExponentialMultipliers = NULL;
   }
   if(mInfo.mpLogarithmicMultipliers != NULL)
   {
      delete mInfo.mpLogarithmicMultipliers;
      mInfo.mpLogarithmicMultipliers = NULL;
   }
   for (int i=0; i<3; ++i)
   {
      if (mInfo.mpEqualizationValues[i] != NULL)
      {
         delete mInfo.mpEqualizationValues[i];
         mInfo.mpEqualizationValues[i] = NULL;
      }
   }
   setActiveTileSet(mInfo.mKey);
   createTiles();
}

// new initialize for different things in the channels
void Image::initialize(int sizeX, int sizeY, DimensionDescriptor band1, DimensionDescriptor band2, DimensionDescriptor band3,
      unsigned int imageSizeX, unsigned int imageSizeY, unsigned int channels, GLenum format,
      EncodingType type1, EncodingType type2, EncodingType type3, ComplexComponent component,
      void *data, StretchType stretchType, std::vector<double>& stretchPointsRed,
      std::vector<double>& stretchPointsGreen, std::vector<double>& stretchPointsBlue,
      RasterElement *pRasterElement1, RasterElement *pRasterElement2, RasterElement *pRasterElement3)
{
   mInfo = ImageData(channels, band1, band2, band3, stretchType, stretchPointsRed, stretchPointsGreen, stretchPointsBlue, sDefaultColorMap, component, GL_RGB, pRasterElement1, pRasterElement2, pRasterElement3);
   mInfo.mTileSizeX = sizeX;
   mInfo.mTileSizeY = sizeY;
   mInfo.mImageSizeX = imageSizeX;
   mInfo.mImageSizeY = imageSizeY;
   mNumTilesX = (imageSizeX + sizeX - 1) / sizeX;
   mNumTilesY = (imageSizeY + sizeY - 1) / sizeY;
   mInfo.mRawType[0] = type1;
   mInfo.mRawType[1] = type2;
   mInfo.mRawType[2] = type3;
   mInfo.mFormat = format;
   mInfo.mpData = data;
   if(mInfo.mpExponentialMultipliers != NULL)
   {
      delete mInfo.mpExponentialMultipliers;
      mInfo.mpExponentialMultipliers = NULL;
   }
   if(mInfo.mpLogarithmicMultipliers != NULL)
   {
      delete mInfo.mpLogarithmicMultipliers;
      mInfo.mpLogarithmicMultipliers = NULL;
   }
   for (int i=0; i<3; ++i)
   {
      if (mInfo.mpEqualizationValues[i] != NULL)
      {
         delete mInfo.mpEqualizationValues[i];
         mInfo.mpEqualizationValues[i] = NULL;
      }
   }
   setActiveTileSet(mInfo.mKey);
   createTiles();
}

Image::~Image()
{
   if (mInfo.mpExponentialMultipliers != NULL)
   {
      delete mInfo.mpExponentialMultipliers;
   }
   if (mInfo.mpLogarithmicMultipliers != NULL)
   {
      delete mInfo.mpLogarithmicMultipliers;
   }
   if (mInfo.mpEqualizationValues[0] != NULL)
   {
      delete mInfo.mpEqualizationValues[0];
   }
   if (mInfo.mpEqualizationValues[1] != NULL)
   {
      delete mInfo.mpEqualizationValues[1];
   }
   if (mInfo.mpEqualizationValues[2] != NULL)
   {
      delete mInfo.mpEqualizationValues[2];
   }
}

void Image::createTiles()
{
   VERIFYNRV(mpTiles != NULL);
   if (mpTiles->size() != 0) 
   {
      return;
   }

   int i, j;
   for (i = 0; i < mNumTilesY; ++i)
   {
      for (j = 0; j < mNumTilesX; ++j)
      {
         Tile* tile = createTile();

         tile->setTexFormat(mInfo.mFormat);
         tile->setTexSize(mInfo.mTileSizeX, mInfo.mTileSizeY);
      
         int geomSizeX = mInfo.mTileSizeX;
         int geomSizeY = mInfo.mTileSizeY;
         if (j == (mNumTilesX - 1))
         {
             geomSizeX = mInfo.mImageSizeX - ((mNumTilesX - 1) * mInfo.mTileSizeX);
         }

         if (i == (mNumTilesY - 1))
         {
             geomSizeY = mInfo.mImageSizeY - ((mNumTilesY - 1) * mInfo.mTileSizeY);
         }

         tile->setGeomSize(geomSizeX, geomSizeY);
         tile->setPos(j * mInfo.mTileSizeX, i * mInfo.mTileSizeY);
         tile->setAlpha(mAlpha);
         addTile(tile);
      }
   }
}

void Image::draw(GLint textureMode)
{
   setActiveTileSet(mInfo.mKey);
   VERIFYNRV(mpTiles != NULL);

   if (mpTiles->empty() == true)
   {
      return;
   }

   vector<int> tileZoomIndices;
   vector<Tile*> tilesToDraw = getTilesToDraw();
   vector<Tile*> tilesToUpdate = getTilesToUpdate(tilesToDraw, tileZoomIndices);

   if (tilesToUpdate.empty() == false)
   {
      updateTiles(tilesToUpdate, tileZoomIndices);
   }

   // move the center of the whole image to the origin
   // this is necessary because the tiles are placed in the image
   // beginning at the origin moving into the (+,+) quadrant
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   GLfloat centerXTrans = (((GLfloat) mInfo.mTileSizeX) / 2);
   GLfloat centerYTrans = (((GLfloat) mInfo.mTileSizeY) / 2);
   glTranslatef(centerXTrans, centerYTrans, 0.0);

   glPushAttrib(GL_COLOR_BUFFER_BIT);
   if (mInfo.mFormat == GL_RGBA || 
      mInfo.mFormat == GL_LUMINANCE_ALPHA || 
      mAlpha != 255)
   {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   }

   drawTiles(tilesToDraw, textureMode);

   glDisable(GL_BLEND);
   glPopAttrib();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   glFlush();
}

class SetTileTexture : public mta::ThreadCommand
{
public:
   SetTileTexture(Tile *pTile, unsigned char *pData, int zoomIndex) :
      mpTile(pTile), mpData(pData), mZoomIndex(zoomIndex) {}
   void run()
   {
      if (mpTile != NULL)
      {
         mpTile->setupTexture(mZoomIndex, mpData);
      }
   }

private:
   Tile *mpTile;
   unsigned char *mpData;
   int mZoomIndex;
};

class ComputeTextureFromTile : public mta::ThreadCommand
{
public:
   ComputeTextureFromTile(Tile *pTile, unsigned char *pData, int zoomIndex) :
      mpTile(pTile), mpData(pData), mZoomIndex(zoomIndex), mSuccess(false) {}
   void run()
   {
      if (mpTile != NULL)
      {
         mSuccess = mpTile->computeTexture(mZoomIndex, mpData);
         if (mSuccess)
         {
            mpTile->setupTexture(mZoomIndex, mpData);
         }
      }
   }
   bool getSuccess() { return mSuccess; }

private:
   Tile *mpTile;
   unsigned char *mpData;
   int mZoomIndex;
   bool mSuccess;
};

class TileThread;
class TileInput
{
public:
   TileInput(vector<Tile*>& tiles, vector<int> &tileZoomIndices, Image::ImageData &info) :
      mTiles(tiles), mTileZoomIndices(tileZoomIndices), mInfo(info) {}
   vector<Tile*>& mTiles;
   vector<int>& mTileZoomIndices;
   Image::ImageData &mInfo;
};
class TileOutput
{
public:
   bool compileOverallResults(const vector<TileThread*>& threads) { return true; }
};
class TileThread : public mta::AlgorithmThread
{
public:
   TileThread(const TileInput &input, int threadCount, int threadIndex, mta::ThreadReporter &reporter) : 
      AlgorithmThread(threadIndex, reporter),
      mTiles(input.mTiles),
      mTileZoomIndices(input.mTileZoomIndices),
      mInfo(input.mInfo),
      mTileRange(getThreadRange(threadCount, mTiles.size()))
   {
   }

   void run();

private:
   vector<Tile*>& mTiles;
   vector<int>& mTileZoomIndices;
   Image::ImageData &mInfo;
   Range mTileRange;

   // grayscale, channel specifies the band to display
   template <class T>
   void createGrayscale(T* pData, ComplexComponent component)
   {
      if (mTileRange.mLast < mTileRange.mFirst)
      {
         return;
      }

      scaleStruct scaleData;
      Image::prepareScale(mInfo, mInfo.mKey.mStretchPoints1, scaleData, 0);

      std::vector<int>::const_iterator badBegin=mInfo.mKey.mBadValues.begin(), badEnd=mInfo.mKey.mBadValues.end();

      int bufSize = mInfo.mTileSizeX * mInfo.mTileSizeY * sizeof(unsigned char);
      bool hasBadValues = (badBegin != badEnd);
      if (mInfo.mFormat == GL_LUMINANCE_ALPHA) bufSize *= 2;
      vector<unsigned char> pTexData(bufSize);

      bool bHas1BadValue = mInfo.mKey.mBadValues.size() == 1;
      int singleBadValue = 0;
      if (bHas1BadValue)
      {
         singleBadValue = mInfo.mKey.mBadValues.front();
      }

      int oldPercentDone = -1;

      for (int tileId = mTileRange.mFirst; tileId <= mTileRange.mLast; ++tileId)
      {
         Tile *pTile = mTiles[tileId];

         bool isComputed = false;
         if (pTile->hasHigherResTexture(mTileZoomIndices[tileId]))
         {
            ComputeTextureFromTile computer(pTile, &pTexData[0], mTileZoomIndices[tileId]);
            runInMainThread(computer);
            isComputed = computer.getSuccess();
         }
         
         if (isComputed == false)
         {
            unsigned int posX = pTile->getPos().mX;
            unsigned int posY = pTile->getPos().mY;
            unsigned int geomSizeX = pTile->getGeomSize().mX;
            unsigned int geomSizeY = pTile->getGeomSize().mY;

            RasterElement *pRasterElement = mInfo.mKey.mpRasterElement[0];
            VERIFYNRV(pRasterElement != NULL);
            RasterDataDescriptor *pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
            VERIFYNRV(pRasterDescriptor != NULL);
            VERIFYNRV(mInfo.mKey.mBand1.isValid());
            FactoryResource<DataRequest> pRequest;
            pRequest->setRows(pRasterDescriptor->getActiveRow(posY), 
               pRasterDescriptor->getActiveRow(posY+geomSizeY-1), geomSizeY);
            pRequest->setColumns(pRasterDescriptor->getActiveColumn(posX), 
               pRasterDescriptor->getActiveColumn(posX+geomSizeX-1), geomSizeX);
            pRequest->setBands(mInfo.mKey.mBand1,
               mInfo.mKey.mBand1, 1);

            DataAccessor da = pRasterElement->getDataAccessor(pRequest.release());
            if (!da.isValid())
            {
               return;
            }

            int reductionFactor = Tile::computeReductionFactor(mTileZoomIndices[tileId]);

            vector<unsigned char>::iterator targetBase = pTexData.begin();

            for (unsigned int y1 = 0; y1 < geomSizeY; y1+=reductionFactor, targetBase += mInfo.mTileSizeX/reductionFactor*(hasBadValues?2:1))
            {
               VERIFYNRV(da.isValid())
               T* source = static_cast<T*>(da->getColumn());

               vector<unsigned char>::iterator target = targetBase;
               vector<unsigned char>::iterator targetStop = target + geomSizeX/reductionFactor*(hasBadValues?2:1);

               for (; target < targetStop; ++target)
               {
                  double dValue = ModelServices::getDataValue(*source, component);
                  *target = Image::scale(dValue, scaleData, mInfo);
                  if (hasBadValues)
                  {
                     ++target;
                     int tempInt = roundDouble(dValue);
                     if (bHas1BadValue)
                     {
                        if (tempInt == singleBadValue)
                        {
                           *target = 0;
                        }
                        else
                        {
                           *target = 0xff;
                        }
                     }
                     else if (binary_search(badBegin, badEnd, tempInt))
                     {
                        *target = 0;
                     }
                     else
                     {
                        *target = 0xff;
                     }
                  }
                  else if (mInfo.mFormat == GL_LUMINANCE_ALPHA)
                  {
                     ++target;
                     *target = 0xff;
                  }

                  da->nextColumn(reductionFactor);
                  source = static_cast<T*>(da->getColumn());
               }

               da->nextRow(reductionFactor);
            }

            runInMainThread(SetTileTexture(pTile, &pTexData[0], mTileZoomIndices[tileId]));
         }

         int percentDone = 100 * (tileId - mTileRange.mFirst + 1) / (mTileRange.mLast - mTileRange.mFirst + 1);
         if (percentDone >= oldPercentDone + 10)
         {
            oldPercentDone = percentDone;
            getReporter().reportProgress(getThreadIndex(), percentDone);
         }
      }
   }

   // Colormap
   template <class T>
   void createColormap(T* pData, ComplexComponent component)
   {
      if (mTileRange.mLast < mTileRange.mFirst)
      {
         return;
      }

      int maxValue = (int) (mInfo.mKey.mColorMap.size());

      scaleStruct scaleData;
      Image::prepareScale(mInfo, mInfo.mKey.mStretchPoints1, scaleData, 0, maxValue-1);

      std::vector<int>::const_iterator badBegin=mInfo.mKey.mBadValues.begin(), badEnd=mInfo.mKey.mBadValues.end();

      bool hasBadValues = (badBegin != badEnd);
      int channels = (mInfo.mFormat == GL_RGBA ? 4 : 3);
      int bufSize = mInfo.mTileSizeX * mInfo.mTileSizeY * channels * sizeof(unsigned char);
      vector<unsigned char> pTexData(bufSize);

      bool bHas1BadValue = mInfo.mKey.mBadValues.size() == 1;
      int singleBadValue = 0;
      if (bHas1BadValue)
      {
         singleBadValue = mInfo.mKey.mBadValues.front();
      }

      int oldPercentDone = -1;

      for (int tileId = mTileRange.mFirst; tileId <= mTileRange.mLast; ++tileId)
      {
         Tile *pTile = mTiles[tileId];

         bool isComputed = false;
         if (pTile->hasHigherResTexture(mTileZoomIndices[tileId]))
         {
            ComputeTextureFromTile computer(pTile, &pTexData[0], mTileZoomIndices[tileId]);
            runInMainThread(computer);
            isComputed = computer.getSuccess();
         }
         
         if (isComputed == false)
         {
            unsigned int posX = pTile->getPos().mX;
            unsigned int posY = pTile->getPos().mY;
            unsigned int geomSizeX = pTile->getGeomSize().mX;
            unsigned int geomSizeY = pTile->getGeomSize().mY;
            RasterElement *pRasterElement = mInfo.mKey.mpRasterElement[0];
            VERIFYNRV(pRasterElement != NULL);
            RasterDataDescriptor *pRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
            VERIFYNRV(pRasterDescriptor != NULL);
            VERIFYNRV(mInfo.mKey.mBand1.isValid());
            
            FactoryResource<DataRequest> pRequest;
            pRequest->setRows(pRasterDescriptor->getActiveRow(posY), 
               pRasterDescriptor->getActiveRow(posY+geomSizeY-1), geomSizeY);
            pRequest->setColumns(pRasterDescriptor->getActiveColumn(posX), 
               pRasterDescriptor->getActiveColumn(posX+geomSizeX-1), geomSizeX);
            pRequest->setBands(mInfo.mKey.mBand1,
               mInfo.mKey.mBand1, 1);

            DataAccessor da = pRasterElement->getDataAccessor(pRequest.release());
            if (!da.isValid())
            {
               return;
            }

            int reductionFactor = Tile::computeReductionFactor(mTileZoomIndices[tileId]);
            vector<unsigned char>::iterator targetBase = pTexData.begin();

            for (unsigned int y1 = 0; y1 < geomSizeY; y1+=reductionFactor, targetBase += channels*mInfo.mTileSizeX/reductionFactor)
            {
               VERIFYNRV(da.isValid())
               vector<unsigned char>::iterator target = targetBase;
               for (unsigned int x1 = 0; x1 < geomSizeX; x1+=reductionFactor)
               {
                  T* source = static_cast<T*>(da->getColumn());
                  double dValue = ModelServices::getDataValue(*source, component);

                  int index = Image::scale(dValue, scaleData, mInfo, maxValue);

                  *target = mInfo.mKey.mColorMap[index].mRed;
                  ++target;

                  *target = mInfo.mKey.mColorMap[index].mGreen;
                  ++target;

                  *target = mInfo.mKey.mColorMap[index].mBlue;
                  ++target;

                  if (hasBadValues)
                  {
                     int tempInt = roundDouble(dValue);
                     if (bHas1BadValue)
                     {
                        if (tempInt == singleBadValue)
                        {
                           *target = 0;
                        }
                        else
                        {
                           *target = mInfo.mKey.mColorMap[index].mAlpha;
                        }
                     }
                     else if (binary_search(badBegin, badEnd, tempInt))
                     {
                        *target = 0;
                     }
                     else
                     {
                        *target = mInfo.mKey.mColorMap[index].mAlpha;
                     }
                     ++target;
                  }
                  else if (channels == 4)
                  {
                     *target = mInfo.mKey.mColorMap[index].mAlpha;
                     ++target;
                  }

                  da->nextColumn(reductionFactor);
               }
               da->nextRow(reductionFactor);
            }

            runInMainThread(SetTileTexture(pTile, &pTexData[0], mTileZoomIndices[tileId]));
         }

         int percentDone = 100 * (tileId- mTileRange.mFirst + 1) / (mTileRange.mLast - mTileRange.mFirst + 1);
         if (percentDone >= oldPercentDone + 10)
         {
            oldPercentDone = percentDone;
            getReporter().reportProgress(getThreadIndex(), percentDone);
         }
      }
   }

   // RGB: channel1=red, channel2=green, channel3=blue band
   template <class T>
   void createRgbTile(T*pData, std::vector<unsigned char> &pTexData, ComplexComponent component, 
      DataAccessor da, unsigned int posX, unsigned int posY, unsigned int geomSizeX, unsigned int geomSizeY, 
      int reductionFactor, scaleStruct &scaleData, int offset)
   {
      T* source;
      vector<unsigned char>::iterator targetBase = pTexData.begin();

      for (unsigned int y1 = 0; y1 < geomSizeY; y1+=reductionFactor, targetBase += 3*mInfo.mTileSizeX/reductionFactor)
      {
         VERIFYNRV(da.isValid());
         unsigned char *target = &*targetBase;
         //vector<unsigned char>::iterator target = targetBase;
         target += offset;
         for (unsigned int x1 = 0; x1 < geomSizeX; x1+=reductionFactor)
         {
            source = static_cast<T*>(da->getColumn());
            double dValue = ModelServices::getDataValue(*source, component);
            *target = Image::scale(dValue, scaleData, mInfo);
            target+=3;

            da->nextColumn(reductionFactor);
         }
         da->nextRow(reductionFactor);
      }
   }

   void createEmptyRgbTile(std::vector<unsigned char> &pTexData, 
      unsigned int posX, unsigned int posY, unsigned int geomSizeX, unsigned int geomSizeY, 
      int reductionFactor, int offset)
   {
      vector<unsigned char>::iterator targetBase = pTexData.begin();

      for (unsigned int y1 = 0; y1 < geomSizeY; y1+=reductionFactor, targetBase += 3*mInfo.mTileSizeX/reductionFactor)
      {
         unsigned char *target = &*targetBase;
         target += offset;
         for (unsigned int x1 = 0; x1 < geomSizeX; x1+=reductionFactor)
         {
            *target = 0; 
            target+=3;
         }
      }
   }

   void createRgb(EncodingType encodingRed, EncodingType encodingGreen, EncodingType encodingBlue,
      ComplexComponent component)
   {
      if (mTileRange.mLast < mTileRange.mFirst)
      {
         return;
      }

      scaleStruct scaleDataRed;
      scaleStruct scaleDataGreen;
      scaleStruct scaleDataBlue;

      Image::prepareScale(mInfo, mInfo.mKey.mStretchPoints1, scaleDataRed, 0);
      Image::prepareScale(mInfo, mInfo.mKey.mStretchPoints2, scaleDataGreen, 1);
      Image::prepareScale(mInfo, mInfo.mKey.mStretchPoints3, scaleDataBlue, 2);
      
      int bufSize = mInfo.mTileSizeX * mInfo.mTileSizeY * 3 * sizeof(unsigned char);
      std::vector<unsigned char> pTexData(bufSize);

      int oldPercentDone = -1;

      for (int tileId = mTileRange.mFirst; tileId <= mTileRange.mLast; ++tileId)
      {
         Tile *pTile = mTiles[tileId];

         bool isComputed = false;
         if (pTile->hasHigherResTexture(mTileZoomIndices[tileId]))
         {
            ComputeTextureFromTile computer(pTile, &pTexData[0], mTileZoomIndices[tileId]);
            runInMainThread(computer);
            isComputed = computer.getSuccess();
         }
         
         if (isComputed == false)
         {
            unsigned int posX = pTile->getPos().mX;
            unsigned int posY = pTile->getPos().mY;
            unsigned int geomSizeX = pTile->getGeomSize().mX;
            unsigned int geomSizeY = pTile->getGeomSize().mY;

            // Create a data accessor for each band
            bool haveRedData = false, haveBlueData = false, haveGreenData = false;
            RasterElement *pRedRasterElement = mInfo.mKey.mpRasterElement[0];
            DimensionDescriptor redBand = mInfo.mKey.mBand1;
            haveRedData = (pRedRasterElement != NULL) && (redBand.isActiveNumberValid());

            RasterElement *pGreenRasterElement = mInfo.mKey.mpRasterElement[1];
            DimensionDescriptor greenBand = mInfo.mKey.mBand2;
            haveGreenData = (pGreenRasterElement != NULL) && (greenBand.isActiveNumberValid());

            RasterElement *pBlueRasterElement = mInfo.mKey.mpRasterElement[2];
            DimensionDescriptor blueBand = mInfo.mKey.mBand3;
            haveBlueData = (pBlueRasterElement != NULL) && (blueBand.isActiveNumberValid());

            int reductionFactor = Tile::computeReductionFactor(mTileZoomIndices[tileId]);

            if (haveRedData)
            {
               RasterDataDescriptor *pRedRasterDescriptor = dynamic_cast<RasterDataDescriptor*>(pRedRasterElement->getDataDescriptor());
               VERIFYNRV(pRedRasterDescriptor != NULL);
               FactoryResource<DataRequest> pRedRequest;
               pRedRequest->setRows(pRedRasterDescriptor->getActiveRow(posY), 
                  pRedRasterDescriptor->getActiveRow(posY+geomSizeY-1), geomSizeY);
               pRedRequest->setColumns(pRedRasterDescriptor->getActiveColumn(posX), 
                  pRedRasterDescriptor->getActiveColumn(posX+geomSizeX-1), geomSizeX);
               pRedRequest->setBands(mInfo.mKey.mBand1, mInfo.mKey.mBand1, 1);

               DataAccessor daRed = pRedRasterElement->getDataAccessor(pRedRequest.release());
               if (!daRed.isValid())
               {
                  return;
               }
               switchOnComplexEncoding(encodingRed, createRgbTile, NULL, pTexData, component, daRed, posX, posY, geomSizeX, geomSizeY, reductionFactor, scaleDataRed, 0);
            }
            else
            {
               createEmptyRgbTile(pTexData, posX, posY, geomSizeX, geomSizeY, reductionFactor, 0);
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
               pGreenRequest->setBands(mInfo.mKey.mBand2, mInfo.mKey.mBand2, 1);

               DataAccessor daGreen = pGreenRasterElement->getDataAccessor(pGreenRequest.release());
               if (!daGreen.isValid())
               {
                  return;
               }
               switchOnComplexEncoding(encodingGreen, createRgbTile, NULL, pTexData, component, daGreen, posX, posY, geomSizeX, geomSizeY, reductionFactor, scaleDataGreen, 1);
            }
            else
            {
               createEmptyRgbTile(pTexData, posX, posY, geomSizeX, geomSizeY, reductionFactor, 1);
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
               pBlueRequest->setBands(mInfo.mKey.mBand3, mInfo.mKey.mBand3, 1);

               DataAccessor daBlue = pBlueRasterElement->getDataAccessor(pBlueRequest.release());
               if (!daBlue.isValid())
               {
                  return;
               }
               switchOnComplexEncoding(encodingBlue, createRgbTile, NULL, pTexData, component, daBlue,  posX, posY, geomSizeX, geomSizeY, reductionFactor, scaleDataBlue, 2);
            }
            else
            {
               createEmptyRgbTile(pTexData, posX, posY, geomSizeX, geomSizeY, reductionFactor, 2);
            }

            runInMainThread(SetTileTexture(pTile, &pTexData[0], mTileZoomIndices[tileId]));
         }

         int percentDone = 100 * (tileId - mTileRange.mFirst + 1) / (mTileRange.mLast - mTileRange.mFirst + 1);
         if (percentDone >= oldPercentDone + 10)
         {
            oldPercentDone = percentDone;
            getReporter().reportProgress(getThreadIndex(), percentDone);
         }
      }
   }
};

void TileThread::run()
{
   if (mInfo.mKey.mStretchPoints2.size() == 0) // grayscale or colormap
   {
      if (mInfo.mKey.mColorMap.size() == 0)
      {
         switchOnComplexEncoding(mInfo.mRawType[0], createGrayscale, mInfo.mpData, mInfo.mKey.mComponent);
      }
      else
      {
         switchOnComplexEncoding(mInfo.mRawType[0], createColormap, mInfo.mpData, mInfo.mKey.mComponent);
      }
   }
   else // rgb
   {
      createRgb(mInfo.mRawType[0], mInfo.mRawType[1], mInfo.mRawType[2], mInfo.mKey.mComponent);
   }
}

void Image::updateTiles(vector<Tile*> &tilesToUpdate, vector<int> &tileZoomIndices)
{
   TileInput tileInput(tilesToUpdate, tileZoomIndices, mInfo);

   TileOutput tileOutput;

   mta::StatusBarReporter barReporter("Generating Image", "app", "1BD64709-7C3B-4d54-8E85-ABCCA4B75B3B");
   mta::StatusBarReporter *pReporter = NULL;
   if (tilesToUpdate.size() > 1)
   {
      pReporter = &barReporter;
   }
   unsigned int threadCount = ConfigurationSettings::getSettingThreadCount();
#pragma message(__FILE__ "(" STRING(__LINE__) ") : warning : Multi-threaded tile generation is not working on Solaris. This disables it until a real fix can be devised. (tclarke)")
#if defined(SOLARIS)
   threadCount = 1;
#endif
   mta::MultiThreadedAlgorithm<TileInput,TileOutput,TileThread> tilingAlgorithm
      (threadCount, tileInput, tileOutput, pReporter);
   tilingAlgorithm.run();
}

bool Image::prepareScale(ImageData &info, std::vector<double> &stretchPoints, scaleStruct &data, unsigned int color, int maxValue)
{
   if (info.mKey.mType == EXPONENTIAL && info.mpExponentialMultipliers == NULL)
   {
      int i;
      info.mpExponentialMultipliers = new double[maxValue+1];
      info.mpExponentialMultipliers[0] = 1.0;
      for (i = 1; i <= maxValue; i++)
      {
         info.mpExponentialMultipliers[i] = pow(10.0, (double) i / maxValue);
         info.mpExponentialMultipliers[i] = (info.mpExponentialMultipliers[i] - 1.0) * maxValue / 9.0 / i;
      }
   }
   if (info.mKey.mType == LOGARITHMIC && info.mpLogarithmicMultipliers == NULL)
   {
      int i;
      info.mpLogarithmicMultipliers = new double[maxValue+1];
      info.mpLogarithmicMultipliers[0] = 1.0;
      for (i = 1; i <= maxValue; i++)
      {
         info.mpLogarithmicMultipliers[i] = maxValue * log10(1.0 + 9.0 * (double) i / maxValue) / i;
      }
   }
   if (info.mKey.mType == EQUALIZATION && color < 3)
   {
      Statistics* pStatistics = NULL;

      RasterElement *pRaster = info.mKey.mpRasterElement[color];
      if (pRaster != NULL)
      {
         const RasterDataDescriptor* pDescriptor =
            dynamic_cast<const RasterDataDescriptor*>(pRaster->getDataDescriptor());
         if (pDescriptor != NULL)
         {
            DimensionDescriptor bandDim;
            color == 0 ? bandDim = info.mKey.mBand1 : color == 1 ? bandDim = info.mKey.mBand2 : bandDim = info.mKey.mBand3;

            if (bandDim.isValid())
            {
               pStatistics = pRaster->getStatistics(bandDim);
            }
         }
      }

      VERIFY(pStatistics != NULL);
      const unsigned int *pHistogram = NULL;
      const double *pBinCenters = NULL;
      pStatistics->getHistogram(pBinCenters, pHistogram);
      VERIFY(pBinCenters != NULL);
      VERIFY(pHistogram != NULL);
      double minData = pStatistics->getMin(info.mKey.mComponent);
      double maxData = pStatistics->getMax(info.mKey.mComponent);
      VERIFY(maxData >= minData);
      double range = maxData - minData;
      data.offset = minData;
      if (range < ((maxValue+0.999) / numeric_limits<double>::max()))
      {
         data.gain = 1.0;
      }
      else
      {
         data.gain = (maxValue+0.999) / range;
      }
      if (info.mpEqualizationValues[color] == NULL)
      {
         info.mpEqualizationValues[color] = new unsigned int[maxValue+1];
         std::vector<unsigned int> counts(257);
         unsigned int count = 0;
         int firstNonzeroBin = 0;
         int lastNonzeroBin = 0;
         {
            for (int i=0; i<256; ++i)
            {
               count += pHistogram[i];
               counts[i] = count;
               if (pHistogram[i] != 0)
               {
                  if (pHistogram[firstNonzeroBin] == 0)
                  {
                     firstNonzeroBin = i;
                  }
                  lastNonzeroBin = i;
               }
            }
            counts[256] = counts[255];
         }

         if (count == 0)
         {
            count = 1;
         }

         for (int i=0; i<=maxValue; ++i)
         {
            int value = 0.5 + maxValue * counts[firstNonzeroBin+i*(lastNonzeroBin-firstNonzeroBin)/maxValue]/count;
            if (stretchPoints[1] < stretchPoints[0]) value = maxValue-value;
            info.mpEqualizationValues[color][i] = value;
         }
      }
   }

   data.type = info.mKey.mType;
   data.color = color;

   switch (info.mKey.mType)
   {
      case LINEAR:
      case EXPONENTIAL:
      case LOGARITHMIC:
      {
         double min, max;
         min = stretchPoints[0];
         max = stretchPoints[1];
         double range = max - min;
         data.offset = min;
         if (fabs(range) < ((maxValue+0.999) / numeric_limits<double>::max()))   // prevent divide-by-zero errors
         {
            data.gain = 1.0;
         }
         else
         {
            data.gain = (maxValue+0.999) / range;
         }
         return true;
      }
      case EQUALIZATION:
         return true;
      default:
         data.offset = 0.0;
         data.gain = 1.0;
         return false;
   }
}

void Image::setActiveTileSet(const ImageKey &key)
{
   unsigned int maxNumTileSets = getMaxNumTileSets();
   VERIFYNRV(maxNumTileSets != 0);

   map<ImageKey,TileSet>::iterator it = mTileSets.find(key);
   if (it == mTileSets.end())
   {
      // delete oldest tileset if necessary
      if (mTileSets.size() > maxNumTileSets)
      {
         map<ImageKey,TileSet>::iterator oldest = mTileSets.begin();
         map<ImageKey,TileSet>::iterator pTileSet = mTileSets.begin();
         for (pTileSet=mTileSets.begin(); pTileSet!=mTileSets.end(); ++pTileSet)
         {
            if ((*pTileSet).second.getId() < (*oldest).second.getId())
            {
               oldest = pTileSet;
            }
         }
         mTileSets.erase(oldest);
      }
      TileSet tileSet;
      mTileSets.insert(std::pair<const ImageKey,TileSet>(key,tileSet));
      it = mTileSets.find(key);
      mpTiles = &((*it).second.getTiles());
   }
   else
   {
      TileSet &tileSet = (*it).second;
      mpTiles = &(tileSet.getTiles());
      tileSet.updateId();
   }
}

unsigned int Image::getMaxNumTileSets() const
{
   unsigned int maxNumTileSets = 1;

   RasterElement* pRasterElement = mInfo.mKey.mpRasterElement[0];
   if (pRasterElement != NULL)
   {
      const RasterDataDescriptor* pDescriptor =
         dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
      if (pDescriptor != NULL)
      {
         maxNumTileSets = pDescriptor->getBandCount();
      }
   }

   return maxNumTileSets;
}

void Image::setAlpha(unsigned int alpha)
{
   if (alpha > 255)
   {
      alpha = 255;
   }

   if (alpha == mAlpha)
   {
      return;
   }

   mAlpha = alpha;

   map<ImageKey, TileSet>::iterator iter;
   for (iter = mTileSets.begin(); iter != mTileSets.end(); ++iter)
   {
      vector<Tile*> &tiles = iter->second.getTiles();

      vector<Tile*>::iterator tileIter;
      for (tileIter = tiles.begin(); tileIter != tiles.end(); ++tileIter)
      {
         Tile* pTile = *tileIter;
         if (pTile != NULL)
         {
            pTile->setAlpha(alpha);
         }
      }
   }
}

Tile *Image::selectNearbyTile() const
{
   int dist = 1000000;
   Tile *pSelectedTile = NULL;
   std::vector<Tile*>::const_iterator ppTile;
   for (ppTile=mpTiles->begin(); ppTile!=mpTiles->end(); ++ppTile)
   {
      VERIFYRV(*ppTile != NULL, NULL);
      if ((*ppTile)->isTextureReady(0) == false)
      {
         LocationType pos = (*ppTile)->getPos();
         int currentDist = abs(pos.mX + 256.0 - mDrawCenter.mX) +
            abs(pos.mY + 256.0 - mDrawCenter.mY);
         if (currentDist < dist)
         {
            dist = currentDist;
            pSelectedTile = *ppTile;
         }
      }
   }
   return pSelectedTile;
}

bool Image::generateFullResTexture()
{
   if (mpTiles == NULL) return false;

   Tile *pTile = selectNearbyTile();
   if (pTile != NULL)
   {
      vector<Tile*> tileToUpdate;
      tileToUpdate.push_back(pTile);
      vector<int> zoomIndex;
      zoomIndex.push_back(0);
      updateTiles(tileToUpdate, zoomIndex);
      return true;
   }

   return false;
}

void Image::generateAllFullResTextures()
{
   if (mpTiles == NULL) 
   {
      return;
   }

   std::vector<Tile*>::const_iterator ppTile;
   vector<Tile*> tileToUpdate;
   vector<int> zoomIndex;
   for (ppTile=mpTiles->begin(); ppTile!=mpTiles->end(); ++ppTile)
   {
      VERIFYNRV(*ppTile != NULL);
      if ((*ppTile)->isTextureReady(0) == false)
      {
         tileToUpdate.push_back(*ppTile);
         zoomIndex.push_back(0);
      }
   }
   updateTiles(tileToUpdate, zoomIndex);
}

const Image::ImageData& Image::getImageData() const
{
   return mInfo;
}

Tile* Image::createTile() const
{
   return (new Tile());
}

const vector<Tile*>* Image::getActiveTiles() const
{
   return mpTiles;
}

void Image::drawTiles(const vector<Tile*>& tiles, GLint textureMode)
{
   if (tiles.empty() == true)
   {
      return;
   }

   glEnable(GL_TEXTURE_2D);

   for (unsigned int i = 0; i < tiles.size(); ++i)
   {
      Tile* pTile = tiles[i];
      if (pTile != NULL)
      {
         pTile->draw(textureMode);
      }
   }

   glDisable(GL_TEXTURE_2D);
}

Image::TileSet::~TileSet()
{
   clearTiles();
}

void Image::TileSet::clearTiles()
{
   vector<Tile*>::iterator iter;
   for (iter = mTiles.begin(); iter != mTiles.end(); ++iter)
   {
      Tile* pTile = *iter;
      if (pTile != NULL)
      {
         delete pTile;
      }
   }
   mTiles.clear();
}

vector<Tile*> Image::getTilesToDraw()
{
   int numTiles = mpTiles->size();

   // determine which tiles we need to draw to refresh the
   // glview. This ensures that we don't draw tiles that are
   // entirely outside the view area.
   unsigned int ulVisStartColumn = 0;
   unsigned int ulVisEndColumn = mInfo.mImageSizeX - 1;
   unsigned int ulVisStartRow = 0;
   unsigned int ulVisEndRow = mInfo.mImageSizeY - 1;
   DrawUtil::restrictToViewport(ulVisStartColumn, ulVisStartRow, ulVisEndColumn, ulVisEndRow);
   mDrawCenter.mX = (ulVisStartColumn+ulVisEndColumn)/2;
   mDrawCenter.mY = (ulVisStartRow+ulVisEndRow)/2;

   vector<Tile*> tilesToDraw;
   tilesToDraw.reserve(numTiles);

   for (int ii = 0; ii < numTiles; ++ii)
   {
      Tile* pTile = mpTiles->at(ii);
      if (pTile != NULL)
      {
         unsigned int left = (ii % mNumTilesX) * mInfo.mTileSizeX;
         unsigned int right = left + mInfo.mTileSizeX;
         unsigned int bottom = (ii / mNumTilesX) * mInfo.mTileSizeY;
         unsigned int top = bottom + mInfo.mTileSizeY;

         if ((left <= ulVisEndColumn) && (right >= ulVisStartColumn) &&
            (bottom <= ulVisEndRow) && (top >= ulVisStartRow))
         {
            tilesToDraw.push_back(pTile);
         }
      }
   }

   return tilesToDraw;
}

vector<Tile*> Image::getTilesToUpdate(const vector<Tile*>& tilesToDraw, vector<int>& tileZoomIndices)
{
   int numTiles = tilesToDraw.size();

   vector<Tile*> tilesToUpdate;
   tilesToUpdate.reserve(numTiles);

   tileZoomIndices.clear();
   tileZoomIndices.reserve(numTiles);

   vector<Tile*>::const_iterator iter;
   for (iter = tilesToDraw.begin(); iter != tilesToDraw.end(); ++iter)
   {
      Tile* pTile = *iter;
      if (pTile != NULL)
      {
         if (pTile->isTextureReady() == false)
         {
            tilesToUpdate.push_back(pTile);
            tileZoomIndices.push_back(pTile->getTextureIndex());
         }
      }
   }

   return tilesToUpdate;
}
