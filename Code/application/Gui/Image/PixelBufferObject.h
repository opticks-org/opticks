/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PIXELBUFFEROBJECT_H
#define PIXELBUFFEROBJECT_H

#ifndef PBO_BUFFER_OFFSET
#define PBO_BUFFER_OFFSET(i) (static_cast<char*>(NULL) + (i))
#endif

#include "glCommon.h"

/**
 *  This class is a wrapper class for the OpenGL extension, 
 *  Pixel Buffer Object (PBO). The PBO provides direct access to
 *  the graphics card texture memory.
 */
class PixelBufferObject
{
public:
   /**
    *  Constructor to the PixelBufferObject class.
    *
    *  @param   numBytes
    *           The number of bytes to allocate on the graphics card
    *           for the PBO.
    *  @param   accessMode
    *           The mode to be used when accessing graphics card
    *           memory.
    */
   PixelBufferObject(int numBytes, GLenum accessMode);

   /**
    *  Destructor to the PixelBufferObject class.
    */
   virtual ~PixelBufferObject();

   void bindBuffer(bool bind = true);

   /**
    *  Binds the PBO to allow it to be used when
    *  uploading or downloading data to the graphics card.
    *
    *  @return  True if PBO was successfully mapped, else false.
    *
    *  @see     releaseBuffer()
    */
   bool mapBuffer();

   /**
    *  Releases the PBO to complete the transfer of data to the graphics card. Data does not
    *  get transfered to or from the graphics card until this method is called.
    *
    *  @return  True if data was successfully transfered, else false.
    *
    *  @see     mapBuffer()
    */
   bool releaseBuffer();

   /**
    *  Copies the data to or from the PBO.
    *
    *  @return  True if data was successfully copied, else false.
    *
    *  @see     releaseBuffer()
    */
   bool copyBytes(void *pData, int numBytes);

   /**
    *  Returns the access mode of the PBO eg: (GL_READ_ONLY).
    *
    *  @return  The OpenGL enumeration type stating the i/o
    *           access mode of the PBO.
    */
   GLenum getAccessMode();

   /**
    *  Returns the size of the PBO in bytes.
    *
    *  @return  The number of bytes allocated for the PBO.
    */
   int getSize();

private: // member methods
   GLuint allocatePixelBufferObject(int numBytes, GLenum access);

private: // member variables
   GLuint mPixelBufferObjectId;
   GLenum mAccessMode;
   GLenum mTarget;
   int mNumBytes;
   void* mpDataBuffer;
};

#endif
