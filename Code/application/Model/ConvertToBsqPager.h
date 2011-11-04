/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONVERTTOBSQPAGER_H
#define CONVERTTOBSQPAGER_H

#include "RasterPager.h"

class RasterElement;

/**
 * This class converts BIP or BIL formatted data to BSQ on the fly.
 */
class ConvertToBsqPager : public RasterPager
{
public:
   ConvertToBsqPager(RasterElement* pRaster);

   virtual ~ConvertToBsqPager(void);

   void releasePage(RasterPage* pPage);

   int getSupportedRequestVersion() const;

   RasterPage* getPage(DataRequest* pOriginalRequest, DimensionDescriptor startRow,
      DimensionDescriptor startColumn, DimensionDescriptor startBand);

private:
   ConvertToBsqPager();

   ConvertToBsqPager& operator=(const ConvertToBsqPager& rhs);

   RasterElement* const mpRaster;
   unsigned int mBytesPerElement;
};

#endif
