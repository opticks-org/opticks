/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppAssert.h"
#include "GpuResourceManager.h"
#include "GpuTile.h"
#include "ImageFilter.h"
#include "ImageUtilities.h"
#include "ImageBuffer.h"
#include "ImagePBO.h"
#include "MessageLogResource.h"
#include "PixelBufferObject.h"
#include "Service.h"
#include "UtilityServices.h"

using namespace std;

GpuTile::GpuTile() :
   mpImageLoader(NULL),
   mpImageReader(NULL),
   mpOutputColorBuffer(NULL),
   mbInitialized(false)
{
}

GpuTile::~GpuTile()
{
   delete mpImageLoader;
   delete mpImageReader;
   for(vector<ImageFilter*>::iterator filter = mFilters.begin(); filter != mFilters.end(); ++filter)
   {
      delete *filter;
   }
}

void GpuTile::setupTile(void *pData, EncodingType encodingType, unsigned int index)
{
   // Set the tile coordinates if necessary
   if (mbInitialized == false)
   {
      GLenum textureFormat = getTexFormat();
      GLenum dataType = ImageUtilities::convertEncodingType(encodingType);
      GLint internalFormat = ImageUtilities::getInternalFormat(encodingType, textureFormat);

      unsigned int alpha = getAlpha();
      LocationType texSize = getTexSize();
      LocationType geomSize = getGeomSize();

      vector<GLfloat> xCoords;
      vector<GLfloat> yCoords;

      xCoords.push_back(static_cast<GLfloat>(-(texSize.mX / 2.0)));
      yCoords.push_back(static_cast<GLfloat>(-(texSize.mY / 2.0)));

      xCoords.push_back(static_cast<GLfloat>(-(texSize.mX / 2.0) + geomSize.mX));
      yCoords.push_back(static_cast<GLfloat>(-(texSize.mY / 2.0)));

      xCoords.push_back(static_cast<GLfloat>(-(texSize.mX / 2.0) + geomSize.mX));
      yCoords.push_back(static_cast<GLfloat>(-(texSize.mY / 2.0) + geomSize.mY));

      xCoords.push_back(static_cast<GLfloat>(-(texSize.mX / 2.0)));
      yCoords.push_back(static_cast<GLfloat>(-(texSize.mY / 2.0) + geomSize.mY));

      setXCoords(xCoords);
      setYCoords(yCoords);

      // create color buffer object to be used to load data to the graphics card
      ColorBuffer *pColorBuffer(new ColorBuffer(GL_TEXTURE_RECTANGLE_ARB, internalFormat, 
                                      static_cast<int>(texSize.mX), static_cast<int>(texSize.mY), 
                                      textureFormat, dataType, alpha));

      if ((pColorBuffer != NULL) && (pColorBuffer->getTextureObjectId() != 0))
      {
         // need to use GpuResourceManager to get the best image loader possible, either
         // the standard, ImageLoader, or ImagePBO, which uses the PBO OpenGL extension to
         // improve texture load time
         Service<GpuResourceManager> pResourceManager;
         int numBytes = static_cast<int>(texSize.mX) * static_cast<int>(texSize.mY) * 
            ImageUtilities::sizeOf(dataType) * ImageUtilities::getNumColorChannels(textureFormat);

         PixelBufferObject *pPixelBufferObject = pResourceManager->getPixelBufferObject(numBytes, GL_WRITE_ONLY);
         if (pPixelBufferObject != NULL)
         {
            mpImageLoader = new ImagePBO(pColorBuffer, pPixelBufferObject);
         }
         else
         {
            mpImageLoader = new ImageLoader(pColorBuffer);
         }

         if (mpImageLoader != NULL)
         {
            mbInitialized = true;
         }
      }
   }

   // Update the input image buffer
   if (mpImageLoader != NULL)
   {
      mpImageLoader->loadData(pData);
   }

   // Run the filters and set the output image buffer
   applyFilters();
}

void GpuTile::draw(CGparameter outputCgTextureParam, GLint textureMode)
{
   const vector<GLfloat>& xCoords = getXCoords();
   const vector<GLfloat>& yCoords = getYCoords();

   if ((xCoords.size() < 4) || (yCoords.size() < 4) || (mpOutputColorBuffer == NULL))
   {
      return;
   }

   GLuint textureId = mpOutputColorBuffer->getTextureObjectId();
   if (textureId == 0)
   {
      return;
   }

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   // Move the tile into it's correct position in the image
   LocationType pos = getPos();
   glTranslatef(static_cast<GLfloat>(pos.mX), static_cast<GLfloat>(pos.mY), 0.0);

   // Set the texture mode
   glEnable(GL_TEXTURE_RECTANGLE_ARB);
   glBindTexture(GL_TEXTURE_RECTANGLE_ARB, textureId);
   glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, static_cast<GLfloat>(textureMode));
   glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, static_cast<GLfloat>(textureMode));
   glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

   // Set the input texture parameter for Cg display program
   cgGLSetTextureParameter(outputCgTextureParam, textureId);
   cgGLEnableTextureParameter(outputCgTextureParam);

   // Draw the texture
   unsigned int alpha = getAlpha();
   LocationType geomSize = getGeomSize();

   GLfloat fsizeX = static_cast<GLfloat>(geomSize.mX);
   GLfloat fsizeY = static_cast<GLfloat>(geomSize.mY);

   glBegin(GL_QUADS);
   glColor4ub(255, 255, 255, alpha);

   glTexCoord2f(0.0, 0.0);
   glVertex3f(xCoords[0], yCoords[0], 0.0);

   glTexCoord2f(fsizeX, 0.0);
   glVertex3f(xCoords[1], yCoords[1], 0.0);

   glTexCoord2f(fsizeX, fsizeY);
   glVertex3f(xCoords[2], yCoords[2], 0.0);

   glTexCoord2f(0.0, fsizeY);
   glVertex3f(xCoords[3], yCoords[3], 0.0);
   glEnd();

   glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
   glDisable(GL_TEXTURE_RECTANGLE_ARB);
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}

ImageFilter *GpuTile::createFilter(ImageFilterDescriptor *pDescriptor)
{
   ImageFilter *pImageFilter = NULL;
   if (hasFilter(pDescriptor) == true)
   {
      return pImageFilter;
   }

   ImageFilterDescriptor::ImageProcessType filterType = pDescriptor->getType();

   try
   {
      switch (filterType)
      {
         case ImageFilterDescriptor::GPU_PROCESS:
            // TODO: create a class for this ImageProcessType
            // NOTE: This type is intended to replace the current
            //       image process steps that are being done in
            //       GpuImage to perform the color lookup and linear stretch.
            //       The other stretch algorithms will then be added.
            break;
         case ImageFilterDescriptor::IMAGE_FILTER:
         case ImageFilterDescriptor::FEEDBACK_FILTER:
            pImageFilter = new ImageFilter(pDescriptor);
            break;
         default:
            break;
      }
   }
   catch (AssertException& assertException)
   {
      string assertMessage = assertException.getText();
      MessageResource msg(assertMessage, "app", "4D6B711C-19EB-43C1-85DB-975DC4F4E092");
   }

   if (pImageFilter != NULL)
   {
      mFilters.push_back(pImageFilter);
   }

   return pImageFilter;
}

ImageFilter *GpuTile::getFilter(ImageFilterDescriptor *pDescriptor) const
{
   if (pDescriptor == NULL)
   {
      return NULL;
   }

   for(vector<ImageFilter*>::const_iterator iter = mFilters.begin(); iter != mFilters.end(); ++iter)
   {
      ImageFilter *pImageFilter = (*iter);
      if(pImageFilter != NULL)
      {
         ImageFilterDescriptor *pImageFilterDescriptor = pImageFilter->getImageFilterDescriptor();
         if (pImageFilterDescriptor == pDescriptor)
         {
            return pImageFilter;
         }
      }
   }

   return NULL;
}

bool GpuTile::hasFilter(ImageFilterDescriptor *pDescriptor) const
{
   ImageFilter *pImageFilter = getFilter(pDescriptor);
   return (pImageFilter != NULL);
}

void GpuTile::destroyFilter(ImageFilterDescriptor *pDescriptor)
{
   if (pDescriptor == NULL)
   {
      return;
   }

   for (vector<ImageFilter*>::iterator iter = mFilters.begin(); iter != mFilters.end(); ++iter)
   {
      ImageFilter* pImageFilter = *iter;
      if (pImageFilter != NULL)
      {
         ImageFilterDescriptor* pImageFilterDescriptor = pImageFilter->getImageFilterDescriptor();
         if (pImageFilterDescriptor == pDescriptor)
         {
            mFilters.erase(iter);
            delete pImageFilter;

            if (mFilters.empty())
            {
               if (mpImageLoader != NULL)
               {
                  mpOutputColorBuffer = mpImageLoader->getColorBuffer();
               }
               else
               {
                  mpOutputColorBuffer = NULL;
               }
            }
            else
            {
               mpOutputColorBuffer = mFilters.back()->getResultsBuffer();
            }

            return;
         }
      }
   }
}

void GpuTile::resetFilter(ImageFilterDescriptor *pDescriptor)
{
   ImageFilter *pImageFilter = getFilter(pDescriptor);
   if (pImageFilter != NULL)
   {
      pImageFilter->resetBuffer();
   }
}

void GpuTile::freezeFilter(ImageFilterDescriptor *pDescriptor, bool toggle)
{
   ImageFilter *pImageFilter = getFilter(pDescriptor);
   if (pImageFilter != NULL)
   {
      pImageFilter->freezeBuffer(toggle);
   }
}

bool GpuTile::getFilterFreezeFlag(ImageFilterDescriptor *pDescriptor) const
{
   ImageFilter *pImageFilter = getFilter(pDescriptor);
   if (pImageFilter != NULL)
   {
      return pImageFilter->isBufferFrozen();
   }
   return false;
}

bool GpuTile::isTextureReady(unsigned int index) const
{
   return mbInitialized;
}

vector<ImageFilterDescriptor*> GpuTile::getFilters() const
{
   vector<ImageFilterDescriptor*> descriptors;
   for(vector<ImageFilter*>::const_iterator filter = mFilters.begin(); filter != mFilters.end(); ++filter)
   {
      descriptors.push_back((*filter)->getImageFilterDescriptor());
   }
   return descriptors;
}

void GpuTile::applyFilters()
{
   // Get the initial image buffer
   ColorBuffer* pColorBuffer = NULL;
   if (mpImageLoader != NULL)
   {
      pColorBuffer = mpImageLoader->getColorBuffer();
   }

   // Apply the filters sequentially
   if (pColorBuffer != NULL)
   {
      for (vector<ImageFilter*>::iterator iter = mFilters.begin(); iter != mFilters.end(); ++iter)
      {
         ImageFilter* pImageFilter = *iter;
         if (pImageFilter != NULL)
         {
            if (pImageFilter->setImage(pColorBuffer))
            {
               pColorBuffer = pImageFilter->applyFilter();
            }
         }
      }
   }

   // Set the resultant output buffer to draw
   mpOutputColorBuffer = pColorBuffer;
}

unsigned int GpuTile::readFilterBuffer(GLint xCoord, GLint yCoord, GLsizei width, GLsizei height, GLvoid *pPixels)
{
   unsigned int numElements = width * height;

   // read back the filtered results buffer

   if (mpOutputColorBuffer != NULL)
   {
      GLint internalFormat = mpOutputColorBuffer->getInternalFormat();
      GLenum textureFormat = mpOutputColorBuffer->getTextureFormat();
      GLenum dataType = mpOutputColorBuffer->getDataType();
      unsigned int alpha = mpOutputColorBuffer->getAlpha();
      
      // switch to the filtered buffers color buffer with the filtered results
      Service<GpuResourceManager> pResourceManager;

      if (mpImageReader == NULL)
      {
         // Initialize the reader to the size of the texture to accommodate all possible reads (i.e. later reads)
         LocationType texSize = getTexSize();
         ColorBuffer *pColorBuffer(new ColorBuffer(GL_TEXTURE_RECTANGLE_ARB, internalFormat, 
                                                   texSize.mX, texSize.mY, textureFormat, dataType, alpha));
         if ((pColorBuffer != NULL) && (pColorBuffer->getTextureObjectId() != 0))
         {
            int numBytes = texSize.mX * texSize.mY * ImageUtilities::sizeOf(dataType) * 
                           ImageUtilities::getNumColorChannels(textureFormat);
            PixelBufferObject *pPixelBufferObject = pResourceManager->getPixelBufferObject(numBytes, GL_READ_ONLY);
            if (pPixelBufferObject != NULL)
            {
               mpImageReader = new ImagePBO(pColorBuffer, pPixelBufferObject);
            }
            else
            {
               mpImageReader = new ImageLoader(pColorBuffer);
            }
         }

         if (mpImageReader == NULL)
         {
            return 0;
         }
      }

      try
      {
         ImageBuffer* pImageBuffer = NULL;
         if (mFilters.empty() == false)
         {
            pImageBuffer = mFilters.back()->getImageBuffer(mpOutputColorBuffer);
         }

         if (pImageBuffer != NULL)
         {
            pImageBuffer->readFromBuffer(mpOutputColorBuffer);

            // Read just the chip size from the reader
            mpImageReader->read(xCoord, yCoord, width, height, textureFormat, dataType, pPixels);
         
            // switch back to the graphics cards framebuffer
            pImageBuffer->readFromBuffer(NULL);
         }
      }
      catch (AssertException& assertException)
      {
         string assertMessage = assertException.getText();
         MessageResource msg(assertMessage, "app", "58719F13-30CF-4739-A1E0-1B296DFB9D92");
      }
   }

   return numElements;
}
