/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef RASTERPAGE_H
#define RASTERPAGE_H

/**
 *  This is the object returned by RasterPager to represent
 *  a page of a RasterElement's data
 *
 *  @see RasterPager
 */
class RasterPage
{
public:
   /** 
    *  Returns a pointer to the raw cube data that was requested
    *  by the RasterPager::getPage() method and that was
    *  loaded from the original data into memory.
    *
    *  @return a pointer to the requested raw cube data.
    */
   virtual void* getRawData() = 0;

   /**
    *  Returns the number of full complete rows of data that are contained
    *  with the memory returned by the getRawData() method.
    *
    *  @return the number of full complete rows
    */
   virtual unsigned int getNumRows() = 0;

   /**
    *  Returns the number of columns that must be skipped in
    *  from the initial pointer returned by getRawData()
    *  in order to access the same column in the second row.
    *
    *  @return the number of columns that must be skipped.
    *          If a value of zero is returned, the RasterElement
    *          will assume that it will require DataDescriptor::getColumnNum()
    *          columns to be skipped in order to access the same
    *          column in the next row.
    */
   virtual unsigned int getNumColumns() = 0;

   /**
    *  Returns the number of bands that must skipped in
    *  order to access the next column in BIP formatted
    *  data, or the same row/column in BIL formatted data.
    *
    *  @return the number of full complete bands.  If a value of
    *          zero is returned, the RasterElement
    *          will assume that it will require DataDescriptor::getBandNum()
    *          bands to be skipped.  If the requested data is
    *          BSQ formatted, a value of zero should be
    *          returned from this method.
    */
   virtual unsigned int getNumBands() = 0;

   /**
    *  Returns the number of inter-line bytes that must
    *  be skipped in order to access the same column
    *  in the next row.
    *
    *  @return the number of inter-line bytes.
    */
   virtual unsigned int getInterlineBytes() = 0;

protected:
   /**
    * This should be destroyed by calling RasterPager::releasePage.
    */
   virtual ~RasterPage() {}
};

#endif
