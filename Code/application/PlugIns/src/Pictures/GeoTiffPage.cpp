/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GeoTiffPage.h"
#include "GeoTiffPager.h"

#include <functional>
#include <algorithm>

using namespace std;

GeoTiffPage::GeoTiffPage(GeoTiffOnDisk::CacheUnit* pCacheUnit, size_t offset, unsigned int rowSkip,
                         unsigned int columnSkip, unsigned int bandSkip) :
   mpCacheUnit(pCacheUnit),
   mStartBlock(0),
   mEndBlock(0),
   mOffset(offset),
   mRowSkip(rowSkip),
   mColumnSkip(columnSkip),
   mBandSkip(bandSkip)
{
   if (pCacheUnit != NULL)
   {
      mStartBlock = *min_element(pCacheUnit->blockNumbers().begin(), pCacheUnit->blockNumbers().end());
      mEndBlock = *max_element(pCacheUnit->blockNumbers().begin(), pCacheUnit->blockNumbers().end());
   }
}

GeoTiffPage::~GeoTiffPage()
{
   if (mpCacheUnit != NULL)
   {
      mpCacheUnit->release();
   }
}

void *GeoTiffPage::getRawData()
{
   if (mpCacheUnit == NULL)
   {
      return NULL;
   }
   return mpCacheUnit->data() + mOffset;
}

unsigned int GeoTiffPage::getNumRows()
{
   return mRowSkip;
}

unsigned int GeoTiffPage::getNumColumns()
{
   return mColumnSkip;
}

unsigned int GeoTiffPage::getNumBands()
{
   return mBandSkip;
}

unsigned int GeoTiffPage::getInterlineBytes()
{
   return 0;
}

unsigned int GeoTiffPage::getStartBlock() const
{
   return mStartBlock;
}

unsigned int GeoTiffPage::getEndBlock() const
{
   return mEndBlock;
}
