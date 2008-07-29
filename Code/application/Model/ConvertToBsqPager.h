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
 * This class converts BIP/BIL formatted data to BSQ on the fly.
 * It subclasses RasterPager, but is itself not a plug-in
 * and therefore cannot be accessed using PlugInServices.
 *
 * When a page is requested, it gets a DataAccessor for the band
 * of data requested.  The RasterPage is fed the BIP data and reformats
 * it to BSQ.  There is no attempt at optimization through caching.
 */
class ConvertToBsqPager : public RasterPager
{
public:
   ConvertToBsqPager(RasterElement *pRaster);

   virtual ~ConvertToBsqPager(void);

   // RasterPage methods
   RasterPage *getPage(DataRequest *pOriginalRequest, 
      DimensionDescriptor startRow,
      DimensionDescriptor startColumn,
      DimensionDescriptor startBand);

   void releasePage(RasterPage *pPage);

   int getSupportedRequestVersion() const;

private:
   ConvertToBsqPager(void);

   RasterElement *mpRaster;
   unsigned int mBytesPerElement;
   unsigned int mSkipBytes; //< Number of bytes between data in a single band.
};

#endif
