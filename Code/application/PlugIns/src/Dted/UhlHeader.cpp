/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "UhlHeader.h"
#include "DtedShared.h"

#include <memory>
#include <string.h>

UhlHeader::UhlHeader() :
   mOne(0),
   mLatitude(0),
   mLongitude(0),
   mLatInterval(0),
   mLongInterval(0),
   mAva(0),
   mLatCount(0),
   mLongCount(0),
   mMultipleAccuracy(0)
{
   memset(mUhl, '\0', UHL_SIZE);
   memset(&mSec_code[0], '\0', UHL_SEC_CODE_SIZE + 1);
   memset(&mUnique_ref, '\0', UHL_UNIQUE_REF_SIZE + 1);
   memset(&mReserved, '\0', UHL_RESERVED_SIZE + 1);
}

bool UhlHeader::readHeader(FILE* pInputFile)
{
   if (pInputFile == NULL)
   {
      return false;
   }

   // DL06232003: throw an exception if something doesn't read right!
   size_t beforeRead = ftell(pInputFile);

   if (0 == fread(&mUhl, sizeof(char), UHL_SIZE, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mOne, sizeof(char), 1, pInputFile))
   {
      return false;
   }

   if (!readLatLong(pInputFile, mLongitude, false))
   {
      return false;
   }

   if (!readLatLong(pInputFile, mLatitude, false))
   {
      return false;
   }

   if (!readLatLongInterval(pInputFile, mLatInterval))
   {
      return false;
   }

   if (!readLatLongInterval(pInputFile, mLongInterval))
   {
      return false;
   }

   if (!readLatLongCount(pInputFile, mAva))
   {
      return false; // use readLatLongCount to get in 4 byte char* -> int
   }

   if (0 == fread(&mSec_code, sizeof(char), UHL_SEC_CODE_SIZE, pInputFile))
   {
      return false;
   }

   if (0 == fread(&mUnique_ref, sizeof(char), UHL_UNIQUE_REF_SIZE, pInputFile))
   {
      return false;
   }

   if (0 == readLatLongCount(pInputFile, mLatCount))
   {
      return false;
   }

   if (0 == readLatLongCount(pInputFile, mLongCount))
   {
      return false;
   }

   if (0 == fread(&mMultipleAccuracy, sizeof(char), 1, pInputFile))
   {
      return false;
   }

   size_t afterRead = ftell(pInputFile);

   // DL06242003: since we're not reading the "reserved size," we need to seek to the next part of the file
   if (0 != fseek(pInputFile, afterRead + UHL_RESERVED_SIZE, SEEK_SET))
   {
      return false; // bad seek -> kick out
   }

   if ((afterRead - beforeRead) != UHL_PROPER_TOTAL)
   {
      return false; // mLatCountwe've read the wrong # of bytes! -> kick out
   }

   // what a relief, we're done and succeeded
   return true;
}

int UhlHeader::getLatCount()
{
   return mLatCount;
}

int UhlHeader::getLongCount()
{
   return mLongCount;
}
