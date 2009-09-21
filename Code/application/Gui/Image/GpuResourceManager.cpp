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
#include "ColorBuffer.h"
#include "DesktopServicesImp.h"
#include "FrameBuffer.h"
#include "GpuResourceManager.h"
#include "ImageBuffer.h"
#include "ImageFilter.h"
#include "ImageFilterDescriptor.h"
#include "ImageFilterDescriptorImp.h"
#include "ImageFilterManager.h"
#include "ImageLoader.h"
#include "ImagePBuffer.h"
#include "ImageUtilities.h"
#include "MessageLogResource.h"
#include "PixelBufferObject.h"

#include <memory>
#include <vector>

using namespace std;

template<>
GpuResourceManager* Service<GpuResourceManager>::get() const
{
   return DesktopServicesImp::instance()->getGpuResourceManager();
}

GpuResourceManager::~GpuResourceManager()
{
#if defined(CG_SUPPORTED)
   vector<GLuint>::iterator textureIter = mTextures.begin();
   while (textureIter != mTextures.end())
   {
      // deallocate texture memory for generated texture object id
      glDeleteTextures(1, &(*textureIter));
      ++textureIter;
   }
#endif
}

GpuResourceManager::GpuResourceManager() :
   mGpuScalingFactorInitialized(false),
   mGpuScalingFactor(3.0f)    // value for ForceWare versions 94.22 and earlier
{
}

PixelBufferObject *GpuResourceManager::getPixelBufferObject(int numBytes, GLenum accessMode)
{
#if defined(CG_SUPPORTED)
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
#ifdef CG_SUPPORTED
   if (glewGetExtension("GL_EXT_framebuffer_object"))
   {
      pImageBuffer = new FrameBuffer();
   }
#if defined(WIN_API)
   else if (wglewGetExtension("WGL_ARB_pixel_format") && wglewGetExtension("WGL_ARB_pbuffer") &&
            wglewGetExtension("WGL_ARB_render_texture") && glewGetExtension("GL_NV_float_buffer"))
   {
      pImageBuffer = new ImagePBuffer();
   }
#endif
#endif
   return pImageBuffer;
}

GLuint GpuResourceManager::allocateTexture(GLenum textureTarget, GLint internalFormat, GLsizei width, 
                                           GLsizei height, GLenum textureFormat, GLenum dataType)
{
   GLuint textureObjectId = 0;
#if defined(CG_SUPPORTED)
   // check format and size of requested texture to see if it can be created
   if (!ImageUtilities::isTextureValid(textureTarget, internalFormat, width, height, textureFormat, dataType) )
   {
      return 0;
   }

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

      glTexImage2D(textureTarget, 0, internalFormat, width, height, 0, textureFormat, dataType, NULL);

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
#if defined(CG_SUPPORTED)
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

float GpuResourceManager::getGpuScalingFactor()
{
   if (mGpuScalingFactorInitialized == false)
   {
      mGpuScalingFactorInitialized = determineScalingFactor(mGpuScalingFactor);
   }

   return mGpuScalingFactor;
}

bool GpuResourceManager::determineScalingFactor(float& scalingFactor)
{
   bool factorFound = false;
   scalingFactor = 3.0f;  // value for ForceWare versions 94.22 and earlier

#ifdef CG_SUPPORTED
   unsigned int texWidth(64);
   unsigned int texHeight(64);

   // create color buffer object to be used to load data to the graphics card
   GLenum textureFormat(GL_LUMINANCE_ALPHA);
   GLenum dataType = ImageUtilities::convertEncodingType(FLT4BYTES);
   GLint internalFormat = ImageUtilities::getInternalFormat(dataType, textureFormat);
   unsigned int alpha(255);
   auto_ptr<ColorBuffer> pColorBuffer(new (nothrow) ColorBuffer(GL_TEXTURE_RECTANGLE_ARB, internalFormat, 
      texWidth, texHeight, textureFormat, dataType, alpha));

   if ((pColorBuffer.get() != NULL) && (pColorBuffer->getTextureObjectId() != 0))
   {
      auto_ptr<ImageLoader> pImageLoader(new (nothrow) ImageLoader(pColorBuffer.get()));

      vector<float> testData(texHeight * texWidth * ImageUtilities::getNumColorChannels(textureFormat), alpha);
      for (unsigned int row = 0; row < texHeight; ++row)
      {
         for (unsigned int col = 0; col < texWidth; ++col)
         {
            const unsigned int offset = (row * texWidth + col) * ImageUtilities::getNumColorChannels(textureFormat);
            testData[offset] = static_cast<float>(col + 1);
         }
      }
      void* pData = reinterpret_cast<void*>(&testData.front());
      pImageLoader->loadData(pData);

      Service<ImageFilterManager> pManager;

      // ImageFilterDescriptor's destructor is protected so wrap the imp in auto-ptr since
      // we have to destroy it when finished.
      auto_ptr<ImageFilterDescriptorImp> pFilterDesc(dynamic_cast<ImageFilterDescriptorImp*>(
         pManager->createFilterDescriptor("ByPass")));
      ImageFilter filter(dynamic_cast<ImageFilterDescriptor*>(pFilterDesc.get()));
      filter.setImage(pColorBuffer.release());
      ColorBuffer* pResultsBuffer = filter.applyFilter();

      internalFormat = pResultsBuffer->getInternalFormat();
      textureFormat = pResultsBuffer->getTextureFormat();
      dataType = pResultsBuffer->getDataType();
      alpha = pResultsBuffer->getAlpha();
      int numValues = texWidth * texHeight * ImageUtilities::getNumColorChannels(textureFormat);

      ImageBuffer* pImageBuffer = filter.getImageBuffer(pResultsBuffer);
      pImageBuffer->readFromBuffer(pResultsBuffer);

      auto_ptr<ImageLoader> pImageReader(new (nothrow) ImageLoader(pResultsBuffer));

      vector<float> data(numValues);
      GLvoid* pPixels = reinterpret_cast<GLvoid*>(&data.front());
      pImageReader->read(0, 0, texWidth, texHeight, textureFormat, dataType, pPixels);
      filter.setImage(NULL);
      pImageBuffer->detachBuffer(pResultsBuffer);

      // make sure something was actually read back from filter buffer
      if (data.front() > 0.0f)
      {
         scalingFactor = data.front();
         factorFound = true;
      }
   }
#else
   factorFound = true;
#endif

   return factorFound;
}
