/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TILE_H
#define TILE_H

#include "glCommon.h"
#include "LocationType.h"
#include "Textures.h"
#include <vector>

class Tile
{
public:
   Tile();
   virtual ~Tile();

   virtual void setTexFormat(GLenum format)
   {
      mTexFormat = format;
   }

   void setPos(int x, int y)
   {
      mPosX = x;
      mPosY = y;
   }

   virtual void setTexSize(int sizeX, int sizeY)
   {
      mTexSizeX = sizeX;
      mTexSizeY = sizeY;
   }

   void setGeomSize(int sizeX, int sizeY)
   {
      mGeomSizeX = sizeX;
      mGeomSizeY = sizeY;
   }

   GLenum getTexFormat() const
   {
      return mTexFormat;
   }

   LocationType getPos() const
   {
      return LocationType(mPosX, mPosY);
   }

   LocationType getTexSize() const
   {
      return LocationType(mTexSizeX, mTexSizeY);
   }

   LocationType getGeomSize() const
   {
      return LocationType(mGeomSizeX, mGeomSizeY);
   }

   virtual bool isTextureReady(unsigned int index) const;
   virtual void setupTexture(unsigned int index, unsigned char* pTextureData);
   void draw(GLfloat textureMode);
   unsigned int getTextureIndex() const;

   virtual void setAlpha(unsigned int alpha);
   unsigned int getAlpha() const;

   static int computeReductionFactor(unsigned int index)
   {
      return 1 << index;
   }

protected:
   void setXCoords(const std::vector<GLfloat>& xCoords);
   void setYCoords(const std::vector<GLfloat>& yCoords);
   const std::vector<GLfloat>& getXCoords() const;
   const std::vector<GLfloat>& getYCoords() const;

private:
   GLenum  mTexFormat;
   std::vector<Texture> mTextures;
   int mTexSizeX;
   int mTexSizeY;
   int mGeomSizeX;
   int mGeomSizeY;
   int mPosX;
   int mPosY;
   std::vector<GLfloat> mXcoords;
   std::vector<GLfloat> mYcoords;
   unsigned int mAlpha;
   static const int INIT_TILE_SIZE;
};

#endif
