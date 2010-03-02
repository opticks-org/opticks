/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include "ColorBuffer.h"

/**
 *  Provides a way to load the data to the graphics card's texture memory.
 *
 *  This class sets the settings needed to transform the data when it is
 *  loaded to the graphics card. It also provides internal information on
 *  the data to load. It's primary function is to load the data to the 
 *  graphics card.
 *
 */
class ImageLoader
{
public:
   /**
    *  Constructor.
    *
    *  @param pColorBuffer
    *         The GL texture which this loader pushes data to. The ImageLoader takes ownership of this object.
    */
   explicit ImageLoader(ColorBuffer *pColorBuffer);

   virtual ~ImageLoader();

   /**
    *  Loads the data to the graphics card using the
    *  OpenGL texture object.
    *
    *  @param   pData
    *           The data to be loaded to the graphics card.
    */
   virtual void loadData(void *pData);

   /**
    *  Gets the color buffer object used to load data to the
    *  graphics card.
    *
    *  @return   The color buffer object.
    */
   virtual ColorBuffer* getColorBuffer() const;

   /**
    *  Writes to the current color buffer.
    *
    *  @param   xCoord
    *           Lower left corner X coordinate.
    *
    *  @param   yCoord
    *           Lower left corner Y coordinate.
    *
    *  @param   width
    *           The width of the rectangle of pixels to write.
    *
    *  @param   height
    *           The height of the rectangle of the pixels to write.
    *
    *  @param   textureFormat
    *           The format of the color buffer to write.
    *
    *  @param   dataType
    *           The type of the data to write (eg: GL_FLOAT).
    *
    *  @param   pPixels
    *           The pixels to be written out to the color buffer.
    *
    *  @see     read()
    */
   virtual void write(GLint xCoord, GLint yCoord, GLsizei width, GLsizei height, 
                      GLenum textureFormat, GLenum dataType, GLvoid *pPixels);

   /**
    *  Reads from the current color buffer.
    *
    *  @param   xCoord
    *           Lower left corner X coordinate.
    *
    *  @param   yCoord
    *           Lower left corner Y coordinate.
    *
    *  @param   width
    *           The width of the rectangle of pixels to read.
    *
    *  @param   height
    *           The height of the rectangle of the pixels to read.
    *
    *  @param   pPixels
    *           The pixels to be read out of the color buffer.
    *
    *  @see     write()
    */
   virtual void read(GLint xCoord, GLint yCoord, GLsizei width, GLsizei height,
      GLenum textureFormat, GLenum dataType, GLvoid *pPixels);

private:
   ColorBuffer* mpColorBuffer;
};

#endif
