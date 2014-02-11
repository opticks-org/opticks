/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MEMORYMAPPEDARRAY_H
#define MEMORYMAPPEDARRAY_H

#include "AppConfig.h"
#include "TypesFile.h"

#if defined(WIN_API)
#include <windows.h>
#include <winbase.h>

struct _iobuf;
typedef struct _iobuf FILE;
#endif

#include <string>
#include <set>

class MemoryMappedArrayView;

class MemoryMappedArray
{
public:
   MemoryMappedArray(const std::string &fileName, uint32_t bytesPerElement,
                      uint32_t startIndex, bool readOnly);

   ~MemoryMappedArray();

   MemoryMappedArrayView* getView(size_t defaultSegmentSize);

   void release(MemoryMappedArrayView* pView);

private:
   std::string mFileName;

   unsigned long mFileSizeLow;
   unsigned long mFileSizeHigh;
   int64_t mFileSize;

#if defined(WIN_API)
   HANDLE mFileHandle;
   HANDLE mHandle;
#else
   int mHandle;
#endif

   std::set<MemoryMappedArrayView*> mViews;

   uint32_t mBytesPerElement;
   uint32_t mStartIndex;
   unsigned int mGranularity;
   bool mReadOnly;
};

#endif
