/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGEPBO_H
#define IMAGEPBO_H

#include "ImageLoader.h"

#include <map>

class PixelBufferObject;

/**
 *  Allows for direct access in downloading image textures to the graphics card.
 *
 *  This class allows for faster texture download times to the graphics card. It
 *  does this by using the two following OpenGL extensions: "GL_ARB_vertex_buffer_object"
 *  and "GL_EXT_pixel_buffer_object". Using these two extensions a block of memory is allocated
 *  on the graphics card that can be directly accessed from a program running on the CPU. 
 *  Traditionally, the graphics driver will first copy the texture data to memory allocated for
 *  the driver on the machine. The driver will then copy the texture data to the graphics card
 *  memory. By using these two extensions one of the slow data copies has been eliminated.
 *
 *  @see     ImageLoader
 */

class ImagePBO : public ImageLoader
{
public:
   /**
    *  Constructor.
    *
    *  @param pColorBuffer
    *         The GL texture which this loader pushes data to.  The ImagePBO takes ownership of this object.
    *  @param pPixelBufferObject
    *         The GL pixel buffer attaches to this ImagePBO. The ImagePBO takes ownership of this object.
    */
   ImagePBO(ColorBuffer *pColorBuffer, PixelBufferObject *pPixelBufferObject);
   
   virtual ~ImagePBO();

   /**
    *  Loads the data from CPU memory to the graphics card memory.
    *
    *  This method copies the pixel data from CPU memory to graphics card memory,
    *  and then does an internal copy on the graphics card to move the pixel data
    *  to texture memory.
    *
    *  @param     pData
    *             The data to be loaded to the graphics card.
    */
   void loadData(void *pData);

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
    *  @param   textureFormat
    *           The format of the color buffer to be read from.
    *
    *  @param   dataType
    *           The type of the data to be read in (eg: GL_FLOAT).
    *
    *  @param   pPixels
    *           The pixels to be read out of the color buffer.
    *
    *  @see     write()
    */
   virtual void read(GLint xCoord, GLint yCoord, GLsizei width, GLsizei height, 
                     GLenum textureFormat, GLenum dataType, GLvoid *pPixels);

private:
   PixelBufferObject* mpPixelBufferObject;
};

#endif
