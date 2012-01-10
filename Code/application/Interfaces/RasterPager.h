/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERPAGER_H
#define RASTERPAGER_H

#include "DimensionDescriptor.h"

class DataRequest;
class RasterPage;

/**
 *  Interface specific to RasterPager plug-ins.
 *
 *  This plug-in interface is used by the RasterElement to
 *  map sensor data from the source into memory so that it can be
 *  accessed from the rest of the application.  Each importer
 *  that supports on disk processing should provide an implementation
 *  of this interface to the RasterElement object representing
 *  the data being loaded by the importer.  This interface has no
 *  set methods because each importer should create it's own
 *  implementation of this interface, meaning that each importer
 *  should pass in any information needed by their implementation
 *  by using any set methods they provide on their implementation
 *  of this interface.  Each importer should actually subclass
 *  RasterPagerShell which provides some default implementation.
 *
 *  An instance of the RasterPager interface must be created by
 *  utilizing PlugInManagerServices::createPlugIn().  This is to ensure
 *  that PlugInManagerServices can reference count the resources currently
 *  be accessed by the Core from within the plugin dll, so that the plugin
 *  dll is not unloaded prematurely.
 *
 *  Each RasterElement instance will have its own unique instance of
 *  the RasterPager interface.  The RasterPager instance will have a lifetime
 *  equal to that of the RasterElement it is associated with.
 *  The RasterPager is required to
 *  return all resources back to the operating system on destruction.
 *  It is not explicitly required to return resources back to the operating
 *  system during the call of its releaseBlock() method.
 *
 *  @see RasterElement
 *  @see RasterPagerShell
 *  @see RasterPage
 */
class RasterPager
{
public:
   /**
    *  This method should return an implementation of the RasterPage
    *  interface that will allow access to an in memory pointer of the
    *  requested data that has been loaded from the original file on disk.
    *
    *  The in memory pointer should point to a section of memory
    *  that adheres to the following constraints:
    *       <ul>
    *         <li>
    *         The memory pointer should point to raw cube data that is
    *         either formatted as specified in the pOriginalRequest parameter.
    *         </li>
    *         <li>
    *         The memory pointer should point to raw cube data where
    *         each pixel value is RasterDataDescriptor::getBytesPerElement() large.
    *         The DataDescriptor object should be retrieved from the
    *         RasterElement that this RasterPager is associated with.
    *         </li>
    *         <li>
    *         The memory pointer should point to raw cube data where
    *         there are only post-line bytes.  If there are post-line
    *         bytes, they should be equal to DatasetParameters::getPostlineBytes()
    *         The DatasetParameters object should be retrieved from the
    *         RasterElement that this RasterPager is associated with.
    *         </li>
    *         <li>
    *         The memory pointer should point to raw cube data that contains
    *         at minimum concurrentRows, concurrentBands, concurrentColumns worth of data
    *         that is directly acccessible in memory.
    *         </li>
    *       </ul>
    *  This method may be called simultaneously by multiple threads and is up to
    *  the implementer of this method to guarantee thread-safety in that case.
    *
    *  @param pOriginalRequest
    *         The request as originally made.  The fields on this object
    *         should be examined to determine if this pager can handle
    *         the request, and how to format it.  Use the other parameters
    *         to this method to determine where to start the RasterPage.
    *  @param startRow
    *         the start row of data that should be loaded from the original
    *         data file on disk into memory.
    *  @param startColumn
    *         the start column of data that should be loaded from the original
    *         data file on disk into memory.
    *  @param startBand
    *         the start band of data that should be loaded from the original
    *         data file on disk into memory.
    *
    *  @return a RasterPage object, that when the getRawData() pointer is called
    *          will return a pointer to the requested cube data.  This RasterPage
    *          object should not be directly deleted, but should be passed to
    *          the releasePage() method below when the RasterPage is no longer needed.
    *          
    *          If the request cannot be fulfilled, return NULL.
    */
   virtual RasterPage* getPage(DataRequest *pOriginalRequest,
      DimensionDescriptor startRow, 
      DimensionDescriptor startColumn, 
      DimensionDescriptor startBand) = 0;

   /**
    *  This method will release the RasterPage* that was requested earlier
    *  via the getPage() method.
    *
    *  This method should only release those
    *  RasterPage* that were returned by the getPage() method of the same
    *  instance of the RasterPager.
    *  This method may be called simultaneously by multiple threads and is up to
    *  the implementer of this method to guarantee thread-safety in that case.
    *
    *  @param pPage
    *         the RasterPage that should be released.
    */
   virtual void releasePage(RasterPage* pPage) = 0;

   /**
    * Get the highest version of DataRequest that this pager supports.
    *
    * RasterPagers can support a variety of conversions from the native data
    * to that request in a DataRequest.  getPage() should be implemented to check for
    * these conversions and return NULL if unsupported.
    *
    * As features are added, additional fields may be added to DataRequest.
    * The defaults for these fields will always be the same as on the RasterElement
    * being accessed.  Since these fields may be added without breaking compatibility with
    * existing RasterPager plug-ins, there will be existing plug-ins which do not know
    * to check these new fields and return NULL if unsupported.
    *
    * Return a value here to state what version of DataRequest is supported.
    * If any higher-version fields are changed from the defaults, the core will
    * assume that the RasterPager is unable to handle them, and the request will not be fulfilled.
    *
    * @return The highest request version supported.
    *
    * @see DataRequest::getRequestVersion()
    */
   virtual int getSupportedRequestVersion() const = 0;

protected:
   /**
    *  Since the RasterPager interface is usually used in conjunction with the
    *  PlugIn and Executable interfaces, this should be destroyed by casting to
    *  the PlugIn interface and calling PlugInManagerServices::destroyPlugIn().
    */
   virtual ~RasterPager() {}

private:
   friend class RasterElementImp;
};

#endif
