/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef PAGE_CACHE_H
#define PAGE_CACHE_H

#include <list>

#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include "CachedPage.h"
#include "DimensionDescriptor.h"
#include "LocationType.h"

#include "TypesFile.h"

class DataRequest;

/**
 * Provides an LRU cache designed to provide faster access to pages if such
 * a page has already been read.
 *
 * For example, a multi-threaded algorithm could get a DataAccessor to odd
 * and even rows. These two threads would be able to share the same page.
 *
 * When clearing units from the cache, it simply removed the oldest units
 * from the cache.  It is possible that a CachedPage still holds a reference
 * to the released unit.  Since the units are consistently referred to with
 * shared_ptrs, the actual memory will not be released until the last page
 * is destroyed.  This does, however, allow duplicate units -- one that the cache
 * knows about, and one that a lingering CachedPage references.
 */
class PageCache
{
public:
   /**
    * Creates a thread-safe LRU PageCache.
    *
    * @param  maxCacheSize
    *         The maximum size of the cache in bytes.
    */
   PageCache(const int maxCacheSize = 20000000);

   /**
    * Destroys the thread-safe LRU PageCache.
    */
   ~PageCache();

   /**
    * Fetches a unit from the cache.
    *
    * See RasterPager::getPage() for details on the parameters.
    *
    * @return A CacheUnit object containing the startRow, startColumn, and startBand,
    *         and containing and least concurrentRows number of rows, concurrentColumns number
    *         of columns, and concurrentBands number of bands.
    */
   CachedPage::UnitPtr getUnit(DataRequest *pOriginalRequest,
      DimensionDescriptor startRow, 
      DimensionDescriptor startBand);

   /**
    * An STL list of PagePtr.
    *
    * This list is provided in such a fashion so that calling std::list<UnitPtr>::erase()
    * on an iterator will also delete the CacheUnit properly if it is the last
    * reference.
    */
   typedef std::list<CachedPage::UnitPtr> UnitList;

   /**
    * Initializes member variables of the cache & resets hit/miss counts.
    *
    * This must be done after construction of the cache.
    *
    * @param  bytesPerBand
    *         The number of bytes each element takes up. Requires the value
    *         contained by RasterDataDescriptor::getBytesPerElement().
    * @param  columnCount
    *         The number of columns in file on disk.
    * @param  bandCount
    *         The number of bands in the file on disk.
    */
   void initialize(int bytesPerBand, int columnCount, int bandCount);

   /**
    * Create a CachedPage for the given cache unit.
    *
    * @param  pUnit
    *         The unit to create the page for.
    * @param  requestedFormat
    *         The format of the page provided.
    * @param  startRow
    *         The desired row.
    * @param  startColumn
    *         The desired column.
    * @param  startBand
    *         The desired band.
    *
    * @return The created page, or NULL if pUnit is NULL.  The caller
    *         takes ownership over the created page.
    */
   CachedPage *PageCache::createPage(CachedPage::UnitPtr pUnit, InterleaveFormatType requestedFormat,
      DimensionDescriptor startRow, DimensionDescriptor startColumn, DimensionDescriptor startBand);

protected:
   const size_t MAX_CACHE_SIZE;
   UnitList mUnits;
   std::string mFilename;
   size_t mCacheSize;
   int mBytesPerBand, mColumnCount, mBandCount;
   
   void enforceCacheSize();
};

#endif
