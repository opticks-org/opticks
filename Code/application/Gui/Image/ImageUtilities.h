/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef IMAGEUTILITIES_H
#define IMAGEUTILITIES_H

#include "glCommon.h"
#include "TypesFile.h"

/**
 * This namespace provides some resources for image processing, such as the number
 * of channels in a texture and the size of an OpenGL type (Ex: GL_UNSIGNED_SHORT).
 */
namespace ImageUtilities
{
   /**
    *  Returns the number of bytes for a particular OpenGL
    *  data type.
    *
    *  @return   The number of bytes of an OpenGL data type.
    */
   unsigned int sizeOf(GLenum dataType);

   /**
    *  Returns the number of channels in the texture object
    *  allocated on the graphics card.
    *
    *  @return   Number of channels eg: (RGB, LUMINANCE).
    */
   unsigned int getNumColorChannels(GLenum textureFormat);

   /**
    *  Converts the application's EncodingType to an OpenGL data type.
    *
    *  @return   OpenGL data enumeration type eg: (GL_UNSIGNED_SHORT).
    */
   GLenum convertEncodingType(EncodingType encodingType);

   /**
    *  Gets the internal format of an OpenGL texture object based
    *  on its texture format.
    *
    *  @return   OpenGL internal format enumeration type eg: (GL_LUMINANCE16).
    */
   GLint getInternalFormat(EncodingType encodingType, GLenum textureFormat);

   GLint getInternalFormat(GLenum dataType, GLenum textureFormat);

   /**
    *  Returns the texture proxy associated with the texture target.
    *
    *  This method retrieves the texture proxy to be used to test whether or not
    *  the texture object can be created with the current size and format.
    *
    *  @param   textureTarget
    *           The target of the texture object to create.
    *
    *  @return  The texture proxy to used in the test.
    */
   GLenum getTextureProxy(GLenum textureTarget);

   /**
    *  Validates whether or not the texture object can be created.
    *
    *  This method returns true if the texture object can be created or returns false if the
    *  texture object cannot be created. It validates the texture object based on the requested 
    *  size and format.
    *
    *    @param   textureTarget
    *             The target of the texture object to create.
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
    *    @return  true if the texture object can be created, false otherwise.
    */
   bool isTextureValid(GLenum textureTarget, GLint internalFormat, GLsizei width, GLsizei height, 
                       GLenum textureFormat, GLenum dataType);
};

#endif
