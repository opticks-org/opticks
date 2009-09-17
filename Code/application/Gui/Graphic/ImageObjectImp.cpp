/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "DrawUtil.h"
#include "Endian.h"
#include "glCommon.h"
#include "GraphicLayer.h"
#include "GraphicLayerImp.h"
#include "ImageObjectImp.h"
#include "View.h"
#include "ViewImp.h"

#include <QtGui/QPixmap>
#include <QtOpenGL/QGLContext>

#include <algorithm>
#include <math.h>
using namespace std;

ImageObjectImp::ImageObjectImp(const string& id, GraphicObjectType type, GraphicLayer* pLayer,
                               LocationType pixelCoord) :
   RectangleObjectImp(id, type, pLayer, pixelCoord),
   mWidth(0),
   mHeight(0),
   mTextureWidth(0),
   mTextureHeight(0),
   mpDrawContext(NULL),
   mTextureResource(0),
   mTempTextureResource(0)
{
   addProperty("Alpha");
}

void ImageObjectImp::draw(double zoomFactor) const
{
   if (QGLContext::currentContext() != mpDrawContext)
   {
      const_cast<ImageObjectImp*>(this)->generateTextures();
   }

   int dataWidth = 0;
   int dataHeight = 0;
   ColorType transparent;
   getObjectImage(dataWidth, dataHeight, transparent);

   LocationType llCorner = getLlCorner();
   LocationType urCorner = getUrCorner();

   glEnable(GL_TEXTURE_2D);

   // switch to using this tile's texture
   GLuint textureId = mTextureResource;
   if (mpDrawContext == mTempTextureResource.getContext())
   {
      textureId = mTempTextureResource;
   }

   glBindTexture(GL_TEXTURE_2D, textureId);

   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   double maxS = dataWidth / static_cast<double>(mTextureWidth);
   double maxT = dataHeight / static_cast<double>(mTextureHeight);

   // image is centered in the texture. compute
   double sOffset = (mTextureWidth - dataWidth) / 2 / static_cast<double>(mTextureWidth);
   double tOffset = (mTextureHeight - dataHeight) / 2 / static_cast<double>(mTextureHeight);

   double dAlpha = getAlpha();
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

   // mTempTextureResource is allocated by the generateTextures method when the gl context has changed
   // Free the texture here; more importantly set mpDrawContext to mTextureResource so that the next
   // draw operation completes successfully without the need to recreate the texture.
   if (mTempTextureResource.get() != NULL)
   {
      ImageObjectImp* pNonConst = const_cast<ImageObjectImp*>(this);
      pNonConst->mTempTextureResource = GlTextureResource(0);
      pNonConst->mpDrawContext = mTextureResource.getContext();
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

bool ImageObjectImp::setImageData(const QImage& image, ColorType transparent)
{
   int iWidth = image.width();
   int iHeight = image.height();
   if (iWidth <= 0 || iHeight <= 0)
   {
      return false;
   }

   vector<unsigned int> data(iWidth * iHeight);
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
      unsigned int* pDestPixel = &data[i * iWidth];
      for (int j = 0; j < iWidth; j++)
      {
         QRgb sourcePixel = image.pixel(j, i);

         if (colorDepth < 32)
         {
            *pDestPixel = (qRed(sourcePixel) << 8) + (qGreen(sourcePixel) << 16) +
               (qBlue(sourcePixel) << 24) + (qAlpha(sourcePixel));
         }
         else
         {
            *pDestPixel = (qRed(sourcePixel)) + (qGreen(sourcePixel) << 8) +
               (qBlue(sourcePixel) << 16) + (qAlpha(sourcePixel) << 24);
         }

         pDestPixel++;
      }
   }

   return setImageData(&data[0], iWidth, iHeight, transparent);
}

bool ImageObjectImp::setImageData(const unsigned int* pData, int iWidth, int iHeight, ColorType transparent)
{
   // Set the image data
   if (iWidth <= 0 || iHeight <= 0)
   {
      return false;
   }

   const unsigned int pixelCount = static_cast<unsigned int>(iWidth) * static_cast<unsigned int>(iHeight);
   mData.resize(pixelCount);
   memcpy(&mData[0], pData, pixelCount * sizeof(unsigned int));
   mWidth = iWidth;
   mHeight = iHeight;
   mTextureWidth = 0;
   mTextureHeight = 0;
   mTransparent = transparent;
   mpDrawContext = NULL;
   mTextureResource = GlTextureResource(0);
   mTempTextureResource = GlTextureResource(0);

   // set alpha values
   unsigned int i;
   if (transparent.isValid())
   {
      unsigned int orMask[2] = {0xff, 0};
      unsigned int transparentCpack = (transparent.mRed << 24) +
         (transparent.mGreen << 16) + (transparent.mBlue << 8);

      if (Endian::getSystemEndian() == LITTLE_ENDIAN_ORDER)
      {
         orMask[0] = 0xff000000;
         orMask[1] = 0;
         transparentCpack = transparent.mRed + (transparent.mGreen << 8) + (transparent.mBlue << 16);
      }

      int whichMask;
      for (i = 0; i < pixelCount; i++)
      {
         if (Endian::getSystemEndian() == LITTLE_ENDIAN_ORDER)
         {
            mData[i] &= 0x00ffffff;
         }
         else
         {
            mData[i] &= 0xffffff00;
         }

         whichMask = (mData[i] == transparentCpack);
         mData[i] = orMask[whichMask] | mData[i];

         if (Endian::getSystemEndian() == BIG_ENDIAN_ORDER)
         {
            mData[i] = ((mData[i] & 0xff00) << 16) | (mData[i] & 0xff0000) |
               ((mData[i] & 0xff000000) >> 16) | (mData[i] & 0xff);
         }
      }
   }
   else if (Endian::getSystemEndian() == BIG_ENDIAN_ORDER)
   {
      for (i = 0; i < pixelCount; i++)
      {
         mData[i] = ((mData[i] & 0xff00) << 16) | (mData[i] & 0xff0000) |
            ((mData[i] & 0xff000000) >> 16) | (mData[i] & 0xff);
      }
   }

   return true;
}

void ImageObjectImp::updateBoundingBox()
{
   int iWidth = 0;
   int iHeight = 0;
   ColorType transparent;
   getObjectImage(iWidth, iHeight, transparent);

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

const unsigned int* ImageObjectImp::getObjectImage(int& width, int& height, ColorType& transparent) const
{
   width = mWidth;
   height = mHeight;
   transparent = mTransparent;
   return mData.empty() ? NULL : &mData[0];
}

static void fillTextureBorder(unsigned int *pTexData, const unsigned int *pData, int textureWidth, int textureHeight,
      int width, int height, int rowOffset, int columnOffset)
{
   int i;
   int j;
   unsigned int* dest = NULL;
   const unsigned int* source = NULL;

// Fill in the texture's blank areas with replications of the images border pixels
// 123
// 4I5
// 678

   // Region 2
   source = &pData[0];
   dest = &pTexData[columnOffset];
   for (j = 0; j <= rowOffset; ++j)
   {
      memcpy(dest, source, 4 * width);
      dest += textureWidth;
   }

   // Region 7
   source = &pData[width * (height-1)];
   dest = &pTexData[textureWidth * (textureHeight-rowOffset) + columnOffset];
   for (j = textureHeight - rowOffset; j < textureHeight; ++j)
   {
      memcpy(dest, source, 4*width);
      dest += textureWidth;
   }

   for (j = 0; j <= rowOffset; ++j)
   {
      // Region 1
      dest = &pTexData[textureWidth * j];
      fill(dest, &dest[columnOffset + 1], pData[0]);

      // Region 3
      dest = &pTexData[textureWidth * (j+1) - columnOffset - 1];
      fill(dest, &dest[columnOffset + 1], pData[width-1]);
   }

   for (i = rowOffset; i < rowOffset + height; ++i)
   {
      // Region 4
      dest = &pTexData[textureWidth * i];
      fill(dest, &dest[columnOffset + 1], pData[width * (i - rowOffset)]);

      // Region 5
      dest = &pTexData[textureWidth * (i+1) - columnOffset - 1];
      fill(dest, &dest[columnOffset + 1], pData[width * (i - rowOffset) + width - 1]);
   }

   for (j = textureHeight - rowOffset; j < textureHeight; ++j)
   {
      // Region 6
      dest = &pTexData[textureWidth * j];
      fill(dest, &dest[columnOffset + 1], pData[0]);

      // Region 8
      dest = &pTexData[textureWidth * (j + 1) - columnOffset - 1];
      fill(dest, &dest[columnOffset + 1], pData[width - 1]);
   }
}

void ImageObjectImp::generateTextures()
{
   if (mData.empty())
   {
      return;
   }

   const unsigned int* pData = &mData[0];

   int pixelCount = mHeight * mWidth;

   // Round height and width up to the next larger powers of 2
   mTextureWidth = pow(2.0, 1.0 + floor((log10(static_cast<double>(mWidth + 2)) / log10(2.0)))) + 0.5;
   mTextureHeight = pow(2.0, 1.0 + floor((log10(static_cast<double>(mHeight + 2)) / log10(2.0)))) + 0.5;
   int bufSize = mTextureWidth * mTextureHeight; // rgb for each texel
   vector<unsigned int> texData(bufSize);

   unsigned int* dest = NULL;
   const unsigned int* source = NULL;

   memset(&texData[0], 0xff, mTextureWidth * mTextureHeight * 4);

   int j;
   int rowOffset = (mTextureHeight - mHeight) / 2;
   int columnOffset = (mTextureWidth - mWidth) / 2;

   fillTextureBorder(&texData[0], pData, mTextureWidth, mTextureHeight,
      mWidth, mHeight, rowOffset, columnOffset);

   // Fill the center of the texture with the image: Region I
   for (j = 0; j < mHeight; j++)
   {
      dest = &texData[mTextureWidth * (j + rowOffset) + columnOffset];
      source = &pData[mWidth * j];
      memcpy(dest, source, 4 * mWidth);
   }

   glEnable(GL_TEXTURE_2D);

   // Reallocate the existing texture or create a temporary texture if the context has changed.
   GLuint textureId;
   if (mTextureResource.get() == NULL || mTextureResource.getContext() == QGLContext::currentContext())
   {
      mTextureResource = GlTextureResource(1);
      textureId = mTextureResource;
      mpDrawContext = mTextureResource.getContext();
   }
   else
   {
      mTempTextureResource = GlTextureResource(1);
      textureId = mTempTextureResource;
      mpDrawContext = mTempTextureResource.getContext();
   }

   if (mpDrawContext == NULL || textureId == 0)
   {
      return;
   }

   glBindTexture(GL_TEXTURE_2D, textureId);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   gluBuild2DMipmaps(GL_TEXTURE_2D, 4, mTextureWidth, mTextureHeight, GL_RGBA, GL_UNSIGNED_BYTE, &texData[0]);
   glDisable(GL_TEXTURE_2D);
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
