//---
// File: ossimInfo.h
// 
// License: MIT
//
// Author:  David Burken
//
// Description: ossimInfo class declaration
//
// See class doxygen descriptions below for more.
// 
//---
// $Id$

#ifndef ossimInfo_HEADER
#define ossimInfo_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimKeywordlist.h>
#include <ossim/base/ossimReferenced.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/imaging/ossimImageHandler.h>
#include <ossim/util/ossimTool.h>
#include <ostream>

class ossimGpt;

/**
 * @brief ossimInfo class.
 *
 * This is a utility class for getting information from the ossim library.
 * This includes information from an image, general library queries, like
 * loaded plugins, and just general stuff like height for point, conversions
 * and so on that are easily obtained through the library.
 */
class OSSIM_DLL ossimInfo : public ossimTool
{
public:
   /** Used by ossimUtilityFactory */
   static const char* DESCRIPTION;

   /** default constructor */
   ossimInfo();

   /** virtual destructor */
   virtual ~ossimInfo();

   /**
    * @brief Adds application arguments to the argument parser.
    * @param ap Parser to add to.
    */
   virtual void setUsage(ossimArgumentParser& ap);

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
   virtual bool initialize(ossimArgumentParser& ap);

   /**
    * @brief execute method.
    *
    * Performs the actual dump of information.  This executes any options
    * set including image operations, i.e. -i -p --dno and so on.
    * 
    * @note Throws ossimException on error.
    */
   virtual bool execute();

   virtual ossimString getClassName() const { return "ossimInfo"; }

   /**
    * @brief handles image options.
    *
    * Handles image type info opions,  i.e. -i -p --dno and so on.
    *
    * @return Number of consumed options.
    */
   ossim_uint32 executeImageOptions(const ossimFilename& file);

   /**
    * @brief getImageInfo Method to open image "file" and get image info
    * in the form of a ossimKeywordlist.
    *
    * Flags turn on various pieces of info.  These equate to options in
    * ossim-info for image information.
    *
    * @param file Image file to get information for.
    * @param dumpFlag      ossim-info -d
    * @param dnoFlag       ossim-info --dno
    * @param imageGeomFlag ossim-info -p
    * @param imageInfoFlag ossim-info -i 
    * @param metaDataFlag  ossim-info -m 
    * @param paletteFlag   ossim-info --palette
    * @param kwl Initialized by this method.
    */
   void getImageInfo( const ossimFilename& file,
                      bool dumpFlag,
                      bool dnoFlag,
                      bool imageGeomFlag,
                      bool imageInfoFlag,
                      bool metaDataFlag,
                      bool paletteFlag,
                      ossimKeywordlist& kwl ) const;

   /**
    * @brief getImageInfo Method to open image "file" and get image info
    * for entry in the form of a ossimKeywordlist.
    *
    * Equivalent of ossim-info -i -p <image> for entry. 
    *
    * Throws ossimException on error if file cannot be opened or entry is
    * invalid.
    *
    * @param file Image file to get information for.
    * @param entry Entry index to open.
    * @return true on success, false on error.
    */
   bool getImageInfo( const ossimFilename& file,
                      ossim_uint32 entry,
                      ossimKeywordlist& kwl ) const;
   
   /**
    * @brief Opens image handler and stores in m_img data member.
    * @param Image to open.
    * @note Throws ossimException if image cannot be opened.
    */
   void openImage(const ossimFilename& file);

   /**
    * @brief Opens image handler and stores in m_img data member.
    * @param Image to open.
    * @note Throws ossimException if image cannot be opened.
    */
   void openImageFromState(const ossimFilename& file);

   /** @brief Closes image if open. */
   void closeImage();

   /**
    * @return Ref pointer to the image handler.  Can be null if an image is
    * not open.
    */
   ossimRefPtr<ossimImageHandler> getImageHandler();

   /** @brief Dumps the image information from ossimInfoFactoryRegistry */
   void prettyPrint(const ossimFilename& file) const;

   /**
    * @brief Dumps the image information from ossimInfoFactoryRegistry to
    * keyword list.
    * @param file Image to dump.
    * @param dnoFlag Dump no overviews. If true do not dump overviews.
    * @param kwl Initialized by this method.
    */
   void dumpImage(const ossimFilename& file,
                  bool dnoFlag,
                  ossimKeywordlist& kwl) const;

   /**
    * @brief Prints factories.
    * @param keywordListFlag If true the result of a saveState will be output
    * for each factory.
    * */
   void printFactories(bool keywordListFlag) const;

   /**
    * @brief Populates keyword list with metadata.
    * This requires open image.
    *
    * @param kwl Keyword list to populate.
    */
   void getImageMetadata(ossimKeywordlist& kwl) const;

   /**
    * @brief Populates keyword list with palette data.
    * This requires open image.
    *
    * @param kwl Keyword list to populate.
    */
   void getImagePalette(ossimKeywordlist& kwl);

   /**
    * @brief Populates keyword list with general image information.
    *
    * This requires open image.
    *
    * @param kwl Keyword list to populate.
    * @param dnoFlag If true no entries flaged as overviews will be output.
    */
   void getImageInfo(ossimKeywordlist& kwl, bool dnoFlag);

   /**
    * @brief Populates keyword list with general image information.
    *
    * @param entry Entry number to select.  Note this is the entry number from
    * the getEntryList call not a simple zero based entry index.
    * 
    * @param kwl Keyword list to populate.
    *
    * @param dnoFlag If true no entries flaged as overviews will be output.
    *
    * This requires open image.
    *
    * @return true if entry info was saved to keyword list false if not.
    */
   bool getImageInfo(ossim_uint32 entry, ossimKeywordlist& kwl, bool dnoFlag);

   /**
    * @brief Populates keyword list with image geometry/projection information.
    *
    * This requires open image.
    *
    * @param kwl Keyword list to populate.
    * @param dnoFlag If true no entries flaged as overviews will be output.
    */
   void getImageGeometryInfo(ossimKeywordlist& kwl, bool dnoFlag);

   /**
    * @brief Populates keyword list with image geometry/projection information.
    *
    * @param entry Entry number to select.  Note this is the entry number
    * from the getEntryList call not a simple zero based entry index.
    * 
    * @param kwl Keyword list to populate.
    *
    * @param dnoFlag If true no entries flaged as overviews will be output.
    *
    * This requires open image.
    *
    * @return true if entry info was saved to keyword list false if not.
    */
   bool getImageGeometryInfo( ossim_uint32 entry, 
                              ossimKeywordlist& kwl, 
                              bool dnoFlag );

   /**
    * @brief Populates keyword list with image center point..
    * @param kwl Keyword list to populate.
    */
   void getCenterImage(ossimKeywordlist& kwl);

   /**
    * @brief Populates keyword list with edge to edge image bounds.
    * @param kwl Keyword list to populate.
    */  
   void getImageBounds(ossimKeywordlist& kwl);

   /**
    * @brief Populates keyword list with image center ground point..
    * @param kwl Keyword list to populate.
    */  
   void getCenterGround(ossimKeywordlist& kwl);

   /**
    * @brief Populates keyword list with ground point for image point.
    *
    * Associated input key values: "img2grd: <x> <y>"
    * Output key: image0.ground_point:  (lat,lon,hgt,datum)
    * 
    * @param kwl Keyword list to populate.
    */
   void getImg2grd(ossimKeywordlist& kwl);

   /**
    * @brief Populates keyword list with image point for grund point.
    *
    * Associated input key values: "grd2img: (lat,lon,hgt,datum)"
    * Output key: image0.image_point:  (x, y)
    * 
    * @param kwl Keyword list to populate.
    */
   void getGrd2img(ossimKeywordlist& kwl);

   /**
    * @brief Populates keyword list with up_is_up_angle.
    *
    * @param kwl Keyword list to populate.
    *
    * This requires open image.
    */
   void getUpIsUpAngle(ossimKeywordlist& kwl);

   /**
    * @brief Populates keyword list with up_is_up_angle.
    *
    * @param entry Entry number to select.  Note this is the entry number
    * from the getEntryList call not a simple zero based entry index.
    * 
    * @param kwl Keyword list to populate.
    *
    * @param dnoFlag If true no entries flaged as overviews will be output.
    *
    * This requires open image.
    */
   void getUpIsUpAngle(ossim_uint32 entry, ossimKeywordlist& kwl);

   /**
    * @brief Populates keyword list with image_to_ground. It will outoput image_point and ground_point
    *
    * @param kwl Keyword list to populate.
    *
    * This requires open image.
    */
   void getImageToGround(ossimKeywordlist& kwl);

   /**
    * @brief Populates keyword list with image_to_ground. It will outoput image_point and ground_point
    *
    * @param entry Entry number to select.  Note this is the entry number
    * from the getEntryList call not a simple zero based entry index.
    * 
    * @param kwl Keyword list to populate.
    *
    * This requires open image.
    */
   void getImageToGround(ossim_uint32 entry, ossimKeywordlist& kwl);
   
   /**
    * @brief Populates keyword list with north_up_angle.
    *
    * @param kwl Keyword list to populate.
    *
    * This requires open image.
    */
   void getNorthUpAngle(ossimKeywordlist& kwl);

   /**
    * @brief Populates keyword list with north_up_angle.
    *
    * @param entry Entry number to select.  Note this is the entry number
    * from the getEntryList call not a simple zero based entry index.
    * 
    * @param kwl Keyword list to populate.
    *
    * @param dnoFlag If true no entries flaged as overviews will be output.
    *
    * This requires open image.
    */
   void getNorthUpAngle(ossim_uint32 entry, ossimKeywordlist& kwl);
   
   /**
    * @brief Populates keyword list with image rectangle.
    *
    * This requires open image.
    *
    * @param kwl Keyword list to populate.
    */
   void getImageRect(ossimKeywordlist& kwl);

   /**
    * @brief Populates keyword list with image rectangle.
    *
    * @param entry Entry number to select.  Note this is the entry number from
    * the getEntryList call not a simple zero based entry index.
    * 
    * @param kwl Keyword list to populate.
    *
    * This requires open image.
    */
   void getImageRect(ossim_uint32 entry, ossimKeywordlist& kwl);

   /**
    * @return true if current open image entry is an overview.
    */
   bool isImageEntryOverview() const;

   /** @brief Checks configuration. */
   void checkConfig() const;

   /**
    * @brief Checks configuration.
    * @param out Output to write to.
    * @return stream
    */
   std::ostream& checkConfig(std::ostream& out) const;

   /** @brief Dumps ossim preferences/configuration data. */
   void printConfiguration() const;

   /**
    * @brief Dumps ossim preferences/configuration data.
    * @param out Output to write to.
    * @return stream
    */
   std::ostream& printConfiguration(std::ostream& out) const;

   /** @brief Dumps datum list to stdout. */
   void printDatums() const;

   /** @brief Dumps datum list to stream. */
   std::ostream& printDatums(std::ostream& out) const;

   /** @brief Prints fonts list to stdout. */
   void printFonts() const;

   /** @brief Prints fonts list to stream. */
   std::ostream& printFonts(std::ostream& out) const;

   /** @brief Converts degrees to radians and outputs to stdout. */
   void deg2rad(const ossim_float64& degrees) const;

   /**
    * @brief Converts degrees to radians and outputs to stream.
    * @param out Output to write to.
    * @return stream
    */
   std::ostream& deg2rad(const ossim_float64& degrees, std::ostream& out) const;
   
   /**
    * @brief Converts ecef point to lat lon height.
    * @param out Output to write to.
    * @return stream
    */
   std::ostream& ecef2llh(const ossimEcefPoint& ecefPoint, std::ostream& out) const;

   /** @brief Converts radians to degrees and outputs to stdout. */
   void rad2deg(const ossim_float64& radians) const;

   /**
    * @brief Converts radians to degrees and outputs to stream. 
    * @param out Output to write to.
    * @return stream
    */
   std::ostream& rad2deg(const ossim_float64& radians, std::ostream& out) const;

   /** @brief Converts feet to meters and outputs to stdout. */
   void ft2mtrs(const ossim_float64& feet, bool us_survey) const;

   /**
    * @brief Converts feet to meters and outputs to stream.
    * @param out Output to write to out.
    * @return stream
    */
   std::ostream& ft2mtrs(const ossim_float64& feet, bool us_survey, std::ostream& out) const;

   /** @brief Converts meters to feet and outputs to stdout. */
   void mtrs2ft(const ossim_float64& meters, bool us_survey) const;

   /**
    * @brief Converts meters to feet and outputs to stream.
    * @param out Output to write to out.
    * @return stream
    */   
   std::ostream& mtrs2ft(const ossim_float64& meters, bool us_survey, std::ostream& out) const;

   /**
    * @brief Get meters per degree for a given latitude and outputs to stdout.
    * @param latitude
    */
   void mtrsPerDeg(const ossim_float64& latitude) const;

   /**
    * @brief Get meters per degree for a given latitude and outputs to stream.
    * @param latitude
    * @param out Output to write to out.
    * @return stream
    */   
   std::ostream& mtrsPerDeg(const ossim_float64& latitude, std::ostream& out) const;

   /**
    * @brief Gets the height for ground point (latitude, longitude). Outputs
    * to stdout.
    * @param gpt Ground point.
    */
   void outputHeight(const ossimGpt& gpt) const;

   /**
    * @brief Gets the height for ground point (latitude, longitude).
    * @param gpt Ground point for requested height.
    * @param kwl Initialized by this with height.
    * @param prefix Optional prefix.
    */
   void getHeight(const ossimGpt& gpt,
                  ossimKeywordlist& kwl,
                  const std::string& prefix) const;

   /**
    * @brief Gets the height for ground point (latitude, longitude). Outputs
    * to out.
    * @param out Output to write to out.
    * @return stream
    */
   std::ostream& outputHeight(const ossimGpt& gpt, std::ostream& out) const;

   /** @brief Prints supported image file extensions to stdout. */
   void printExtensions() const;

   /**
    * @brief Prints supported image file extensions to stream.
    * @param out Output to write to out.
    * @return stream
    */
   std::ostream& printExtensions(std::ostream& out) const;

   /** @brief Prints loaded plugins to stdout. */
   void printPlugins() const;

   /**
    * @brief Prints loaded plugins to stream.
    * @param out Output to write to out.
    * @return stream
    */
   std::ostream& printPlugins(std::ostream& out) const;

   /**
    * @brief Test a plugin load  and outputs to stdout.
    * 
    * @param plugin Plugin to test.
    */
   void testPlugin(const ossimFilename& plugin) const;

   /**
    * @brief Test a plugin load outputs to stream.
    * 
    * @param plugin Plugin to test.
    * @param out Stream to write to.
    * @param stream
    */
   std::ostream& testPlugin(const ossimFilename& plugin, std::ostream& out) const;

   /** @brief Prints overview types to stdout. */
   void printOverviewTypes() const;

   /** @brief Prints overview types to stream. */
   std::ostream& printOverviewTypes(std::ostream& out) const;

   /** @breif Prints projections to stdout. */
   void printProjections() const;

   /** @breif Prints projections to stream. */
   std::ostream& printProjections(std::ostream& out) const;

   /** @brief Prints reader properties to stdout. */
   void printReaderProps() const;

   /** @brief Prints reader properties to stream. */
   std::ostream& printReaderProps(std::ostream& out) const;

   /** @brief Prints resampler filters to stdout. */
   void printResamplerFilters() const;

   /** @brief Prints resampler filters to stream. */
   std::ostream& printResamplerFilters(std::ostream& out) const;

   /** @brief Prints list of available writers to stdout. */
   void printWriters() const;

   /** @brief Prints list of available writers to stream. */
   std::ostream& printWriters(std::ostream& out) const;

   /** @brief Prints writer properties to stdout. */
   void printWriterProps() const;

   /** @brief Prints writer properties to stream. */
   std::ostream& printWriterProps(std::ostream& out) const;

   /** @brief Prints zoom levels to stdout. */
   void printZoomLevelGsds() const;

   /** @brief Prints zoom levels to stream. */
   std::ostream& printZoomLevelGsds(std::ostream& out) const;

   /**
    * @brief Gets the radiometry string, i.e. "8-bit" and so on, from scalar.
    * @param scalar Scalar type.
    * @param s String to initialize.
    */
   void getRadiometry(ossimScalarType scalar, std::string& s) const;

   /**
    * @brief Gets build date.
    * @param s String to initialize.
    */
   void getBuildDate(std::string& s) const;

   /**
    * @brief Gets revision number.
    * @param s String to initialize.
    */
   void getRevisionNumber(std::string& s) const;

   /**
    * @brief Gets version.
    * @param s String to initialize.
    */
   void getVersion(std::string& s) const;

private:

   /**
    * @brief Populates keyword list with metadata.
    * @param ih Pointer to an image handler.
    * @param kwl Keyword list to populate.
    */
   void getImageMetadata( const ossimImageHandler* ih, 
                          ossimKeywordlist& kwl ) const;
 
  /**
   * @brief Populates keyword list with palette data.
   * @param ih Pointer to an image handler.
   * @param kwl Keyword list to populate.
   */
   void getImagePalette( ossimImageHandler* ih, 
                         ossimKeywordlist& kwl ) const;
  
   /**
    * @brief Populates keyword list with general image information.
    * @param ih Pointer to an image handler.
    * @param kwl Keyword list to populate.
    * @param dnoFlag If true no entries flaged as overviews will be output.
    */
   void getImageInfo( ossimImageHandler* ih, 
                      ossimKeywordlist& kwl, 
                      bool dnoFlag ) const;

   /**
    * @brief Populates keyword list with general image information.
    * @param ih Pointer to an image handler.
    * @param entry Entry number to select.  Note this is the entry number from
    * the getEntryList call not a simple zero based entry index.
    * @param kwl Keyword list to populate.
    * @param dnoFlag If true no entries flaged as overviews will be output.
    * @return true if entry info was saved to keyword list false if not.
    */
   bool getImageInfo( ossimImageHandler* ih, 
                      ossim_uint32 entry, 
                      ossimKeywordlist& kwl, 
                      bool dnoFlag ) const;
   
   /**
    * @brief Populates keyword list with image geometry/projection information.
    * @param ih Pointer to an image handler.
    * @param kwl Keyword list to populate.
    * @param dnoFlag If true no entries flaged as overviews will be output.
    */
   void getImageGeometryInfo( ossimImageHandler* ih,
                              ossimKeywordlist& kwl, 
                              bool dnoFlag ) const;

   /**
    * @brief Populates keyword list with image geometry/projection information.
    * @param ih Pointer to an image handler.
    * @param entry Entry number to select.  Note this is the entry number
    * from the getEntryList call not a simple zero based entry index.
    * @param kwl Keyword list to populate.
    * @param dnoFlag If true no entries flaged as overviews will be output.
    * @return true if entry info was saved to keyword list false if not.
    */
   bool getImageGeometryInfo( ossimImageHandler* ih,
                              ossim_uint32 entry, 
                              ossimKeywordlist& kwl, 
                              bool dnoFlag ) const;

   void getCenterImage( ossimImageHandler* ih,
                        ossimKeywordlist& kwl ) const;
   void getCenterImage( ossimImageHandler* ih,
                        ossim_uint32 entry, 
                        ossimKeywordlist& kwl ) const;

   void getImageBounds( ossimImageHandler* ih,
                        ossimKeywordlist& kwl ) const;
   void getImageBounds( ossimImageHandler* ih,
                        ossim_uint32 entry, 
                        ossimKeywordlist& kwl ) const;

   void getCenterGround( ossimImageHandler* ih,
                         ossimKeywordlist& kwl ) const;
   void getCenterGround( ossimImageHandler* ih,
                         ossim_uint32 entry, 
                         ossimKeywordlist& kwl ) const;

   /**
    * @brief Gets gound point from image point.
    *
    * Input key:value "img2grd: <x> <y>"
    *
    * @param Pointer to an image handler.
    * @param kwl Keyword list to populate.
    */
   void getImg2grd( ossimImageHandler* ih,
                    ossimKeywordlist& kwl ) const;
   /**
    * @brief Gets gound point from image point.
    *
    * Input key:value "img2grd: <x> <y>"
    *
    * @param Pointer to an image handler.
    * @param entry Entry number to select. Note this is the entry number
    * from the getEntryList call not a simple zero based entry index.
    * @param kwl Keyword list to populate.
    */
   void getImg2grd( ossimImageHandler* ih,
                    ossim_uint32 entry, 
                    ossimKeywordlist& kwl ) const;

   /**
    * @brief Gets gound point from image point.
    *
    * Input key:value "img2grd: <x> <y>"
    *
    * @param Pointer to an image handler.
    * @param kwl Keyword list to populate.
    */
   void getGrd2img( ossimImageHandler* ih,
                    ossimKeywordlist& kwl ) const;
   /**
    * @brief Gets gound point from image point.
    *
    * Input key:value "img2grd: <x> <y>"
    *
    * @param Pointer to an image handler.
    * @param entry Entry number to select. Note this is the entry number
    * from the getEntryList call not a simple zero based entry index.
    * @param kwl Keyword list to populate.
    */
   void getGrd2img( ossimImageHandler* ih,
                    ossim_uint32 entry, 
                    ossimKeywordlist& kwl ) const;
   
   /**
    * @brief Populates keyword list with up_is_up_angle.
    * @param kwl Keyword list to populate.
    */
   void getUpIsUpAngle( ossimImageHandler* ih,
                        ossimKeywordlist& kwl ) const;

   /**
    * @brief Populates keyword list with up_is_up_angle.
    * @param entry Entry number to select.  Note this is the entry number from
    * the getEntryList call not a simple zero based entry index.
    * @param kwl Keyword list to populate.
    */  
   void getUpIsUpAngle( ossimImageHandler* ih,
                        ossim_uint32 entry, 
                        ossimKeywordlist& kwl ) const;
   
  /**
    * @brief Populates keyword list with north_up_angle.
    * @param kwl Keyword list to populate.
    */
   void getNorthUpAngle( ossimImageHandler* ih,
                         ossimKeywordlist& kwl ) const;

   /**
    * @brief Populates keyword list with north_up_angle.
    * @param entry Entry number to select.  Note this is the entry number from
    * the getEntryList call not a simple zero based entry index.
    * @param kwl Keyword list to populate.
    */  
   void getNorthUpAngle( ossimImageHandler* ih,
                         ossim_uint32 entry, 
                         ossimKeywordlist& kwl ) const;

   /**
    * @brief Populates keyword list with image rectangle.
    *
    * @param kwl Keyword list to populate.
    */
   void getImageRect( ossimImageHandler* ih,
                      ossimKeywordlist& kwl ) const;

   /**
    * @brief Populates keyword list with image rectangle.
    * @param entry Entry number to select.  Note this is the entry number from
    * the getEntryList call not a simple zero based entry index.
    * @param kwl Keyword list to populate.
    */
   void getImageRect( ossimImageHandler* ih,
                      ossim_uint32 entry, 
                      ossimKeywordlist& kwl ) const;

   /**
    * @brief Populates keyword list with rgb bands if available.
    *
    * This is image handler specific and within image handler specific to
    * internal metadata, e.g. NITF IREPBAND keys.
    *
    * Example of key:value:
    * rgb_bands:(2,1,0)
    *
    * @param ih Pointer to image handler.
    * @param entry Current entry of image handler.
    * @param kwl Keyword list to populate.
    * @return true on success; false, on error.
    */
   bool getRgbBands( ossimImageHandler* ih,
                     ossim_uint32 entry,
                     ossimKeywordlist& kwl ) const;

   /** @return true if current open image entry is an overview. */
   bool isImageEntryOverview( const ossimImageHandler* ih ) const;

   /**
    * @brief Convert keyword list to xml then outputs to standard out.
    * @param kwl Keyword list to output.
    */
   void outputXml( const ossimKeywordlist& kwl ) const;
   
   /**
    * @brief Convert keyword list to xml then outputs to file.
    * @param kwl Keyword list to output.
    * @param file Output file to write to.
    */
   void outputXml( const ossimKeywordlist& kwl, const ossimFilename& file ) const;
  
  /**
   * @brief Opens image.
   * @param Image to open.
   * @return ossimRefPtr with image handler.
   * @note Throws ossimException if image cannot be opened.
   */
   ossimRefPtr<ossimImageHandler> openImageHandler(const ossimFilename& file) const;
   
   /** @return true if key is set to true; false, if not. */
   bool keyIsTrue( const std::string& key ) const;

   /** Holds the open image. */
   ossimRefPtr<ossimImageHandler> m_img;
};

#endif /* #ifndef ossimInfo_HEADER */
