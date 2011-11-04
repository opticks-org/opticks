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

Tile::Tile() :
   mTexFormat(GL_RGB),
   mTexSizeX(INIT_TILE_SIZE),
   mTexSizeY(INIT_TILE_SIZE),
   mGeomSizeX(INIT_TILE_SIZE),
   mGeomSizeY(INIT_TILE_SIZE),
   mPosX(0),
   mPosY(0),
   mXcoords(4, 0.0),
   mYcoords(4, 0.0),
   mAlpha(255)
{}

Tile::~Tile()
{}

unsigned int Tile::getTextureIndex() const
{
   double pixelSize = DrawUtil::getPixelSize(mXcoords[0], mYcoords[0], mXcoords[2], mYcoords[2]);
   double targetSize = 0.5;
   unsigned int index = 0;

   if (pixelSize == 0.0)
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

   if (index > 3)
   {
      index = 3;
   }

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

void Tile::draw(GLfloat textureMode)
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
   glTranslatef(static_cast<GLfloat>(mPosX), static_cast<GLfloat>(mPosY), 0.0);

   mTextures[index].bind();

   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureMode);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureMode);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   float ratioX = static_cast<GLfloat>(mGeomSizeX) / static_cast<GLfloat>(mTexSizeX);
   float ratioY = static_cast<GLfloat>(mGeomSizeY) / static_cast<GLfloat>(mTexSizeY);

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

void Tile::setupTexture(unsigned int index, unsigned char* pTextureData)
{
   while (mTextures.size() <= index)
   {
      mTextures.push_back(Texture());
   }

   if (mTextures[index].isAllocated())
   {
      return;
   }

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
   const int factor = computeReductionFactor(index);
   const int height = mTexSizeY / factor;
   const int width = mTexSizeX / factor;
   const int size = width * height * channels;
   mTextures[index].genTexture(size);
   mTextures[index].bind();

   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, channels, width, height, 0, mTexFormat, GL_UNSIGNED_BYTE, pTextureData);
   glBindTexture(GL_TEXTURE_2D, 0);
   glDisable(GL_TEXTURE_2D);
   glFlush();
}

void Tile::setAlpha(unsigned int alpha)
{
   mAlpha = alpha;
}

unsigned int Tile::getAlpha() const
{
   return mAlpha;
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
