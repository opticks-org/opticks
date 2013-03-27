/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "CachedPage.h"

const DimensionDescriptor CachedPage::CacheUnit::ALL_BANDS = DimensionDescriptor();

CachedPage::CacheUnit::CacheUnit(char* pData, DimensionDescriptor startRow, int concurrentRows, size_t size,
                                 DimensionDescriptor band, unsigned int interlineBytes) :
   mpData(pData),
   mStartRow(startRow),
   mConcurrentRows(concurrentRows),
   mBand(band),
   mSize(size),
   mInterlineBytes(interlineBytes)
{
}

CachedPage::CacheUnit::~CacheUnit()
{
   delete [] mpData;
}

DimensionDescriptor CachedPage::CacheUnit::getBand()
{
   return mBand;
}

bool CachedPage::CacheUnit::matches(DimensionDescriptor startRow, int concurrentRows, DimensionDescriptor band)
{
   if (startRow.getActiveNumber() >= mStartRow.getActiveNumber() && 
      (mBand == band) &&
      startRow.getActiveNumber() + concurrentRows <= mStartRow.getActiveNumber() + mConcurrentRows)
   {
      return true;
   }
   return false;
}

DimensionDescriptor CachedPage::CacheUnit::getStartRow() const
{
   return mStartRow;
}

size_t CachedPage::CacheUnit::getSize() const
{
   return mSize;
}

char *CachedPage::CacheUnit::getRawData()
{
   return mpData;
}

unsigned int CachedPage::CacheUnit::getConcurrentRows()
{
   return mConcurrentRows;
}

unsigned int CachedPage::CacheUnit::getInterlineBytes()
{
   return mInterlineBytes;
}

CachedPage::CachedPage(UnitPtr pCacheUnit, size_t offset, DimensionDescriptor startRow) :
   mpCacheUnit(pCacheUnit),
   mOffset(offset),
   mStartRow(startRow)
{
}

CachedPage::~CachedPage()
{
}

void* CachedPage::getRawData()
{
   return mpCacheUnit->getRawData() + mOffset;
}

unsigned int CachedPage::getNumRows()
{
   unsigned int rowOffset = mStartRow.getActiveNumber() - mpCacheUnit->getStartRow().getActiveNumber();
   return mpCacheUnit->getConcurrentRows() - rowOffset;
}

unsigned int CachedPage::getNumColumns()
{
   return 0;
}

unsigned int CachedPage::getNumBands()
{
   return 0;
}

unsigned int CachedPage::getInterlineBytes()
{
   return mpCacheUnit->getInterlineBytes();
}
