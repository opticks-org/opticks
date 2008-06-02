/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef IMAGE_BUFFER_H
#define IMAGE_BUFFER_H

#include "glCommon.h"

// STL header files
#include <map>

class ColorBuffer;

/**
 *  Provides an offscreen image buffer to render data.
 *
 *  This class provides an interface for rendering
 *  data to an offscreen buffer.
 *
 *  @see     ImageColorBuffer
 */
class ImageBuffer
{
public:
   ImageBuffer();
   virtual ~ImageBuffer();

   /**
    *  Attaches a color buffer to the ImageBuffer object.
    *
    *  This method provides the capability to attach a maximum
    *  number of color buffers to the ImageBuffer object, ranging
    *  from 1 to maximum number of color buffers allowed.
    *
    *  NOTE: Attaching color buffers of different sizes or internal
    *        formats will impair performance when rendering to or
    *        reading from the attached color buffers.
    *
    *  @param   pColorBuffer
    *           The color buffer to attach.
    *
    *  @return  true if the method is successfull
    *           false if the method is unsuccessfull
    *
    *  @see     detachBuffer()
    */
   virtual bool attachBuffer(ColorBuffer *pColorBuffer) = 0;

   /**
    *  Dettaches a color buffer from the ImageBuffer object.
    *
    *  This method provides the capability to dettach a color
    *  buffer from the ImageBuffer object.
    *
    *  @param   pColorBuffer
    *           The color buffer to dettach.
    *
    *  @see     attachBuffer()
    */
   virtual void detachBuffer(ColorBuffer *pColorBuffer) = 0;

    /**
    *  Checks to see if a color buffer is attached to the ImageBuffer object.
    *
    *  This method provides the capability to search for a specified
    *  color buffer to see if it is attached to the ImageBuffer object.
    *
    *  @param   pColorBuffer
    *           The color buffer.
    *
    *  @return  true if the color buffer is attached
    *           false if the color buffer is not attached
    *
    *  @see     attachBuffer(), detachBuffer()
    */
   bool isBufferAttached(ColorBuffer *pColorBuffer) const;

   /**
    *  Returns the number of available color buffer attachments.
    *
    *  This method provides the number of available color buffer attachments.
    *
    *  @return  The number of available color buffer attachments.
    *
    *  @see     attachBuffer(), generateColorBuffer()
    */
   virtual unsigned int colorBufferAttachments() const = 0;

   /**
    *  Switches to the color buffer to render to.
    *
    *  This method provides the capability to switch to
    *  a color buffer to render to.
    *
    *  @param   pColorBuffer
    *           The color buffer to switch to.
    *
    *  @see     readFromBuffer()
    */
   virtual void drawToBuffer(ColorBuffer *pColorBuffer) = 0;

   /**
    *  Switches to the color buffer to read from.
    *
    *  This method provides the capability to switch to
    *  a color buffer to read from.
    *
    *  @param   pColorBuffer
    *           The color buffer to switch to.
    *
    *  @see     drawToBuffer()
    */
   virtual void readFromBuffer(ColorBuffer *pColorBuffer) = 0;


protected: // member methods
   /**
    *  Adds the color buffer to the map of currently attached color buffers.
    *
    *  This method adds the color buffer to the currently attached color buffers.
    *
    *  @param   pColorBuffer
    *           The color buffer to add.
    *
    *  @return  true if the method is successfull
    *           false if the method is unsuccessfull
    *
    *  @see     removeBuffer()
    */
   bool addBuffer(ColorBuffer *pColorBuffer);

   /**
    *  Removes the color buffer from the map of currently attached color buffers.
    *
    *  This method removes the color buffer from the currently attached color buffers.
    *
    *  @param   pColorBuffer
    *           The color buffer to add.
    *
    *  @see     addBuffer()
    */
   void removeBuffer(ColorBuffer *pColorBuffer);

   /**
    *  Gets the color buffer attachment id for the color buffer.
    *
    *  @param  pColorBuffer
    *          Get the attachment id associated with this ColorBuffer.
    *
    *  @return  Color buffer attachment point id if the color buffer is attached.
    *           Otherwise, returns 0.
    *
    *  @see     getNextColorBufferId()
    */
   GLenum getColorBufferId(ColorBuffer *pColorBuffer) const;

   /**
    *  Gets the next available color attachment id that the image buffer object has.
    *
    *  @return  Next available color buffer attachment point id.
    *
    *  @see     colorBufferAttachments()
    */
   virtual GLenum getNextColorBufferId() const = 0;

   // TODO: NOTE: The copy constructor, assignment operator, and "==" operator
   //             are not fully implemented due to problem of creating and
   //             destroying OpenGL texture objects
   // copy construct and assignment operation methods
   ImageBuffer(const ImageBuffer &imageBuffer) { mAttachedBuffers = imageBuffer.mAttachedBuffers; }
   ImageBuffer& operator=(const ImageBuffer &imageBuffer)
   {
      mAttachedBuffers = imageBuffer.mAttachedBuffers;
      return *this;
   }

   bool operator==(const ImageBuffer &imageBuffer)
   {
      return (mAttachedBuffers == imageBuffer.mAttachedBuffers);
   }

private: // member varaibles
   std::map<ColorBuffer*, GLenum> mAttachedBuffers; // current color buffers
};

#endif
