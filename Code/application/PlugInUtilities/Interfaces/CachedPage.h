/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef CACHED_PAGE_H
#define CACHED_PAGE_H

#include "DimensionDescriptor.h"
#include "RasterPage.h"

#include <boost/shared_ptr.hpp>

/**
 * Provides means of using page sharing with the DataAccessor class.
 *
 * Page sharing/caching allows multiple data accessors to have read-only
 * access to the same page. The CachedPage and RasterPage
 * are thread-safe classes.
 */
class CachedPage : public RasterPage
{
public:
   class CacheUnit
   {
   public:
      /**
       * A DimensionDescriptor signifying that the page contains all of the cube's bands.
       */
      static const DimensionDescriptor ALL_BANDS;

      /**
       * Construct a CacheUnit with the given parameters.
       *
       * @param pData
       *        The buffer which has already been populated with the data for the
       *        cache unit.  Must be at least \p size bytes long, and must have
       *        been allocated with new char[n].  The cache unit takes ownership
       *        of this buffer.
       * @param startRow
       *        The starting row for this unit.
       * @param concurrentRows
       *        The number of concurrent rows provided.
       * @param size
       *        The size of the buffer provided in \p pData.
       * @param band
       *        The band provided if BSQ, or ALL_BANDS if all bands are provided.
       * @param interlineBytes
       *        The number of interline bytes within the buffer.
       */
      CacheUnit(char *pData, DimensionDescriptor startRow, int concurrentRows, size_t size, 
         DimensionDescriptor band = ALL_BANDS, unsigned int interlineBytes = 0);

      /**
       * Destroy a CacheUnit.
       */
      ~CacheUnit();

      /**
       * Returns the band for this dataset if interleave type is BSQ; for BIP, returns ALL_BANDS.
       *
       * @return For BSQ data, the band number of the data loaded. For any other interleave type, ALL_BANDS.
       */
      DimensionDescriptor getBand();

      /**
       * A function that determines if a specification of number of rows, concurrent rows, and a
       * band (for BSQ) matches (ie. is contained by) this current page.
       *
       * @param  startRow
       *         The start row of the block that may be requested.
       * @param  concurrentRows
       *         The number of rows needed at any given time.
       * @param  band
       *         For BSQ data, the band number. When called on BIP data, this is assumed to be ALL_BANDS.
       */
      bool matches(DimensionDescriptor startRow, int concurrentRows, DimensionDescriptor band);

      /**
       * Accessor function to private data.
       *
       * @return The start row of this block.
       */
      DimensionDescriptor getStartRow() const;

      /**
       * Accessor function to private data.
       *
       * @return The total size, in bytes, of the entire block.
       */
      size_t getSize() const;

      /**
       * Get the raw data contained in the cache unit.
       *
       * @return The raw data contained in the cache unit.
       */
      char *getRawData();

      /**
       * Get the number of concurrent rows contained in the cache unit.
       *
       * @return The number of concurrent rows contained in the cache unit.
       */
      unsigned int getConcurrentRows();

      /**
       * Get the number of interline bytes contained in the cache unit.
       *
       * @return The number of interline bytes contained in the cache unit.
       */
      unsigned int getInterlineBytes();

   private:
      char *mpData;
      DimensionDescriptor mStartRow;
      int mConcurrentRows;
      DimensionDescriptor mBand; // for BSQ
      size_t mSize;
      unsigned int mInterlineBytes;
   };

   typedef boost::shared_ptr<CacheUnit> UnitPtr;

   /**
    * Creates a CachedPage.
    *
    * @param pCacheUnit
    *        The CacheUnit for this page
    * @param offset
    *        The number of bytes to offset into the block. This does not account for size of the
    *        data type.
    * @param startRow
    *        The start row for this page.
    */
   CachedPage(UnitPtr pCacheUnit, size_t offset, DimensionDescriptor startRow);

   /**
    * Virtual destructor to ensure proper deletion of inherited classes.
    */
   virtual ~CachedPage();

   /**
    * Obligation from base class; returns a pointer to the raw data.
    *
    * @return A pointer to the memory owned by the shared_array.
    */
   void* getRawData();

   /**
    * Accessor to private data.
    *
    * @return The number of concurrent rows in this page.
    */
   unsigned int getNumRows();


   /**
    * Inherited obligations that return 0.
    *
    * @return Returns 0.
    */
   unsigned int getNumColumns();
   
   /**
    * Inherited obligations that return 0.
    *
    * @return Returns 0.
    */
   unsigned int getNumBands();
   
   /**
    * Access the number of interline bytes in the page.
    *
    * @return The number of interline bytes.
    */
   unsigned int getInterlineBytes();

private:
   UnitPtr mpCacheUnit;

   /**
    * The offset into the cache unit.
    *
    * The CachedPager and PageCache use this method to provide better performance for
    * the following case: two data accessors request rows 0-127 and rows 128-255 and a block
    * exists in the PageCache that contains rows 0-255.
    */
   size_t mOffset;

   DimensionDescriptor mStartRow;
};

#endif
