/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "Textures.h"
#include "glCommon.h"

#include <algorithm>

std::vector<TextureImpl*> TextureImpl::sAllTextures;
int TextureImpl::sMaxOldTextureSize = 200000000;
int TextureImpl::sTotalSize = 0;

TextureImpl::TextureImpl() :
   mHandle(0),
   mSize(0),
   mTimestamp(0),
   mReferenceCount(1)
{
   sAllTextures.push_back(this);
}

TextureImpl::~TextureImpl()
{
   deleteTexture();
   std::vector<TextureImpl*>::iterator it = std::find(sAllTextures.begin(), sAllTextures.end(), this);
   if (it != sAllTextures.end())
   {
      sAllTextures.erase(it);
   }
}

bool TextureImpl::isAllocated() const
{
   return mHandle != 0;
}

void TextureImpl::updateTimestamp() const
{
   mTimestamp = time(NULL);
}

void TextureImpl::genTexture(int size)
{
   static int sGenCount = 0;
   deleteTexture(); 

   sGenCount++;
   if (sGenCount % 20)
   {
      deleteOldTextures();
   }

   while (mHandle == 0 && sMaxOldTextureSize > 100000)
   {
      glGenTextures(1, &mHandle); 
      if (mHandle == 0)
      {
         sMaxOldTextureSize /= 2;
         deleteOldTextures();
      }
   }

   if (mHandle != 0)
   {
      updateTimestamp(); 
      mSize = size;
      sTotalSize += size;
   }
}

void TextureImpl::deleteTexture()
{
   if (mHandle != 0) 
   {
      glDeleteTextures(1, &mHandle);
      sTotalSize -= mSize;
   }
   mHandle = 0; 
   mTimestamp = 0;
   mSize = 0;
}

void TextureImpl::bind() const
{
   if (mHandle != 0)
   {
      glBindTexture(GL_TEXTURE_2D, mHandle);
      updateTimestamp();
   }
}

time_t TextureImpl::timestamp() const
{
   return mTimestamp;
}

int TextureImpl::getSize() const
{
   return mSize;
}

void TextureImpl::incrementRefCount() const
{
   ++mReferenceCount;
}

void TextureImpl::decrementRefCount()
{
   --mReferenceCount;
   if (mReferenceCount == 0)
   {
      delete this;
   }
}

bool TextureImplGr(const TextureImpl *pLhs, const TextureImpl *pRhs)
{
   return pLhs->timestamp() > pRhs->timestamp();
}

void TextureImpl::deleteOldTextures()
{
   int sum = 0;

   if (sTotalSize > sMaxOldTextureSize)
   {
      std::sort(sAllTextures.begin(), sAllTextures.end(), TextureImplGr);

      std::vector<TextureImpl*>::iterator it;
      for (it = sAllTextures.begin(); it != sAllTextures.end(); ++it)
      {
         sum += (*it)->getSize();
         if (sum > sMaxOldTextureSize)
         {
            break;
         }
      }

      for (; it!= sAllTextures.end(); ++it)
      {
         (*it)->deleteTexture();
      }
   }
}

Texture::Texture() :
   mpImpl(new TextureImpl)
{
}

Texture::Texture(const Texture& rhs) :
   mpImpl(rhs.mpImpl)
{
   mpImpl->incrementRefCount();
}

Texture& Texture::operator=(const Texture& rhs)
{
   mpImpl->decrementRefCount();
   mpImpl = rhs.mpImpl;
   mpImpl->incrementRefCount();
   return *this;
}

Texture::~Texture()
{
   mpImpl->decrementRefCount();
}

bool Texture::isAllocated() const
{
   return mpImpl->isAllocated();
}

void Texture::genTexture(int size)
{
   mpImpl->genTexture(size);
}

void Texture::deleteTexture()
{
   mpImpl->deleteTexture();
}

void Texture::bind()
{
   mpImpl->bind();
}

time_t Texture::timestamp() const
{
   return mpImpl->timestamp();
}
