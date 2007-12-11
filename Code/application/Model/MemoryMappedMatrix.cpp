/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "AppVerify.h"
#include "MemoryMappedMatrix.h"
#include "MemoryMappedMatrixView.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdexcept>
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

using namespace std;

MemoryMappedMatrix::MemoryMappedMatrix(const string &fileName, unsigned int headerOffset,
                        InterleaveFormatType interleave, unsigned int bytesPerElement,
                        unsigned int rowNum, unsigned int columnNum, unsigned int bandNum,
                        unsigned int interLineBytes, unsigned int interBandBytes, bool readOnly)
{
   mInterleave = interleave;
   mBytesPerElement = bytesPerElement;
   mRowNum = rowNum;
   mColumnNum = columnNum;
   mBandNum = bandNum;
   mReadOnly = readOnly;
   mInterLineBytes = interLineBytes;
   mInterBandBytes = interBandBytes;

   mAddressOffset = 0;
   mFileName = fileName;
   mHeaderOffset = headerOffset;

#if defined(WIN_API)
   // All addresses must align on a page boundary.
   SYSTEM_INFO info;
   GetSystemInfo(&info);
   mGranularity = info.dwAllocationGranularity;

   unsigned int openPermissions = GENERIC_READ | GENERIC_WRITE;
   unsigned int mapPermissions = PAGE_READWRITE;

   if(readOnly)
   {
      openPermissions = GENERIC_READ;
      mapPermissions = PAGE_READONLY;
   }

   mFileHandle = CreateFile(mFileName.c_str(),
      openPermissions,
      FILE_SHARE_READ,
      NULL,
      OPEN_ALWAYS, //OPEN_EXISTING,CREATE_ALWAYS
      0,
      NULL);

   mFileSizeLow = GetFileSize(mFileHandle, & mFileSizeHigh);
   if((mFileSizeLow == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
   {
      mFileSizeLow = 0;
      mFileSizeHigh = 0;
   }

   mFileSize = mFileSizeHigh;
   mFileSize *= static_cast<LONG64>(UINT_MAX) + 1;
   mFileSize += mFileSizeLow;

   mHandle = CreateFileMapping(mFileHandle,
      NULL,
      mapPermissions,
      0,
      0,
      NULL); 
       
    if(mHandle == 0) // bad file handle :(
    {
    	throw std::out_of_range("Unable to map file (in MemoryMappedMatrix.cpp)");
    }
       
#else
   unsigned int openPermissions = O_RDWR;

   if(readOnly)
   {
      openPermissions = O_RDONLY;
   }

   mHandle = open(mFileName.c_str(), openPermissions); //|O_CREAT, 0777);
   if(mHandle == -1)
   {
      throw std::out_of_range("Unable to map file (in MemoryMappedMatrix.cpp)");
   }

   struct stat fileStats;
   fstat(mHandle, &fileStats);
   mFileSize = fileStats.st_size;
   mFileSizeLow = mFileSize / numeric_limits<unsigned int>::max();;
   mFileSizeHigh = mFileSize % numeric_limits<unsigned int>::max();;
   mGranularity = fileStats.st_blksize; //65536;
#endif
}

MemoryMappedMatrix::~MemoryMappedMatrix()
{
#if defined(WIN_API)
   CloseHandle(mHandle);
   CloseHandle(mFileHandle);
#else
   close(mHandle);
#endif
}

MemoryMappedMatrixView* MemoryMappedMatrix::getView(size_t defaultSegmentSize)
{
   MemoryMappedMatrixView* pView = new MemoryMappedMatrixView(mHandle, mHeaderOffset, 
                                      defaultSegmentSize, mInterleave,
                                      mBytesPerElement, mRowNum, mColumnNum, mBandNum,
                                      mInterLineBytes, mInterBandBytes, mReadOnly, mGranularity, mFileSize);

   views.insert(pView);
   return pView;
}

void MemoryMappedMatrix::release(MemoryMappedMatrixView* pView)
{
   views.erase(pView);
}
