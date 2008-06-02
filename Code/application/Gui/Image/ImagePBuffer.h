/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGEPBUFFER_H
#define IMAGEPBUFFER_H

#include "AppConfig.h"
#if defined(WIN_API)
#include <windows.h>
#endif

#include "ImageBuffer.h"

class ColorBuffer;

/**
*  Provides an offscreen image buffer to render data.
*
*  This class provides an implementation for rendering
*  data to an offscreen color buffer. This class uses
*  the OpenGL pbuffer extension to provide support for
*  offscreen rendering. When using the pbuffer extension
*  to render data to an offscreen buffer, OpenGL context
*  switching happens. Also, when the pbuffer is created,
*  its pixel format is highly dependent on the graphics
*  driver. Auxilary buffers may also be created when the
*  pbuffer is created which takes up more video memory.
*  The creation of the auxilary buffers seems to be dependent
*  on the graphics driver.
*
*  @see     FrameBuffer
*/
class ImagePBuffer : public ImageBuffer
{
public:
   ImagePBuffer();
   ~ImagePBuffer();

   /**
   *  Attaches the color buffer to the FrameBuffer object.
   *
   *  @param   pColorBuffer
   *           Color buffer.
   *
   *  @see     detachBuffer()
   */
   virtual bool attachBuffer(ColorBuffer *pColorBuffer);

   /**
   *  Dettaches the color buffer from the FrameBuffer object.
   *
   *  @param   pColorBuffer
   *           Color buffer.
   *
   *  @see     attachBuffer()
   */
   virtual void detachBuffer(ColorBuffer *pColorBuffer);

   /**
   *  Assigns the color buffer as the buffer to render.
   *
   *  @param   pColorBuffer
   *           Color buffer.
   *
   *  @see     readFromBuffer()
   */
   virtual void drawToBuffer(ColorBuffer *pColorBuffer);

   /**
   *  Assigns the color buffer as the buffer to read.
   *
   *  @param   pColorBuffer
   *           Color buffer.
   *
   *  @see     drawToBuffer()
   */
   virtual void readFromBuffer(ColorBuffer *pColorBuffer);

   /**
   *  Returns the number of available color buffer attachments.
   *
   *  This method provides the number of available color buffer attachments.
   *
   *  @return  The number of available color buffer attachments.
   *
   *  @see     attachBuffer(), generateColorBuffer()
   */
   virtual unsigned int colorBufferAttachments() const;

protected:
   /**
   *  Gets the next available color attachment id that the image buffer object has.
   *
   *  @return  Next available color buffer attachment point id.
   *
   *  @see     colorBufferAttachments()
   */
   virtual GLenum getNextColorBufferId() const;

private:
   // These are private in order to prevent someone from calling them
   ImagePBuffer(const ImagePBuffer &imagePBuffer);
   ImagePBuffer& operator=(const ImagePBuffer &imagePBuffer);

   /**
   *  Creates the pbuffer to contain the color buffer.
   *
   *  This method creates a pbuffer based on the color buffer passed in.
   *
   *  @return  true if the pbuffer was successfully created, false otherwise.
   */
   bool createPixelBuffer(ColorBuffer *pColorBuffer);

   /**
   *  Destroys the pbuffer.
   */
   void destroyPixelBuffer();

private:
#if defined(WIN_API)
   HDC mDeviceContext;              // pbuffer's device context
   HGLRC mRenderContext;            // pbuffer's rendering context
   HDC mPrevDeviceContext;          // previous device context
   HGLRC mPrevRenderContext;        // previous rendering context
   HDC mOSDeviceContext;            // device context in which pbuffer was created
   HGLRC mOSRenderContext;          // rendering context in which pbuffer was created
   HPBUFFERARB mPBuffer;            // pbuffer object
#endif

   GLenum mPrevBuffer;                       // previous color buffer attached
   unsigned int mMaxColorBufferAttachments;  // maximum number of color buffer attachments
   unsigned int mNextBuffer;        // next color buffer id to use when attaching next color buffer

   ColorBuffer *mpPrevColorBuffer;
};

#endif
