/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "DataRequest.h"
#include "PageCache.h"
#include "TypesFile.h"

#include <algorithm>
#include <boost/bind.hpp>
#include <sstream>
using namespace std;

PageCache::PageCache(const int maxCacheSize) :
   MAX_CACHE_SIZE(maxCacheSize),
   mCacheSize(0)
{
   initialize(0, 0, 0);
}

PageCache::~PageCache()
{
}

CachedPage::UnitPtr PageCache::getUnit(DataRequest *pOriginalRequest,
   DimensionDescriptor startRow, 
   DimensionDescriptor startBand)
{
   CachedPage::UnitPtr pUnit;

   VERIFYRV(pOriginalRequest != NULL, pUnit);

   InterleaveFormatType requestedFormat = pOriginalRequest->getInterleaveFormat();
   unsigned int concurrentRows = pOriginalRequest->getConcurrentRows();

   DimensionDescriptor band = CachedPage::CacheUnit::ALL_BANDS; // default to all bands
   if (requestedFormat == BSQ)
   {
      band = startBand;
   }
   UnitList ::iterator ppMatchingUnit = find_if (mUnits.begin(), mUnits.end(), 
      boost::bind(&CachedPage::CacheUnit::matches, _1, startRow, concurrentRows, band));

   if (ppMatchingUnit != mUnits.end()) // cache hit
   {
      pUnit = (*ppMatchingUnit);

      // Remove from the list -- it will be re-added to the end in createPage()
      mUnits.erase(ppMatchingUnit);
      mCacheSize -= pUnit->getSize();
   }

   return pUnit;
}

CachedPage *PageCache::createPage(CachedPage::UnitPtr pUnit, InterleaveFormatType requestedFormat,
   DimensionDescriptor startRow, DimensionDescriptor startColumn, DimensionDescriptor startBand)
{
   if (pUnit.get() == NULL)
   {
      return NULL;
   }

   mUnits.push_back(pUnit);
   mCacheSize += pUnit->getSize();
   enforceCacheSize();

   int columnOffset = mColumnCount*(startRow.getActiveNumber()-pUnit->getStartRow().getActiveNumber());
   unsigned int offset = 0;
   if (requestedFormat == BIP)
   {
      columnOffset += startColumn.getActiveNumber();
      offset = mBytesPerBand*(columnOffset*mBandCount + startBand.getActiveNumber());
   }
   else if (requestedFormat == BSQ) // a BSQ row is 1 row of 1 band of data
   {
      columnOffset += startColumn.getActiveNumber();
      offset = mBytesPerBand*columnOffset;
   }
   else if (requestedFormat == BIL)
   {
      columnOffset *= mBandCount; // get to the appropriate row in page
      columnOffset += startBand.getActiveNumber()*mColumnCount + // get to the appropriate band in page
                      startColumn.getActiveNumber(); // get to the appropriate column in page
      offset = mBytesPerBand*columnOffset;
   }
   else
   {
      return NULL;
   }

   return new CachedPage(pUnit, offset, startRow);
}

void PageCache::enforceCacheSize()
{
   while (mCacheSize > MAX_CACHE_SIZE && !mUnits.empty())
   {
      mCacheSize -= mUnits.front()->getSize();
      mUnits.pop_front();
   }
}

void PageCache::initialize(int bytesPerBand, int columnCount, int bandCount)
{
   mBytesPerBand = bytesPerBand;
   mColumnCount = columnCount;
   mBandCount = bandCount;
}
