/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef UHLHEADER_H
#define UHLHEADER_H

#include <stdio.h>

// DL06242003: #define values for UHL Header
const int UHL_SIZE = 3;
const int UHL_SEC_CODE_SIZE = 3;
const int UHL_UNIQUE_REF_SIZE = 12;
const int UHL_RESERVED_SIZE = 24;
const int UHL_PROPER_TOTAL = 56;

// NOTE: YOU MUST DO A TRY .. CATCH(std::string) for the FILE* constructor
class UhlHeader
{
public:
   /*
    * Public constructor - does nothing except initialize all values to 0
    */
   UhlHeader();

   /*
    * Deserialize does all the work for us
    * 
    * @param   pInputFile
    *          Pointer to the file that the UHL Header will be loaded from; uses a pointer
    *          to avoid having to open and reopen files when loading all 3 headers needed
    *          for DTED.
    */
   bool readHeader(FILE* pInputFile);
   int getLatCount();
   int getLongCount();

private:
   char mUhl[UHL_SIZE + 1];
   char mOne;
   float mLatitude;     // note these will be read in as DDDMMSSH, and converted to FLOATS
   float mLongitude;    // note these will be read in as DDDMMSSH, and converted to FLOATS
   float mLatInterval;  // note these are read as DDD.D and must be converted to FLOATS
   float mLongInterval; // note these are read as DDD.D and must be converted to FLOATS
   int mAva; // note these will be read in as chars, but need be read
   char mSec_code[UHL_SEC_CODE_SIZE + 1];
   char mUnique_ref[UHL_UNIQUE_REF_SIZE + 1];
   int mLatCount;
   int mLongCount;
   char mMultipleAccuracy; // will need to be converted to a BOOL eventually
   char mReserved[UHL_RESERVED_SIZE + 1];
};

#endif
