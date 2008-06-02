/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef _MEMORYMAPPEDMATRIX
#define _MEMORYMAPPEDMATRIX

#if defined(WIN_API)
#include <windows.h>
#include <winbase.h>

struct _iobuf;
typedef struct _iobuf FILE;
#endif

#include "AppConfig.h"
#include "TypesFile.h"

#include <string>
#include <set>

class MemoryMappedMatrixView;

class MemoryMappedMatrix
{
public:
   MemoryMappedMatrix(const std::string &fileName, unsigned int headerOffset, 
                      InterleaveFormatType interleave, unsigned int bytesPerElement,
                      unsigned int rowNum, unsigned int columnNum, unsigned int bandNum,
                      unsigned int interLineBytes, unsigned int interBandBytes, bool readOnly);

   ~MemoryMappedMatrix();

   MemoryMappedMatrixView* getView(size_t defaultSegmentSize);

   void release(MemoryMappedMatrixView* pView);

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

   std::set<MemoryMappedMatrixView*> views;


   InterleaveFormatType mInterleave;
   unsigned int mBytesPerElement;

   unsigned int mRowNum;
   unsigned int mColumnNum;
   unsigned int mBandNum;

   unsigned int mInterLineBytes;
   unsigned int mInterBandBytes;

   unsigned int mGranularity;
   unsigned int currentSegment;

   unsigned char* mpBlock;
   size_t mBlockSize;

   unsigned int mRequestedAddressLow;
   unsigned int mRequestedAddressHigh;
   unsigned int mAddressOffset;
   unsigned int mAddressLow;
   unsigned int mAddressHigh;

   unsigned int mHeaderOffset;
   bool mReadOnly;
};

#endif  // _MEMORYMAPPEDMATRIX
