/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppAssert.h"
#include "ImagePBO.h"
#include "ImageUtilities.h"
#include "PixelBufferObject.h"

ImagePBO::ImagePBO(ColorBuffer *pColorBuffer, PixelBufferObject *pPixelBufferObject) :
   ImageLoader(pColorBuffer),
   mpPixelBufferObject(pPixelBufferObject)
{
   REQUIRE(mpPixelBufferObject != NULL);
}

ImagePBO::~ImagePBO()
{
   delete mpPixelBufferObject;
}

void ImagePBO::loadData(void *pData)
{
   GLenum accessMode = mpPixelBufferObject->getAccessMode();
   if ((accessMode & GL_WRITE_ONLY) != GL_WRITE_ONLY)
   {
      return;
   }

   ColorBuffer* pColorBuffer = getColorBuffer();
   if (pColorBuffer == NULL)
   {
      return;
   }

   GLuint texId = pColorBuffer->getTextureObjectId();
   if (texId == 0)
   {
      return;
   }

   int width = pColorBuffer->getWidth();
   int height = pColorBuffer->getHeight();
   unsigned int numBytes = pColorBuffer->getSize();

   GLenum textureTarget = pColorBuffer->getTextureTarget();
   GLenum textureFormat = pColorBuffer->getTextureFormat();
   GLenum dataType = pColorBuffer->getDataType();

   // bind pixel buffer object
   mpPixelBufferObject->bindBuffer();
      
   // map pixel buffer object
   if (mpPixelBufferObject->mapBuffer())
   {
      // copy image data from CPU memory to graphics card memory buffer
      mpPixelBufferObject->copyBytes(pData, numBytes);

      // unmap pixel buffer object to transfer data         
      mpPixelBufferObject->releaseBuffer();
    
      // bind the texture object
      glEnable(textureTarget);
      glBindTexture(textureTarget, texId);

      // copy buffer contents into the texture memory
      glTexSubImage2D(textureTarget, 0, 0, 0, width, height, textureFormat, dataType, PBO_BUFFER_OFFSET(0));

      // unbind texture
      glBindTexture(textureTarget, 0);
      glDisable(textureTarget);
   }

   // unbind pixel buffer object
   mpPixelBufferObject->bindBuffer(false);
}

void ImagePBO::read(GLint xCoord, GLint yCoord, GLsizei width, GLsizei height, 
                    GLenum textureFormat, GLenum dataType, GLvoid *pPixels)
{
   // get the number of bytes to allocate for the PBO
   ColorBuffer* pColorBuffer = getColorBuffer();
   if (pColorBuffer == NULL)
   {
      return;
   }

   if (width > 0 && height > 0)
   {
      // bind pixel buffer object
      mpPixelBufferObject->bindBuffer();
      
      // read pixels from current color buffer using bound PBO
      glReadPixels(xCoord, yCoord, width, height, textureFormat, dataType, PBO_BUFFER_OFFSET(0));

      if (mpPixelBufferObject->mapBuffer())
      {
         int numBytes = width * height * ImageUtilities::sizeOf(pColorBuffer->getDataType()) * 
                           ImageUtilities::getNumColorChannels(pColorBuffer->getTextureFormat());
         // copy data from color buffer to pixel buffer object
         mpPixelBufferObject->copyBytes(pPixels, numBytes);

         // unmap pixel buffer object to transfer data         
         mpPixelBufferObject->releaseBuffer();
      }

      // unbind pixel buffer object
      mpPixelBufferObject->bindBuffer(false);
   }
}
