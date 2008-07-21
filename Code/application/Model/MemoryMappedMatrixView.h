/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef _MEMORYMAPPEDMATRIXVIEW
#define _MEMORYMAPPEDMATRIXVIEW

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

class MemoryMappedMatrixView
{
public:
   MemoryMappedMatrixView(HANDLE_TYPE handle, unsigned int headerOffset, size_t segmentSize,
                      InterleaveFormatType interleave, unsigned int bytesPerElement,
                      unsigned int rowNum, unsigned int columnNum, unsigned int bandNum,
                      unsigned int interLineBytes, unsigned int interBandBytes, bool readOnly, 
                      unsigned int granularity, int64_t fileSize);

   ~MemoryMappedMatrixView();

   unsigned char* getAddress();

   void setSegmentSize(size_t segmentSize);

   unsigned char* firstSegment();

   unsigned char* getSegment(unsigned int row, unsigned int column, unsigned int band);
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

   InterleaveFormatType mInterleave;
   unsigned int mBytesPerElement;

   unsigned int mRowNum;
   unsigned int mColumnNum;
   unsigned int mBandNum;

   unsigned int mMinorSize;
   unsigned int mMiddleSize;
   int64_t mMajorSize;
   unsigned int mInterLineBytes;
   unsigned int mInterBandBytes;

   unsigned int mSegmentSize;
   unsigned int mRequestedSegmentSize;

   unsigned int mGranularity;
   unsigned int currentSegment;

   unsigned char* mpBlock;
   size_t mBlockSize;

   int64_t mRequestedAddress;
   int64_t mAddressOffset;
   int64_t mAddress;
   int64_t mTotalSize;

   int64_t mHeaderOffset;
   int64_t mFileSize;
};

#endif  // _MEMORYMAPPEDMATRIXVIEW
