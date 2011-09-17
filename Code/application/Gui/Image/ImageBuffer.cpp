/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ColorBuffer.h"
#include "ImageBuffer.h"

using namespace std;

ImageBuffer::ImageBuffer()
{
}

ImageBuffer::~ImageBuffer()
{
   ColorBuffer* pColorBuffer = NULL;
   map<ColorBuffer*, GLenum>::iterator colorBufferIter = mAttachedBuffers.begin();
   while (colorBufferIter != mAttachedBuffers.end())
   {
      pColorBuffer = colorBufferIter->first;
      if (pColorBuffer != NULL)
      {
         delete pColorBuffer;
      }
      ++colorBufferIter;
   }
}

bool ImageBuffer::addBuffer(ColorBuffer *pColorBuffer)
{
   if (pColorBuffer != NULL && isBufferAttached(pColorBuffer) == false)
   {
      // get the next available color buffer attachment id
      GLenum attachmentId = getNextColorBufferId();

      // insert into map
      mAttachedBuffers.insert(pair<ColorBuffer*, GLenum>(pColorBuffer, attachmentId));
      return true;
   }

   return false;
}

void ImageBuffer::removeBuffer(ColorBuffer* pColorBuffer)
{
   map<ColorBuffer*, GLenum>::iterator bufferIter = mAttachedBuffers.find(pColorBuffer);
   if (bufferIter != mAttachedBuffers.end())
   {
      mAttachedBuffers.erase(bufferIter);
   }
}

bool ImageBuffer::isBufferAttached(ColorBuffer* pColorBuffer) const
{
   map<ColorBuffer*, GLenum>::const_iterator bufferIter = mAttachedBuffers.find(pColorBuffer);
   return bufferIter != mAttachedBuffers.end();
}

GLenum ImageBuffer::getColorBufferId(ColorBuffer* pColorBuffer) const
{
   map<ColorBuffer*, GLenum>::const_iterator bufferIter = mAttachedBuffers.find(pColorBuffer);
   if (bufferIter != mAttachedBuffers.end())
   {
      return bufferIter->second;
   }

   return 0;
}
