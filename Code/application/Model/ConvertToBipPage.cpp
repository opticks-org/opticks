/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ConvertToBipPage.h"
#include <string.h>

ConvertToBipPage::ConvertToBipPage(unsigned int rows, unsigned int columns, 
   unsigned int bands, unsigned int bytesPerElement) :
   mCache(rows*columns*bands*bytesPerElement), mRows(rows),
   mColumns(columns), mBands(bands), mBytesPerElement(bytesPerElement),
   mPixelOffset(bands*bytesPerElement)
{
}

ConvertToBipPage::ConvertToBipPage()
{
}

ConvertToBipPage::~ConvertToBipPage(void)
{
}

unsigned int ConvertToBipPage::getNumBands()
{
   return mBands;
}

unsigned int ConvertToBipPage::getNumRows()
{
   return mRows;
}

unsigned int ConvertToBipPage::getNumColumns()
{
   return mColumns;
}

unsigned int ConvertToBipPage::getInterlineBytes()
{
   return 0;
}

void *ConvertToBipPage::getRawData()
{
   return &mCache.front();
}

bool ConvertToBipPage::feed(unsigned int row, unsigned int band, void *pData)
{

   if (band > mBands || row > mRows)
   {
      return false;
   }

   for (unsigned int cachePos = mBytesPerElement * mColumns * row + mBytesPerElement * band, sourcePos = 0;
      sourcePos < mColumns * mBytesPerElement;
      cachePos += mPixelOffset, sourcePos += mBytesPerElement)
   {
      memcpy(&mCache[cachePos], 
         reinterpret_cast<unsigned char*>(pData) + sourcePos, mBytesPerElement);
   }

   return true;
}
