/*
 * The information in this file is
 * Copyright(c) 2010 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONVERTTOBILPAGER_H
#define CONVERTTOBILPAGER_H

#include "RasterPager.h"

class RasterElement;

/**
 * This class converts BSQ or BIP formatted data to BIL on the fly.
 */
class ConvertToBilPager : public RasterPager
{
public:
   ConvertToBilPager(RasterElement* pRaster);

   virtual ~ConvertToBilPager(void);

   void releasePage(RasterPage* pPage);

   int getSupportedRequestVersion() const;

   RasterPage* getPage(DataRequest* pOriginalRequest, DimensionDescriptor startRow,
      DimensionDescriptor startColumn, DimensionDescriptor startBand);

private:
   ConvertToBilPager();

   RasterElement* const mpRaster;
   unsigned int mBytesPerElement;
};

#endif
