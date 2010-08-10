/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef JPEG2000PAGE_H
#define JPEG2000PAGE_H
#include "AppConfig.h"
#if defined (JPEG2000_SUPPORT)

#include "RasterPage.h"

#include <stddef.h>

namespace Jpeg2000Cache
{
   class CacheUnit;
}

class Jpeg2000Page : public RasterPage
{
public:
   Jpeg2000Page(Jpeg2000Cache::CacheUnit* pCacheUnit, size_t offset, unsigned int rowSkip,
      unsigned int columnSkip, unsigned int bandSkip);
   ~Jpeg2000Page();

   // RasterPage
   void *getRawData();
   unsigned int getNumRows();
   unsigned int getNumColumns();
   unsigned int getNumBands();
   unsigned int getInterlineBytes();

   unsigned int getStartBlock() const;
   unsigned int getEndBlock() const;

private:
   Jpeg2000Cache::CacheUnit* mpCacheUnit;
   unsigned int mStartBlock;
   unsigned int mEndBlock;
   size_t mOffset;
   unsigned int mRowSkip;
   unsigned int mColumnSkip;
   unsigned int mBandSkip;
};

#endif
#endif