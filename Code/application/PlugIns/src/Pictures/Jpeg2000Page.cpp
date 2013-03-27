/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#if defined (JPEG2000_SUPPORT)

#include "Jpeg2000Page.h"
#include "Jpeg2000Pager.h"

#include <functional>
#include <algorithm>

using namespace std;

Jpeg2000Page::Jpeg2000Page(Jpeg2000Cache::CacheUnit* pCacheUnit, size_t offset, unsigned int rowSkip,
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

Jpeg2000Page::~Jpeg2000Page()
{
   if (mpCacheUnit != NULL)
   {
      mpCacheUnit->release();
   }
}

void *Jpeg2000Page::getRawData()
{
   if (mpCacheUnit == NULL)
   {
      return NULL;
   }
   return mpCacheUnit->data() + mOffset;
}

unsigned int Jpeg2000Page::getNumRows()
{
   return mRowSkip;
}

unsigned int Jpeg2000Page::getNumColumns()
{
   return mColumnSkip;
}

unsigned int Jpeg2000Page::getNumBands()
{
   return mBandSkip;
}

unsigned int Jpeg2000Page::getInterlineBytes()
{
   return 0;
}

unsigned int Jpeg2000Page::getStartBlock() const
{
   return mStartBlock;
}

unsigned int Jpeg2000Page::getEndBlock() const
{
   return mEndBlock;
}

#endif