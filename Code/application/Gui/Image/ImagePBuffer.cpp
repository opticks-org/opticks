/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ColorBuffer.h"
#include "AppConfig.h"
#include "AppVerify.h"
#include "ImagePBuffer.h"
#include "ImageUtilities.h"

// STL header files
#include <vector>

using namespace std;

ImagePBuffer::ImagePBuffer() :
ImageBuffer(),
mPrevDeviceContext(0),
mDeviceContext(0),
mPrevRenderContext(0),
mRenderContext(0),
mOSDeviceContext(0),
mOSRenderContext(0),
mPrevBuffer(0),
mPBuffer(0),
mMaxColorBufferAttachments(1),
mNextBuffer(0)
{
}

ImagePBuffer::ImagePBuffer(const ImagePBuffer &imagePBuffer) :
ImageBuffer(imagePBuffer)
{
   mPrevDeviceContext = imagePBuffer.mPrevDeviceContext;
   mDeviceContext = imagePBuffer.mDeviceContext;
   mPrevRenderContext = imagePBuffer.mPrevRenderContext;
   mRenderContext = imagePBuffer.mRenderContext;
   mOSDeviceContext = imagePBuffer.mOSDeviceContext;
   mOSRenderContext = imagePBuffer.mOSRenderContext;
   mPrevBuffer = imagePBuffer.mPrevBuffer;
   mPBuffer = imagePBuffer.mPBuffer;
}

ImagePBuffer& ImagePBuffer::operator =(const ImagePBuffer &imagePBuffer)
{
   ImageBuffer::operator =(imagePBuffer);

   mPrevDeviceContext = imagePBuffer.mPrevDeviceContext;
   mDeviceContext = imagePBuffer.mDeviceContext;
   mPrevRenderContext = imagePBuffer.mPrevRenderContext;
   mRenderContext = imagePBuffer.mRenderContext;
   mOSDeviceContext = imagePBuffer.mOSDeviceContext;
   mOSRenderContext = imagePBuffer.mOSRenderContext;
   mPrevBuffer = imagePBuffer.mPrevBuffer;
   mPBuffer = imagePBuffer.mPBuffer;

   return *this;
}

ImagePBuffer::~ImagePBuffer()
{
   destroyPixelBuffer();
}

bool ImagePBuffer::createPixelBuffer(ColorBuffer *pColorBuffer)
{
   if (pColorBuffer == NULL)
   {
      return false;
   }

   GLenum textureFormat = pColorBuffer->getTextureFormat();

#if defined(WIN_API)
   // Get the current openGL device and rendering contexts
   mOSDeviceContext = wglGetCurrentDC();
   mOSRenderContext = wglGetCurrentContext();

   // Create pixel format for buffer
   vector<int> pixelFormatIntegerAttr;
   pixelFormatIntegerAttr.push_back(WGL_DRAW_TO_PBUFFER_ARB);
   pixelFormatIntegerAttr.push_back(GL_TRUE);
   pixelFormatIntegerAttr.push_back(WGL_FLOAT_COMPONENTS_NV);
   pixelFormatIntegerAttr.push_back(GL_TRUE);

   unsigned int dataTypeBits = 32;
   unsigned int numChannels = ImageUtilities::getNumColorChannels(textureFormat);

   // default to one channel
   GLenum wglBindToTextureRectangleFloat = WGL_BIND_TO_TEXTURE_RECTANGLE_FLOAT_R_NV;
   for (unsigned int ii = 1; ii < numChannels; ii++)
   {
      wglBindToTextureRectangleFloat++;
   }
   pixelFormatIntegerAttr.push_back(wglBindToTextureRectangleFloat);
   pixelFormatIntegerAttr.push_back(GL_TRUE);

   unsigned int colorChannel = WGL_RED_BITS_ARB;
   for (unsigned int ii = 0; ii < numChannels; ii++)
   {
      pixelFormatIntegerAttr.push_back(colorChannel);
      pixelFormatIntegerAttr.push_back(dataTypeBits);

      // need to increment by 2 since the #defines for the
      // color channels are 2 unsigned integers apart
      colorChannel += 2;
   }
   pixelFormatIntegerAttr.push_back(0);

   vector<float> pixelFormatFloatAttr;
   pixelFormatFloatAttr.push_back(0.0f);
   pixelFormatFloatAttr.push_back(0.0f);

   int nPixelFormat = 0;
   unsigned int nPixelFormats;

   // Choose the closest pixel format to the one desired
   VERIFYRV_MSG(wglChoosePixelFormatARB(mOSDeviceContext,
                                        &pixelFormatIntegerAttr[0],
                                        &pixelFormatFloatAttr[0],
                                        1,
                                        &nPixelFormat,
                                        &nPixelFormats), false, "ImagePBuffer: Unable to get pixel format")

   // default to one channel
   GLenum wglTextureFormat = WGL_TEXTURE_FLOAT_R_NV;
   for (unsigned int ii = 1; ii < numChannels; ii++)
   {
      wglTextureFormat++;
   }

   vector<int> pixelBufferIntegerAttr;
   pixelBufferIntegerAttr.push_back(WGL_TEXTURE_TARGET_ARB);
   pixelBufferIntegerAttr.push_back(WGL_TEXTURE_RECTANGLE_NV);
   pixelBufferIntegerAttr.push_back(WGL_TEXTURE_FORMAT_ARB);
   pixelBufferIntegerAttr.push_back(wglTextureFormat);
   pixelBufferIntegerAttr.push_back(WGL_PBUFFER_LARGEST_ARB);
   pixelBufferIntegerAttr.push_back(0);
   pixelBufferIntegerAttr.push_back(0);

   // Create the pbuffer
   mPBuffer = wglCreatePbufferARB(mOSDeviceContext, nPixelFormat, pColorBuffer->getWidth(), 
      pColorBuffer->getHeight(), &pixelBufferIntegerAttr[0]);
   VERIFYRV_MSG(mPBuffer != 0, false, "ImagePBuffer: Unable to get pixel buffer");

   // Create pbuffer's device and rendering contexts
   mDeviceContext = wglGetPbufferDCARB(mPBuffer);
   mRenderContext = wglCreateContext(mDeviceContext);


   // Add pbuffer's rendering context to a share list with the current
   // OpenGL rendering context to share the texture object
   wglShareLists(mOSRenderContext, mRenderContext);

   // Switch to the pbuffer's device and rendering contexts 
   wglMakeCurrent(mDeviceContext, mRenderContext);

   // Switch back to the device and rendering contexts used to create the pbuffer
   wglMakeCurrent(mOSDeviceContext, mOSRenderContext);
#endif
   return true;
}

void ImagePBuffer::destroyPixelBuffer()
{
#if defined(WIN_API)
   // Get the current openGL device and rendering contexts
   HDC currentDeviceContext = wglGetCurrentDC();
   HGLRC currentRenderContext = wglGetCurrentContext();

   // Switch to the context in which the pbuffer was created
   wglMakeCurrent(mOSDeviceContext, mOSRenderContext);

   if (mPBuffer != 0 && mDeviceContext != 0 && mRenderContext != 0)
   {
      // Release the pbuffer's device context
      wglReleasePbufferDCARB(mPBuffer, mDeviceContext);

      // Destroy the pbuffer
      wglDestroyPbufferARB(mPBuffer);

      // Delete the pbuffer's rendering context
      wglDeleteContext(mRenderContext);
   }

   // switch back to current context
   wglMakeCurrent(currentDeviceContext, currentRenderContext);
#endif
}

bool ImagePBuffer::attachBuffer(ColorBuffer *pColorBuffer)
{
   if (pColorBuffer == NULL)
   {
      return false;
   }

   bool success = false;

   // currently the pbuffer can only attach one color buffer
   if (mNextBuffer == 1)
   {
      return false;
   }

   // check to see if pbuffer has been created yet
   if (mPBuffer == 0)
   {
      // create pbuffer
      success = createPixelBuffer(pColorBuffer);
   }

   if (success)
   {
#if defined(WIN_API)
      // Get the current openGL device and rendering contexts
      HDC currentDeviceContext = wglGetCurrentDC();
      HGLRC currentRenderContext = wglGetCurrentContext();

      // add color buffer to image buffer object
      success = addBuffer(pColorBuffer);
      if (success == true && currentRenderContext != 0)
      {
         wglShareLists(currentRenderContext, mRenderContext);

         // Switch to the pbuffer's device and rendering contexts 
         wglMakeCurrent(mDeviceContext, mRenderContext);

         // Bind the pbuffer's texture object for color buffer 1
         GLuint texId = pColorBuffer->getTextureObjectId();
         glBindTexture(pColorBuffer->getTextureTarget(), texId);

         // Bind the pbuffer as a texture object
         if (!wglBindTexImageARB(mPBuffer, WGL_FRONT_LEFT_ARB))
         {
            success = false;
         }

         // switch back to current context
         wglMakeCurrent(currentDeviceContext, currentRenderContext);
      }
#endif
   }

   if (success)
   {
      mNextBuffer++;
   }

   return success;
}

void ImagePBuffer::detachBuffer(ColorBuffer *pColorBuffer)
{
   // currently the pbuffer can only attach to one color buffer
   if (isBufferAttached(pColorBuffer))
   {
      destroyPixelBuffer();
      removeBuffer(pColorBuffer);
   }
}

void ImagePBuffer::drawToBuffer(ColorBuffer *pColorBuffer)
{
   GLenum textureTarget;
   GLenum textureObjectId;
   if (pColorBuffer == NULL)
   {
      if (mpPrevColorBuffer != NULL)
      {
         textureTarget = mpPrevColorBuffer->getTextureTarget();
         textureObjectId = mpPrevColorBuffer->getTextureObjectId();


         if (mPrevDeviceContext != 0 && mPrevRenderContext != 0)
         {
            // Bind to the specified texture to allow the pbuffer to bind it
            glBindTexture(textureTarget, textureObjectId);

            // Bind the texture object to the pbuffer
            wglBindTexImageARB(mPBuffer, WGL_FRONT_LEFT_ARB);

            wglMakeCurrent(mPrevDeviceContext, mPrevRenderContext);

            // Bind the '0' texture object to prevent any incidental changes to the pbuffer's texture object
            glBindTexture(textureTarget, 0);
         }
      }
   }
   else
   {
      if (isBufferAttached(pColorBuffer))
      {
         mpPrevColorBuffer = pColorBuffer;
         textureTarget = pColorBuffer->getTextureTarget();
         textureObjectId = pColorBuffer->getTextureObjectId();

         // check to see if pixel buffer has been created
         if (mPBuffer != 0 && mDeviceContext != 0 && mRenderContext != 0)
         {
            if (textureObjectId != 0)
            {
               // get current OpenGL device and rendering contexts
               HDC currentDeviceContext = wglGetCurrentDC();
               HGLRC currentRenderContext = wglGetCurrentContext();

               if (currentDeviceContext != mDeviceContext && currentRenderContext != mRenderContext)
               {
                  mPrevDeviceContext = currentDeviceContext;
                  mPrevRenderContext = currentRenderContext;

                  mPrevBuffer = getColorBufferId(pColorBuffer);
                  wglMakeCurrent(mDeviceContext, mRenderContext);
               }

               // Bind to the specified texture to allow the pbuffer to release it
               glBindTexture(textureTarget, textureObjectId);

               // Release texture object since the pbuffer can't be bound to a texture object that is being rendered to
               wglReleaseTexImageARB(mPBuffer, WGL_FRONT_LEFT_ARB);
            }
         }
      }
   }
}

void ImagePBuffer::readFromBuffer(ColorBuffer *pColorBuffer)
{
   if (pColorBuffer == NULL)
   {
      if (mPrevDeviceContext != 0 && mPrevRenderContext != 0)
      {
         wglMakeCurrent(mPrevDeviceContext, mPrevRenderContext);
      }
   }
   else
   {
      if (isBufferAttached(pColorBuffer))
      {
         // check to see if pixel buffer has been created
         if (mPBuffer != 0 && mDeviceContext != 0 && mRenderContext != 0)
         {
            // get current OpenGL device and rendering contexts
            HDC currentDeviceContext = wglGetCurrentDC();
            HGLRC currentRenderContext = wglGetCurrentContext();

            if (currentDeviceContext != mDeviceContext && currentRenderContext != mRenderContext)
            {
               mPrevDeviceContext = currentDeviceContext;
               mPrevRenderContext = currentRenderContext;
               mPrevBuffer = getColorBufferId(pColorBuffer);
               wglMakeCurrent(mDeviceContext, mRenderContext);

               glReadBuffer(WGL_FRONT_LEFT_ARB);
            }
         }
      }
   }
}

unsigned int ImagePBuffer::colorBufferAttachments() const
{
   return (mMaxColorBufferAttachments - mNextBuffer);
}

GLenum ImagePBuffer::getNextColorBufferId() const
{
   return (mNextBuffer + 1);
}
