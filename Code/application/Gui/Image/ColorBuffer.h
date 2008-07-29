/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef COLOR_BUFFER_H
#define COLOR_BUFFER_H

#include "glCommon.h"

/**
 *  This class provides specific information about the data being sent to OpenGL texture object.
 *
 *  It provides:
 *    - the OpenGL id of the texture
 *    - the size of the texture
 *    - the texture target (eg: GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB, etc.)
 *    - the texture format (eg: GL_LUMINANCE, GL_RGB, etc.)
 *    - the internal format of the texture (eg: GL_LUMINANCE16, GL_R32_FLOAT_NV, etc.)
 *    - the data type (eg: GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, etc.)
 */
class ColorBuffer
{
public:
   /**
    *    Constructor method that creates a ColorBuffer object to be attached to a FrameBuffer object
    *    or used in the ImageLoader class to load textures to the graphics card.
    *
    *    @param   textureTarget
    *             The target of the texture object. (Ex: GL_TEXTURE_2D)
    *
    *    @param   internalFormat
    *             The internal format of the texture object. (Ex: GL_RGB16)
    *
    *    @param   width
    *             The width of the texture object.
    *
    *    @param   height
    *             The height of the texture object.
    *
    *    @param   textureFormat
    *             The format of the texture object. (Ex: GL_LUMINANCE)
    *
    *    @param   dataType
    *             The type of data the color buffer will contain. (Ex: GL_UNSIGNED_SHORT)
    *
    *    @param   alpha
    *             The alpha value for the buffer which resides between 0 and 255.
    */
   ColorBuffer(GLenum textureTarget, GLint internalFormat, int width, int height, 
                       GLenum textureFormat, GLenum dataType, unsigned int alpha = 255);
   
   /**
    *    Constructor method that creates a ColorBuffer object to be attached to a ImagePBuffer object.
    *    
    *    NOTE: This constructor should not be called in the ImageLoader class since it does not
    *          any texture memory on the graphics card. The ImagePBuffer has an OpenGL pbuffer
    *          which is used as the texture memory.
    *
    *    @param   textureTarget
    *             The target of the texture object. (Ex: GL_TEXTURE_2D)
    *
    *    @param   internalFormat
    *             The internal format of the texture object. (Ex: GL_RGB16)
    *
    *    @param   width
    *             The width of the texture object.
    *
    *    @param   height
    *             The height of the texture object.
    *
    *    @param   textureFormat
    *             The format of the texture object. (Ex: GL_LUMINANCE)
    *
    *    @param   dataType
    *             The type of data the color buffer will contain. (Ex: GL_UNSIGNED_SHORT)
    *
    *    @param   alpha
    *             The alpha value for the buffer which resides between 0 and 255.
    */
   ColorBuffer(int width, int height, GLenum textureFormat, GLenum dataType, unsigned int alpha = 255);

   /**
    *  Destructor for the ColorBuffer.
    */
   virtual ~ColorBuffer();

   /**
    *  Gets the size of the OpenGL texture object.
    *
    *  @return    Size in bytes of the OpenGL texture object.
    */
   int getSize() const;

   /**
    *  Gets the width of the OpenGL texture object.
    *
    *  @return   The width of the texture object.
    */
   int getWidth () const;

   /**
    *  Gets the height of the OpenGL texture object.
    *
    *  @return   The height of the texture object.
    */
   int getHeight() const;

   /**
    *  Gets the alpha value of the OpenGL texture object.
    *
    *  @return   The alpha value of the texture object.
    */
   unsigned int getAlpha() const;

   /**
    *  Gets the texture target of the OpenGL texture object.
    *
    *  @return   The texture target of the texture object.
    */
   GLenum getTextureTarget() const;

   /**
    *  Gets the internal format of the OpenGL texture object.
    *
    *  @return   The internal format of the texture object.
    */
   GLint getInternalFormat() const;

   /**
    *  Gets the format of the OpenGL texture object.
    *
    *  @return   The format of the texture object.
    */
   GLenum getTextureFormat() const;

   /**
    *  Gets the data type of the OpenGL texture object.
    *
    *  @return   The data type of the texture object.
    */
   GLenum getDataType() const;

   /**
    *  Gets the Id of the current OpenGL texture object.
    *
    *  @return   The texture object Id.
    */
   GLuint getTextureObjectId() const;

   bool isTextureResident() const;
   void clear();

private: // member variables
   GLuint mTextureObjectId;      // OpenGL texture object id
   int mWidth;                   // width of the texture object
   int mHeight;                  // height of the texture object
   GLint mInternalFormat;        // internal format of the texture object
   GLenum mTextureFormat;        // external format of the texture object
   GLenum mTextureTarget;        // texture target of the texture object
   GLenum mDataType;             // data type of the texture object
   unsigned int mAlpha;          // alpha channel of the texture object
};

#endif
