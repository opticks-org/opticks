/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <math.h>

#include <QtGui/QPixmap>

#include "AppConfig.h"
#include "DrawUtil.h"
#include "Endian.h"
#include "glCommon.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "ImageObjectImp.h"
#include "View.h"
#include "ViewImp.h"

#include <algorithm>
using namespace std;

ImageObjectImp::ImageObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                               LocationType pixelCoord) :
   RectangleObjectImp(id, type, pLayer, pixelCoord),
   mpImageData(NULL),
   mNeedUpdate(true)
{
   addProperty("Alpha");
}

ImageObjectImp::~ImageObjectImp()
{
   delete mpImageData;
}

const unsigned int* ImageObjectImp::getImage(int& iWidth, int& iHeight, ColorType& transparent) const
{
   if (mpImageData != NULL)
   {
      return mpImageData->getImageData(iWidth, iHeight, transparent);
   }

   iWidth = iHeight = 0;
   transparent = ColorType();
   return NULL;
}

void ImageObjectImp::draw(double zoomFactor) const
{
   if (mpImageData == NULL)
   {
      return;
   }
   if (mNeedUpdate)
   {
      mpImageData->generateTextures();
      mNeedUpdate = false;
   }

   int dataWidth = 0, dataHeight = 0;
   unsigned int textureId = 0;
   int textureWidth = 0, textureHeight = 0;

   ColorType transparent;
   textureId = mpImageData->getTexture(textureWidth, textureHeight);
   getImage(dataWidth, dataHeight, transparent);

   if (textureId == 0)
   {
     return;
   }

   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   glEnable(GL_TEXTURE_2D);

   // switch to using this tile's texture
   glBindTexture(GL_TEXTURE_2D, textureId);

   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   double maxS, maxT;
   double sOffset, tOffset;

   maxS = dataWidth / (double) textureWidth;
   maxT = dataHeight / (double) textureHeight;

   // image is centered in the texture. compute
   sOffset = (textureWidth - dataWidth) / 2 / (double)(textureWidth);
   tOffset = (textureHeight - dataHeight) / 2 / (double)(textureHeight);

   double dAlpha = -1.0;
   dAlpha = getAlpha();
   if (dAlpha == -1.0)
   {
      dAlpha = 255.0;
   }

   glBegin(GL_QUADS);
   glColor4ub(255, 255, 255, dAlpha);

   // Determine if the bounding box is flipped based on its screen locations
   bool bHorizontalFlip = false;
   bool bVerticalFlip = false;

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      pLayer->isFlipped(llCorner, urCorner, bHorizontalFlip, bVerticalFlip);

      // Account for the difference between screen origin (upper left) and OpenGL origin (lower left)
      bVerticalFlip = !bVerticalFlip;
   }

   // Draw the image left to right and right side up
   if ((bHorizontalFlip == false) && (bVerticalFlip == false))
   {
      glTexCoord2f(0.0 + sOffset, 0.0 + tOffset);
      glVertex3f(llCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(maxS + sOffset, 0.0 + tOffset);
      glVertex3f(urCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(maxS + sOffset, maxT + tOffset);
      glVertex3f(urCorner.mX, urCorner.mY, 0.0);

      glTexCoord2f(0.0 + sOffset, maxT + tOffset);
      glVertex3f(llCorner.mX, urCorner.mY, 0.0);
   }
   else if ((bHorizontalFlip == true) && (bVerticalFlip == false))
   {
      glTexCoord2f(maxS + sOffset, 0.0 + tOffset);
      glVertex3f(llCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(0.0 + sOffset, 0.0 + tOffset);
      glVertex3f(urCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(0.0 + sOffset, maxT + tOffset);
      glVertex3f(urCorner.mX, urCorner.mY, 0.0);

      glTexCoord2f(maxS + sOffset, maxT + tOffset);
      glVertex3f(llCorner.mX, urCorner.mY, 0.0);
   }
   else if ((bHorizontalFlip == true) && (bVerticalFlip == true))
   {
      glTexCoord2f(maxS + sOffset, maxT + tOffset);
      glVertex3f(llCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(0.0 + sOffset, maxT + tOffset);
      glVertex3f(urCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(0.0 + sOffset, 0.0 + tOffset);
      glVertex3f(urCorner.mX, urCorner.mY, 0.0);

      glTexCoord2f(maxS + sOffset, 0.0 + tOffset);
      glVertex3f(llCorner.mX, urCorner.mY, 0.0);
   }
   else if ((bHorizontalFlip == false) && (bVerticalFlip == true))
   {
      glTexCoord2f(0.0 + sOffset, maxT + tOffset);
      glVertex3f(llCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(maxS + sOffset, maxT + tOffset);
      glVertex3f(urCorner.mX, llCorner.mY, 0.0);

      glTexCoord2f(maxS + sOffset, 0.0 + tOffset);
      glVertex3f(urCorner.mX, urCorner.mY, 0.0);

      glTexCoord2f(0.0 + sOffset, 0.0 + tOffset);
      glVertex3f(llCorner.mX, urCorner.mY, 0.0);
   }

   glEnd();

   glDisable(GL_ALPHA_TEST);
   glDisable(GL_BLEND);
   glDisable(GL_TEXTURE_2D);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   if(mpImageData != NULL)
   {
      mpImageData->glContextPop();
   }
}

const string& ImageObjectImp::getObjectType() const
{
   static string type("ImageObjectImp");
   return type;
}

bool ImageObjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ImageObject"))
   {
      return true;
   }

   return RectangleObjectImp::isKindOf(className);
}

void ImageObjectImp::temporaryGlContextChange()
{
   if(mpImageData != NULL)
   {
      mpImageData->glContextPush();
   }
   mNeedUpdate = true;
}

bool ImageObjectImp::setImageData(const QImage& image, ColorType transparent)
{
   int iWidth = image.width();
   int iHeight = image.height();
   auto_ptr<unsigned int> pData(new (nothrow) unsigned int [iWidth * iHeight]);
   if (pData.get() == NULL)
   {
      return false;
   }

   int colorDepth = 0;

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      ViewImp* pView = dynamic_cast<ViewImp*>(pLayer->getView());
      if (pView != NULL)
      {
         colorDepth = pView->depth();
      }
   }

   if (colorDepth == 0)
   {
      colorDepth = QPixmap::defaultDepth();
   }

   for (int i = 0; i < iHeight; i++)
   {
      QRgb* pSourcePixel = (QRgb*) image.scanLine(i);
      unsigned int* pDestPixel = &pData.get()[i * iWidth];
      for (int j = 0; j < iWidth; j++)
      {
         QRgb sourcePixel = image.pixel(j, i);
         *pDestPixel = (qRed(sourcePixel)) + (qGreen(sourcePixel) << 8) +
            (qBlue(sourcePixel) << 16);

         if (colorDepth < 32)
         {
            *pDestPixel <<= 8;
         }

         pSourcePixel++;
         pDestPixel++;
      }
   }

   return setImageData(pData.get(), iWidth, iHeight, transparent);
}

bool ImageObjectImp::setImageData(const unsigned int* pData, int iWidth, int iHeight, ColorType transparent)
{
   delete mpImageData;

   // Set the image data
   try
   {
      mpImageData = new ImageData(pData, iWidth, iHeight, transparent);
   }
   catch(std::bad_alloc exc)
   {
      return false;
   }

   return mpImageData != NULL;
}

void ImageObjectImp::updateBoundingBox()
{
   if (mpImageData == NULL)
   {
      return;
   }

   int iWidth = 0;
   int iHeight = 0;
   ColorType transparent;
   getImage(iWidth, iHeight, transparent);

   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   GraphicLayer* pLayer = getLayer();
   if (pLayer != NULL)
   {
      pLayer->translateDataToScreen(llCorner.mX, llCorner.mY, llCorner.mX, llCorner.mY);
      pLayer->translateDataToScreen(urCorner.mX, urCorner.mY, urCorner.mX, urCorner.mY);
   }

   llCorner.mX = min(llCorner.mX, urCorner.mX);
   llCorner.mY = min(llCorner.mY, urCorner.mY);
   urCorner.mX = llCorner.mX + iWidth;
   urCorner.mY = llCorner.mY + iHeight;

   if (pLayer != NULL)
   {
      pLayer->translateScreenToData(llCorner.mX, llCorner.mY, llCorner.mX, llCorner.mY);
      pLayer->translateScreenToData(urCorner.mX, urCorner.mY, urCorner.mX, urCorner.mY);
   }

   setBoundingBox(llCorner, urCorner);
}

ImageObjectImp::ImageData::ImageData(const unsigned int* pData, int iWidth, int iHeight, ColorType transparent) :
   mpData(NULL), mTextureWidth(0), mTextureHeight(0), mWidth(0), mHeight(0), mReferenceCount(0),
   mTransparent(transparent), mTextureId(0)
{
   int pixelCount = iWidth * iHeight;

   mpData = auto_ptr<unsigned int>(new unsigned int [pixelCount]);
      memcpy(mpData.get(), pData, pixelCount * 4);

   mWidth = iWidth;
   mHeight = iHeight;

   // set alpha values
   int i;
   if (transparent.isValid())
   {
      unsigned int orMask[2] = {0xff, 0};
      unsigned int transparentCpack = (transparent.mRed << 24) +
         (transparent.mGreen << 16) + (transparent.mBlue << 8);

      if (Endian::getSystemEndian() == LITTLE_ENDIAN)
      {
         orMask[0] = 0xff000000;
         orMask[1] = 0;
         transparentCpack = transparent.mRed + (transparent.mGreen << 8) + (transparent.mBlue << 16);
      }

      int whichMask;
      for (i = 0; i < pixelCount; i++)
      {
         if (Endian::getSystemEndian() == LITTLE_ENDIAN)
         {
            mpData.get()[i] &= 0x00ffffff;
         }
         else
         {
            mpData.get()[i] &= 0xffffff00;
         }

         whichMask = (mpData.get()[i] == transparentCpack);
         mpData.get()[i] = orMask[whichMask] | mpData.get()[i];

         if (Endian::getSystemEndian() == BIG_ENDIAN)
         {
            mpData.get()[i] = ((mpData.get()[i]&0xff00)<<16) | (mpData.get()[i]&0xff0000) |
               ((mpData.get()[i]&0xff000000)>>16) | (mpData.get()[i] & 0xff);
         }
      }
   }
   else
   {
      for (i = 0; i < pixelCount; i++)
      {
         if (Endian::getSystemEndian() == LITTLE_ENDIAN)
         {
            mpData.get()[i] |= 0xff000000;
         }
         else
         {
            mpData.get()[i] |= 0xff;
            mpData.get()[i] = ((mpData.get()[i]&0xff00) << 16) | (mpData.get()[i]&0xff0000) |
               ((mpData.get()[i]&0xff000000) >> 16) | (mpData.get()[i] & 0xff);
         }
      }
   }
}

ImageObjectImp::ImageData::~ImageData()
{
   while(!mTextureIdStack.empty())
   {
      // this does not attempt to delete any textures
      // except the top level texture since the GLContext
      // for other textures is likely to be invalid.
      // This shouldn't occur if callers are pairing
      // a temp context change with a draw...this just
      // prevents a possible crash due to an invalid texture id
      mTextureId = mTextureIdStack.top();
      mTextureIdStack.pop();
   }
   if (mTextureId != 0)
   {
      glDeleteTextures(1, &mTextureId);
   }
}

const unsigned int* ImageObjectImp::getObjectImage(int &width, int &height, ColorType &transparent) const
{
   return getImage(width, height, transparent);
}

const unsigned int* ImageObjectImp::ImageData::getImageData(int& iWidth, int& iHeight, ColorType& transparent) const
{
   iWidth = mWidth;
   iHeight = mHeight;
   transparent = mTransparent;

   return mpData.get();
}

unsigned int ImageObjectImp::ImageData::getTexture(int &iWidth, int &iHeight) const
{
   if (mTextureId == 0)
   {
      const_cast<ImageData*>(this)->generateTextures();
   }

   iWidth = mTextureWidth;
   iHeight = mTextureHeight;

   return mTextureId;
}

static void fillTextureBorder(unsigned int *pTexData, const unsigned int *pData, int textureWidth, int textureHeight,
      int width, int height, int rowOffset, int columnOffset)
{
   int i, j;
   unsigned int *dest=NULL;
   const unsigned int*source = NULL;

// Fill in the texture's blank areas with replications of the images border pixels
// 123
// 4I5
// 678

   // Region 2
   source = &pData[0];
   dest=&pTexData[columnOffset];
   for(j=0; j<=rowOffset; ++j)
   {
      memcpy(dest, source, 4*width);
      dest += textureWidth;
   }

   // Region 7
   source = &pData[width * (height-1)];
   dest=&pTexData[textureWidth * (textureHeight-rowOffset) + columnOffset];
   for (j=textureHeight-rowOffset; j<textureHeight; ++j)
   {
      memcpy(dest, source, 4*width);
      dest += textureWidth;
   }

   for(j=0; j<=rowOffset; ++j)
   {
      // Region 1
      dest=&pTexData[textureWidth * j];
      std::fill(dest, &dest[columnOffset + 1], pData[0]);

      // Region 3
      dest=&pTexData[textureWidth * (j+1) - columnOffset - 1];
      std::fill(dest, &dest[columnOffset + 1], pData[width-1]);
   }

   for (i=rowOffset; i<rowOffset+height; ++i)
   {
      // Region 4
      dest=&pTexData[textureWidth * i];
      std::fill(dest, &dest[columnOffset + 1], pData[width*(i-rowOffset)]);

      // Region 5
      dest=&pTexData[textureWidth * (i+1) - columnOffset - 1];
      std::fill(dest, &dest[columnOffset + 1], pData[width*(i-rowOffset)+width-1]);
   }

   for (j=textureHeight-rowOffset; j<textureHeight; ++j)
   {
      // Region 6
      dest=&pTexData[textureWidth * j];
      std::fill(dest, &dest[columnOffset + 1], pData[0]);

      // Region 8
      dest=&pTexData[textureWidth * (j+1) - columnOffset - 1];
      std::fill(dest, &dest[columnOffset + 1], pData[width-1]);
   }
}

void ImageObjectImp::ImageData::generateTextures()
{
   if (mTextureId != 0)
   {
      glDeleteTextures(1, &mTextureId);
      mTextureId = 0;
   }

   const unsigned int* pData = NULL;
   pData = mpData.get();
   if (pData == NULL)
   {
      return;
   }

   int pixelCount = mHeight * mWidth;

   // Round height and width up to the next larger powers of 2
   mTextureWidth = pow(2.0, 1.0 + floor((log10((double) (mWidth + 2)) / log10(2.0)))) + 0.5;
   mTextureHeight = pow(2.0, 1.0 + floor((log10((double) (mHeight + 2)) / log10(2.0)))) + 0.5;
   int bufSize = mTextureWidth * mTextureHeight; // rgb for each texel
   auto_ptr<unsigned int> pTexData(new unsigned int [bufSize]);

   unsigned int *dest = NULL;
   const unsigned int *source = NULL;

   memset(pTexData.get(), 0xff, mTextureWidth * mTextureHeight * 4);

   int j;
   int rowOffset = (mTextureHeight - mHeight) / 2;
   int columnOffset = (mTextureWidth - mWidth) / 2;

   fillTextureBorder(pTexData.get(), pData, mTextureWidth, mTextureHeight,
      mWidth, mHeight, rowOffset, columnOffset);

   // Fill the center of the texture with the image: Region I
   for(j = 0; j < mHeight; j++)
   {
      dest = &pTexData.get()[mTextureWidth * (j + rowOffset) + columnOffset];
      source = &pData[mWidth * j];
      memcpy(dest, source, 4 * mWidth);
   }

   glEnable(GL_TEXTURE_2D);
   glGenTextures (1, &mTextureId);
   glBindTexture(GL_TEXTURE_2D, mTextureId);
   glTexParameterf(GL_TEXTURE_2D,  GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D,  GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,   GL_LINEAR);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   gluBuild2DMipmaps(GL_TEXTURE_2D, 4, mTextureWidth, mTextureHeight, GL_RGBA, GL_UNSIGNED_BYTE, pTexData.get());
   glDisable(GL_TEXTURE_2D);
}

void ImageObjectImp::ImageData::glContextPush()
{
   mTextureIdStack.push(mTextureId);
   mTextureId = 0;
}

void ImageObjectImp::ImageData::glContextPop()
{
   if(mTextureIdStack.empty())
   {
      // no corresponding push
      return;
   }
   if(mTextureId != 0)
   {
      glDeleteTextures(1, &mTextureId);
   }
   mTextureId = mTextureIdStack.top();
   mTextureIdStack.pop();
}

bool ImageObjectImp::processMouseMove(LocationType screenCoord, 
                               Qt::MouseButton button,
                               Qt::MouseButtons buttons,
                               Qt::KeyboardModifiers modifiers)
{
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   VERIFY(pLayer != NULL);

   pLayer->completeInsertion(false);
   VERIFY(false); // should never get here
}

bool ImageObjectImp::processMouseRelease(LocationType screenCoord, 
                               Qt::MouseButton button,
                               Qt::MouseButtons buttons,
                               Qt::KeyboardModifiers modifiers)
{
   GraphicLayerImp* pLayer = dynamic_cast<GraphicLayerImp*>(getLayer());
   VERIFY(pLayer != NULL);

   pLayer->completeInsertion(false);
   VERIFY(false); // should never get here
}
