/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConfigurationSettings.h"
#include "Textures.h"
#include "glCommon.h"

#include <algorithm>

std::vector<TextureImpl*> TextureImpl::sAllTextures;
uint64_t TextureImpl::sTextureCacheSize = 0;
uint64_t TextureImpl::sTotalSize = 0;
uint64_t TextureImpl::sLastUsedCounter = 0;

TextureImpl::TextureImpl() :
   mHandle(0),
   mSize(0),
   mLastUsed(0),
   mReferenceCount(1)
{}

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
   updateLastUsed();
   return mHandle != 0;
}

void TextureImpl::updateLastUsed() const
{
   ++sLastUsedCounter; //the start value of sLastUsedCounter, 0, is reserved.
   mLastUsed = sLastUsedCounter;
}

uint64_t TextureImpl::getTextureCacheSize()
{
   if (sTextureCacheSize == 0)
   {
      sTextureCacheSize = static_cast<uint64_t>(ConfigurationSettings::getSettingGpuTextureCacheSize()) * 1024 * 1024;
   }
   return sTextureCacheSize;
}

void TextureImpl::downsizeTextureCache()
{
   sTextureCacheSize /= 2;
}

void TextureImpl::genTexture(int size)
{
   static int sGenCount = 0;
   ++sGenCount;
   if (sGenCount % 20 == 0)
   {
      // by modding > 1, we ensure that we don't purge
      // the texture cache to enforce the cache
      // max every time we gen a new texture.  That
      // is a performance optimization.
      // we don't mod by 100 or so, because what ever
      // value we mod by is the amount of textures
      // that could overfill the max texture cache size.
      // 20 was chosen as a good compromise, but could
      // be adjusted as hardware changes.
      deleteOldTextures();
   }
   //call this after deleteOldTextures because doing
   //it before causes it to be the oldest texture
   //in the heap. Putting this here also ensures
   //this texture isn't the reason we go over the
   //cache limit.
   deleteTexture();

   glGenTextures(1, &mHandle); 
   if (mHandle == 0)
   {
      const uint64_t minTextureCache = 100 * 1024 * 1024; //100 MB, no particular reason this was chosen as the min
      deleteOldTextures();
      glGenTextures(1, &mHandle); 
      while (mHandle == 0 && getTextureCacheSize() > minTextureCache)
      {
         downsizeTextureCache();
         deleteOldTextures();
         glGenTextures(1, &mHandle); 
      }
   }

   if (mHandle != 0)
   {
      if (std::find(sAllTextures.begin(), sAllTextures.end(), this) == sAllTextures.end())
      {
         sAllTextures.push_back(this);
      }
      updateLastUsed(); 
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
   mLastUsed = 0;
   mSize = 0;
}

void TextureImpl::bind() const
{
   if (mHandle != 0)
   {
      glBindTexture(GL_TEXTURE_2D, mHandle);
      updateLastUsed();
   }
}

uint64_t TextureImpl::lastUsed() const
{
   return mLastUsed;
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
   return pLhs->lastUsed() > pRhs->lastUsed();
}

void TextureImpl::deleteOldTextures()
{
   uint64_t textureCacheSize = getTextureCacheSize();
   if (sTotalSize > textureCacheSize)
   {
      std::make_heap(sAllTextures.begin(), sAllTextures.end(), TextureImplGr);

      while (sTotalSize > textureCacheSize)
      {
         TextureImpl* pTexture = sAllTextures.front();
         std::pop_heap(sAllTextures.begin(), sAllTextures.end(), TextureImplGr);
         sAllTextures.pop_back();
         pTexture->deleteTexture();
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

void Texture::bind()
{
   mpImpl->bind();
}

uint64_t Texture::lastUsed() const
{
   return mpImpl->lastUsed();
}
