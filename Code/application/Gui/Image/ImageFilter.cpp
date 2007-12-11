/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ImageFilter.h"

#include "AppConfig.h"
#include "AppAssert.h"
#if defined(CG_SUPPORTED)
#include "CgContext.h"
#endif
#include "DynamicObject.h"
#include "GpuProgramDescriptor.h"
#include "ImageBuffer.h"
#include "ImageLoader.h"
#include "ImageUtilities.h"
#include "GpuResourceManager.h"
#include "MessageLogResource.h"

#include <algorithm>
#include <string>
using namespace std;

ImageFilter::ImageFilter(ImageFilterDescriptor *pDescriptor) :
   mpInputColorBuffer(NULL),
   mpResultsBuffer(NULL),
   mpDescriptor(pDescriptor),
   mFreezeFlag(false)
{
   // check for valid image filter descriptor
   REQUIRE(mpDescriptor != NULL);

   // get GPU program descriptors
   const vector<GpuProgramDescriptor*>& gpuProgramDescriptors = mpDescriptor->getGpuPrograms();
   REQUIRE(gpuProgramDescriptors.empty() != true);

   // create GpuProgram objects using the descriptors
   vector<GpuProgramDescriptor*>::const_iterator iter;
   for (iter = gpuProgramDescriptors.begin(); iter != gpuProgramDescriptors.end(); ++iter)
   {
      try
      {
         GpuProgram* pGpuProgram = new GpuProgram(*iter);
         if(pGpuProgram != NULL)
         {
            mGpuPrograms.push_back(pGpuProgram);
         }
      }
      catch(AssertException& assertException)
      {
         string assertMessage = assertException.getText();
         MessageResource msg(assertMessage, "app", "FED1A354-7CB4-4C61-848C-5F576ACC7034");
      }
   }

   // check to make sure GpuProgram objects were created
   REQUIRE(mGpuPrograms.empty() != true);
}

ImageFilter::~ImageFilter()
{
   // delete gpu programs
   for(vector<GpuProgram*>::iterator gpuProgram = mGpuPrograms.begin(); gpuProgram != mGpuPrograms.end(); ++gpuProgram)
   {
      delete *gpuProgram;
   }
   
   // delete color buffers
   for(vector<ImageBuffer*>::iterator imageBuffer = mImageBuffers.begin(); imageBuffer != mImageBuffers.end(); ++imageBuffer)
   {
      delete *imageBuffer;
   }
}

ImageFilterDescriptor::ImageProcessType ImageFilter::getFilterType() const
{
   return mpDescriptor->getType();
}

bool ImageFilter::setImage(ColorBuffer *pInputColorBuffer)
{
   bool success = true;
   if(mpInputColorBuffer != pInputColorBuffer)
   {
      // set the input image texture id
      mpInputColorBuffer = pInputColorBuffer;

      // populate texture filter parameters
      success = populateTextureParameters(pInputColorBuffer);
   }
   return success;
}

ColorBuffer *ImageFilter::applyFilter()
{
   ImageBuffer *pImageBuffer = NULL;
   if (mBuffers.empty() == false)
   {
      size_t colorBufferPos = 0;

      for(vector<GpuProgram*>::iterator gpuProgramIter = mGpuPrograms.begin();
          gpuProgramIter != mGpuPrograms.end(); ++gpuProgramIter)
      {
         ColorBuffer *pRenderBuffer = mBuffers[colorBufferPos];
         if(!mFreezeFlag || (pRenderBuffer != mBuffers.back()))
         {
            try
            {
               pImageBuffer = getImageBuffer(pRenderBuffer);
               if(pImageBuffer == NULL)
               {
                  return NULL;
               }

               // render filter results to results image color buffer
               pImageBuffer->drawToBuffer(pRenderBuffer);

               // bind GPU proram to filter input image
               // binding the image sets whatever parameter values
               // are currently in the GpuProgramDescriptors
               (*gpuProgramIter)->bind();

               int width = mpResultsBuffer->getWidth();
               int height = mpResultsBuffer->getHeight();

               // Disable scissoring to ensure the image is drawn completely in a product
               GLboolean scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
               if (scissorEnabled == GL_TRUE)
               {
                  glDisable(GL_SCISSOR_TEST);
               }

               // render results to color buffer
               glMatrixMode(GL_PROJECTION);
               glPushMatrix();
               glLoadIdentity();
               gluOrtho2D(0, width, 0, height);
               glMatrixMode(GL_MODELVIEW);
               glPushMatrix();
               glLoadIdentity();

               render(width, height);

               glMatrixMode(GL_PROJECTION);
               glPopMatrix();
               glMatrixMode(GL_MODELVIEW);
               glPopMatrix();

               // Restore scissoring
               if (scissorEnabled == GL_TRUE)
               {
                  glEnable(GL_SCISSOR_TEST);
               }

               // disable and unbind GPU program
               (*gpuProgramIter)->disable();
            }
            catch(AssertException& assertException)
            {
               MessageResource msg(assertException.getText(), "app", "522B8F56-5D4F-4ECF-90FF-E04582FC1989");
            }
         }
         ++colorBufferPos;
         if(colorBufferPos == mBuffers.size())
         {
            colorBufferPos = 0;
         }
      }
   }

   // switch back to main color buffer
   if(pImageBuffer != NULL)
   {
      pImageBuffer->drawToBuffer(NULL);
   }

   // return filtered results
   return mpResultsBuffer;
}

void ImageFilter::resetBuffer()
{
   // first color buffer in vector is always the buffer
   // with the filter results
   if(!mBuffers.empty())
   {
      mBuffers.front()->clear();
   }
}

void ImageFilter::freezeBuffer(bool toggle)
{
   mFreezeFlag = toggle;
}

void ImageFilter::render(unsigned int width, unsigned int height)
{
   glBegin(GL_QUADS);

   glTexCoord2f(0.0, 0.0);
   glVertex2f(0.0, 0.0);

   glTexCoord2f(static_cast<GLfloat>(width), 0.0);
   glVertex2f(static_cast<GLfloat>(width), 0.0);

   glTexCoord2f(static_cast<GLfloat>(width), static_cast<GLfloat>(height));
   glVertex2f(static_cast<GLfloat>(width), static_cast<GLfloat>(height));

   glTexCoord2f(0.0, static_cast<GLfloat>(height));
   glVertex2f(0.0, static_cast<GLfloat>(height));

   glEnd();
}

ColorBuffer *ImageFilter::copyColorBuffer(ColorBuffer *pColorBuffer)
{
   if (pColorBuffer == NULL)
   {
      return NULL;
   }

   // get the input color buffer and texture properties in order
   // to allocate a ColorBuffer object
   GLenum textureTarget = pColorBuffer->getTextureTarget();
   GLenum textureFormat = pColorBuffer->getTextureFormat();
   GLenum dataType = GL_FLOAT;

   int width = pColorBuffer->getWidth();
   int height = pColorBuffer->getHeight();
   unsigned int alpha = pColorBuffer->getAlpha();
   unsigned int numChannels = ImageUtilities::getNumColorChannels(textureFormat);

   GLint internalFormat = GL_FLOAT_R32_NV;
   if(numChannels == 2)
   {
      internalFormat = GL_FLOAT_RG32_NV;
   }
   else if(numChannels == 3)
   {
      internalFormat = GL_FLOAT_RGB32_NV;
   }
   else if(numChannels == 4)
   {
      internalFormat = GL_FLOAT_RGBA32_NV;
   }

   ColorBuffer *pNewColorBuffer = NULL;
   if(glewGetExtension("GL_EXT_framebuffer_object"))
   {
      // allocate color buffer
      pNewColorBuffer = new ColorBuffer(textureTarget, internalFormat, width, height, 
                                            textureFormat, dataType, alpha);
   }
#ifdef WIN_API
   else if(wglewGetExtension("WGL_ARB_pixel_format") && wglewGetExtension("WGL_ARB_pbuffer") &&
           wglewGetExtension("WGL_ARB_render_texture") && glewGetExtension("GL_NV_float_buffer"))
   {
      // allocate color buffer
      pNewColorBuffer = new ColorBuffer(width, height, textureFormat, dataType, alpha);
   }
#endif

   if(pNewColorBuffer != NULL)
   {
      // clear color buffer by setting values to zero
      pNewColorBuffer->clear();
   }

   return pNewColorBuffer;
}

bool ImageFilter::populateTextureParameters(ColorBuffer *pInputColorBuffer)
{
   if(pInputColorBuffer == NULL)
   {
      return false;
   }

#if defined(CG_SUPPORTED)
   CgContext *pCgContext = CgContext::instance();
   if(pCgContext == NULL)
   {
      return false;
   }
#endif

   bool success = false;

   vector<GpuProgram*>::iterator gpuProgramIter = mGpuPrograms.begin();
   map<string, ColorBuffer*> colorBuffers;


   // put the input image parameter value into the GPU program descriptor parameter lists
   while(gpuProgramIter != mGpuPrograms.end())
   {
      GpuProgramDescriptor& gpuProgramDescriptor((*gpuProgramIter)->getGpuProgramDescriptor());

      const DynamicObject *pParameters = gpuProgramDescriptor.getParameters();
      if(pParameters == NULL)
      {
         return false;
      }

      string parameterName;
      DataVariant parameterValue;
#if defined(CG_SUPPORTED)
      vector<CGparameter> programParameters = 
         pCgContext->getParameters((*gpuProgramIter)->getProgramId());
      vector<CGparameter>::const_iterator progParameterIter = programParameters.begin();
      while(progParameterIter != programParameters.end())
      {
         parameterName = cgGetParameterName(*progParameterIter);
         parameterValue = pParameters->getAttribute(parameterName);
         if(parameterValue.isValid())
         {
            // get the type of the parameter
            CGtype cgParameterType = cgGetParameterNamedType(*progParameterIter);
            if(cgParameterType == CG_SAMPLERRECT)
            {
               // check to see if color buffer was already created
               map<string, ColorBuffer*>::const_iterator colorBufferIter = 
                  colorBuffers.find(parameterName);
               if(colorBufferIter == colorBuffers.end())
               {
                  if(colorBuffers.empty())
                  {
                     DataVariant value(pInputColorBuffer->getTextureObjectId());
                     if(value.isValid())
                     {
                        gpuProgramDescriptor.setParameter(parameterName, value);
                     }

                     colorBuffers.insert(pair<const string, ColorBuffer*>(parameterName, pInputColorBuffer));

                  }
                  else
                  {
                     ColorBuffer *pColorBuffer = copyColorBuffer(pInputColorBuffer);
                     if(pColorBuffer == NULL)
                     {
                        return false;
                     }

                     success = attachToImageBuffer(pColorBuffer);
                     if(success == false)
                     {
                        delete pColorBuffer;
                        pColorBuffer = NULL;
                        return false;
                     }

                     DataVariant value(pColorBuffer->getTextureObjectId());
                     if(value.isValid())
                     {
                        gpuProgramDescriptor.setParameter(parameterName, value);
                     }

                     colorBuffers.insert(pair<const string, ColorBuffer*>(parameterName, pColorBuffer));

                     mBuffers.push_back(pColorBuffer);  
                  }
               }
               else
               {
                  DataVariant value(colorBufferIter->second->getTextureObjectId());
                  if(value.isValid())
                  {
                     gpuProgramDescriptor.setParameter(parameterName, value);
                  }
               }
            }
         }

         ++progParameterIter;
      }
#endif
      ++gpuProgramIter;
   }
      
   // get the number of off-screen color buffers
   // NOTE: There is one input color buffer.
   size_t numColorBuffers = colorBuffers.size() - 1;
   if(numColorBuffers < mGpuPrograms.size())
   {
      // set which color buffer to render to first
      ColorBuffer *pColorBuffer = copyColorBuffer(pInputColorBuffer);
      if(pColorBuffer == NULL)
      {
         return false;
      }

      success = attachToImageBuffer(pColorBuffer);
      if(success)
      {
         string resultsBufferName = "resultsBuffer";
         colorBuffers.insert(pair<const string, ColorBuffer*>(resultsBufferName, pColorBuffer));

         mBuffers.push_back(pColorBuffer);
      }
      else
      {
         delete pColorBuffer;
         pColorBuffer = NULL;
         return false;
      }
   }
   
   // check the color buffers vector
   if(colorBuffers.empty())
   {
      return false;
   }

   gpuProgramIter = mGpuPrograms.begin();
   while(gpuProgramIter != mGpuPrograms.end())
   {
      (*gpuProgramIter)->setColorBuffers(colorBuffers);
      ++gpuProgramIter;
   }

   if(getFilterType() == ImageFilterDescriptor::FEEDBACK_FILTER)
   {
      reverse(mBuffers.begin(), mBuffers.end());

      mpResultsBuffer = mBuffers.front();
   }
   else
   {
      mpResultsBuffer = mBuffers.back();
   }

   return success;
}

bool ImageFilter::attachToImageBuffer(ColorBuffer *pColorBuffer)
{
   bool success = false;

   // try to attach the new color buffer to one of the image buffers in the vector
   for(vector<ImageBuffer*>::iterator imageBufferIter = mImageBuffers.begin(); imageBufferIter != mImageBuffers.end() && !success;
         ++imageBufferIter)
   {
      success = (*imageBufferIter)->attachBuffer(pColorBuffer);
   }

   // need to get another image buffer
   if(!success)
   {
      // get an image buffer with available color buffer attachment 
      // points from GPU resources
      try
      {
         ImageBuffer *pImageBuffer = Service<GpuResourceManager>()->allocateImageBuffer();
         if(pImageBuffer == NULL)
         {
            return false;
         }

         if(success = pImageBuffer->attachBuffer(pColorBuffer))
         {
            mImageBuffers.push_back(pImageBuffer);
         }
         else
         {
            delete pImageBuffer;
         }
      }
      catch(AssertException& assertException)
      {
         string assertMessage = assertException.getText();
         MessageResource msg(assertMessage, "app", "2D08BEF0-9F7E-4C13-B3FD-012DC8B3070E");
      }
   }

   return success;
}

ImageBuffer* ImageFilter::getImageBuffer(ColorBuffer *pColorBuffer)
{
   vector<ImageBuffer*>::iterator imageBufferIter = mImageBuffers.begin();
   while(imageBufferIter != mImageBuffers.end())
   {
      if((*imageBufferIter)->isBufferAttached(pColorBuffer))
      {
         return *imageBufferIter;
      }
      ++imageBufferIter;
   }

   return NULL;
}
