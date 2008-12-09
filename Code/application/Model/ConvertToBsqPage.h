/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CONVERTTOBSQPAGE_H
#define CONVERTTOBSQPAGE_H

#include "RasterPage.h"

#include <vector>

/**
 * This class works with ConvertToBsqPager
 * to convert BIP or BIL data to BSQ.
 */
class ConvertToBsqPage : public RasterPage
{
public:
   ConvertToBsqPage(unsigned int rows, unsigned int columns, unsigned int bytesPerElement);
   virtual ~ConvertToBsqPage();

   // RsterPage methods
   unsigned int getNumBands();
   unsigned int getNumRows();
   unsigned int getNumColumns();
   unsigned int getInterlineBytes();
   void* getRawData();

   /**
    * Feed the page with data for a row.
    *
    * This method will condense the data in pData around
    * in BSQ format.
    *
    * @param row
    *        The row which the data corresponds to.
    *
    * @param skipBytes
    *        The number of bytes between data elements
    *        in the data.
    *
    * @param pData
    *        Pointer to the (all-band) data to condense.
    *        This must be of length (columns*bytes per pixel*bands)
    *
    * @return True if the operation was a success, false otherwise.
    */
   bool feed(unsigned int row, size_t skipBytes, void* pData);

private:
   std::vector<unsigned char> mCache;

   unsigned int mRows;
   unsigned int mColumns;
   unsigned int mBytesPerElement; //< Number of bytes in a single band of a pixel.
};

#endif
