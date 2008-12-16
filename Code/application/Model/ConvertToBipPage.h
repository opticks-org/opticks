/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONVERTTOBIPPAGE_H
#define CONVERTTOBIPPAGE_H

#include "RasterPage.h"

#include <vector>

/**
 * This class works with ConvertToBipPager
 * to convert BSQ or BIL data to BIP.
 */
class ConvertToBipPage : public RasterPage
{
public:
   ConvertToBipPage(unsigned int rows, unsigned int columns, unsigned int bands, unsigned int bytesPerElement);
   virtual ~ConvertToBipPage();

   // RasterPage methods
   unsigned int getNumBands();
   unsigned int getNumRows();
   unsigned int getNumColumns();
   unsigned int getInterlineBytes();
   void* getRawData();

   /**
    * Feed the page with data for a band.
    *
    * This method will sprinkle the data in pData around
    * in BIP format.
    *
    * @param row
    *        The row which the data corresponds to.
    *
    * @param band
    *        The band which the data corresponds to.  This
    *        determines the initial offset.
    *
    * @param pData
    *        Pointer to the (single band) data to sprinkle.
    *        This must be of length (columns*bytes per pixel)
    *
    * @return True if the operation was a success, false otherwise.
    */
   bool feed(unsigned int row, unsigned int band, void* pData);

private:
   std::vector<unsigned char> mCache;

   unsigned int mRows;
   unsigned int mColumns;
   unsigned int mBands;
   unsigned int mBytesPerElement;
   unsigned int mPixelOffset;
};

#endif
