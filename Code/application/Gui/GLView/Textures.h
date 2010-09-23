/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef TEXTURES_H
#define TEXTURES_H

#include "glCommon.h"
#include <time.h>
#include <vector>

class TextureImpl
{
public:
   TextureImpl();
   ~TextureImpl();

   bool isAllocated() const;
   void genTexture(int size);
   void deleteTexture();
   void bind() const;
   uint64_t lastUsed() const;
   int getSize() const;
   void incrementRefCount() const;
   void decrementRefCount();

private:
   void updateLastUsed() const;
   static void deleteOldTextures();
   static uint64_t getTextureCacheSize(); 
   static void downsizeTextureCache();
   unsigned int mHandle;
   unsigned int mSize;
   mutable uint64_t mLastUsed;
   mutable unsigned int mReferenceCount;
   static std::vector<TextureImpl*> sAllTextures;
   static uint64_t sTextureCacheSize;
   static uint64_t sTotalSize;
   static uint64_t sLastUsedCounter;
};

class Texture
{
public:
   Texture();
   Texture(const Texture& rhs);
   Texture& operator=(const Texture& rhs);
   ~Texture();

   bool isAllocated() const;
   void genTexture(int size);
   void bind();
   uint64_t lastUsed() const;

private:
   TextureImpl* mpImpl;
};

#endif
