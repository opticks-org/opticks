/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <memory>
#include <string>
#include "AppConfig.h"
#ifdef UNIX_API
#include <strings.h>
#endif

#include "AccHeader.h"

AccHeader::AccHeader() :
   mTotalHeaderSize(0)
{
}

bool AccHeader::readHeader(FILE* pInputFile)
{
   if (pInputFile == NULL)
   {
      return false;
   }

   size_t beforeRead = ftell(pInputFile);
   /* DL06252003: We're not doing anything with this stuff, but it's a good idea to
   save it anyway.
   */

   memset(&mAcc, '\0', ACC_SIZE);

   if (0 == fread(&mAcc, sizeof(char), ACC_SIZE, pInputFile))
   {
      return false;
   }

   mTotalHeaderSize = ftell(pInputFile);
   
   size_t afterRead = ftell(pInputFile);
   if ((afterRead - beforeRead) != ACC_PROPER_TOTAL) // we've read the wrong # of bytes!
   {
      return false;
   }

   // we've succeded and are done
   return true;
}

size_t AccHeader::getTotalHeaderSize()
{
   return mTotalHeaderSize;
}
