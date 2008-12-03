/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef CACHEDPAGER_H
#define CACHEDPAGER_H

#include <string>

#include "CachedPage.h"
#include "PageCache.h"
#include "RasterPagerShell.h"
#include "RasterPage.h"

#include <memory>

class RasterDataDescriptor;
class RasterElement;
namespace mta
{
   class DMutex;
}

/**
 *  This class represents provides cached access to pages.
 *
 *  By 'cached', we mean that a block may be indexed into
 *  by multiple data accessors (ie. multithreading an algorithm
 *  to function with 2 threads, each reading odd and even rows).
 *  developers would take this class and extend it to support their 
 *  algorithm specific code.
 */
class CachedPager : public RasterPagerShell
{
public:
   /**
    * The name to use for the raster element argument.
    *
    * This argument should be populated with the RasterElement
    * that this object will page.  Arguments 
    * with this name should be of the type RasterElement.
    */
   static std::string PagedElementArg()
   {
      return "Paged Element";
   }

   /**
    * The name to use for the filename argument.
    *
    * This argument should be populated with the Filename that
    * this object will page.  Arguments with this name should 
    * be of the type Filename.
    */
   static std::string PagedFilenameArg()
   {
      return "Paged Filename";
   }

   /**
    * Creates a CachedPager PlugIn.
    *
    * Sets cache size to 10 MB. Sets writable flag to false.
    *
    * Subclasses need to override private pure virtual methods to
    * open the file and get a block from that file.
    */
   CachedPager();

   /**
    * Creates a CachedPager PlugIn.
    *
    * Sets cache size to cacheSize bytes. Sets writable flag to false.
    *
    * Subclasses need to override private pure virtual methods to
    * open the file and get a block from that file.
    *
    * @param cacheSize
    *        Number of bytes in the page cache.
    */
   CachedPager(const size_t cacheSize);

   /**
    * Destructor
    */
   ~CachedPager();
   
   /**
    *  Get Plug-In Input Specification.
    *
    *  The getInputSpecification() method is used by the
    *  Plug-In Manager to determine the input parameters
    *  to generically execute the Plug-In.
    *
    *  @param   pArgList
    *           Returns a pointer to a %PlugInArgList specifying the 
    *           the Plug-In input parameters.
    *
    *  @return  This method returns true if the input parameter
    *           argument list was successfully created.
    */
   bool getInputSpecification(PlugInArgList *&pArgList);

   /**
    *  Get Plug-In Output Specification.
    *
    *  The getOutputSpecification() method is used by the
    *  Plug-In Manager to determine the output parameters
    *  of the generically executed the Plug-In.
    *
    *  @param   pArgList
    *           Returns a pointer to a %PlugInArgList specifying the 
    *           the Plug-In output parameters.
    *
    *  @return  This method returns true if the output parameter
    *           argument list was successfully created.
    */
   bool getOutputSpecification(PlugInArgList *&pArgList);


   /**
    *  Executes the plug-in.
    *
    *  @param   pInputArgList
    *           On input, pInputArgList contains a complete input
    *           argument list for the plug-in.  On return, this
    *           argument list may be updated to reflect changes made
    *           by the plug-in.
    *  @param   pOutputArgList
    *           On input, pOutputArgList contains a complete output
    *           argument list for the plug-in, although actual 
    *           values and default values will be ignored.  On return,
    *           this argument list will be updated to indicate all
    *           output parameters made by the plug-in.
    *
    *  @return  True if the execution was successful.  False is
    *           returned if the user cancelled the plug-in while in
    *           interactive mode.
    */
   bool execute(PlugInArgList *pInputArgList, PlugInArgList *pOutputArgList);

   /**
    * Parses %PlugInArgList pInputArgList. 
    *
    * Assigns values from the input argument list to member variables for
    * use during execute.
    *
    * @param    pInputArgList
    *           The input argument list to parse. Should not be NULL.
    *
    * @return   TRUE if the operation succeeds; FALSE if pInputArgList is NULL
    *           or if the operation fails.
    */
   virtual bool parseInputArgs(PlugInArgList *pInputArgList);

   /**
    *  This method should return a CachedPage (which inherits RasterPage)
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
    *  the implementor of this method to guarantee thread-safety in that case.
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
   RasterPage* getPage(DataRequest *pOriginalRequest,
      DimensionDescriptor startRow, 
      DimensionDescriptor startColumn, 
      DimensionDescriptor startBand);

   /**
    *  This method will release the RasterPage* that was requested earlier
    *  via the getPage() method.
    *
    *  NOTE: This method will check to ensure that the RasterPage is a CachedPage
    *        prior to removal and deletion.
    *
    *  This method should only release those
    *  RasterPage* that were returned by the getPage() method of the same
    *  instance of the RasterPager.
    *  This method may be called simultaneously by multiple threads and is up to
    *  the implementor of this method to guarantee thread-safety in that case.
    *
    *  @param  pPage
    *          the RasterPage that should be released.
    */
   void releasePage(RasterPage *pPage);

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
   int getSupportedRequestVersion() const;
   
protected:
   /**
    *  Accessor function for subclasses to gain access to private member variables.
    *
    *  @return The number of bytes in a single element of data.
    *          For a 200 row x 100 column x 10 band x INT2UBYTES dataset, this
    *          function would return 2.
    */
   const int getBytesPerBand() const;

   /**
    *  Accessor function for subclasses to gain access to private member variables.
    *
    *  @return The number of columns in the dataset.
    */
   const int getColumnCount() const;

   /**
    *  Accessor function for subclasses to gain access to private member variables.
    *
    *  @return The number of bands in the dataset.
    */
   const int getBandCount() const;

   /**
    *  Accessor function for subclasses to gain access to private member variables.
    *
    *  @return A pointer to the RasterElement.
    */
   const RasterElement* getRasterElement() const;

   /**
    *  Returns a reasonable chunk size.
    *
    *  Reasonable chunk sizes are important in keeping performance high, since reading row
    *  by row could be as small as 16KB at a time (ie 2 bytes x 1024 columns x 8 bands) and
    *  would not optimize for IO. Instead, the CachedPager uses chunk sizes to
    *  read in X MB of whole rows (including bands if BIP).
    *
    *  @return  A reasonable chunk size, in bytes. Default implementation returns 1048576 bytes (1 MB).
    */
   virtual double getChunkSize() const;

private:
   PageCache mCache;
   std::auto_ptr<mta::DMutex> mpMutex;
   std::string mFilename;
   RasterDataDescriptor* mpDescriptor;
   RasterElement* mpRaster;
   int mBytesPerBand;
   int mColumnCount;
   int mBandCount;
   int mRowCount;

   /**
    *  This method should be implemented to open the file and store a file handle to be
    *  closed upon destruction.
    *
    *  Open the file and maintain handles in derived class constructor, close in destructor.
    *
    *  @param   filename
    *           The file name to open.
    *
    *  @return  TRUE if the open succeeds, FALSE otherwise.
    */
   virtual bool openFile(const std::string& filename) = 0;

   /**
    *  Fetches a CacheUnit from disk.
    *
    *  Essentially provides the same functionality as RasterPage::getPage() but for
    *  use in a cache. CachedPages also have an offset that would allow for
    *  higher performance if there is a circumstance where a block is read once
    *  and two separate DataAccessors wish to access different parts of the same
    *  page.
    *
    *  @param pOriginalRequest
    *         The request to fulfill.
    */
   virtual CachedPage::UnitPtr fetchUnit(DataRequest *pOriginalRequest) = 0;
};

#endif
