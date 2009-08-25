/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "ImageBuffer.h"
#include "glCommon.h"

// STL header files
#include <vector>

/**
 *  Provides an offscreen image buffer to render data.
 *
 *  This class provides an implementation for rendering
 *  data to an offscreen color buffer. This class uses the
 *  OpenGL FrameBuffer Object (FBO) extension to provide the
 *  capability to render to an offscreen buffer. In using the
 *  FBO extension, the FrameBuffer class avoids OpenGL context
 *  switching. Another benefit of using the FBO extension is
 *  that it takes up less video memory on the graphics card.
 *
 *  @see     ImageBuffer
 */
class FrameBuffer : public ImageBuffer
{
public:
   FrameBuffer();
   ~FrameBuffer();

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

   GLuint createFrameBufferObject() const;
   bool checkFrameBufferStatus(GLenum status);     // checks the status of the FBO object

private:
   // These are private in order to prevent someone from calling them
   FrameBuffer(const FrameBuffer &frameBuffer);    
   FrameBuffer& operator=(const FrameBuffer &frameBuffer);
   bool operator==(const FrameBuffer &frameBuffer);

private:
   GLuint mFrameBufferObject;                      // Frame buffer OpenGL extension object
   GLuint mPreviousFrameBufferObject;              // Previous Frame buffer OpenGL extension object
   GLint mPreviousColorBufferId;                   // Previous color buffer id that was used
   GLenum mNextColorBufferId;                      // Next available color buffer id
   std::vector<int> mPreviousViewPort;             // viewport of the previously bound framebuffer object
   std::vector<GLenum> mColorBufferAttachments;    // vector of available color buffer attachments for this FBO
};

#endif
