/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

// ImageLoader header file
#include "ImageLoader.h"

ImageLoader::ImageLoader(ColorBuffer* pColorBuffer) :
   mpColorBuffer(pColorBuffer)
{
}

ColorBuffer* ImageLoader::getColorBuffer() const
{
   return mpColorBuffer;
}

ImageLoader::~ImageLoader()
{
   if (mpColorBuffer != NULL)
   {
      delete mpColorBuffer;
   }
}

void ImageLoader::loadData(void* pData)
{
   if ((pData == NULL) || (mpColorBuffer == NULL))
   {
      return;
   }

   // Get the texture parameters
   GLuint textureId = mpColorBuffer->getTextureObjectId();
   if (textureId == 0)
   {
      return;
   }

   GLenum textureTarget = mpColorBuffer->getTextureTarget();
   int width = mpColorBuffer->getWidth();
   int height = mpColorBuffer->getHeight();
   GLenum textureFormat = mpColorBuffer->getTextureFormat();
   GLenum dataType = mpColorBuffer->getDataType();

   // Bind texture object to texture target
   glEnable(textureTarget);
   glBindTexture(textureTarget, textureId);

   // load data to texture on graphics card
   // Note: Need to call glTexSubImage2D here to load the data because some graphics cards will
   //       try to deallocate and reallocate memory on the graphics card if glTexImage2D is used
   glTexSubImage2D(textureTarget, 0, 0, 0, width, height, textureFormat, dataType, pData);

   // unbind texture
   glBindTexture(textureTarget, 0);
   glDisable(textureTarget);
}

void ImageLoader::write(GLint xCoord, GLint yCoord, GLsizei width, GLsizei height, 
                        GLenum textureFormat, GLenum dataType, GLvoid *pPixels)
{
   // set up the projection and model view matrices
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   gluOrtho2D(0, width, 0, height);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   // position where to write the data
   glRasterPos2i(xCoord, yCoord);

   // write the pixels to the color buffer
   glDrawPixels(width, height, textureFormat, dataType, pPixels);

   // restore the projection and model view matrices
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}

void ImageLoader::read(GLint xCoord, GLint yCoord, GLsizei width, GLsizei height, GLenum textureFormat,
                       GLenum dataType, GLvoid *pPixels)
{
   // set up the projection and model view matrices
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   gluOrtho2D(0, width, 0, height);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   // read the pixels from the color buffer
   glReadPixels(xCoord, yCoord, width, height, textureFormat, dataType, pPixels);

   // restore the projection and model view matrices
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}
