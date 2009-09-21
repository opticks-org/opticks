/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ColorBuffer.h"
#include "GpuResourceManager.h"
#include "ImageUtilities.h"
#include "MessageLogResource.h"

#include <vector>
using namespace std;

ColorBuffer::ColorBuffer(GLenum textureTarget, GLint internalFormat, int width, int height, GLenum textureFormat,
                         GLenum dataType, unsigned int alpha) :
   mTextureObjectId(0),
   mWidth(width),
   mHeight(height),
   mInternalFormat(internalFormat),
   mTextureFormat(textureFormat),
   mTextureTarget(textureTarget),
   mDataType(dataType),
   mAlpha(alpha)
{
   // allocate GPU texture memory
   Service<GpuResourceManager> pGpuResourceManager;
   mTextureObjectId = pGpuResourceManager->allocateTexture(textureTarget, internalFormat, width, height,
      textureFormat, dataType);

   if (isTextureResident() == false)
   {
      MessageResource("Unable to allocate sufficient texture memory on the graphics card.", "app",
         "6944D2F4-3C6D-4457-B8D7-AAF5BDAD6B7F");
   }
}

ColorBuffer::ColorBuffer(int width, int height, GLenum textureFormat, GLenum dataType, unsigned int alpha) :
   mTextureObjectId(0),
   mWidth(width),
   mHeight(height),
   mInternalFormat(GL_LUMINANCE),
   mTextureFormat(textureFormat),
   mTextureTarget(GL_TEXTURE_RECTANGLE_ARB),
   mDataType(dataType),
   mAlpha(alpha)
{
   // NOTE: Since the OpenGL FBO extension is not supported, do not need to allocate
   //       texture memory on the graphics card. The pbuffer extension allows the use
   //       of its surfaces to render data to. These surfaces are then accessed using
   //       the "WGL_ARB_render_texture" extension which uses an OpenGL texture object
   //       to allow access to the data.

   // generate texture object id
   glGenTextures(1, &mTextureObjectId);
}

ColorBuffer::~ColorBuffer()
{
   // release Gpu texture memory resources
   if (mTextureObjectId != 0)
   {
      Service<GpuResourceManager> pGpuResourceManager;
      pGpuResourceManager->deallocateTexture(mTextureObjectId);
   }
}

int ColorBuffer::getSize() const
{
   return (mWidth * mHeight * ImageUtilities::sizeOf(mDataType) * ImageUtilities::getNumColorChannels(mTextureFormat));
}

int ColorBuffer::getWidth() const
{
   return mWidth;
}

int ColorBuffer::getHeight() const
{
   return mHeight;
}

unsigned int ColorBuffer::getAlpha() const
{
   return mAlpha;
}

GLenum ColorBuffer::getTextureFormat() const
{
   return mTextureFormat;
}

GLint ColorBuffer::getInternalFormat() const
{
   return mInternalFormat;
}

GLuint ColorBuffer::getTextureTarget() const
{
   return mTextureTarget;
}

GLenum ColorBuffer::getDataType() const
{
   return mDataType;
}

GLuint ColorBuffer::getTextureObjectId() const
{
   return mTextureObjectId;
}

bool ColorBuffer::isTextureResident() const
{
   bool bResident = false;
   if (mTextureObjectId != 0)
   {
      GLenum textureTarget = getTextureTarget();

      glEnable(textureTarget);
      glBindTexture(textureTarget, mTextureObjectId);

      GLint resident;
      glGetTexParameteriv(textureTarget, GL_TEXTURE_RESIDENT, &resident);
      if (resident == GL_TRUE)
      {
         bResident = true;
      }

      glBindTexture(textureTarget, 0);
      glDisable(textureTarget);
   }

   return bResident;
}

void ColorBuffer::clear()
{
   GLuint textureObjectId = getTextureObjectId();
   if (textureObjectId == 0)
   {
      return;
   }

   GLenum textureTarget = getTextureTarget();
   GLenum textureFormat = getTextureFormat();
   GLenum dataType = getDataType();
   int width = getWidth();
   int height = getHeight();
   unsigned int numChannels = ImageUtilities::getNumColorChannels(textureFormat);

   glEnable(textureTarget);
   glBindTexture(textureTarget, textureObjectId);

   switch (dataType)
   {
   case GL_UNSIGNED_BYTE:
      {
         vector<unsigned char> data(width * height * numChannels, 0);
         glTexSubImage2D(textureTarget, 0, 0, 0, width, height, textureFormat, dataType, &data[0]);
      }
      break;
   case GL_BYTE:
      {
         vector<char> data(width * height * numChannels, 0);
         glTexSubImage2D(textureTarget, 0, 0, 0, width, height, textureFormat, dataType, &data[0]);
      }
      break;
   case GL_UNSIGNED_SHORT:
      {
         vector<unsigned short> data(width * height * numChannels, 0);
         glTexSubImage2D(textureTarget, 0, 0, 0, width, height, textureFormat, dataType, &data[0]);
      }
      break;
   case GL_SHORT:
      {
         vector<short> data(width * height * numChannels, 0);
         glTexSubImage2D(textureTarget, 0, 0, 0, width, height, textureFormat, dataType, &data[0]);
      }
      break;
   case GL_UNSIGNED_INT:
      {
         vector<unsigned int> data(width * height * numChannels, 0);
         glTexSubImage2D(textureTarget, 0, 0, 0, width, height, textureFormat, dataType, &data[0]);
      }
      break;
   case GL_INT:
      {
         vector<int> data(width * height * numChannels, 0);
         glTexSubImage2D(textureTarget, 0, 0, 0, width, height, textureFormat, dataType, &data[0]);
      }
      break;
   case GL_FLOAT:
      {
         vector<float> data(width * height * numChannels, 0);
         glTexSubImage2D(textureTarget, 0, 0, 0, width, height, textureFormat, dataType, &data[0]);
      }
      break;
   default:
      break;
   }

   glBindTexture(textureTarget, 0);
   glDisable(textureTarget);
}
