/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MEMORYMAPPEDARRAYVIEW_H
#define MEMORYMAPPEDARRAYVIEW_H

#include "AppConfig.h"
#include "TypesFile.h"

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <limits>

#if defined(WIN_API)
#include <windows.h>
#include <direct.h>
#include <math.h>
#include <winbase.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif

class MemoryMappedArrayView
{
public:
   MemoryMappedArrayView(HANDLE_TYPE handle, size_t segmentSize,
                      uint32_t bytesPerElement,
                      uint32_t startIndex, bool readOnly, 
                      unsigned int granularity, int64_t fileSize);

   ~MemoryMappedArrayView();

   void setSegmentSize(size_t segmentSize);

   unsigned char* firstSegment();

   unsigned char* getSegmentByIndex(uint32_t index);
   unsigned char* getSegment(int64_t address);

   unsigned char* nextSegment();

   int64_t ensureGranularity(int64_t suggestedValue);

   int64_t ensureGranularityLower(int64_t suggestedValue);

   int64_t calculateSize(int64_t address, int64_t offset);

   unsigned char *getEndOfSegment() const;

private:
   bool mReadOnly;
   int mAccessPermissions;

   HANDLE_TYPE mHandle;

   uint32_t mBytesPerElement;

   uint32_t mStartIndex;

   unsigned int mSegmentSize;
   unsigned int mRequestedSegmentSize;

   unsigned int mGranularity;

   unsigned char* mpBlock;
   size_t mBlockSize;

   int64_t mAddressOffset;
   int64_t mAddress;

   int64_t mFileSize;
};

#endif
