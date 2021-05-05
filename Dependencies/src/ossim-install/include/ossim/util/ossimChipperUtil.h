//----------------------------------------------------------------------------
//
// File: ossimChipperUtil.h
// 
// License:  MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
// 
// Utility class to for chipping out images. Orthorectifying imagery with an
// added slant toward doing digital elevation model(DEM) operations.
// 
//----------------------------------------------------------------------------
// $Id: ossimChipperUtil.h 23423 2015-07-13 19:07:38Z dburken $

#ifndef ossimChipperUtil_HEADER
#define ossimChipperUtil_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageSource.h>
#include <ossim/imaging/ossimSingleImageChain.h>
#include <ossim/imaging/ossimImageFileWriter.h>
#include <ossim/projection/ossimMapProjection.h>
#include <ossim/base/ossimConnectableContainer.h>

#include <map>
#include <vector>

// Forward class declarations:
class ossimAnnotationSource;
class ossimArgumentParser;
class ossimDpt;
class ossimFilename;
class ossimGeoPolygon;
class ossimGpt;
class ossimImageData;
class ossimImageFileWriter;
class ossimImageGeometry;
class ossimImageViewAffineTransform;
class ossimIrect;
class ossimKeywordlist;

/**
 * @brief ossimChipperUtil class.
 *
 * This is a utility class to orthorectify imagery with an added slant toward
 * doing digital elevation model(DEM) operations.
 *
 * See the ossim-dem application for a code usage example.
 *
 * @note Almost all methods use throw for stack unwinding.  This is not in
 * method declarations to alleviate build errors on windows.  Sorry...
 *
 * @note "bumpshade" and "hillshade" intermixed throughout.  The class to do
 * a hillshade is the ossimBumpShadeTileSource.
 */
class OSSIM_DLL ossimChipperUtil : public ossimReferenced
{
public:

   /** emumerated operations */
   enum ossimChipperOperation
   {
      OSSIM_CHIPPER_OP_UNKNOWN      = 0,
      OSSIM_CHIPPER_OP_HILL_SHADE   = 1,
      OSSIM_CHIPPER_OP_COLOR_RELIEF = 2,
      OSSIM_CHIPPER_OP_ORTHO        = 3,
      OSSIM_CHIPPER_OP_2CMV         = 4, // two color multiview
      OSSIM_CHIPPER_OP_CHIP         = 5, // image space
      OSSIM_CHIPPER_OP_PSM          = 6  // pan sharpened multispectral
   };

   /** emumerated output projections */
   enum ossimChipperOutputProjection
   {
      OSSIM_CHIPPER_PROJ_UNKNOWN    = 0,
      OSSIM_CHIPPER_PROJ_GEO        = 1,
      OSSIM_CHIPPER_PROJ_GEO_SCALED = 2,
      OSSIM_CHIPPER_PROJ_INPUT      = 3,
      OSSIM_CHIPPER_PROJ_UTM        = 4
   };

   /** default constructor */
   ossimChipperUtil();

   /** virtual destructor */
   virtual ~ossimChipperUtil();

   /**
    * @brief Disconnects and clears the dem and image layers.
    */
   void clear();
   
   /**
    * @brief Initial method to be ran prior to execute.
    * @param ap Arg parser to initialize from.
    * @note Throws ossimException on error.
    * @note A throw with an error message of "usage" is used to get out when
    * a usage is printed.
    */
   bool initialize(ossimArgumentParser& ap);


   /**
    * @brief Initialize method to be ran prior to execute.
    * 
    * @note Throws ossimException on error.
    */
   void initialize(const ossimKeywordlist& kwl);

   /**
    * @brief execute method.  Performs the actual product write.
    * @note Throws ossimException on error.
    */
   void execute();

   void abort();

   /**
    * @brief Gets initialized area of interest(aoi) from chain.
    * @return Pointer to ossimImageData holding chip.  Pointer
    * can be null if not initialized properly so caller should
    * check.
    */
//   ossimRefPtr<ossimImageData> getChip();

   /**
   * The options will only support a couple modifications while chipping.  
   * this is mainly here to support a moving chip window so we do not need to do
   * a full initialize for every chip.  We can specify a new cut bounds and
   * cut width height in pixels if desired.
   *
   *  cut_wms_bbox: ..........
   *  cut_width:
   *  cut_height:
   *
   *
   *
   */
   ossimRefPtr<ossimImageData> getChip(const ossimKeywordlist& optionsKwl= ossimKeywordlist());

   /**
    * @brief Gets the output file name.
    * @param f Initialized by this with the filename.
    */
   void getOutputFilename(ossimFilename& f) const;

private:

   /**
    * @brief Initial method to be ran prior to execute.
    *
    * @note Throws ossimException on error.
    */
   void initialize();

   /**
    * @brief Builds image chains returns ref pointer to image source
    * and initializes area of interest(aoi).
    * @return Ref pointer to image chain.
    */
   ossimRefPtr<ossimImageSource> initializeChain( ossimIrect& aoi );

   void setOptionsToChain( ossimIrect& aoi, const ossimKeywordlist& kwl );

   /**
    * @brief Initializes a color relief chain.
    * @return Ref pointer to image chain.
    */
   ossimRefPtr<ossimImageSource> initializeColorReliefChain();

   /**
    * @brief Initializes a bump shade chain.
    * @return Ref pointer to image chain.
    */
   ossimRefPtr<ossimImageSource> initializeBumpShadeChain();

   /**
    * @brief Combines two images into a two color multi view chain.
    * @return ossimRefPtr with pointer to ossimImageSource.  Can be null.
    */
   ossimRefPtr<ossimImageSource> initialize2CmvChain();

   /**
    * @brief Initializes a psm (pan sharpening multispectra) chain.
    * @return Ref pointer to image chain.
    */
   ossimRefPtr<ossimImageSource> initializePsmChain();

   /**
    * @brief Initializes the output projection and propagates to image chains.
    * @note Throws ossimException on error.
    */
   void initializeOutputProjection();
   
   /** @brief Create chains for all dems. */
   void addDemSources();
   
   /**
    * @brief Method to create a chain and add to dem layers from file.
    * @param file Image to open.
    * @parm entryIndex Entry to open.
    */
   void addDemSource(const ossimFilename& file,
                     ossim_uint32 entryIndex);

   /**
    * @brief  Method to create a chain and add to dem layers from a
    * ossimSrcRecord.
    */
   void addDemSource(const ossimSrcRecord& rec);

   /** @brief Creates chains for all images. */
   void addImgSources();
   
   /**
    * @brief Method to create a chain and add to img layers from file.
    * @param file Image to open.
    * @parm entryIndex Entry to open.
    */
   void addImgSource(const ossimFilename& file,
                     ossim_uint32 entryIndex);

   /**
    * @brief  Method to create a chain and add to img layers from a
    * ossimSrcRecord.
    */
   void addImgSource(const ossimSrcRecord& rec);

   /**
    * @brief Creates a ossimSingleImageChain from file.
    * @param file File to open.
    * @param entryIndex Entry to open.
    * @param isDemSource True if dem source, false if not. This controls chain
    * options like histogram stretches.
    * @return Ref pointer to ossimSingleImageChain.
    * @note Throws ossimException on error.
    */
   ossimRefPtr<ossimSingleImageChain> createChain(const ossimFilename& file,
                                                  ossim_uint32 entryIndex,
                                                  bool isDemSource) const;

   /**
    * @brief Creates a ossimSingleImageChain from ossimSrcRecord.
    * @param src Record.
    * @param isDemSource True if dem source, false if not. This controls chain
    * options like histogram stretches.
    * @return Ref pointer to ossimSingleImageChain.
    * @note Throws ossimException on error.
    */
   ossimRefPtr<ossimSingleImageChain> createChain(const ossimSrcRecord& rec,
                                                  bool isDemSource) const;

   /**
    * @brief Creates the output or view projection.
    * @note All chains should be constructed prior to calling this.
    */
   void createOutputProjection();

   /**
    * @brief Rotates the view map projection to align with the top row in the input image.
    * @note Omly available with ortho operation on single image.
    */
   void rotateMapToInput ();

   /**
    * @brief Sets the single image chain for identity operations view to
    * a ossimImageViewAffineTransform.  This will have a rotation if
    * up is up is selected.  Also set m_outputProjection to the input's
    * for area of interest.
    */
   void createIdentityProjection();

   /**
    * @brief Gets the first input projection.
    *
    * This gets the output projection of the first dem layer if present;
    * if not, the first image layer.
    * 
    * @return ref ptr to projection, could be null.
    */
   ossimRefPtr<ossimMapProjection> getFirstInputProjection();

   /**
    * @brief Convenience method to get geographic projection.
    * @return new ossimEquDistCylProjection.
    */
   ossimRefPtr<ossimMapProjection> getNewGeoProjection();

   /**
    * @brief Convenience method to get geographic projection.
    *
    * This method sets the origin to the center of the scene bounding rect
    * of all layers.
    * @return new ossimEquDistCylProjection.
    */
   ossimRefPtr<ossimMapProjection> getNewGeoScaledProjection();

    /**
    * @brief Convenience method to get a projection from an srs code.
    * @return new ossimMapProjection.
    */  
   ossimRefPtr<ossimMapProjection> getNewProjectionFromSrsCode(
      const std::string& code );

   /**
    * @brief Convenience method to get a utm projection.
    * @return new ossimUtmProjection.
    */     
   ossimRefPtr<ossimMapProjection> getNewUtmProjection();

   /**
    * @brief Convenience method to get a pointer to the  output map
    * projection.
    *
    * Callers should check for valid() as the pointer could be
    * 0 if not initialized.
    * 
    * @returns The ossimMapProjection* from the m_outputGeometry as a ref
    * pointer.
    */
   ossimRefPtr<ossimMapProjection> getMapProjection();

   /**
    * @brief Sets the projection tie point to the scene bounding rect corner.
    * @note Throws ossimException on error.
    */
   void initializeProjectionTiePoint ();

   /**
    * @brief Initializes the projection gsd.
    *
    * This loops through all chains to find the best resolution gsd.
    *
    * @note Throws ossimException on error.
    */
   void initializeProjectionGsd();   

   /**
    * @brief Initializes the image view transform(IVT) scale.
    *
    * Chip mode only. Sets IVT scale to output / input.
    * 
    * @note Throws ossimException on error.
    */
   void initializeIvtScale();   

   /**
    * @brief Loops through all layers to get the upper left tie point.
    * @param tie Point to initialize.
    */
   void getTiePoint(ossimGpt& tie);

   /**
    * @brief Gets the upper left tie point from a chain.
    * @param chain The chain to get tie point from.
    * @param tie Point to initialize.
    * @note Throws ossimException on error.
    */
   void getTiePoint(ossimSingleImageChain* chain, ossimGpt& tie);

   /**
    * @brief Loops through all layers to get the upper left tie point.
    * @param tie Point to initialize.
    */
   void getTiePoint(ossimDpt& tie);

   /**
    * @brief Gets the upper left tie point from a chain.
    * @param chain The chain to get tie point from.
    * @param tie Point to initialize.
    * @note Throws ossimException on error.
    */
   void getTiePoint(ossimSingleImageChain* chain, ossimDpt& tie);

   /**
    * @brief Loops through all layers to get the best gsd.
    * @param gsd Point to initialize.
    */
   void getMetersPerPixel(ossimDpt& gsd);

   /**
    * @brief Gets the gsd from a chain.
    * @param chain The chain to get gsd from.
    * @param gsd Point to initialize.
    * @note Throws ossimException on error.
    */   
   void getMetersPerPixel(ossimSingleImageChain* chain, ossimDpt& gsd);

   /**
    * @brief Gets value of key "central_meridan" if set, nan if not.
    *
    * @return Value as a double or nan if keyord is not set.
    * 
    * @note Throws ossimException on range error.
    */
   ossim_float64 getCentralMeridian() const;

   /**
    * @brief Gets value of key "origin_latitude" if set, nan if not.
    *
    * @return Value as a double or nan if keyord is not set.
    * 
    * @note Throws ossimException on range error.
    */
   ossim_float64 getOriginLatitude() const;

   /**
    * @brief Loops through all layers to get the scene center ground point.
    * @param gpt Point to initialize.
    * @note Throws ossimException on error.
    */
   void getSceneCenter(ossimGpt& gpt);

   /**
    * @brief Gets the scene center from a chain.
    * @param chain The chain to get scene center from.
    * @param gpt Point to initialize.
    * @note Throws ossimException on error.
    */   
   void getSceneCenter(ossimSingleImageChain* chain, ossimGpt& gpt);

   /**
    * @brief Creates a new writer.
    *
    * This will use the writer option (-w or --writer), if present; else,
    * it will be derived from the output file extension.
    *
    * This will also set any writer properties passed in.
    *
    * @return new ossimImageFileWriter.
    * @note Throws ossimException on error.
    */
   ossimRefPtr<ossimImageFileWriter> createNewWriter() const;

   /**
    * @brief loops through all chains and sets the output projection.
    * @note Throws ossimException on error.
    */
   void propagateOutputProjectionToChains();
   
   /**
    * @brief loops through all chains and sets the viewport aoi.  This is used
    *        if viewport stretch is enabled based on center tile request.
    */
   void propagateViewportStretch(const ossimIrect& aoi);

   /**
    * @brief Combines all layers into an ossimImageMosaic.
    * @return ossimRefPtr with pointer to ossimImageSource.  Can be null.
    */
   ossimRefPtr<ossimImageSource> combineLayers(
      std::vector< ossimRefPtr<ossimSingleImageChain> >& layers) const;

   /** @brief Combines dems(m_demLayer) and images(m_imgLayer). */
   ossimRefPtr<ossimImageSource> combineLayers();

   /**
    * @brief Creates ossimIndexToRgbLutFilter and connects to source.
    * @param Source to connect to.
    * @return End of chain with lut filter on it.
    * @note Throws ossimException on error.
    */
   ossimRefPtr<ossimImageSource> addIndexToRgbLutFilter(
      ossimRefPtr<ossimImageSource> &source) const;

   /**
    * @brief Creates ossimScalarRemapper and connects to source.
    * @param Source to connect to.
    * @param scalar Scalar type.
    * @return End of chain with remapper on it.
    * @note Throws ossimException on error.
    */
   ossimRefPtr<ossimImageSource> addScalarRemapper(
      ossimRefPtr<ossimImageSource> &source,
      ossimScalarType scalar) const;

   /**
    * @brief Add annotation source to chain.
    * @param Source to connect to.
    * @return End of chain with annotion source on it.
    */
   ossimRefPtr<ossimImageSource> addAnnotations(
      ossimRefPtr<ossimImageSource> &source) const;

   /**
    * @brief Adds cross hair graphic to annotation source.
    * @param Annotator to add objects to.
    * @param prefix e.g. annotation0.
    */
   void addCrossHairAnnotation(
      ossimRefPtr<ossimAnnotationSource> annotator,
      const std::string& prefix ) const;

   /**
    * @brief Set up ossimHistogramRemapper for a chain.
    * @param chain Chain to set up.
    * @return true on success, false on error.
    */
   bool setupChainHistogram( ossimRefPtr<ossimSingleImageChain>& chain, 
                             std::shared_ptr<ossimSrcRecord> srcRecordPtr=0) const;

   /**
    * @brief Sets entry for a chain.
    * @param chain Chain to set up.
    * @param entryIndex Zero based index.
    * @return true on success, false on error.
    */
   bool setChainEntry( ossimRefPtr<ossimSingleImageChain>& chain,
                       ossim_uint32 entryIndex ) const;

   /**
    * @brief Initializes "rect" with the output area of interest.
    *
    * Initialization will either come from user defined cut options or the
    * source bounding rect with user options taking precidence.
    *
    * @param source Should be the end of the processing chain.
    * @param rect Rectangle to initialize.  This is in output (view) space.
    *
    * @note Throws ossimException on error.
    */
   void getAreaOfInterest( ossimImageSource* source, ossimIrect& rect ) const;

   /**
    * Gets rect from string in the form of <x>,<y>,<w>,<h>.
    * @param s String to parse.
    * @rect Initialized by this.
    * @return true on success, false, on error.
    */
   bool getIrect( const std::string& s, ossimIrect& rect ) const;

   /**
    * Gets image rect from string in the form of <lat>,<lon>,<w>,<h>.
    *
    * Computes image rect from world point assumed to be center of aoi.
    *
    * @param chain
    * @param s String to parse.
    * @rect Initialized by this.
    * @return true on success, false, on error.
    */
   bool getIrect( ossimRefPtr<ossimSingleImageChain>& chain,
                  const std::string& s, ossimIrect& rect ) const;
   
   /**
    * Gets 256 x 256 image rect from center of the image.
    *
    * Computes image rect from world point assumed to be center of aoi.
    *
    * @param chain
    * @rect Initialized by this.
    * @return true on success, false, on error.
    */
   bool getIrect( ossimRefPtr<ossimSingleImageChain>& chain,
                  ossimIrect& rect ) const;
   
   /**
    * @brief Method to calculate and initialize scale and area of interest
    * for making a thumbnail.
    *
    * Sets the scale of the output projection so that the adjusted rectangle
    * meets the cut rect and demension requirements set in options.
    *
    * @param originalRect Original scene area of interest.
    * @param adjustedRect New rect for thumbnail.
    *
    * @note Throws ossimException on error.
    */
   void initializeThumbnailProjection(const ossimIrect& originalRect,
                                      ossimIrect& adjustedRect);

   /** @return true if BANDS keyword is set; false, if not. */
   bool hasBandSelection() const;

   /**
    * @brief Gets the band list if BANDS keyword is set.
    *
    * NOTE: BANDS keyword values are ONE based.  bandList values are
    * ZERO based.
    *
    * @param bandList List initialized by this.
    */
   void getBandList( std::vector<ossim_uint32>& bandList ) const;

   /** @return true if annotation options are set; false, if not. */
   bool hasAnnotations() const;

   /** @return true if color table (lut) is set; false, if not. */
   bool hasLutFile() const;

   /** @return true if brightness or contrast option is set; false, if not. */
   bool hasBrightnesContrastOperation() const;

   /** @return true if any Geo Poly cutter option is set */
   bool hasGeoPolyCutterOption()const;
   
   /**
    * @return true if any bump share options have been set by user; false,
    * if not.
    */
   bool hasBumpShadeArg() const;

   /** @return true if thumbnail option is set; false, if not. */
   bool hasThumbnailResolution() const;

   /** @return true if histogram option is set; false, if not. */
   bool hasHistogramOperation() const;


  /** @return true if histogram option is set; false, if not. */
   bool hasGammaCorrection() const;


   /** @return true if file extension is "hgt", "dem" or contains "dtN" (dted). */
   bool isDemFile(const ossimFilename& file) const;

   /** @return true if file extension is "src" */
   bool isSrcFile(const ossimFilename& file) const;

   /** @brief Initializes m_srcKwl if option was set. */
   void initializeSrcKwl();

   /**
    * @return The number of DEM_KW and IMG_KW found in the m_kwl and m_srcKwl
    * keyword list.
    */
   ossim_uint32 getNumberOfInputs() const;

   /**
    * @brief Gets the emumerated output projection type.
    *
    * This looks in m_kwl for ossimKeywordNames::PROJECTION_KW.
    * @return The enumerated output projection type.
    * @note This does not cover SRS keyword which could be any type of projection.
    */
   ossimChipperOutputProjection getOutputProjectionType() const;

   /**
    * @brief Returns the scalar type from OUTPUT_RADIOMETRY_KW keyword if
    * present. Deprecated SCALE_2_8_BIT_KW is also checked.
    *
    * @return ossimScalarType Note this can be OSSIM_SCALAR_UNKNOWN if the
    * keywords are not present.
    */
   ossimScalarType getOutputScalarType() const;

   /** @return true if scale to eight bit option is set; false, if not. */
   bool scaleToEightBit() const;

   /** @return true if snap tie to origin option is set; false, if not. */
   bool snapTieToOrigin() const;

   /**
    * @brief Gets the image space scale.
    *
    * This is a "chip" operation only.
    *
    * Keys: 
    * IMAGE_SPACE_SCALE_X_KW
    * IMAGE_SPACE_SCALE_Y_KW
    * FULLRES_XYS
    *
    * Scale will be 1.0, 1.0 if keys not found. 
    */
   void getImageSpaceScale(ossimDpt &imageSpaceScale) const;

   /**
    * @brief Gets the image space pivot.
    *
    * This is a "chip" operation only.  Will extract the center
    * from the FULLRES keyword
    *
    * Keys: 
    * FULLRES_XYS
    *
    * This will return NaN if not set 
    */
   void getImageSpacePivot(ossimDpt &imageSpacePivot) const;

   /**
    * @brief Gets rotation.
    *
    * @return Rotation in decimal degrees if ROTATION_KW option is set;
    * ossim::nan, if not.
    *
    * @note Throws ossimException on range error.
    */
   ossim_float64 getRotation() const;

   /** @return true if ROTATION_KW option is set; false, if not. */
   bool hasRotation() const;

   /** @return true if UP_IS_UP_KW option is set; false, if not. */
   bool upIsUp() const;

   /** @return true if NORTH_UP_KW option is set; false, if not. */
   bool northUp() const;

   /** @return true if operation is "chip" or identity; false, if not. */
   bool isChipMode() const;

   /** @return true if key is set to true; false, if not. */
   bool keyIsTrue( const std::string& key ) const;

   /**
    * @return The entry number if set.  Zero if ossimKeywordNames::ENTRY_KW not
    * found.
    */
   ossim_uint32 getEntryNumber() const;

   /**
    * @return The zone if set.  Zero if ossimKeywordNames::ZONE_KW not
    * found.
    */
   ossim_int32 getZone() const;

   /**
    * @return The hemisphere if set. Empty string if
    * ossimKeywordNames::HEMISPHERE_KW not found.
    */
   std::string getHemisphere() const;

   /**
    * @return True if any input has a sensor model input, false if all input
    * projections are map projections.
    */
   bool hasSensorModelInput();

   /**
    * @return true if all size cut box width height keywords are true.
    */
   bool hasCutBoxWidthHeight() const;

   /**
   *  @return true if the WMS style cut and the width and height keywords are set
   */
   bool hasWmsBboxCutWidthHeight() const;

   /**
    * @return true if meters, degrees or cut box with width and height option.
    */  
   bool hasScaleOption() const;
   
   /**
    * @return true if three band out is true, false if not.
    */  
   bool isThreeBandOut() const;

   /**
    * @return true if pad thumbnail is true, false if not.
    */  
   bool padThumbnail() const;

   /**
    * @brief Passes reader properties to single image handler if any.
    * @param ih Image handler to set properties on.
    */
   void setReaderProps( ossimImageHandler* ih ) const;
   
   /**
    * @brief Adds application arguments to the argument parser.
    * @param ap Parser to add to.
    */
   void addArguments(ossimArgumentParser& ap);

   void getClipPolygon(ossimGeoPolygon& polygon)const;
   /**
    * @brief Gets the brightness level.
    * 
    * This will return 0.0 if the keyword is not found or if the range check
    * is not between -1.0 and 1.0.
    *
    * @return brightness
    */
   ossim_float64 getBrightness() const;

   /**
    * @brief Gets the contrast level.
    * 
    * This will return 1.0 if the keyword is not found or if the range check
    * is not between 0.0 and 20.0.
    *
    * @return brightness
    */   
   ossim_float64 getContrast() const;

   /**
    * @brief Gets the gamma.
    * 
    * This will return 1.0 if the keyword is not found.
    *
    * @return gamma
    */   
   ossim_float64 getGamma() const;

   /**
    * @brief Gets the sharpen mode.
    *
    * Valid modes: light, medium, heavy
    * 
    * @return sharpness mode
    */
   std::string getSharpenMode() const;

   /**
    * @brief Gets the sharpen mode.
    *
    * Valid percent: values between 0 and 1
    * 
    * @return sharpen percent as string
    */
   std::string getSharpenPercent() const;

   int getHistoMode() const;

   /** @brief Initializes arg parser and outputs usage. */
   void usage(ossimArgumentParser& ap);

   /** @brief Hidden from use copy constructor. */
   ossimChipperUtil( const ossimChipperUtil& obj );

   ossimRefPtr<ossimImageSource> getFinalInput(const ossimIrect& aoi, ossimRefPtr<ossimImageSource> currentSource);
   void setStretch(ossimRefPtr<ossimHistogramRemapper> remapper)const;

   /** @brief Hidden from use assignment operator. */
   const ossimChipperUtil& operator=( const ossimChipperUtil& rhs );

   ossimRefPtr<ossimImageSource> createCombiner()const;

   /** Enumerated operation to perform. */
   ossimChipperOperation m_operation;
   
   /** Hold all options passed into intialize. */
   ossimRefPtr<ossimKeywordlist> m_kwl;

   /** Hold contents of src file if --src is used. */
   ossimRefPtr<ossimKeywordlist> m_srcKwl;

   /**
    * The image geometry.  In chip mode this will be from the input image. So
    * this may or may not have a map projection. In any other mode it
    * will the view or output geometry which will be a map projection.
    */
   ossimRefPtr<ossimImageGeometry> m_geom;

   /**
    * Image view transform(IVT). Only set/used in "chip"(identity) operation as
    * the IVT for the resampler(ossimImageRenderer).
    */
   ossimRefPtr<ossimImageViewAffineTransform> m_ivt;

   /**  Array of dem chains. */
   std::vector< ossimRefPtr<ossimSingleImageChain> > m_demLayer;

   /**  Array of image source chains. */
   std::vector< ossimRefPtr<ossimSingleImageChain> > m_imgLayer;
   
   /**
    *  We need access to the writer so we can support aborting
    */
   mutable ossimRefPtr<ossimImageFileWriter> m_writer;

   /**
   * We need to support changing clips without doing a full initialization.  
   * we will save the ImageSource pointer on first initialization
   */
    ossimRefPtr<ossimImageSource> m_source;

   mutable bool m_viewPortStretchEnabled;

   /**
    * Final container that holds any cuts or stretching, ... etc just before we output 
    */
   ossimRefPtr<ossimConnectableContainer> m_container;
};

#endif /* #ifndef ossimChipperUtil_HEADER */
