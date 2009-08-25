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
   time_t timestamp() const;
   int getSize() const;
   void incrementRefCount() const;
   void decrementRefCount();

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
   Texture();
   Texture(const Texture& rhs);
   Texture& operator=(const Texture& rhs);
   ~Texture();

   bool isAllocated() const;
   void genTexture(int size);
   void deleteTexture();
   void bind();
   time_t timestamp() const;

private:
   TextureImpl* mpImpl;
};

#endif
