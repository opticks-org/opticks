/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONVERTTOBIPPAGER_H
#define CONVERTTOBIPPAGER_H

#include "RasterPager.h"

class RasterElement;

/**
 * This class converts BSQ or BIL formatted data to BIP on the fly.
 */
class ConvertToBipPager : public RasterPager
{
public:
   ConvertToBipPager(RasterElement* pRaster);

   virtual ~ConvertToBipPager(void);

   void releasePage(RasterPage* pPage);

   int getSupportedRequestVersion() const;

   RasterPage *getPage(DataRequest* pOriginalRequest,  DimensionDescriptor startRow,
      DimensionDescriptor startColumn, DimensionDescriptor startBand);

private:
   ConvertToBipPager();

   ConvertToBipPager& operator=(const ConvertToBipPager& rhs);

   RasterElement* const mpRaster;
   unsigned int mBytesPerElement;
};

#endif
