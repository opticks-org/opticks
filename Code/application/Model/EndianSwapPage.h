/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ENDIANSWAPPAGE_H
#define ENDIANSWAPPAGE_H

#include "RasterPage.h"
#include "TypesFile.h"
#include <vector>

class EndianSwapPage : public RasterPage
{
public:
   /**
    * Create a new EndianSwapPage.
    *
    * This will make a copy of the source data, removing any interline bytes.
    *
    * @param pSrcData
    *        Pointer to the source data. This should contain at least (rows * (bytesPerRow + interlineBytes)) bytes of data.
    * @param encoding
    *        Defined the size of individual data points.
    * @param rows
    *        The number of rows in pSrcData.
    * @param columns
    *        The number of columns in pSrcData.
    * @param bytesPerRow
    *        The number of bytes per row of real data. This does not include interline bytes.
    * @param interlineBytes
    *        The number of bytes between rows in pSrcData. These will not be copied and endian swapped.
    * @param pEndOfSegment
    *        Pointer to the end of the memory segment containing pSrcData. Do not attempt to access past this point.
    *        This is ignored if NULL.
    */
   EndianSwapPage(void* pSrcData, EncodingType encoding, unsigned int rows, unsigned int columns,
      unsigned int bytesPerRow, unsigned int interlineBytes, unsigned char* pEndOfSegment);

   ~EndianSwapPage();

   void* getRawData();
   unsigned int getNumRows();
   unsigned int getNumColumns();
   unsigned int getNumBands();
   unsigned int getInterlineBytes();

private:
   std::vector<char> mData;
   unsigned int mRows;
   unsigned int mColumns;
};

#endif
