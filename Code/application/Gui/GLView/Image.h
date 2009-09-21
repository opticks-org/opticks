/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGE_H
#define IMAGE_H

#include "glCommon.h"
#include "ColorType.h"
#include "ComplexData.h"
#include "AppVerify.h"
#include "DimensionDescriptor.h"
#include "LocationType.h"
#include "TypesFile.h"

#include <vector>
#include <map>

class RasterElement;
class Tile;

class ScaleStruct
{
public:
   ScaleStruct() :
      gain(1.0),
      offset(0.0),
      type(LINEAR),
      color(0)
   {
   }

   double gain;
   double offset;
   StretchType type;
   unsigned int color;
};

class ImageKey
{
public:
   ImageKey() :
      mChannels(0),
      mBand1(),
      mBand2(),
      mBand3(),
      mType(LINEAR),
      mComponent(COMPLEX_MAGNITUDE),
      mFormat(GL_LUMINANCE)
   {
      mpRasterElement[0] = NULL;
      mpRasterElement[1] = NULL;
      mpRasterElement[2] = NULL;
   }

   ImageKey(unsigned int channels, DimensionDescriptor band1, DimensionDescriptor band2, DimensionDescriptor band3, 
      StretchType type, const std::vector<double>& points1, const std::vector<double>& points2,
      const std::vector<double>& points3, const std::vector<ColorType>& colorMap,
      ComplexComponent component, GLenum format, RasterElement* pRasterElement1, RasterElement* pRasterElement2,
      RasterElement* pRasterElement3,
      const std::vector<int> &badValues = std::vector<int>()) :
      mChannels(channels),
      mBand1(band1),
      mBand2(band2),
      mBand3(band3),
      mType(type),
      mStretchPoints1(points1),
      mStretchPoints2(points2),
      mStretchPoints3(points3),
      mColorMap(colorMap),
      mComponent(component),
      mFormat(format),
      mBadValues(badValues)
   {
      mpRasterElement[0] = pRasterElement1;
      mpRasterElement[1] = pRasterElement2;
      mpRasterElement[2] = pRasterElement3;
   }

   void operator=(const class ImageKey &rhs)
   {
      mChannels = rhs.mChannels;
      mBand1 = rhs.mBand1;
      mBand2 = rhs.mBand2;
      mBand3 = rhs.mBand3;
      mType = rhs.mType;
      mStretchPoints1 = rhs.mStretchPoints1;
      mStretchPoints2 = rhs.mStretchPoints2;
      mStretchPoints3 = rhs.mStretchPoints3;
      mColorMap = rhs.mColorMap;
      mComponent = rhs.mComponent;
      mFormat = rhs.mFormat;
      mpRasterElement[0] = rhs.mpRasterElement[0];
      mpRasterElement[1] = rhs.mpRasterElement[1];
      mpRasterElement[2] = rhs.mpRasterElement[2];
      mBadValues = rhs.mBadValues;
   }

   bool operator==(const class ImageKey &rhs) const
   {
      if (mChannels != rhs.mChannels)
      {
         return false;
      }

      if (mBand1 != rhs.mBand1)
      {
         return false;
      }

      if (mBand2 != rhs.mBand2)
      {
         return false;
      }

      if (mBand3 != rhs.mBand3)
      {
         return false;
      }

      if (mType != rhs.mType)
      {
         return false;
      }

      if (mComponent != rhs.mComponent)
      {
         return false;
      }

      if (mFormat != rhs.mFormat)
      {
         return false;
      }

      if (mStretchPoints1 != rhs.mStretchPoints1)
      {
         return false;
      }

      if (mStretchPoints2 != rhs.mStretchPoints2)
      {
         return false;
      }

      if (mStretchPoints3 != rhs.mStretchPoints3)
      {
         return false;
      }

      if (mColorMap != rhs.mColorMap)
      {
         return false;
      }

      if (mpRasterElement[0] != rhs.mpRasterElement[0])
      {
         return false;
      }

      if (mpRasterElement[1] != rhs.mpRasterElement[1])
      {
         return false;
      }

      if (mpRasterElement[2] != rhs.mpRasterElement[2])
      {
         return false;
      }

      if (mBadValues != rhs.mBadValues)
      {
         return false;
      }

      return true;
   }

   bool operator<(const class ImageKey &rhs) const
   {
      if (mChannels < rhs.mChannels)
      {
         return true;
      }

      if (mChannels > rhs.mChannels)
      {
         return false;
      }

      if (mBand1 < rhs.mBand1)
      {
         return true;
      }

      if (mBand1 > rhs.mBand1)
      {
         return false;
      }

      if (mBand2 < rhs.mBand2)
      {
         return true;
      }

      if (mBand2 > rhs.mBand2)
      {
         return false;
      }

      if (mBand3 < rhs.mBand3)
      {
         return true;
      }

      if (mBand3 > rhs.mBand3)
      {
         return false;
      }

      if (mType < rhs.mType)
      {
         return true;
      }

      if (mType > rhs.mType)
      {
         return false;
      }

      if (mComponent < rhs.mComponent)
      {
         return true;
      }

      if (mComponent > rhs.mComponent)
      {
         return false;
      }

      if (mFormat < rhs.mFormat)
      {
         return true;
      }
      if (mFormat > rhs.mFormat)
      {
         return false;
      }

      VERIFY(mStretchPoints1.size() == 2);
      VERIFY(rhs.mStretchPoints1.size() == 2);

      if (mStretchPoints1[0] < rhs.mStretchPoints1[0])
      {
         return true;
      }

      if (mStretchPoints1[0] > rhs.mStretchPoints1[0])
      {
         return false;
      }

      if (mStretchPoints1[1] < rhs.mStretchPoints1[1])
      {
         return true;
      }

      if (mStretchPoints1[1] > rhs.mStretchPoints1[1])
      {
         return false;
      }

      if (mpRasterElement[0] < rhs.mpRasterElement[0])
      {
         return true;
      }

      if (mpRasterElement[0] > rhs.mpRasterElement[0])
      {
         return false;
      }

      if (mFormat == GL_RGB)
      {
         VERIFY(mStretchPoints2.size() == 2);
         VERIFY(rhs.mStretchPoints2.size() == 2);

         if (mStretchPoints2[0] < rhs.mStretchPoints2[0])
         {
            return true;
         }

         if (mStretchPoints2[0] > rhs.mStretchPoints2[0])
         {
            return false;
         }

         if (mStretchPoints2[1] < rhs.mStretchPoints2[1])
         {
            return true;
         }

         if (mStretchPoints2[1] > rhs.mStretchPoints2[1])
         {
            return false;
         }

         VERIFY(mStretchPoints3.size() == 2);
         VERIFY(rhs.mStretchPoints3.size() == 2);

         if (mStretchPoints3[0] < rhs.mStretchPoints3[0])
         {
            return true;
         }

         if (mStretchPoints3[0] > rhs.mStretchPoints3[0])
         {
            return false;
         }

         if (mStretchPoints3[1] < rhs.mStretchPoints3[1])
         {
            return true;
         }

         if (mStretchPoints3[1] > rhs.mStretchPoints3[1])
         {
            return false;
         }

         if (mpRasterElement[1] < rhs.mpRasterElement[1])
         {
            return true;
         }

         if (mpRasterElement[1] > rhs.mpRasterElement[1])
         {
            return false;
         }

         if (mpRasterElement[2] < rhs.mpRasterElement[2])
         {
            return true;
         }

         if (mpRasterElement[2] > rhs.mpRasterElement[2])
         {
            return false;
         }
      }

      if (mFormat == GL_LUMINANCE || mFormat == GL_LUMINANCE_ALPHA)
      {
         if (mColorMap.size() < rhs.mColorMap.size())
         {
            return true;
         }

         if (mColorMap.size() > rhs.mColorMap.size())
         {
            return false;
         }

         for (unsigned int i = 0; i < mColorMap.size(); ++i)
         {
            if (mColorMap[i].mRed < rhs.mColorMap[i].mRed)
            {
               return true;
            }

            if (mColorMap[i].mRed > rhs.mColorMap[i].mRed)
            {
               return false;
            }

            if (mColorMap[i].mGreen < rhs.mColorMap[i].mGreen)
            {
               return true;
            }

            if (mColorMap[i].mGreen > rhs.mColorMap[i].mGreen)
            {
               return false;
            }

            if (mColorMap[i].mBlue < rhs.mColorMap[i].mBlue)
            {
               return true;
            }

            if (mColorMap[i].mBlue > rhs.mColorMap[i].mBlue)
            {
               return false;
            }
         }
      }

      if (mBadValues.size() < rhs.mBadValues.size())
      {
         return true;
      }

      if (mBadValues.size() > rhs.mBadValues.size())
      {
         return false;
      }

      for (unsigned int i = 0; i < mBadValues.size(); ++i)
      {
         if (mBadValues[i] < rhs.mBadValues[i])
         {
            return true;
         }

         if (mBadValues[i] > rhs.mBadValues[i])
         {
            return false;
         }
      }

      return false;
   }

   unsigned int mChannels;
   DimensionDescriptor mBand1;
   DimensionDescriptor mBand2;
   DimensionDescriptor mBand3;
   StretchType mType;
   std::vector<double> mStretchPoints1;
   std::vector<double> mStretchPoints2;
   std::vector<double> mStretchPoints3;
   std::vector<ColorType> mColorMap;
   ComplexComponent mComponent;
   GLenum mFormat;
   RasterElement* mpRasterElement[3];
   std::vector<int> mBadValues;
};

class Image
{
#ifdef CPPTESTS // allow testing of image rendering
   friend class FilterRedrawTestCase;
#endif

public:
   class ImageData
   {
   public:
      ImageData(unsigned int channels, DimensionDescriptor band1, DimensionDescriptor band2, DimensionDescriptor band3,
                StretchType type, const std::vector<double>& points1, const std::vector<double>& points2,
                const std::vector<double>& points3, const std::vector<ColorType>& colorMap, ComplexComponent component,
                GLenum format, RasterElement* pRasterElement1, RasterElement* pRasterElement2,
                RasterElement* pRasterElement3, const std::vector<int>& badValues = std::vector<int>()) :
         mKey(channels, band1, band2, band3, type, points1, points2, points3, colorMap, component, format,
            pRasterElement1, pRasterElement2, pRasterElement3, badValues),
         mTileSizeX(0),
         mTileSizeY(0),
         mImageSizeX(0),
         mImageSizeY(0),
         mFormat(GL_LUMINANCE),
         mpData(NULL),
         mpExponentialMultipliers(NULL),
         mpLogarithmicMultipliers(NULL)
      {
         mpEqualizationValues[0] = NULL;
         mpEqualizationValues[1] = NULL;
         mpEqualizationValues[2] = NULL;
      }

      void operator=(const class ImageData& rhs)
      {
         mKey = rhs.mKey;
      }

      ImageKey mKey;
      int mTileSizeX;
      int mTileSizeY;
      int mImageSizeX;
      int mImageSizeY;
      EncodingType mRawType[3];
      GLenum mFormat;
      void* mpData;
      double* mpExponentialMultipliers;
      double* mpLogarithmicMultipliers;
      unsigned int* mpEqualizationValues[3];
   };

   class TileSet
   {
   public:
      TileSet() : mId(getNextId()) {}
      ~TileSet();

      void clearTiles();

      unsigned int getId() const
      {
         return mId;
      }

      const std::vector<Tile*>& getTiles() const
      {
         return mTiles;
      }

      std::vector<Tile*>& getTiles()
      {
         return mTiles;
      }

      void updateId()
      {
         mId = TileSet::getNextId();
      }
   private:
      unsigned int mId;
      std::vector<Tile*> mTiles;
      static unsigned int sNextId;
      static unsigned int getNextId() 
      { 
         return sNextId++; 
      }
   };

   Image();

   // Grayscale
   virtual void initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX,
      unsigned int imageSizeY, unsigned int channels, GLenum format, EncodingType type, void* data,
      StretchType stretchType, std::vector<double>& stretchPoints, RasterElement* pRasterElement,
      const std::vector<int> &badValues);
   virtual void initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX,
      unsigned int imageSizeY, unsigned int channels, GLenum format, EncodingType type, ComplexComponent component,
      void* data, StretchType stretchType, std::vector<double>& stretchPoints, RasterElement* pRasterElement,
      const std::vector<int>& badValues);

   // Colormap
   virtual void initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX,
      unsigned int imageSizeY, unsigned int channels, GLenum format, EncodingType type, void* data,
      StretchType stretchType, std::vector<double>& stretchPoints, RasterElement* pRasterElement,
      const std::vector<ColorType>& colorMap, const std::vector<int>& badValues);
   virtual void initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX,
      unsigned int imageSizeY, unsigned int channels, GLenum format, EncodingType type, ComplexComponent component,
      void* data, StretchType stretchType, std::vector<double>& stretchPoints, RasterElement* pRasterElement,
      const std::vector<ColorType>& colorMap, const std::vector<int>& badValues);

   // RGB
   virtual void initialize(int sizeX, int sizeY, DimensionDescriptor band1, DimensionDescriptor band2,
      DimensionDescriptor band3, unsigned int imageSizeX, unsigned int imageSizeY, unsigned int channels,
      GLenum format, EncodingType type, void* data, StretchType stretchType, std::vector<double>& stretchPointsRed,
      std::vector<double>& stretchPointsGreen, std::vector<double>& stretchPointsBlue, RasterElement* pRasterElement);
   virtual void initialize(int sizeX, int sizeY, DimensionDescriptor band1, DimensionDescriptor band2,
      DimensionDescriptor band3, unsigned int imageSizeX, unsigned int imageSizeY, unsigned int channels,
      GLenum format, EncodingType type, ComplexComponent component, void* data, StretchType stretchType,
      std::vector<double>& stretchPointsRed, std::vector<double>& stretchPointsGreen,
      std::vector<double>& stretchPointsBlue, RasterElement* pRasterElement);
   // Separate RasterElements for each channel
   virtual void initialize(int sizeX, int sizeY, DimensionDescriptor band1, DimensionDescriptor band2,
      DimensionDescriptor band3, unsigned int imageSizeX, unsigned int imageSizeY, unsigned int channels,
      GLenum format, EncodingType type1, EncodingType type2, EncodingType type3, ComplexComponent component,
      void* data, StretchType stretchType, std::vector<double>& stretchPointsRed,
      std::vector<double>& stretchPointsGreen, std::vector<double>& stretchPointsBlue,
      RasterElement* pRasterElement1, RasterElement* pRasterElement2, RasterElement* pRasterElement3);

   virtual ~Image();

   void addTile(Tile* tile)
   {
      mpTiles->push_back (tile);
   }

   void draw(GLfloat textureMode);

   void setAlpha(unsigned int alpha); // 0-255
   unsigned int getAlpha() const;

   static bool prepareScale(ImageData &info, std::vector<double> &stretchPoints, ScaleStruct& data,
      unsigned int color, int maxValue = 255);
   static inline unsigned int scale(double value, const ScaleStruct& data, const ImageData &info,
      double maxValue = 256.0)
   {
      value = (value - data.offset) * data.gain;

      if (value < 0.0)
      {
         value = 0.0;
      }
      else if (value >= maxValue)
      {
         value = maxValue - 0.001;
      }

      if (data.type == LINEAR)
      {
         return static_cast<unsigned int>(value);
      }
      else if (data.type == EXPONENTIAL)
      {
         value *= info.mpExponentialMultipliers[static_cast<int>(value)];
      }
      else if (data.type == LOGARITHMIC)
      {
         value *= info.mpLogarithmicMultipliers[static_cast<int>(value)];
      }
      else if (data.type == EQUALIZATION)
      {
         value = info.mpEqualizationValues[data.color][static_cast<int>(value)];
      }

      if (value >= maxValue)
      {
         value = maxValue - 0.001;
      }

      return static_cast<unsigned int>(value);
   }

   bool generateFullResTexture();
   void generateAllFullResTextures();

protected:
   const ImageData& getImageData() const;
   virtual Tile* createTile() const;
   const std::vector<Tile*>* getActiveTiles() const;
   const std::map<ImageKey, TileSet>& getTileSets() const;
   virtual void updateTiles(std::vector<Tile*>& tilesToUpdate, std::vector<unsigned int>& tileZoomIndices);
   virtual void drawTiles(const std::vector<Tile*>& tiles, GLfloat textureMode);
   virtual void setActiveTileSet(const ImageKey &key);
   virtual unsigned int getMaxNumTileSets() const;
   std::vector<Tile*> getTilesToDraw();
   virtual std::vector<Tile*> getTilesToUpdate(const std::vector<Tile*>& tilesToDraw,
      std::vector<unsigned int>& tileZoomIndices);

   ImageData mInfo;

private:
   int mNumTilesX;
   int mNumTilesY;
   std::map<ImageKey, TileSet> mTileSets;
   std::vector<Tile*>* mpTiles;
   unsigned int mAlpha;
   LocationType mDrawCenter;

   void createTiles();
   static std::vector<ColorType> sDefaultColorMap;

   Tile* selectNearbyTile() const;
};

#endif
