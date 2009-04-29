/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ImageUtilities.h"

unsigned int ImageUtilities::sizeOf(GLenum dataType)
{
   switch (dataType)
   {
   case(GL_UNSIGNED_BYTE):
   case(GL_BYTE):
      return 1;
      break;
   case(GL_UNSIGNED_SHORT):
   case(GL_SHORT):
      return 2;
      break;
   case(GL_UNSIGNED_INT):
   case(GL_INT):
   case(GL_FLOAT):
      return 4;
      break;
   default:
      return 0;
      break;
   }
}

unsigned int ImageUtilities::getNumColorChannels(GLenum textureFormat)
{
   switch (textureFormat)
   {
   case GL_RED:
   case GL_GREEN:
   case GL_BLUE:
   case GL_LUMINANCE:
      return 1;
      break;
   case GL_LUMINANCE_ALPHA:
      return 2;
      break;
   case GL_RGB:
      return 3;
      break;
   case GL_RGBA:
      return 4;
      break;
   default:
      break;
   }

   return 1;
}

GLenum ImageUtilities::convertEncodingType(EncodingType encodingType)
{
   switch (encodingType)
   {
   case INT1SBYTE:
      return GL_BYTE;
      break;
   case INT1UBYTE:
      return GL_UNSIGNED_BYTE;
      break;
   case INT2SBYTES:
      return GL_SHORT;
      break;
   case INT2UBYTES:
      return GL_UNSIGNED_SHORT;
      break;
   case INT4SBYTES:
      return GL_INT;
      break;
   case INT4UBYTES:
      return GL_UNSIGNED_INT;
      break;
   case FLT4BYTES:
      return GL_FLOAT;
      break;
   case INT4SCOMPLEX:
   case FLT8BYTES:
   case FLT8COMPLEX:
   default:
      return GL_NONE;
      break;
   }
}

GLint ImageUtilities::getInternalFormat(EncodingType encodingType, GLenum textureFormat)
{
   GLint internalFormat = textureFormat;
   switch (encodingType)
   {
   case INT2UBYTES:
   case INT2SBYTES:
      {
         if (textureFormat == GL_LUMINANCE)
         {
            internalFormat = GL_LUMINANCE16;
         }
         else if (textureFormat == GL_LUMINANCE_ALPHA)
         {
            internalFormat = GL_LUMINANCE16_ALPHA16;
         }
         else if (textureFormat == GL_RGB)
         {
            internalFormat = GL_RGB16;
         }
         else if (textureFormat == GL_RGBA)
         {
            internalFormat = GL_RGBA16;
         }
      }
      break;
   case FLT4BYTES:
      {
         if (textureFormat == GL_LUMINANCE)
         {
            internalFormat = GL_FLOAT_R32_NV;
         }
         else if (textureFormat == GL_LUMINANCE_ALPHA)
         {
            internalFormat = GL_FLOAT_RGBA_NV;
         }
         else if (textureFormat == GL_RGB)
         {
            internalFormat = GL_FLOAT_RGB32_NV;
         }
         else if (textureFormat == GL_RGBA)
         {
            internalFormat = GL_FLOAT_RGBA32_NV;
         }
      }
      break;
   default:
      break;
   }

   return internalFormat;
}

GLint ImageUtilities::getInternalFormat(GLenum dataType, GLenum textureFormat)
{
   GLint internalFormat = textureFormat;
   switch (dataType)
   {
   case GL_SHORT:
   case GL_UNSIGNED_SHORT:
      {
         if (textureFormat == GL_LUMINANCE)
         {
            internalFormat = GL_LUMINANCE16;
         }
         else if (textureFormat == GL_LUMINANCE_ALPHA)
         {
            internalFormat = GL_LUMINANCE16_ALPHA16;
         }
         else if (textureFormat == GL_RGB)
         {
            internalFormat = GL_RGB16;
         }
         else if (textureFormat == GL_RGBA)
         {
            internalFormat = GL_RGBA16;
         }
      }
      break;
   case GL_FLOAT:
      {
         if (textureFormat == GL_LUMINANCE)
         {
            internalFormat = GL_FLOAT_R32_NV;
         }
         else if (textureFormat == GL_LUMINANCE_ALPHA)
         {
            internalFormat = GL_FLOAT_RGBA_NV;
         }
         else if (textureFormat == GL_RGB)
         {
            internalFormat = GL_FLOAT_RGB32_NV;
         }
         else if (textureFormat == GL_RGBA)
         {
            internalFormat = GL_FLOAT_RGBA32_NV;
         }
      }
      break;
   default:
      break;
   }

   return internalFormat;
}

GLenum ImageUtilities::getTextureProxy(GLenum textureTarget)
{
   switch (textureTarget)
   {
   case GL_TEXTURE_RECTANGLE_ARB:
      return GL_PROXY_TEXTURE_RECTANGLE_ARB;
      break;
   case GL_TEXTURE_1D:
      return GL_PROXY_TEXTURE_1D;
      break;
   case GL_TEXTURE_2D:
      return GL_PROXY_TEXTURE_2D;
      break;
   case GL_TEXTURE_3D:
      return GL_PROXY_TEXTURE_3D;
      break;
   default:
      return GL_PROXY_TEXTURE_1D;
      break;
   }
}

bool ImageUtilities::isTextureValid(GLenum textureTarget, GLint internalFormat, GLsizei width, GLsizei height, 
                                    GLenum textureFormat, GLenum dataType)
{
   int proxyWidth = 0;
   int proxyHeight = 0;
   int proxyInternalFormat = 0;
   GLenum textureProxy = ImageUtilities::getTextureProxy(textureTarget);

   // test to see if the appropriate texture can be created
   glTexImage2D(textureProxy, 0, internalFormat, width, height, 0, textureFormat, dataType, NULL);
   glGetTexLevelParameteriv(textureProxy, 0, GL_TEXTURE_WIDTH, &proxyWidth);
   glGetTexLevelParameteriv(textureProxy, 0, GL_TEXTURE_HEIGHT, &proxyHeight);
   glGetTexLevelParameteriv(textureProxy, 0, GL_TEXTURE_INTERNAL_FORMAT, &proxyInternalFormat);
   if (proxyWidth == 0 || proxyHeight == 0 || proxyInternalFormat == 0)
   {
      return false;
   }
   else
   {
      return true;
   }
}
