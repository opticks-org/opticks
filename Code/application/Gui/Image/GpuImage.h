/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GPUIMAGE_H
#define GPUIMAGE_H

#include "Image.h"
#include "ImageFilterDescriptor.h"
#include "glCommon.h"
#include "TypesFile.h"

#include <vector>

class GpuTile;
class ImageFilter;

class GpuImage : public Image
{
public:
   GpuImage();
   ~GpuImage();

   // Grayscale
   void initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX, unsigned int imageSizeY,
      unsigned int channels, GLenum format, EncodingType type, void* pData, StretchType stretchType,
      std::vector<double>& stretchPoints, RasterElement* pRasterElement, const std::vector<int>& badValues);
   void initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX, unsigned int imageSizeY,
      unsigned int channels, GLenum format, EncodingType type, ComplexComponent component, void* pData,
      StretchType stretchType, std::vector<double>& stretchPoints, RasterElement* pRasterElement,
      const std::vector<int>& badValues);

   // Colormap
   void initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX, unsigned int imageSizeY,
      unsigned int channels, GLenum format, EncodingType type, void* pData, StretchType stretchType,
      std::vector<double>& stretchPoints, RasterElement* pRasterElement, const std::vector<ColorType>& colorMap,
      const std::vector<int>& badValues);
   void initialize(int sizeX, int sizeY, DimensionDescriptor channel, unsigned int imageSizeX, unsigned int imageSizeY,
      unsigned int channels, GLenum format, EncodingType type, ComplexComponent component, void* pData,
      StretchType stretchType, std::vector<double>& stretchPoints, RasterElement* pRasterElement,
      const std::vector<ColorType>& colorMap, const std::vector<int>& badValues);

   // RGB
   void initialize(int sizeX, int sizeY, DimensionDescriptor band1, DimensionDescriptor band2,
      DimensionDescriptor band3, unsigned int imageSizeX, unsigned int imageSizeY, unsigned int channels,
      GLenum format, EncodingType type, void* pData, StretchType stretchType, std::vector<double>& stretchPointsRed,
      std::vector<double>& stretchPointsGreen, std::vector<double>& stretchPointsBlue, RasterElement* pRasterElement,
      const std::vector<int>& badValues1, const std::vector<int>& badValues2, const std::vector<int>& badValues3);
   void initialize(int sizeX, int sizeY, DimensionDescriptor band1, DimensionDescriptor band2,
      DimensionDescriptor band3, unsigned int imageSizeX, unsigned int imageSizeY, unsigned int channels,
      GLenum format, EncodingType type, ComplexComponent component, void* pData, StretchType stretchType,
      std::vector<double>& stretchPointsRed, std::vector<double>& stretchPointsGreen,
      std::vector<double>& stretchPointsBlue, RasterElement* pRasterElement, const std::vector<int>& badValues1,
      const std::vector<int>& badValues2, const std::vector<int>& badValues3);
   void initialize(int sizeX, int sizeY, DimensionDescriptor band1, DimensionDescriptor band2,
      DimensionDescriptor band3, unsigned int imageSizeX, unsigned int imageSizeY, unsigned int channels,
      GLenum format, EncodingType type1, EncodingType type2, EncodingType type3, ComplexComponent component,
      void* pData, StretchType stretchType, std::vector<double>& stretchPointsRed,
      std::vector<double>& stretchPointsGreen, std::vector<double>& stretchPointsBlue,
      RasterElement* pRasterElement1, RasterElement* pRasterElement2, RasterElement* pRasterElement3,
      const std::vector<int>& badValues1, const std::vector<int>& badValues2, const std::vector<int>& badValues3);

   // Filters
   void enableFilter(ImageFilterDescriptor *pDescriptor);
   void enableFilters(const std::vector<ImageFilterDescriptor*>& descriptors);
   void disableFilter(ImageFilterDescriptor *pDescriptor);
   bool isFilterEnabled(ImageFilterDescriptor *pDescriptor) const;
   void resetFilter(ImageFilterDescriptor *pDescriptor);
   void freezeFilter(ImageFilterDescriptor *pDescriptor, bool toggle = true);
   unsigned int readTiles(double xCoord, double yCoord, GLsizei width, GLsizei height, std::vector<float>& values, bool& hasAlphas);

   static void setMaxTextureSize(GLint maxSize = 0);
   static GLint getMaxTextureSize();

protected:
   void initializeGrayscale();
   void initializeColormap(const std::vector<ColorType>& colorMap);
   void initializeRgb();
   void calculateTileSize(EncodingType dataType, const unsigned int imageWidth, const unsigned int imageHeight,
      int& tileWidth, int& tileHeight) const;

   Tile* createTile() const;
   void updateTiles(std::vector<Tile*>& tilesToUpdate, std::vector<unsigned int>& tileZoomIndices);
   void drawTiles(const std::vector<Tile*>& tiles, GLfloat textureMode);
   void setActiveTileSet(const ImageKey &key);
   unsigned int getMaxNumTileSets() const;
   std::vector<Tile*> getTilesToUpdate(const std::vector<Tile*>& tilesToDraw,
      std::vector<unsigned int>& tileZoomIndices);
   void getTilesToRead(int xCoord, int yCoord, GLsizei width, GLsizei height, 
                       std::vector<Tile*> &tiles, std::vector<LocationType> &tileLocations);
   unsigned int readTile(Tile* pTile, const LocationType& tileLocation, int x1Coord, int y1Coord,
                         GLsizei& calculatedWidth, GLsizei& calculatedHeight, GLvoid* pValues);

private:
   void setCgParameterValues();
   float getTextureStretchValue(float rawValue, EncodingType dataType) const;
   void initializeFilter(ImageFilterDescriptor *pDescriptor);

private:
   // Grayscale display program
   CGprogram mGrayscaleProgram;
   ImageKey mGrayscaleImageKey;

   // Colormap display program
   CGprogram mColormapProgram;
   GlTextureResource mColormapTexture;
   ImageKey mColormapImageKey;

   // RGB display program
   CGprogram mRgbProgram;
   ImageKey mRgbImageKey;

   // Active display program
   CGprogram* mpCgProgram;
   CGprofile mFragmentProfile;   // Cg display program's profile
   CGparameter mInputTexture;    // Image to be displayed
   std::vector<CGparameter> mCgParameters;

   // Member variable to keep track of whether or not there was a band change
   unsigned int mPreviousBand;

   friend class FeedbackBufferTestCase;

   static GLint mMaxTextureSize;
   static bool mAlwaysAlpha;
   static bool mAlphaConfigChecked;
};

#endif
