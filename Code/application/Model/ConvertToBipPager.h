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
 * It subclasses RasterPager, but is itself not a plug-in
 * and therefore cannot be accessed using PlugInServices.
 *
 * When a page is requested, it gets a DataAccessor for each band
 * of data requested.  The RasterPage is fed the BSQ/BIL data and reformats
 * it to BIP.  There is no attempt at optimization through caching.
 */
class ConvertToBipPager : public RasterPager
{
public:
   ConvertToBipPager(RasterElement *pRaster);

   virtual ~ConvertToBipPager(void);

   // RasterPager methods
   RasterPage *getPage(DataRequest *pOriginalRequest, 
      DimensionDescriptor startRow,
      DimensionDescriptor startColumn,
      DimensionDescriptor startBand);

   void releasePage(RasterPage *pPage);

   int getSupportedRequestVersion() const;

private:
   ConvertToBipPager();

   RasterElement* mpRaster;
   unsigned int mBytesPerElement;
};

#endif
