/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ColorBuffer.h"
#include "FrameBuffer.h"
#include "MessageLogResource.h"
#include "UtilityServices.h"

// STL header files
#include <string>

FrameBuffer::FrameBuffer() :
   ImageBuffer(),
   mFrameBufferObject(createFrameBufferObject()),
   mPreviousFrameBufferObject(0),
   mPreviousColorBufferId(0),
   mNextColorBufferId(0)
{
   // allocate memory for the view port of the FrameBuffer object (FBO)
   mPreviousViewPort.resize(4);
   mPreviousViewPort[0] = 0;
   mPreviousViewPort[1] = 0;
   mPreviousViewPort[2] = 1;
   mPreviousViewPort[3] = 1;

   if (mFrameBufferObject != 0)
   {
      GLint maxColorBuffers = 0;
      GLint currentFrameBufferObject = 0;

      // get the currently bound FBO
      glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFrameBufferObject);

      // bind framebuffer object
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFrameBufferObject);

      // get the maximum number of color buffers the FBO can handle
      glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &maxColorBuffers);

      // insert the color buffer attachments into the vector for this FBO
      for (int ii = 0; ii < maxColorBuffers; ii++)
      {
         mColorBufferAttachments.push_back(GL_COLOR_ATTACHMENT0_EXT + ii);
      }

      // bind back to the current FrameBuffer object
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, currentFrameBufferObject);
   }
}

// copy constructor
FrameBuffer::FrameBuffer(const FrameBuffer& frameBuffer) :
   ImageBuffer(frameBuffer),
   mFrameBufferObject(frameBuffer.mFrameBufferObject),
   mPreviousFrameBufferObject(frameBuffer.mPreviousFrameBufferObject)
{
   for (unsigned int i = 0; i < 4; i++)
   {
      mPreviousViewPort[i] = frameBuffer.mPreviousViewPort[i];
   }

   mColorBufferAttachments = frameBuffer.mColorBufferAttachments;
}

// assignment operator
FrameBuffer& FrameBuffer::operator =(const FrameBuffer &frameBuffer)
{
   ImageBuffer::operator =(frameBuffer);

   mFrameBufferObject = frameBuffer.mFrameBufferObject;
   mPreviousFrameBufferObject = frameBuffer.mPreviousFrameBufferObject;

   for (unsigned int ii = 0; ii < 4; ii++)
   {
      mPreviousViewPort[ii] = frameBuffer.mPreviousViewPort[ii];
   }

   mColorBufferAttachments = frameBuffer.mColorBufferAttachments;

   return *this;
}

// equal? operator
bool FrameBuffer::operator ==(const FrameBuffer &frameBuffer)
{
   return ((ImageBuffer::operator ==(frameBuffer)) &&
           (mFrameBufferObject == frameBuffer.mFrameBufferObject) &&
           (mPreviousFrameBufferObject == frameBuffer.mPreviousFrameBufferObject) &&
           (mPreviousViewPort == frameBuffer.mPreviousViewPort) &&
           (mColorBufferAttachments == frameBuffer.mColorBufferAttachments));
}

FrameBuffer::~FrameBuffer()
{
   GLint currentFrameBufferObject = 0;

   // get the currently bound FBO
   glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFrameBufferObject);

   // if the currently bound FBO equals this class's FBO,
   // bind to the previous FBO and draw to the previous buffer
   if (mFrameBufferObject != 0 && mFrameBufferObject == currentFrameBufferObject)
   {
      // bind FBO
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mPreviousFrameBufferObject);

      // draw to previous buffer
      glDrawBuffer(mPreviousColorBufferId);

      // set the previous view port coordinates
      glViewport(mPreviousViewPort[0], mPreviousViewPort[1], mPreviousViewPort[2], mPreviousViewPort[3]);
   }

   // delete the OpenGL FBO to free up GPU memory resources
   if (mFrameBufferObject != 0)
   {
      glDeleteFramebuffersEXT(1, &mFrameBufferObject);
   }
}

GLuint FrameBuffer::createFrameBufferObject() const
{
   GLuint frameBufferObject = 0;

   // allocate memory for FBO
   glGenFramebuffersEXT(1, &frameBufferObject);
   return frameBufferObject;
}

bool FrameBuffer::checkFrameBufferStatus(GLenum status)
{
   std::string msg;
   std::string key;
   switch (status)
   {
   case GL_FRAMEBUFFER_COMPLETE_EXT:
      return true;
   case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
      msg = "Framebuffer incomplete.";
      key = "C4B4FB9E-9C4D-4B71-B9AC-31E872E8A6F2";
      break;
   case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      msg = "Unsupported framebuffer format.";
      key = "25334E34-8E6B-4A45-A720-934D2E6669A7";
      break;
   case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      msg = "Framebuffer incomplete, missing attachment.";
      key = "5724AD06-F582-44BA-848D-81B7D3C95DFB";
      break;
   case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      msg = "Framebuffer incomplete, attached images must have same dimensions.";
      key = "997AB306-8337-490C-8857-1E59A86CDFA8";
      break;
   case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      msg = "Framebuffer incomplete, attached images must have same format.";
      key = "23C4781D-AC52-4961-926E-AA225B441EF0";
      break;
   case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      msg = "Framebuffer incomplete, missing draw buffer.";
      key = "D4865D8A-7994-4B45-9F28-846FBB7E2D9C";
      break;
   case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      msg = "Framebuffer incomplete, missing read buffer.";
      key = "1CAEE070-EE28-4A35-889E-7E6A764D834E";
      break;
   default:
      msg = "Unknown Framebuffer error.";
      key = "4BF739B4-3FBD-4E74-831B-BA41C76065D7";
      break;
   }
   MessageResource m(msg, "app", key);

   return false;
}

void FrameBuffer::drawToBuffer(ColorBuffer *pColorBuffer)
{
   GLint currentFrameBufferObject = 0;

   // get the currently bound FrameBuffer object
   glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFrameBufferObject);

   if (pColorBuffer == NULL)
   {
      if (currentFrameBufferObject != mPreviousFrameBufferObject)
      {
         // bind to the previous FBO
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mPreviousFrameBufferObject);

         // draw to the previous buffer
         glDrawBuffer(mPreviousColorBufferId);
         
         // set the previous view port coordinates
         glViewport(mPreviousViewPort[0], mPreviousViewPort[1], mPreviousViewPort[2], mPreviousViewPort[3]);
      }
   }
   else
   {
      GLenum colorBufferId = getColorBufferId(pColorBuffer);
      if (colorBufferId == 0 || mFrameBufferObject == 0)
      {
         return;
      }

      if (mFrameBufferObject != currentFrameBufferObject)
      {
         // store the current FrameBuffer object
         mPreviousFrameBufferObject = currentFrameBufferObject;

         // store the current color buffer id
         glGetIntegerv(GL_DRAW_BUFFER, &mPreviousColorBufferId);

         // store current view port coordinates
         glGetIntegerv(GL_VIEWPORT, &mPreviousViewPort[0]);

         // bind framebuffer object
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFrameBufferObject);

         // Draw to the buffer
         glDrawBuffer(colorBufferId);

         // set view port for color buffer attachment
         glViewport(0, 0, pColorBuffer->getWidth(), pColorBuffer->getHeight());
      }
      else // currently bound FBO is this FrameBuffer class object's FBO
      {
         // Draw to the buffer
         glDrawBuffer(colorBufferId);
      }
   }
}

void FrameBuffer::readFromBuffer(ColorBuffer *pColorBuffer)
{
   GLint currentFrameBufferObject = 0;

   // get the currently bound FrameBuffer object
   glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFrameBufferObject);

   if (pColorBuffer == NULL)
   {
      if (currentFrameBufferObject != mPreviousFrameBufferObject)
      {
         // bind to the graphics card's framebuffer
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mPreviousFrameBufferObject);
    
         // set the previous view port coordinates
         glViewport(mPreviousViewPort[0], mPreviousViewPort[1], mPreviousViewPort[2], mPreviousViewPort[3]);

         // read from the previous buffer
         glReadBuffer(mPreviousColorBufferId);
      }
   }
   else
   {
      GLenum colorBufferId = getColorBufferId(pColorBuffer);
      if (colorBufferId == 0 || mFrameBufferObject == 0)
      {
         return;
      }

      if (mFrameBufferObject != currentFrameBufferObject)
      {
         // store the current FrameBuffer object
         mPreviousFrameBufferObject = currentFrameBufferObject;

         // get the current draw buffer
         glGetIntegerv(GL_READ_BUFFER, &mPreviousColorBufferId);
         
         // get current view port coordinates
         glGetIntegerv(GL_VIEWPORT, &mPreviousViewPort[0]);

         // bind framebuffer object
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFrameBufferObject);

         // set view port for color buffer attachment
         glViewport(0, 0, pColorBuffer->getWidth(), pColorBuffer->getHeight());

         // Read from the buffer
         glReadBuffer(colorBufferId);
      }
      else
      {
         // Read from the buffer
         glReadBuffer(colorBufferId);
      }
   }
}

bool FrameBuffer::attachBuffer(ColorBuffer *pColorBuffer)
{
   // check to make sure the FBO was created and the color buffer is valid
   if (mFrameBufferObject == 0 || pColorBuffer == NULL)
   {
      return false;
   }

   // check to see if the color buffer was already attached
   if (isBufferAttached(pColorBuffer))
   {
      return false;
   }

   bool bValue = false;
   GLint maxColorBuffers = 0;
   GLenum status = GL_FRAMEBUFFER_UNSUPPORTED_EXT;
   GLint currentFrameBufferObject = 0;

   // check that there are available color buffer attachments
   if (mNextColorBufferId < mColorBufferAttachments.size())
   {
      GLenum colorBufferId = getNextColorBufferId();
      if (colorBufferId == 0)
      {
         return false;
      }

      glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFrameBufferObject);
      if (currentFrameBufferObject != mFrameBufferObject)
      {
         // bind to the class's FBO
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFrameBufferObject);
      }

      // attach texture to framebuffer object
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, colorBufferId, pColorBuffer->getTextureTarget(), 
                                pColorBuffer->getTextureObjectId(), 0);

      // check to make sure the texture successfully attached to the framebuffer object
      status = static_cast<GLenum>(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));

      if (currentFrameBufferObject != mFrameBufferObject)
      {
         // bind back to the previous framebuffer
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, currentFrameBufferObject);
      }
   }

   // read the status of the attachment
   bValue = checkFrameBufferStatus(status);
   if (bValue == true)
   {
      // add color buffer to image buffer object
      bool bAdded = addBuffer(pColorBuffer);
      if (bAdded == false)
      {
         return false;
      }
      mNextColorBufferId = mNextColorBufferId + 1;
   }
   
   return bValue;
}

void FrameBuffer::detachBuffer(ColorBuffer *pColorBuffer)
{
   // check to make sure the FBO was created and the color buffer is valid
   if (mFrameBufferObject == 0 || pColorBuffer == NULL)
   {
      return;
   }

   // check to see if the color buffer was attached
   if (isBufferAttached(pColorBuffer) == false)
   {
      return;
   }

   GLint currentFrameBufferObject = 0;

   // get the currently bound FrameBuffer object
   glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFrameBufferObject);
   if (currentFrameBufferObject != mFrameBufferObject)
   {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFrameBufferObject);  // bind framebuffer object
   }

   // get the texture target
   GLenum textureTarget = pColorBuffer->getTextureTarget();
   GLenum colorBufferId = getColorBufferId(pColorBuffer);
   if (colorBufferId != 0)
   {
      // detach texture from FrameBuffer's color buffer attachment
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, colorBufferId, textureTarget, 0, 0);

      if (currentFrameBufferObject != mFrameBufferObject)
      {
         // bind back to previous framebuffer object
         glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, currentFrameBufferObject);
      }

      // remove color buffer
      removeBuffer(pColorBuffer);

      if (mNextColorBufferId != 0)
      {
         // decrement mNextColorBuffer to show how many color buffer attachments are available
         mNextColorBufferId--;
      }
   }
}

unsigned int FrameBuffer::colorBufferAttachments() const
{
   // returns the number of available color buffer attachment ids
   return (mColorBufferAttachments.size() - mNextColorBufferId);
}

GLenum FrameBuffer::getNextColorBufferId() const
{
   // Note: This method returns one of the FBO's color buffer
   //       attachment ids. If the attachBuffer(GLenum buffer)
   //       method fails or is not called before this method is 
   //       called again, the 'mNextColorBuffer' variable will 
   //       not be incremented. Therefore, the next time this
   //       method is called the same color buffer attachment
   //       id will be returned.
   return (mColorBufferAttachments[mNextColorBufferId]);
}
