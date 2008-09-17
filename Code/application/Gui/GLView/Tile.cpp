/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Tile.h"
#include "DrawUtil.h"

const int Tile::INIT_TILE_SIZE = 512;

Tile::Tile() : mXcoords(4), mYcoords(4)
{
   mTexFormat = GL_RGB;
   mTexSizeX = INIT_TILE_SIZE;
   mTexSizeY = INIT_TILE_SIZE;
   mGeomSizeX = INIT_TILE_SIZE;
   mGeomSizeY = INIT_TILE_SIZE;
   mPosX = 0;
   mPosY = 0;

   mXcoords[0] = 0;
   mYcoords[0] = 0;
   mXcoords[1] = 0;
   mYcoords[1] = 0;
   mXcoords[2] = 0;
   mYcoords[2] = 0;
   mXcoords[3] = 0;
   mYcoords[3] = 0;

   mAlpha = 255;
}

Tile::~Tile()
{
}

unsigned int Tile::getTextureIndex() const
{
   double pixelSize = DrawUtil::getPixelSize(mXcoords[0], mYcoords[0], mXcoords[1], mYcoords[1]);
   double targetSize = 0.5;
   unsigned int index = 0;

   if(pixelSize == 0.0)
   {
      // return the full sized tile if we
      // ever get in here with a 0 area tile
      return 0;
   }
   while (pixelSize < targetSize)
   {
      index++;
      pixelSize *= 2.0;
   }
   if (index > 3) index = 3;
   return index;
}

bool Tile::isTextureReady(unsigned int index) const
{
   if (mTextures.size() <= index)
   {
      return false;
   }

   return mTextures[index].isAllocated();
}

void Tile::draw(GLint textureMode)
{
   unsigned int index = getTextureIndex();
   if (mTextures.size() <= index)
   {
      return;
   }

   // we need to be able to undo this transformation 
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   // move the tile into it's correct position in the image
   glTranslatef((GLfloat) mPosX, (GLfloat) mPosY, 0.0);

   mTextures[index].bind();

   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureMode);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureMode);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   float fsizeX = (GLfloat) mTexSizeX;
   float fsizeY = (GLfloat) mTexSizeY;
   float ratioX = (GLfloat) mGeomSizeX / (GLfloat) mTexSizeX;
   float ratioY = (GLfloat) mGeomSizeY / (GLfloat) mTexSizeY;

   glBegin(GL_QUADS);
   glColor4ub(255, 255, 255, mAlpha);

   glTexCoord2f(0.0, 0.0);
   glVertex3f(mXcoords[0], mYcoords[0], 0.0);

   glTexCoord2f(ratioX, 0.0);
   glVertex3f(mXcoords[1], mYcoords[1], 0.0); 

   glTexCoord2f(ratioX, ratioY);
   glVertex3f(mXcoords[2], mYcoords[2], 0.0); 

   glTexCoord2f(0.0, ratioY);
   glVertex3f(mXcoords[3], mYcoords[3], 0.0); 

   glEnd();

   glBindTexture(GL_TEXTURE_2D, 0);
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}

void Tile::allocateTexture(unsigned int index, int channels, unsigned char *pTextureData)
{
   int factor = computeReductionFactor(index);

   mTextures[index].genTexture(channels*512*512/(factor*factor));
   mTextures[index].bind();

   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   glTexImage2D(GL_TEXTURE_2D, 0, channels, mTexSizeX / factor, mTexSizeY / factor, 0, mTexFormat,
      GL_UNSIGNED_BYTE, pTextureData);
}

void Tile::setupTexture(unsigned int index, unsigned char* pTextureData)
{
   while (mTextures.size() <= index)
   {
      mTextures.push_back(Texture());
   }

   if (mTextures[index].isAllocated()) return;

   mXcoords[0] = -(mTexSizeX / 2);
   mYcoords[0] = -(mTexSizeY / 2);

   mXcoords[1] = -(mTexSizeX / 2) + mGeomSizeX;
   mYcoords[1] = -(mTexSizeY / 2);

   mXcoords[2] = -(mTexSizeX / 2) + mGeomSizeX;
   mYcoords[2] = -(mTexSizeY / 2) + mGeomSizeY;

   mXcoords[3] = -(mTexSizeX / 2);
   mYcoords[3] = -(mTexSizeY / 2) + mGeomSizeY;

   int channels = 1;
   if (mTexFormat == GL_RGB)
   {
      channels = 3;
   }
   else if (mTexFormat == GL_RGBA)
   {
      channels = 4;
   }
   else if (mTexFormat == GL_LUMINANCE_ALPHA)
   {
      channels = 2;
   }

   glEnable(GL_TEXTURE_2D);

   unsigned char *pResTextureData = pTextureData;
//   std::vector<unsigned char> dataSpace;
//   if (index != 0)
//   {
//      int reductionFactor = computeReductionFactor(index);
//      dataSpace.resize(mTexSizeX * mTexSizeY * 4 / reductionFactor / reductionFactor);
//      pResTextureData = &dataSpace[0];
//      createLowResTexData(pTextureData, pResTextureData, mTexSizeX, mTexSizeY, channels, reductionFactor);
//   }

   allocateTexture(index, channels, pResTextureData);

   glBindTexture(GL_TEXTURE_2D, 0);
   glDisable(GL_TEXTURE_2D);
   glFlush();
}

Texture *Tile::getTextureToDeAllocate()
{
   unsigned int oldestTime = 0xffffffff;
   int indexOfOldest = -1;
   unsigned int i;
   for (i = 0; i < mTextures.size(); ++i)
   {
      unsigned int timestamp = mTextures[i].timestamp();
      if (mTextures[i].isAllocated() && timestamp < oldestTime)
      {
         oldestTime = timestamp;
         indexOfOldest = i;
      }
   }

   if (indexOfOldest == -1)
   {
      return NULL;
   }

   return &mTextures[indexOfOldest];
}

bool Tile::computeTexture(unsigned int index, unsigned char *pTextureData)
{
   for (int i=index-1; i>=0; --i)
   {
      if (static_cast<unsigned int>(i)<mTextures.size() && mTextures[i].isAllocated())
      {
         glEnable(GL_TEXTURE_2D);
         mTextures[i].bind();
         GLint width=0;
         GLint height=0;
         GLint components=0;
         glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
         glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
         glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &components);
         std::vector<unsigned char> pHighResTexture(components*width*height);
         glGetTexImage(GL_TEXTURE_2D, 0, mTexFormat, GL_UNSIGNED_BYTE, &pHighResTexture[0]);
         int reductionFactor=1;
         for (unsigned int j=i; j<index; ++j)
         {
            reductionFactor *= 2;
         }
         createLowResTexData(&pHighResTexture[0], pTextureData, width, height, components, reductionFactor);
         glBindTexture(GL_TEXTURE_2D, 0);
         glDisable(GL_TEXTURE_2D);
         return true;
      }
   }
   return false;
}

void Tile::createLowResTexData(unsigned char* pSourceData, unsigned char* pDestDataPtr, int texSizeX,
                               int texSizeY, int channels, int reductionFactor)
{
   const int MAX_CHANNELS = 4;
   int j, k, l;
   int texFactorChannel = reductionFactor*channels;
   int texSizeXFactorChannel = texFactorChannel * texSizeX;

   unsigned char *pDestChannelPtr = NULL;
   unsigned char* pBase = NULL;
   unsigned char* pSourceBase = NULL;

   pDestChannelPtr = pDestDataPtr;

   for (j = 0; j < texSizeY; j += reductionFactor)
   {
      pSourceBase = pSourceData;
      for (l = 0; l < texSizeX; l += reductionFactor)
      {
         pBase = pSourceBase;

         for (k = 0; k < channels; ++k)
         {
            *pDestChannelPtr = *pBase;
            ++pDestChannelPtr;
            ++pBase;
         }

         pSourceBase += texFactorChannel;
      }
      pSourceData += texSizeXFactorChannel;
   }
}

void Tile::setXCoords(const std::vector<GLfloat>& xCoords)
{
   mXcoords = xCoords;
}

void Tile::setYCoords(const std::vector<GLfloat>& yCoords)
{
   mYcoords = yCoords;
}

const std::vector<GLfloat>& Tile::getXCoords() const
{
   return mXcoords;
}

const std::vector<GLfloat>& Tile::getYCoords() const
{
   return mYcoords;
}
