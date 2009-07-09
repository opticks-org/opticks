/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

// PixelBufferOjbect header file
#include "PixelBufferObject.h"

#include "AppAssert.h"
#include <string.h>

PixelBufferObject::PixelBufferObject(int numBytes, GLenum accessMode) :
   mPixelBufferObjectId(0),
   mAccessMode(accessMode),
   mNumBytes(numBytes),
   mpDataBuffer(NULL)
{
   REQUIRE(mNumBytes != 0);

   switch (mAccessMode)
   {
   case GL_READ_ONLY:
      mTarget = GL_PIXEL_PACK_BUFFER_ARB;
      mPixelBufferObjectId = allocatePixelBufferObject(mNumBytes, GL_STATIC_READ_ARB);
      break;
   case GL_WRITE_ONLY:
      mTarget = GL_PIXEL_UNPACK_BUFFER_ARB;
      mPixelBufferObjectId = allocatePixelBufferObject(mNumBytes, GL_STREAM_DRAW_ARB);
      break;
   default:
      break;
   }

   REQUIRE(mPixelBufferObjectId != 0);
}

PixelBufferObject::~PixelBufferObject()
{
   // release memory on the graphics card
   glDeleteBuffersARB(1, &mPixelBufferObjectId);
}
void PixelBufferObject::bindBuffer(bool bind)
{
   if (bind)
   {
      glBindBufferARB(mTarget, mPixelBufferObjectId);
   }
   else
   {
      glBindBufferARB(mTarget, 0);
   }
}

bool PixelBufferObject::mapBuffer()
{
   mpDataBuffer = glMapBufferARB(mTarget, mAccessMode);
   return (mpDataBuffer != NULL);
}

bool PixelBufferObject::releaseBuffer()
{
   if (glUnmapBufferARB(mTarget))
   {
      return true;
   }
   else
   {
      return false;
   }
}

bool PixelBufferObject::copyBytes(void *pData, int numBytes)
{
   if (numBytes > mNumBytes || numBytes <= 0)
   {
      return false;
   }

   void* pReturn = NULL;
   if (mpDataBuffer != NULL && pData != NULL)
   {
      switch (mAccessMode)
      {
         case GL_READ_ONLY:
            pReturn = memcpy(pData, mpDataBuffer, numBytes);
            break;
         case GL_WRITE_ONLY:
            pReturn = memcpy(mpDataBuffer, pData, numBytes);
            break;
         default:
            break;
      }
   }

   return (pReturn != NULL);
}

GLenum PixelBufferObject::getAccessMode()
{
   return mAccessMode;
}

int PixelBufferObject::getSize()
{
   return mNumBytes;
}

GLuint PixelBufferObject::allocatePixelBufferObject(int numBytes, GLenum access)
{
   GLuint pixelBufferObject = 0;
   
   // allocate pixel buffer object
   glGenBuffersARB(1, &pixelBufferObject);
   if (numBytes == 0 || pixelBufferObject == 0)
   {
      return 0;
   }

   // bind pixel-buffer object
   glBindBufferARB(mTarget, pixelBufferObject);
                          
   // create pixel-buffer data container
   glBufferDataARB(mTarget, numBytes, NULL, access);

   // unbind pixel-buffer object
   glBindBufferARB(mTarget, 0);
   
   return pixelBufferObject;
}
