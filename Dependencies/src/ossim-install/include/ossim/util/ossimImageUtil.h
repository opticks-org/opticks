//----------------------------------------------------------------------------
// File: ossimImageUtil.h
// 
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: ossimImageUtil
//
// See class descriptions below for more.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimImageUtil_HEADER
#define ossimImageUtil_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimFileProcessorInterface.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimOverviewBuilderBase.h>
#include <ostream>
#include <vector>
#include <mutex>

class ossimArgumentParser;
class ossimFileWalker;
class ossimGpt;
class ossimPropertyInterface;
class ossimApplicationUsage;
class ossimImageHandler;
/**
 * @brief ossimImageUtil class.
 *
 * Utility class for processing image recursively.  This is for doing things like:
 * 
 * building overview, histograms, compute min/max, extract vertices.
 */
class OSSIM_DLL ossimImageUtil :
   public ossimReferenced, public ossimFileProcessorInterface
{
public:

   /** default constructor */
   ossimImageUtil();

   /** virtual destructor */
   virtual ~ossimImageUtil();

   void addOptions(ossimApplicationUsage* au);
   /**
    * @brief Adds application arguments to the argument parser.
    * @param ap Parser to add to.
    */
   void addArguments(ossimArgumentParser& ap);

   /**
    * @brief Initial method.
    *
    * Typically called from application prior to execute.  This parses
    * all options and put in keyword list m_kwl.
    * 
    * @param ap Arg parser to initialize from.
    *
    * @return true, indicating process should continue with execute.
    */
   bool initialize(ossimArgumentParser& ap);

   /**
    * @brief Execute method.
    *
    * This launches file walking mechanism.
    *
    * @return int, 0 = good, non-zero something happened.  Because this can
    * process multiple files a non-zero return may indicate just one file
    * did not complete, e.g. building overviews.
    * 
    * @note Throws ossimException on error.
    */
   ossim_int32 execute();

   /**
    * @brief ProcessFile method.
    *
    * Satisfies pure virtual ossimFileProcessorInterface::processFile.
    *
    * This method is linked to the ossimFileWalker::walk method via a callback
    * mechanism.  It is called by the ossimFileWalk (caller).  In turn this
    * class (callee) calls ossimFileWalker::setRecurseFlag and
    * ossimFileWalker::setAbortFlag to control the waking process.
    * 
    * @param file to process.
    */
   virtual void processFile(const ossimFilename& file);

   /**
    * @brief Sets create overviews flag keyword CREATE_OVERVIEWS_KW used by
    * processFile method.
    *
    * @param flag If true overview will be created if image does not already
    * have the required or if the REBUILD_OVERVIEWS_KW is set.
    *
    * @note Number of required overviews is controlled by the ossim preferences
    * keyword overview_stop_dimension.
    */
   void setCreateOverviewsFlag(bool flag);

   /** @return true if CREATE_OVERVIEWS_KW is found and set to true. */
   bool createOverviews() const;

   /**
    * @brief Sets create thumbnails flag keyword CREATE_THUMBNAILS_KW used by
    * processFile method.
    *
    * @param flag If true thumbnail will be created if image does not already.
    *
    * @note Overviews must be created before this works
    */
   void setCreateThumbnailsFlag(bool flag);

   /**
    *  @param value can be of values png or jpeg
    */
   void setThumbnailType(const std::string& value);

   /**
    * @param value can be of values none,auto-minmax,auto-percentile,std-stretch-1,std-stretch-2,std-stretch-3
    */
   void setThumbnailStretchType(const std::string& value);

   /** @return true if CREATE_THUMBNAILS_KW is found and set to true. */
   bool createThumbnails() const;

   /**
    * @brief Sets the rebuild overview flag keyword REBUILD_OVERVIEWS_KW used by
    * processFile method.
    *
    * @param flag If true forces a rebuild of overviews even if image has
    * required number of reduced resolution data sets.
    *
    * @note Number of required overviews is controlled by the ossim preferences
    * keyword overview_stop_dimension.
    */
   void setRebuildOverviewsFlag( bool flag );

   /** @return true if REBUILD_OVERVIEWS_KW is found and set to true. */
   bool rebuildOverviews() const;

   /**
    * @brief Sets the rebuild histogram flag keyword REBUILD_HISTOGRAM_KW used by
    * processFile method.
    *
    * @param flag If true forces a rebuild of histogram even if image has one already.
    */
   void setRebuildHistogramFlag( bool flag );

   /** @return true if REBUILD_HISTOGRAM_KW is found and set to true. */
   bool rebuildHistogram() const;

   /**
    * @brief Sets key OVERVIEW_TYPE_KW.
    *
    * Available types depends on plugins.  Known types:
    * ossim_tiff_box ( defualt )
    * ossim_tiff_nearest
    * ossim_kakadu_nitf_j2k ( kakadu plugin )
    * gdal_tiff_nearest	    ( gdal plugin )
    * gdal_tiff_average	    ( gdal plugin )
    * gdal_hfa_nearest      ( gdal plugin )	
    * gdal_hfa_average      ( gdal plugin )	
    * 
    * @param type One of the above.
    */
   void setOverviewType( const std::string& type );
   
   /**
    * @brief sets the overview stop dimension.
    *
    * The overview builder will decimate the image until both dimensions are
    * at or below this dimension.
    *
    * @param dimension
    *
    * @note Recommend a power of 2 value, i.e. 8, 16, 32 and so on.
    */
   void setOverviewStopDimension( ossim_uint32 dimension );
   void setOverviewStopDimension( const std::string& dimension );

   /**
    * @brief Sets the tile size.
    *
    * @param tileSize
    *
    * @note Must be a multiple of 16, i.e. 64, 128, 256 and so on.
    */
   void setTileSize( ossim_uint32 tileSize );

   /**
    * @brief Gets the tile size.
    * @param tileSize Initialized by this.
    * @return true on success, false if not in options list.
    */
   bool getTileSize( ossimIpt& tileSize ) const;

   /**
    * @return Overview stop dimension or 0 if OVERVIEW_STOP_DIM_KW is not
    * found.
    */
   ossim_uint32 getOverviewStopDimension() const;

   /**
    * @brief Sets create histogram flag keyword CREATE_HISTOGRAM_KW used by
    * processFile method.
    *
    * @param flag If true a full histogram will be created.
    */
   void setCreateHistogramFlag( bool flag );
   
   /** @return true if CREATE_HISTOGRAM_KW is found and set to true. */
   bool createHistogram() const;

   /**
    * @brief Sets create histogram flag keyword CREATE_HISTOGRAM_FAST_KW used by
    * processFile method.
    *
    * @param flag If true a histogram will be created in fast mode.
    */
   void setCreateHistogramFastFlag( bool flag );

   /** @return true if CREATE_HISTOGRAM_FAST_KW is found and set to true. */
   bool createHistogramFast() const;

   /**
    * @brief Sets create histogram "R0" flag keyword CREATE_HISTOGRAM_R0_KW used by
    * processFile method.
    *
    * @param flag If true a histogram will be created from R0.
    */
   void setCreateHistogramR0Flag( bool flag );

   /** @return true if CREATE_HISTOGRAM_R0_KW is found and set to true. */
   bool createHistogramR0() const;

   /** @return true if any of the histogram options are set. */
   bool hasHistogramOption() const;

   /** @return Histogram mode or OSSIM_HISTO_MODE_UNKNOWN if not set. */
   ossimHistogramMode getHistogramMode() const;

   /**
    * @brief Sets scan for min/max flag keyword SCAN_MIN_MAX_KW used by
    * processFile method.
    *
    * @param flag If true a file will be scanned for min/max and a file.omd
    * will be written out.
    */
   void setScanForMinMax( bool flag );
   
   /** @return true if SCAN_MIN_MAX_KW is found and set to true. */
   bool scanForMinMax() const;

   /**
    * @brief Sets scan for min/max/null flag keyword SCAN_MIN_MAX_KW used by
    * processFile method.
    *
    * @param flag If true a file will be scanned for min/max/null and a file.omd
    * will be written out.
    */
   void setScanForMinMaxNull( bool flag );

   /** @return true if SCAN_MIN_MAX_NULL_KW is found and set to true. */
   bool scanForMinMaxNull() const;

   /**
    * @brief Sets the writer property for compression quality.
    *
    * @param quality For TIFF JPEG takes values from 1
    * to 100, where 100 is best.  For J2K plugin (if available),
    * numerically_lossless, visually_lossless, lossy.
    */
   void setCompressionQuality( const std::string& quality );

   /**
    * @brief Sets the compression type to use when building overviews.
    *  
    * @param compression_type Current supported types:
    * - deflate 
    * - jpeg
    * - lzw
    * - none
    * - packbits
    */
   void setCompressionType( const std::string& type );

   /**
    * @brief Sets the overview builder copy all flag.
    * @param flag
    */
   void setCopyAllFlag( bool flag );

   /**
    * @return true if COPY_ALL_FLAG_KW key is found and value is true; else,
    * false.
    */
   bool getCopyAllFlag() const;

   /**
    * @brief Sets the dump filteredImageList flag.
    * @param flag
    */
   void setDumpFilteredImageListFlag( bool flag );

   /**
    * @return true if DUMP_FILTERED_IMAGES_KW key is found and value is true; else,
    * false.
    * 
    * DUMP_FILTERED_IMAGES_KW = "dump_filtered_images"
    */
   bool getDumpFilterImagesFlag() const;

   /**
    * @brief Sets the overview builder internal overviews flag.
    * @param flag
    */
   void setInternalOverviewsFlag( bool flag );

   /**
    * @return true if INTERNAL_OVERVIEWS_FLAG_KW key is found and value is true; else,
    * false.
    */
   bool getInternalOverviewsFlag() const;
   
   /**
    * @brief Sets the output directory.  Typically overviews and histograms
    * are placed parallel to image file.  This overrides.
    *  
    * @param directory
    */
   void setOutputDirectory( const std::string& directory );
 
   /**
    * @brief Sets the output file name flag OUTPUT_FILENAMES_KW.
    *
    * If set to true all files that we can successfully open will be output.
    *  
    * @param flag
    */
   void setOutputFileNamesFlag( bool flag );

   /**
    * @return true if OUTPUT_FILENAMES_KW key is found and value is true; else,
    * false.
    */
   bool getOutputFileNamesFlag() const;

   /**
    * @brief Sets the override filtered images flag.
    * @param flag
    */
   void setOverrideFilteredImagesFlag( bool flag );

   /**
    * @return true if DUMP_FILTERED_IMAGES_KW key is found and value is true; else,
    * false.
    * 
    * DUMP_FILTERED_IMAGES_KW = "dump_filtered_images"
    */
   bool getOverrideFilteredImagesFlag() const;
   
   /**
    * @brief Set number of threads to use.
    *
    * This is only used in execute method if a directory is given to
    * application to walk.
    *
    * @param threads Defaults to 1 if THREADS_KW is not found.
    */
   void setNumberOfThreads( ossim_uint32 threads );
   void setNumberOfThreads( const std::string& threads );

   /** @return The list of filtered out files. */
   const std::vector<std::string>& getFilteredImages() const;

   /**
    * @brief Non const method to allow access for
    * adding or deleting extensions from the list.
    *
    * The list is used by the private isFiltered method to avoid trying to
    * process unwanted files.
    */
   std::vector<std::string>& getFilteredImages();

private:

   /** @return true if key is set to true; false, if not. */
   bool keyIsTrue( const std::string& key ) const;

   /**
    * @brief Convenience method to check file to see is if file should be
    * processed.
    *
    * @param f File to check.
    * 
    * @return true if f is in filter list, false if not.
    */
   bool isFiltered(const ossimFilename& f) const;
   
   /**
    * @brief Initializes the filter list with a default set of filtered out
    * file names.
    */
   void initializeDefaultFilterList();

   /** @brief Dumps filtered image list to std out. */
   void dumpFilteredImageList() const;

   void createOverview(ossimRefPtr<ossimImageHandler>& ih,
                       bool& consumedHistogramOptions,
                       bool& consumedCmmOptions);

   void createOverview(ossimRefPtr<ossimImageHandler>& ih,
                       ossimRefPtr<ossimOverviewBuilderBase>& ob,
                       ossim_uint32 entry,
                       bool useEntryIndex,
                       bool& consumedHistogramOptions);

   void createThumbnail(ossimRefPtr<ossimImageHandler> &ih);

   /** @return true if entry has required overviews. */
   bool hasRequiredOverview( ossimRefPtr<ossimImageHandler>& ih,
                             ossimRefPtr<ossimOverviewBuilderBase>& ob );

   /** @return true if any compute min, max or null options are set. */
   bool hasCmmOption() const;

   void createHistogram(ossimRefPtr<ossimImageHandler>& ih);

   void createHistogram(ossimRefPtr<ossimImageHandler>& ih,
                       ossim_uint32 entry,
                       bool useEntryIndex);

   void computeMinMax(ossimRefPtr<ossimImageHandler>& ih);

   void computeMinMax(ossimRefPtr<ossimImageHandler>& ih,
                      ossim_uint32 entry,
                      bool useEntryIndex);
   
   /** @brief Initializes arg parser and outputs usage. */
   void usage(ossimArgumentParser& ap);

   void outputOverviewWriterTypes() const;

   /**
    * @return true if file is a directory based image and the stager should go
    * on to next directory; false if stager should continue with directory.
    */
   bool isDirectoryBasedImage(const ossimImageHandler* ih) const;

   /**
    * @brief Initializes type from OVERVIEW_TYPE_KW or sets to default
    * ossim_tiff_box if not found.
    */
   void getOverviewType(std::string& type) const;

   /** @brief set reader or writer properties based on cast of pi. */
   void setProps(ossimPropertyInterface* pi) const;
   
   /**
    * @return Threads to use.  Defaults to 1 if THREADS_KW is not found.
    */
   ossim_uint32 getNumberOfThreads() const;

   /** @return the next writer prop index. */
   ossim_uint32 getNextWriterPropIndex() const;

   /** @return the next reader prop index. */
   ossim_uint32 getNextReaderPropIndex() const;

   /** @return the next reader prop index. */
   ossim_uint32 getThumbnailSize() const;

   int getThumbnailStretchType()const;
   std::string getThumbnailType()const;
   std::string getThumbnailFilename(ossimImageHandler *ih) const;
   /**
    * @brief Adds option to m_kwl with mutex lock.
    * @param key
    * @param value
    */
   void addOption(const std::string &key, ossim_uint32 value);
   void addOption( const std::string& key, const std::string& value );

   /**
    * @brief Sets the m_errorStatus for return on execute.
    */
   void setErrorStatus( ossim_int32 status );

   /** @brief run prep system commands. */
   void executePrepCommands() const;

   /** @brief run per file system commands. */
   void executeFileCommands( const ossimFilename& file ) const;

   /** @brief run post system commands. */
   void executePostCommands() const;

   /** @brief system commands. */
   void executeCommands( const std::string& prefix,
                         const ossimFilename& file ) const;

    /** @brief Expands variables in a command string. */
   void substituteCommandString( const ossimFilename& file,
                                 const std::string& prefix,
                                 const std::string& commandKey,
                                 ossimString& command ) const;

   /** @brief Expands file level variables in a command string. */
   void substituteFileStrings( const ossimFilename& file,
                               ossimString& command ) const;

    /** @brief Expands date variables in a command string. */
   void gsubDate( const std::string& commandPrefix,
                  ossimString& command ) const;
   
   /** Holds all options passed into intialize except writer props. */
   ossimRefPtr<ossimKeywordlist> m_kwl;

   ossimFileWalker*   m_fileWalker;
   std::mutex m_mutex;

   ossim_int32 m_errorStatus;

   /** Hold images we never want to process. */
   std::vector<std::string> m_filteredImages; 
};

#endif /* #ifndef ossimImageUtil_HEADER */
