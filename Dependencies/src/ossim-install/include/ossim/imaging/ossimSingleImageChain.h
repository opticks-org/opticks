//---
//
// License: MIT
//
// Author:  David Burken
//
// Description: Utility class declaration for a single image chain.
// 
//---
// $Id$
#ifndef ossimSingleImageChain_HEADER
#define ossimSingleImageChain_HEADER 1

#include <ossim/base/ossimConstants.h> /* OSSIM_DLL */
#include <ossim/imaging/ossimBandSelector.h>
#include <ossim/imaging/ossimBrightnessContrastSource.h>
#include <ossim/imaging/ossimCacheTileSource.h>
#include <ossim/imaging/ossimGeoPolyCutter.h>
#include <ossim/imaging/ossimHistogramRemapper.h>
#include <ossim/imaging/ossimImageChain.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/imaging/ossimImageRenderer.h>
#include <ossim/imaging/ossimImageSharpenFilter.h>
#include <ossim/imaging/ossimNullPixelFlip.h>
#include <ossim/imaging/ossimScalarRemapper.h>
#include <ossim/imaging/ossimGammaRemapper.h>
#include <vector>

// Forward class declarations:
class ossimFilename;
class ossimGeoPolygon;
class ossimSrcRecord;

/**
 * @class ossimSingleImageChain
 * 
 * @brief Single image chain class.
 *
 * Convenience class for a single image chain.
 * 
 * For code example see: ossim/src/test/ossim-single-image-chain-test.cpp  
 *
 * Just a clarification on "start of chain" versus "end of chain" in this
 * file.
 *
 * Given chain of:
 * 1) image handler
 * 2) band selector (optional)
 * 3) histogram remapper(optional)
 * 4) scalar remapper (optional)
 * 5) resampler cache
 * 6) resampler
 * 7) band selector (optional when going one band to three)
 * 8) chain cache
 *
 * The "image handle" is the "start of chain".
 * The "chain cache" is the "end of chain".
 */
class OSSIM_DLL ossimSingleImageChain : public ossimImageChain
{
public:

   /** default constructor */
   ossimSingleImageChain();

   /** Constructor that takes flags.*/
   ossimSingleImageChain(bool addNullPixelFlipFlag,
                         bool addHistogramFlag,
                         bool addResamplerCacheFlag,
                         bool addChainCacheFlag,
                         bool remapToEightBitFlag,
                         bool threeBandFlag,
                         bool threeBandReverseFlag,
                         bool brightnessContrastFlag=false,
                         bool sharpenFlag=false,
                         bool geoPolyCutterFlag=false);
   
   /** virtual destructor */
   virtual ~ossimSingleImageChain();

   /**
    * @brief reset method
    * This deletes all links in the chain, zero's out all data members, and
    * sets all flags back to default.
    */
   void reset();
   
   /**
    * @brief open method that takes an image file.
    *
    * Opens file and creates a simple chain with ossimImageHandler.
    *
    * @param file File to open.

    * @param openOverview If true image handler will attempt to open overview.
    * Note that if you are planning on doing a rendered chain or want to go
    * between res levels you should set this to true.  default = true
    *
    * @return true on success, false on error.
    *
    * @note This will close previous chain if one was opened.
    */
   bool open(const ossimFilename& file, bool openOverview=true);

   /**
    * @brief open method that takes an ossimSrcRecord.
    *
    * Opens file and creates a simple chain with ossimImageHandler.
    *
    * @return true on success, false on error.
    *
    * @note This will close previous chain if one was opened.
    */
   bool open(const ossimSrcRecord& src);
   
   /** @return true if image handler is opened. */
   bool isOpen() const;

   /** @brief close method to delete the image handler. */
   void close();

   /** @return The filename of the image. */
   ossimFilename getFilename() const;

   /**
    * @brief Create a rendered image chain.
    *
    * Typical usage is to call this after "open" method returns true like:
    * if ( myChain->open(myFile) == true )
    * {
    *    myChain->createRenderedChain();
    *    code-goes-here();
    * }
    *
    * Typical chain is:
    * 
    * 1) image handler
    * 2) band selector (optional)
    * 3) histogram remapper(optional)
    * 4) scalar remapper (optional)
    * 5) resampler cache
    * 6) resampler
    * 7) band selector (optional when going one band to three)
    * 8) chain cache
    *
    * NOTES:
    * 1) Cache on left hand side of resampler is critical to speed if you
    *    have the ossimImageRender enabled.
    *    
    * 2) If doing a sequential write where tiles to the right of the
    *    resampler will not be revisited the chain cache could be
    *    disabled to save memory.
    */
   void createRenderedChain();

   /**
    * @brief Create a rendered image chain that takes an ossimSrcRecord.
    */
   void createRenderedChain(const ossimSrcRecord& src);

   /**
    * @brief Adds an image handler for file.
    * 
    * @param file File to open.
    *
    * @param openOverview If true image handler will attempt to open overview.
    * Note that if you are planning on doing a rendered chain or want to go
    * between res levels you should set this to true. default = true
    *
    * @return true on success, false on error.
    */
   bool addImageHandler(const ossimFilename& file,
                        bool openOverview=true);

   /**
    * @brief Adds an image handler from src record.
    *
    * This take an ossimSrcRecord which can contain a supplemental directory
    * to look for overviews.
    * 
    * @param rec Record to open.
    * @return true on success, false on error.
    */
   bool addImageHandler(const ossimSrcRecord& src);
   
   /** @brief Adds a band selector to the end of the chain. */
   void addBandSelector();

   /**
    * @brief Adds a band selector.
    *
    * This takes an ossimSrcRecord which can contain a band selection list.
    * 
    * @param src Record to initialize band selector from.
    * 
    */
   void addBandSelector(const ossimSrcRecord& src);

   /** @brief Adds histogram remapper to the chain. */
   void addHistogramRemapper();

   /**
    * @brief Adds a band selector.
    *
    * This takes an ossimSrcRecord which can contain a histogram
    * operation to be performed.
    * 
    * @param src Record to initialize band selector from.
    */
   void addHistogramRemapper(const ossimSrcRecord& src);

   /** @brief Adds histogram remapper to the chain. */
   void addGammaRemapper();

   /**
    * @brief Adds a band selector.
    *
    * This takes an ossimSrcRecord which can contain a histogram
    * operation to be performed.
    * 
    * @param src Record to initialize band selector from.
    */
   void addGammaRemapper(const ossimSrcRecord& src);


   /**
    * @brief Adds a new cache to the current end of the chain.
    * @return Pointer to cache.
    */
   ossimRefPtr<ossimCacheTileSource> addCache();

   /** @brief Adds a resampler (a.k.a. "renderer") to the end of the chain. */
   void addResampler();

   /**
    * @brief Adds a resampler (a.k.a. "renderer") to the end of the chain.
    * This method in turn calls "addResampler()".
    */
   void addRenderer();

   /**
    * @brief Adds scalar remapper either to the left of the resampler cache
    * or at the end of the chain if not present.
    */
   void addScalarRemapper();

   /**
    * @brief Adds brightness contrast filter the end of the chain if not
    * present.
    */
   void addBrightnessContrast();

   /**
    * @brief Adds sharpen filter the end of the chain if not present.
    */
   void addSharpen();


   /**
   * Adds the null pixel flip just after the band selection
   */
   void addNullPixelFlip();
   void addNullPixelFlip(const ossimSrcRecord& src);
   /**
   *
   * @brief Adds a geo polycutter to allow for cropping imagery or nulling out
   * regions.  This has no affect on modification of the bounds
   *
   */
   void addGeoPolyCutter();

   void addGeoPolyCutterPolygon(const std::vector<ossimGpt>& polygon);
   void addGeoPolyCutterPolygon(const ossimGeoPolygon& polygon);

   /**
    * @return ossimRefPtr containing the image handler.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<const ossimImageHandler> getImageHandler() const;

   /**
    * @return ossimRefPtr containing the image handler.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<ossimImageHandler> getImageHandler();

   /**
    * @return ossimRefPtr containing  the band selector.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<const ossimBandSelector> getBandSelector() const;

   /**
   *  @return the null pixel flip
   */
   ossimRefPtr<const ossimNullPixelFlip> getNullPixelFlip() const;

   /**
    * @return ossimRefPtr containing  the band selector.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<ossimBandSelector> getBandSelector();

   /**
    * @return ossimRefPtr containing the histogram remapper.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<const ossimHistogramRemapper> getHistogramRemapper() const;

   /**
    * @return ossimRefPtr containing the histogram remapper.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<ossimHistogramRemapper> getHistogramRemapper();

   /**
    * @return ossimRefPtr containing the histogram remapper.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<const ossimGammaRemapper> getGammaRemapper() const;

   /**
    * @return ossimRefPtr containing the histogram remapper.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<ossimGammaRemapper> getGammaRemapper();

   /**
    * @return  ossimRefPtr containing the resampler cache.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<const ossimCacheTileSource> getResamplerCache() const;

   /**
    * @return  ossimRefPtr containing the resampler cache.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<ossimCacheTileSource> getResamplerCache();

   /**
    * @return ossimRefPtr containing the resampler (a.k.a. "renderer").
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<const ossimImageRenderer> getImageRenderer() const;

   /**
    * @return ossimRefPtr containing the resampler (a.k.a. "renderer").
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<ossimImageRenderer> getImageRenderer();

   /**
    * @return ossimRefPtr containing the scalar remapper.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<const ossimScalarRemapper> getScalarRemapper() const;

   /**
    * @return ossimRefPtr containing the scalar remapper.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<ossimScalarRemapper> getScalarRemapper();

   /**
    * @return ossimRefPtr containing the brightness contrast filter.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<const ossimBrightnessContrastSource>
      getBrightnessContrast() const;

   /**
    * @return ossimRefPtr containing the brightness contrast filter.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<ossimBrightnessContrastSource> getBrightnessContrast();

   /**
    * @return ossimRefPtr containing the sharpen filter.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<const ossimImageSharpenFilter> getSharpenFilter() const;

   /**
    * @return ossimRefPtr containing the sharpen.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<ossimImageSharpenFilter> getSharpenFilter();
   
   /**
    * @return ossimRefPtr containing the chain cache.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<const ossimCacheTileSource> getChainCache() const;

   /**
    * @return ossimRefPtr containing the chain cache.
    * @note Can contain a null pointer so callers should validate.
    */
   ossimRefPtr<ossimCacheTileSource> getChainCache();

   /**
    * @brief If flag is true a null pixel flip will be added to the chain at create time.
    * @param flag
    */
   void setAddNullPixelFlipFlag(bool flag);

   /**
    * @brief Gets the add histogram flag.
    * @return true or false.
    */
   bool getNullPixelFlipFlag() const;

 

   /**
    * @brief If flag is true a histogram will be added to the chain at create time.
    * @param flag
    */
   void setAddHistogramFlag(bool flag);

   /**
    * @brief Gets the add histogram flag.
    * @return true or false.
    */
   bool getAddHistogramFlag() const;

   /**
    * @brief If flag is true a gamma remapper will be added to the chain at create time.
    * @param flag
    */
   void setAddGammaFlag(bool flag);

   /**
    * @brief Gets the add gamma flag.
    * @return true or false.
    */
   bool getAddGammaFlag() const;

   /**
    * @brief If flag is true a resampler cache will be added to the chain at create time.
    * This is a cache to the left of the resampler.
    * @param flag
    */
   void setAddResamplerCacheFlag(bool flag);

   /**
    * @brief Gets the add resampler cache flag.
    * @return true or false.
    */
   bool getAddResamplerCacheFlag() const;

   /**
    * @brief If flag is true a chain cache will be added to the chain at create time.
    * This is a cache at the end of the chain.
    * @param flag
    */
   void setAddChainCacheFlag(bool flag);

   /**
    * @brief Gets the add chain cache flag.
    * @return true or false.
    */
   bool getAddChainCacheFlag() const;

   /**
    * @brief Sets remap to eigth bit flag.
    * @param flag
    */
   void setRemapToEightBitFlag(bool flag);

   /**
    * @brief Get the remap to eight bit flag.
    * @return true or false.
    */
   bool getRemapToEightBitFlag() const;
   
   /**
    * @brief Sets the three band flag.
    *
    * If set will for a three band output.  So if one band it will duplicate
    * so that rgb = b1,b1,b1. An attempt is made to derive rgb bands from the
    * image handler.
    * 
    * @param flag
    */
   void setThreeBandFlag(bool flag);

   /**
    * @brief Get the three band flag.
    * @return true or false.
    */
   bool getThreeBandFlag() const;
   
   /**
    * @brief Sets the three band reverse flag.
    *
    * @param flag
    */
   void setThreeBandReverseFlag(bool flag);

   /**
    * @brief Get the three band reverse flag.
    * @return true or false.
    */
   bool getThreeBandReverseFlag() const;
   
   /**
    * @brief Sets the brightness contrast flag.
    *
    * @param flag
    */
   void setBrightnessContrastFlag(bool flag);

   /**
    * @brief Get the brightness contrast flag.
    * @return true or false.
    */
   bool getBrightnessContrastFlag() const;

   /**
    * @brief Sets the sharpenflag.
    *
    * @param flag
    */
   void setSharpenFlag(bool flag);

   /**
    * @brief Get the sharpen flag.
    * @return true or false.
    */
   bool getSharpenFlag() const;

   /**
    * @brief Utility method to force 3 band output.
    *
    * Set band selector to a three band (rgb) output. If image has less than
    * three bands it will set to rgb = b1,b1,b1.  If image has three or
    * more bands the band selector will be see to rgb = b1, b2, b3.
    *
    * @note This will not work unless the image handler is initialized.
    */
   void setToThreeBands();
   
   /**
    * @brief Utility method to set to 3 bandsand reverse them.  This is
    * mainly used by NITF and Lndsat color data where the bands are in bgr
    * format and you want it in rgb combination.  If image has less than
    * three bands it will set to rgb = b1,b1,b1.  If image has three or
    * more bands the band selector will be see to rgb = b3, b2, b1.
    *
    * @note This will not work unless the image handler is initialized.
    */
   void setToThreeBandsReverse();

   /**
    * @brief method to set band selector.
    *
    * This will set the band selection to bandList.  If a band selector is
    * not in the chain yet it will be added.
    *
    * @param bandList The list of bands.
    */
   void setBandSelection(const std::vector<ossim_uint32>& bandList);

   void setDefaultBandSelection();
   
   /**
    * @brief Convenience method to return the scalar type of the image handler.
    * 
    * @return Scalar type of the image handler.
    *
    * This can return OSSIM_SCALAR_UNKNOWN if the image handler has not been
    * set yet.  Also, this is NOT the same as calling getOutputScalarType
    * which could have a different scalar type than the image if the
    * m_remapToEightBitFlag has been set.
    */
   ossimScalarType getImageHandlerScalarType() const;

   /**
    * @brief Convenience method to open the histogram and apply a default
    * stretch.
    *
    * This will only work if the image is open, there is a histogram remapper
    * in the chain, and there was a histogram created.
    * 
    * Valid stretches (from ossimHistogramRemapper.h):
    *
    @verbatim
      ossimHistogramRemapper::LINEAR_ONE_PIECE
      ossimHistogramRemapper::LINEAR_1STD_FROM_MEAN
      ossimHistogramRemapper::LINEAR_2STD_FROM_MEAN
      ossimHistogramRemapper::LINEAR_3STD_FROM_MEAN
      ossimHistogramRemapper::LINEAR_AUTO_MIN_MAX
    @endverbatim
    *
    * @return true on success, false on error.
    */
   bool openHistogram( ossimHistogramRemapper::StretchMode mode );

private:

   /**  Pointers to links in chain. */
   ossimRefPtr<ossimImageHandler>             m_handler;
   ossimRefPtr<ossimBandSelector>             m_bandSelector;
   ossimRefPtr<ossimNullPixelFlip>            m_nullPixelFlip;
   ossimRefPtr<ossimHistogramRemapper>        m_histogramRemapper;
   ossimRefPtr<ossimGammaRemapper>            m_gammaRemapper;
   ossimRefPtr<ossimBrightnessContrastSource> m_brightnessContrast;
   ossimRefPtr<ossimImageSharpenFilter>       m_sharpen;   
   ossimRefPtr<ossimScalarRemapper>           m_scalarRemapper;
   ossimRefPtr<ossimCacheTileSource>          m_resamplerCache;
   ossimRefPtr<ossimImageRenderer>            m_resampler;
   ossimRefPtr<ossimGeoPolyCutter>            m_geoPolyCutter;
   ossimRefPtr<ossimCacheTileSource>          m_chainCache;
   /** control flags */
   bool m_addNullPixelFlipFlag;
   bool m_addHistogramFlag;
   bool m_addGammaFlag;
   bool m_addResamplerCacheFlag;
   bool m_addChainCacheFlag;
   bool m_remapToEightBitFlag;
   bool m_threeBandFlag;
   bool m_threeBandReverseFlag;
   bool m_brightnessContrastFlag;
   bool m_sharpenFlag;
   bool m_geoPolyCutterFlag;

};

#endif /* #ifndef ossimSingleImageChain_HEADER */
