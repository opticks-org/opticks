/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppAssert.h"
#include "AppConfig.h"
#include "DesktopServicesImp.h"
#include "FrameBuffer.h"
#include "GpuResourceManager.h"
#include "ImagePBuffer.h"
#include "ImageUtilities.h"
#include "MessageLogResource.h"
#include "PixelBufferObject.h"

using namespace std;

template<>
GpuResourceManager* Service<GpuResourceManager>::get() const
{
   return DesktopServicesImp::instance()->getGpuResourceManager();
}

GpuResourceManager::~GpuResourceManager()
{
#if defined(WIN_API)
   vector<GLuint>::iterator textureIter = mTextures.begin();
   while (textureIter != mTextures.end())
   {
      // deallocate texture memory for generated texture object id
      glDeleteTextures(1, &(*textureIter));
      ++textureIter;
   }
#endif
}

GpuResourceManager::GpuResourceManager()
{
}

PixelBufferObject *GpuResourceManager::getPixelBufferObject(int numBytes, GLenum accessMode)
{
   #if defined(WIN_API)
   if (glewGetExtension("GL_ARB_pixel_buffer_object"))
   {
      try
      {
         return new PixelBufferObject(numBytes, accessMode);
      }
      catch (AssertException& assertException)
      {
         string assertMessage = assertException.getText();
         MessageResource msg(assertMessage, "app", "D48723A5-E838-479F-BEA0-0FBF4D3A4E18");
      }
   }
#endif
  
   return NULL;
}

ImageBuffer *GpuResourceManager::allocateImageBuffer()
{
   ImageBuffer* pImageBuffer = NULL;
#ifdef WIN_API
   if (glewGetExtension("GL_EXT_framebuffer_object"))
   {
      pImageBuffer = new FrameBuffer();
   }
   else if (wglewGetExtension("WGL_ARB_pixel_format") && wglewGetExtension("WGL_ARB_pbuffer") &&
            wglewGetExtension("WGL_ARB_render_texture") && glewGetExtension("GL_NV_float_buffer"))
   {
      pImageBuffer = new ImagePBuffer();
   }
#endif
   return pImageBuffer;
}

GLuint GpuResourceManager::allocateTexture(GLenum textureTarget, GLint internalFormat, GLsizei width, 
                                           GLsizei height, GLenum textureFormat, GLenum dataType)
{
   GLuint textureObjectId = 0;
#if defined(WIN_API)
   // check format and size of requested texture to see if it can be created
   if (!ImageUtilities::isTextureValid(textureTarget, internalFormat, width, height, textureFormat, dataType) )
   {
      return 0;
   }

   unsigned int numChannels = ImageUtilities::getNumColorChannels(textureFormat);

   // generate texture object id
   glGenTextures(1, &textureObjectId);
   if (textureObjectId != 0)
   {
      // bind texture object id
      glEnable(textureTarget);
      glBindTexture(textureTarget, textureObjectId);

      // Initialize the texture to be empty
      if ((width % 4 == 0) && (height % 4 == 0))
      {
         glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      }
      else
      {
         glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      }

      switch (dataType)
      {
      case GL_UNSIGNED_BYTE:
         {
            vector<unsigned char> data(width*height*numChannels);
            // allocate memory for generated texture object id
            memset(&data[0], 0, (width * height * numChannels * sizeof(unsigned char)));
            glTexImage2D(textureTarget, 0, internalFormat, width, height, 0, textureFormat, dataType, &data[0]);
         }
         break;
      case GL_BYTE:
         {
            vector<char> data(width*height*numChannels);
            // allocate memory for generated texture object id
            memset(&data[0], 0, (width * height * numChannels * sizeof(char)));
            glTexImage2D(textureTarget, 0, internalFormat, width, height, 0, textureFormat, dataType, &data[0]);
         }
         break;
      case GL_UNSIGNED_SHORT:
         {
            vector<unsigned short> data(width*height*numChannels);
            // allocate memory for generated texture object id
            memset(&data[0], 0, (width * height * numChannels * sizeof(unsigned short)));
            glTexImage2D(textureTarget, 0, internalFormat, width, height, 0, textureFormat, dataType, &data[0]);
         }
         break;
      case GL_SHORT:
         {
            vector<short> data(width*height*numChannels);
            // allocate memory for generated texture object id
            memset(&data[0], 0, (width * height * numChannels * sizeof(short)));
            glTexImage2D(textureTarget, 0, internalFormat, width, height, 0, textureFormat, dataType, &data[0]);
         }
         break;
      case GL_UNSIGNED_INT:
         {
            vector<unsigned int> data(width*height*numChannels);
            // allocate memory for generated texture object id
            memset(&data[0], 0, (width * height * numChannels * sizeof(unsigned int)));
            glTexImage2D(textureTarget, 0, internalFormat, width, height, 0, textureFormat, dataType, &data[0]);
         }
         break;
      case GL_INT:
         {
            vector<int> data(width*height*numChannels);
            // allocate memory for generated texture object id
            memset(&data[0], 0, (width * height * numChannels * sizeof(int)));
            glTexImage2D(textureTarget, 0, internalFormat, width, height, 0, textureFormat, dataType, &data[0]);
         }
         break;
      case GL_FLOAT:
         {
            vector<float> data(width*height*numChannels);
            // allocate memory for generated texture object id
            memset(&data[0], 0, (width * height * numChannels * sizeof(float)));
            glTexImage2D(textureTarget, 0, internalFormat, width, height, 0, textureFormat, dataType, &data[0]);
         }
         break;
      default:
         {
            vector<unsigned char> data(width*height*numChannels);
            // allocate memory for generated texture object id
            memset(&data[0], 0, (width * height * numChannels * sizeof(unsigned char)));
            glTexImage2D(textureTarget, 0, internalFormat, width, height, 0, textureFormat, GL_UNSIGNED_BYTE, &data[0]);
         }
         break;
      }

      // unbind texture object id
      glBindTexture(textureTarget, 0);
      glDisable(textureTarget);

      // store texture object id
      mTextures.push_back(textureObjectId);
   }
#endif
   return textureObjectId;
}

void GpuResourceManager::deallocateTexture(GLuint textureId)
{
#if defined(WIN_API)
   vector<GLuint>::iterator textureIter = mTextures.begin();
   while (textureIter != mTextures.end())
   {
      if (*textureIter == textureId)
      {
         // deallocate texture memory for generated texture object id
         glDeleteTextures(1, &textureId);

         // remove texture object id from vector of texture object ids
         mTextures.erase(textureIter);
         break;
      }
      textureIter++;
   }
#endif
}
