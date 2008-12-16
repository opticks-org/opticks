/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConvertToBsqPage.h"
#include <string.h>

ConvertToBsqPage::ConvertToBsqPage(unsigned int rows, unsigned int columns, unsigned int bytesPerElement) :
   mCache(rows * columns * bytesPerElement),
   mRows(rows),
   mColumns(columns),
   mBytesPerElement(bytesPerElement)
{
}

ConvertToBsqPage::~ConvertToBsqPage()
{
}

unsigned int ConvertToBsqPage::getNumBands()
{
   return 1;
}

unsigned int ConvertToBsqPage::getNumRows()
{
   return mRows;
}

unsigned int ConvertToBsqPage::getNumColumns()
{
   return mColumns;
}

unsigned int ConvertToBsqPage::getInterlineBytes()
{
   return 0;
}

void* ConvertToBsqPage::getRawData()
{
   return &mCache.front();
}

bool ConvertToBsqPage::feed(unsigned int row, size_t skipBytes, void* pData)
{
   if (row > mRows)
   {
      return false;
   }

   if (skipBytes == mBytesPerElement) // BIL
   {
      unsigned int cachePos = row * mColumns * mBytesPerElement;
      memcpy(&mCache[cachePos], reinterpret_cast<unsigned char*>(pData), mBytesPerElement * mColumns);
   }
   else
   {
      for (unsigned int cachePos = mBytesPerElement * mColumns * row, sourcePos = 0;
         sourcePos < mColumns*(skipBytes); 
         cachePos += mBytesPerElement, sourcePos += skipBytes)
      {
         memcpy(&mCache[cachePos], reinterpret_cast<unsigned char*>(pData) + sourcePos, mBytesPerElement);
      }
   }

   return true;
}
