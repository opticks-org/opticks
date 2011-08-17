/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"

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

#include "TypesFile.h"
#include "MemoryMappedMatrixView.h"

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <limits>

using namespace std;

MemoryMappedMatrixView::MemoryMappedMatrixView(HANDLE_TYPE handle, unsigned int headerOffset, size_t segmentSize,
                                               InterleaveFormatType interleave, unsigned int bytesPerElement,
                                               unsigned int rowNum, unsigned int columnNum, unsigned int bandNum,
                                               unsigned int interLineBytes, unsigned int interBandBytes, bool readOnly,
                                               unsigned int granularity, int64_t fileSize) :
   mReadOnly(readOnly),
   mHandle(handle),
   mInterleave(interleave),
   mBytesPerElement(bytesPerElement),
   mRowNum(rowNum),
   mColumnNum(columnNum),
   mBandNum(bandNum),
   mInterLineBytes(interLineBytes),
   mInterBandBytes(interBandBytes),
   mSegmentSize(0),
   mRequestedSegmentSize(segmentSize),
   mGranularity(granularity),
   mpBlock(NULL),
   mBlockSize(0),
   mAddressOffset(0),
   mAddress(0),
   mHeaderOffset(headerOffset),
   mFileSize(fileSize)
{
   if (mInterleave == BIP)
   {
      mMinorSize = mBytesPerElement;
      mMiddleSize = mMinorSize * mBandNum;
      mMajorSize = mMiddleSize * mColumnNum + mInterLineBytes;
   }
   else if (mInterleave == BSQ)
   {
      mMinorSize = mBytesPerElement;
      mMiddleSize = mMinorSize * mColumnNum + mInterLineBytes;
      mMajorSize = mMiddleSize * mRowNum + mInterBandBytes;
   }
   else if (mInterleave == BIL)
   {
      mMinorSize = mBytesPerElement;
      mMiddleSize = mMinorSize * mColumnNum;
      mMajorSize = mMiddleSize * mBandNum + mInterLineBytes;
   }

#if defined(WIN_API)
   mAccessPermissions = FILE_MAP_READ | FILE_MAP_WRITE;
   if (readOnly)
   {
      mAccessPermissions = FILE_MAP_READ;
   }
#else
   mAccessPermissions = PROT_WRITE | PROT_READ;
   if (readOnly)
   {
      mAccessPermissions = PROT_READ;
   }
#endif

   mSegmentSize = static_cast<unsigned int>(ensureGranularity(mRequestedSegmentSize));
}

MemoryMappedMatrixView::~MemoryMappedMatrixView()
{
   if (mpBlock != NULL)
   {
#if defined(WIN_API)
      UnmapViewOfFile(mpBlock);
#else
      munmap(reinterpret_cast<char*>(mpBlock), mBlockSize);
#endif
   }
}

unsigned char* MemoryMappedMatrixView::getAddress()
{
   return mpBlock + mHeaderOffset;
}

void MemoryMappedMatrixView::setSegmentSize(size_t segmentSize)
{
   mSegmentSize = static_cast<unsigned int>(ensureGranularity(segmentSize));
   mRequestedSegmentSize = segmentSize;
}

unsigned char* MemoryMappedMatrixView::firstSegment()
{
   return getSegment(mHeaderOffset);
}

unsigned char* MemoryMappedMatrixView::getSegment(unsigned int row, unsigned int column, unsigned int band)
{
   int64_t start = 0;

   if (mInterleave == BIP)
   {
      start = row * mMajorSize;
      start += column * mMiddleSize;
      start += band * mMinorSize;
      start += mHeaderOffset;
   }
   else if (mInterleave == BSQ)
   {
      start = band * mMajorSize;
      start += row * mMiddleSize;
      start += column * mMinorSize;
      start += mHeaderOffset;
   }
   else if (mInterleave == BIL)
   {
      start = row * mMajorSize;
      start += band * mMiddleSize;
      start += column * mMinorSize;
      start += mHeaderOffset;
   }

   return getSegment(start);
}

unsigned char* MemoryMappedMatrixView::getSegment(int64_t address)
{
   if (mpBlock != NULL)
   {
#if defined(WIN_API)
      UnmapViewOfFile(mpBlock);
#else
      munmap(reinterpret_cast<char*>(mpBlock), mBlockSize);
#endif
   }

   mAddress = ensureGranularityLower(address);
   mAddressOffset = address - mAddress;

   mBlockSize = static_cast<size_t>(calculateSize(mAddress, mAddressOffset));
   if (mBlockSize == 0)
   {
      mpBlock = NULL;
      return NULL;
   }

#if defined(WIN_API)
   static const LONG64 MY_INT_MAX = static_cast<LONG64>(UINT_MAX) + 1;
   unsigned int addressHigh = static_cast<unsigned int>(mAddress / (MY_INT_MAX));
   unsigned int addressLow = static_cast<unsigned int>(mAddress % (MY_INT_MAX));

   mpBlock = static_cast<unsigned char*>(MapViewOfFile(mHandle, mAccessPermissions, addressHigh, addressLow,
      mBlockSize));
   if (mpBlock == NULL)
   {
      int error = GetLastError();
      LPVOID lpMsgBuf = NULL;
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
          reinterpret_cast<LPSTR>(&lpMsgBuf), 0, NULL);
      LocalFree(lpMsgBuf);
      return NULL;
   }
#else
   mpBlock = reinterpret_cast<unsigned char*>(mmap(static_cast<caddr_t>(0), mBlockSize,
                  mAccessPermissions, MAP_SHARED, mHandle, mAddress));
   if (mpBlock == reinterpret_cast<void*>(-1))
   {
      // FAILURE THROW AN EXCEPTION
      mpBlock = NULL;
      return NULL;
   }
   if (mpBlock == NULL)
   {
      return NULL;
   }
#endif
   return mpBlock + mAddressOffset;
}

unsigned char* MemoryMappedMatrixView::nextSegment()
{
   return getSegment(mAddress + mRequestedSegmentSize + mAddressOffset);
}

/**
 * adjusts the size of the memory map to ensure that it aligns with the
 * system granularity.  This function is public for testing purposes only
 */
int64_t MemoryMappedMatrixView::ensureGranularity(int64_t suggestedValue)
{
   return (suggestedValue / mGranularity + 1) * mGranularity;
}

// mapSize must also be a multiple of the system granularity
int64_t MemoryMappedMatrixView::ensureGranularityLower(int64_t suggestedValue)
{
   return (suggestedValue / mGranularity) * mGranularity;
}

/**
 * Calculate the size of the segment without going past the end of file.
 *
 * @param   low
 *          The lower part of the 64-bit address.
 * @param   high
 *          The higher part of the 64-bit address.
 * @return  The segment size.
 */
int64_t MemoryMappedMatrixView::calculateSize(int64_t address, int64_t offset)
{
   int64_t difference;

   difference = mFileSize;
   difference -= address;

   if (difference < 0)
   {
      return 0;
   }
   if (difference < (mRequestedSegmentSize + offset))
   {
      return difference;
   }
   return (mRequestedSegmentSize + offset);
}

unsigned char *MemoryMappedMatrixView::getEndOfSegment() const
{
   return (mpBlock == NULL) ? NULL : (mpBlock + mBlockSize);
}
