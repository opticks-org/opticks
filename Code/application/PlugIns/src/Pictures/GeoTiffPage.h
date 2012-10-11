/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOTIFFPAGE_H
#define GEOTIFFPAGE_H

#include "RasterPage.h"

#include <stddef.h>

namespace GeoTiffOnDisk
{
   class CacheUnit;
}

class GeoTiffPage : public RasterPage
{
public:
   GeoTiffPage(GeoTiffOnDisk::CacheUnit* pCacheUnit, size_t offset, unsigned int rowSkip,
      unsigned int columnSkip, unsigned int bandSkip);
   ~GeoTiffPage();

   // RasterPage
   void *getRawData();
   unsigned int getNumRows();
   unsigned int getNumColumns();
   unsigned int getNumBands();
   unsigned int getInterlineBytes();

private:
   GeoTiffOnDisk::CacheUnit* mpCacheUnit;
   size_t mOffset;
   unsigned int mRowSkip;
   unsigned int mColumnSkip;
   unsigned int mBandSkip;
};

#endif
