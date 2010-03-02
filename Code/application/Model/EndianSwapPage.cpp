/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "EndianSwapPage.h"
#include "Endian.h"
#include "switchOnEncoding.h"
#include "RasterUtilities.h"

#include <stdlib.h>
#include <string.h>

EndianSwapPage::EndianSwapPage(void* pSrcData, EncodingType encoding, unsigned int rows, unsigned int columns,
                               unsigned int bytesPerRow, unsigned int interlineBytes, unsigned char* pEndOfSegment) :
   mData(rows * bytesPerRow), mRows(rows), mColumns(columns)
{
   if (interlineBytes == 0)
   {
      // if there are no interline bytes, then we can do this more efficiently
      unsigned int count = bytesPerRow * rows;
      if (pEndOfSegment != NULL)
      {
         count = std::min(count, static_cast<unsigned int>(pEndOfSegment - static_cast<unsigned char*>(pSrcData)));
      }
      memcpy(&mData.front(), pSrcData, count);
   }
   else
   {
      unsigned int destOffset = 0;
      unsigned int srcOffset = 0;
      for (unsigned int row = 0; row < rows; row++)
      {
         unsigned char* pStart = static_cast<unsigned char*>(pSrcData) + srcOffset;
         unsigned int count = bytesPerRow;
         if (pEndOfSegment != NULL)
         {
            if (pStart >= pEndOfSegment)
            {
               break;
            }
            count = std::min(count, static_cast<unsigned int>(pEndOfSegment - pStart));
         }
         memcpy(&mData[destOffset], pStart, count);
         if (count < bytesPerRow)
         {
            break;
         }
         destOffset += count;
         srcOffset += count + interlineBytes;
      }
   }

   Endian endian;
   switchOnComplexEncoding(encoding, endian.swapBuffer, &mData.front(),
      mData.size() / RasterUtilities::bytesInEncoding(encoding));
}

EndianSwapPage::~EndianSwapPage()
{
}

void* EndianSwapPage::getRawData()
{
   return &mData.front();
}

unsigned int EndianSwapPage::getNumRows()
{
   return mRows;
}

unsigned int EndianSwapPage::getNumColumns()
{
   return mColumns;
}

unsigned int EndianSwapPage::getNumBands()
{
   return 0;
}

unsigned int EndianSwapPage::getInterlineBytes()
{
   return 0;
}
