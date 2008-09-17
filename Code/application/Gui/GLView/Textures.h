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
   bool isAllocated() const { return mHandle != 0; }
   void genTexture(int size);
   void deleteTexture();
   void bind() const { if (mHandle != 0) { glBindTexture(GL_TEXTURE_2D, mHandle); updateTimestamp(); } }
   time_t timestamp() const { return mTimestamp; }
   int getSize() const { return mSize; }
   void incrementRefCount() const { ++mReferenceCount; }
   void decrementRefCount() { --mReferenceCount; if (mReferenceCount == 0) delete this; }
private:
   void updateTimestamp() const;
   static void deleteOldTextures();
   unsigned int mHandle;
   unsigned int mSize;
   mutable time_t mTimestamp;
   mutable unsigned int mReferenceCount;
   static std::vector<TextureImpl*> sAllTextures;
   static int sMaxOldTextureSize;
   static int sTotalSize;
};

class Texture
{
public:
   Texture() : mpImpl(new TextureImpl) {}
   Texture(const Texture& rhs) : mpImpl(rhs.mpImpl) { mpImpl->incrementRefCount(); }
   Texture &operator=(const Texture&rhs) { mpImpl->decrementRefCount(); mpImpl = rhs.mpImpl; mpImpl->incrementRefCount(); return *this;}
   ~Texture() { mpImpl->decrementRefCount(); }
   bool isAllocated() const { return mpImpl->isAllocated(); }
   void genTexture(int size) { mpImpl->genTexture(size); }
   void deleteTexture() { mpImpl->deleteTexture(); }
   void bind() { mpImpl->bind(); }
   time_t timestamp() const { return mpImpl->timestamp(); }
private:
   TextureImpl *mpImpl;
};

#endif
