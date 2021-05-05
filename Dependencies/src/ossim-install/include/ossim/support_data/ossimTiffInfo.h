//----------------------------------------------------------------------------
//
// License: MIT
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: TIFF Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimTiffInfo_HEADER
#define ossimTiffInfo_HEADER 1

#include <ossim/base/ossimConstants.h>
#include <ossim/support_data/ossimInfoBase.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimIosFwd.h>
#include <iosfwd>
#include <string>
#include <vector>
#include <memory>

class ossimDpt;
class ossimEndian;
class ossimKeywordlist;
class ossimTieGptSet;

/**
 * @brief TIFF info class.
 *
 * Encapsulates the listgeo functionality.
 */
class OSSIM_DLL ossimTiffInfo : public ossimInfoBase
{
public:

   enum WordType
   {
      TWO_OR_EIGHT  = 0,
      FOUR_OR_EIGHT = 1
   };
   
   /** default constructor */
   ossimTiffInfo();

   /** virtual destructor */
   virtual ~ossimTiffInfo();

   /**
    * @brief open method.
    *
    * @param file File name to open.
    *
    * @return true on success false on error.
    */
   virtual bool open(const ossimFilename& file);
   virtual bool open( std::shared_ptr<ossim::istream>& str,
                      const std::string& connectionString );

   /**
    * Print method.
    *
    * @param out Stream to print to.
    * 
    * @return std::ostream&
    */
   virtual std::ostream& print(std::ostream& out) const;

   /**
    * @brief Print method.
    * 
    * Print method that takes a stream that should be positions at the start
    * of tiff stream. This was added to see embedded information on embedded
    * geotiff's in a geojp2 file.
    *
    * @param out Stream to print to.
    * 
    * @return std::ostream&
    */
   virtual std::ostream& print(std::istream& inStr,
                               std::ostream& outStr) const;

   /**
    *  @brief Extracts geometry info to keyword list.  This method assumes
    *  that open has been called and theFile is set.
    *  
    *  Populates the keyword list with image geometry information.  This
    *  method is used to relay projection/model information to users.
    *
    *  @param geomKwl Keyword list that will be initialized with geometry info.
    *  Returns true if geometry info is present, false if not.
    *
    *  @param entryIndex Entry to get geometry from. 
    */
   bool getImageGeometry(ossimKeywordlist& geomKwl,
                         ossim_uint32 entryIndex) const;

   /**
    *  @brief Extracts geometry info from stream to keyword list.
    *  
    *  Populates the keyword list with image geometry information.  This
    *  method is used to relay projection/model information to users.
    *
    *  @note The str should be position at start of tiff portion. This
    *  was written to allow passing a stream positioned at an embedded tiff
    *  image in a jp2 file.
    *
    *  @param str Stream position at start of tiff image.
    *
    *  @param geomKwl Keyword list that will be initialized with geometry info.
    *  Returns true if geometry info is present, false if not.
    *
    *  @param entryIndex Entry to get geometry from. 
    */
   bool getImageGeometry(std::istream& str,
                         ossimKeywordlist& geomKwl,
                         ossim_uint32 entryIndex) const;

   /**
    * @brief extracts geometry info from stream to keyword list.
    *  
    * Populates geomKwl from gtiffKwl.
    *
    * @param gtiffKwl Raw keywordlist extracted from tiff/geotiff keys.
    * 
    * @param geomKwl Keyword list that will be initialized with geometry info.
    * Returns true if geometry info is present, false if not.
    *
    * @param entryIndex Entry to get geometry from. 
    */
   bool getImageGeometry(const ossimKeywordlist& gtiffKwl,
                         ossimKeywordlist& geomKwl,
                         ossim_uint32 entryIndex) const;

   /**
    * @brief Extracts geometry info from geotiff keys to keyword list.
    *
    * Helper method used to extract keywords from geotiff keys embedded in
    * files, e.g. LAS data.  Because this is just the geotiff keys, the output
    * will be missing the image part, i.e. line, samples, radiometry...
    *
    * This output is intended to be usable to get a projection from a call to
    * ossimProjectionFactoryRegistry::createProjection(kwl).
    *
    * Internally calls printGeoKeys and then the private getImageGeometry.
    *
    * @param geoKeyLength Length of geoKeyBlock.
    * @param geoKeyBlock Pointer to array of keys.
    * @param geoDoubleLength Length of geoDoubleBlock.
    * @param geoDoubleBlock Pointer to array of double values.
    * @param geoAsciiLength Length of geoAsciiBlock.
    * @param geoAsciiBlock Pointer to array of ascii values.
    * @param projKwl The keyword list to initialize.
    *
    * @return Because the resulting keyword list (geomKwl) is not complete
    * there will be no return.
    */
   void getImageGeometry(ossim_uint64   geoKeyLength,
                         ossim_uint16*  geoKeyBlock,
                         ossim_uint64   geoDoubleLength,
                         ossim_float64* geoDoubleBlock,
                         ossim_uint64   geoAsciiLength,
                         ossim_int8*    geoAsciiBlock,
                         ossimKeywordlist& geomKwl) const;

   /**
    * @brief Prints geo keys to stream.
    *
    * Helper method used to extract keywords from geotiff keys embedded in
    * files, e.g. LAS data.  Because this is just the geotiff keys, the output
    * will be missing the image part, i.e. line, samples, radiometry...
    *
    * This output is the raw tiff tags/geotiff keys not necessarily useable to construct
    * a projection from.
    *
    * @param out Stream to dump to.
    * @param prefix if any.  This will be added to the keys, e.g.
    * tiff0.coord_trans where "tiff0." is the prefix and "coord_trans" the key.
    * @param geoKeyLength Length of geoKeyBlock.
    * @param geoKeyBlock Pointer to array of keys.
    * @param geoDoubleLength Length of geoDoubleBlock.
    * @param geoDoubleBlock Pointer to array of double values.
    * @param geoAsciiLength Length of geoAsciiBlock.
    * @param geoAsciiBlock Pointer to array of ascii values.
    */
   std::ostream& printGeoKeys(std::ostream& out,
                              const std::string& prefix,
                              ossim_uint64   geoKeyLength,
                              ossim_uint16*  geoKeyBlock,
                              ossim_uint64   geoDoubleLength,
                              ossim_float64* geoDoubleBlock,
                              ossim_uint64   geoAsciiLength,
                              ossim_int8*    geoAsciiBlock) const;

   /**
    * @brief Prints tag 50844
    *
    * Note: This is in RPC00B polynomial format.
    *
    * http://geotiff.maptools.org/rpc_prop.html 
    */
   std::ostream& printRpcs(std::ostream& out,
                           const std::string& prefix,
                           ossim_uint16 type,
                           ossim_uint64 count,
                           ossim_uint8* valueArray) const;

private:
   
   /** Initializes s reference.  Does byte swapping as needed. */
   void readShort(ossim_uint16& s, std::istream& str) const;
   
   /** Initializes l reference.  Does byte swapping as needed. */
   void readLong(ossim_uint32& l, std::istream& str) const;
   
   /** Initializes l reference.  Does byte swapping as needed. */
   void readLongLong(ossim_uint64& l, std::istream& str) const;
   
   /** Attempts to parse the pcs code (3072). */
   void parsePcsCode(int code);
   
   /** Attempts to parse the projection geo key (3074). */
   void parseProjGeoCode(int code);
   
   /**
    *  Converts double passed in to meters if needed.  The conversion is
    *  base on "theLiniarUnitsCode".
    */
   double convert2meters(double d) const;
   
   /**
    * This will read either 4 or 8 bytes depending on the version and
    * initialize offset.
    *
    * @return true if stream is good, false if not.
    */
   bool getOffset(std::streamoff& offset, std::istream& str,
                  ossim_uint16 version) const;
   
   /**
    * This will read either 2 , 4 or 8 bytes depending on the version and
    * WORD_TYPE.
    *
    * @return true if stream is good, false if not.
    */
   bool getValue(ossim_uint64& val, std::istream& str, WordType type,
                 ossim_uint16 version) const;
   
   ossim_uint64 getArraySizeInBytes(ossim_uint64 length,
                                    ossim_uint16 type) const;
   
   /**
    * @return The byte size of each tiff type or 0 if unhandled.
    */
   ossim_uint16 getTypeByteSize(ossim_uint16 type) const;
   
   /**
    * Eats the value field.  Either 4 or 8 bytes depending on the version.
    */
   void eatValue(std::istream& str, ossim_uint16 version) const;
   
   void swapBytes(ossim_uint8* v, ossim_uint16 type, ossim_uint64 count) const;
   
   /** Initializes v. */
   template <class T> void getArrayValue(T& v,
                                         ossim_uint8* array,
                                         ossim_uint64 position) const;
   
   std::ostream& print(std::ostream& out,
                       const std::string&  prefix,
                       ossim_uint64   tagIdx,
                       ossim_uint16   tag,
                       ossim_uint16   type,
                       ossim_uint64   count,
                       ossim_uint64   arraySizeInBytes,
                       ossim_uint8*   valueArray) const;
   
   std::ostream& printValue(std::ostream& out,
                            ossim_uint16 type,
                            ossim_uint8* valueArray) const;
   
   std::ostream& printArray(std::ostream& out,
                            ossim_uint16 type,
                            ossim_uint64 count,
                            ossim_uint8* valueArray) const;
   
   std::ostream& printGdalMetadata(std::ostream& out,
                                   const std::string& prefix,
                                   ossim_uint64 count,
                                   ossim_uint8* valueArray) const;

   /** @brief Prints OTIFFTAG_XMLPACKET (700) */ 
   std::ostream& printXmpMetadata(std::ostream& out,
                                  const std::string& prefix,
                                  ossim_uint64 count,
                                  ossim_uint8* valueArray) const;

   /** @brief Prints key 1024 */
   std::ostream& printModelType(std::ostream& out,
                                const std::string& prefix,
                                ossim_uint16 code) const;

   /** @brief Prints key 1025 */
   std::ostream& printRasterType(std::ostream& out,
                                 const std::string& prefix,
                                 ossim_uint16 code) const;

   /** @brief Prints key 2054 */
   std::ostream& printAngularUnits(std::ostream& out,
                                   const std::string& prefix,
                                   ossim_uint16 code) const;

   /** @brief Prints key 3075 */
   std::ostream& printCoordTrans(std::ostream& out,
                                  const std::string& prefix,
                                  ossim_uint16 code) const;

   /** @brief Prints key from code from section 6.3.1.3 */
   std::ostream& printLinearUnits(std::ostream& out,
                                  const std::string& prefix,
                                  const std::string& key,
                                  ossim_uint16 code) const;

   /** @brief Prints tag 274 */
   std::ostream& printOrientation(std::ostream& out,
                                  const std::string& prefix,
                                  ossim_uint16 code) const;
  
   /** @brief adds imageN. to prefix where N is zero base directory index. */
   void getDirPrefix(ossim_int32 dirIndex, std::string& prefix) const;

   /**
    * @brief Gets the required pixel scale from keyword list looking for the
    * key model_pixel_scale.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param scale The point to initialize.
    * @return true on success, false on error.
    */
   bool getPixelScale(const ossimString& gtiffPrefix,
                      const ossimKeywordlist& gtiffKwl,
                      ossimDpt& scale) const;
   
   /**
    * @brief Gets the model_tie_point array.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param ties The array to initialize.
    * @return true on success, false on error.
    */
   bool getTiePoint(const ossimString& gtiffPrefix,
                    const ossimKeywordlist& gtiffKwl,
                    std::vector<ossim_float64>& ties) const;

   /**
    * @brief Gets the model_transform array.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param ties The array to initialize.
    * @return true on success, false on error.
    */
   bool getModelTransform(const ossimString& gtiffPrefix,
                          const ossimKeywordlist& gtiffKwl,
                          std::vector<ossim_float64>& xfrm) const;

   /**
    * @brief Extracts float values from keyword list that match key.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param key to look for.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param floats Array to stuff.  Will have size of zero on failure.
    * @return true on success, false on error.  If size of array is zero it
    * will return false.
    */
   bool  getFloats(const ossimString& gtiffPrefix,
                   const ossimString& key,
                   const ossimKeywordlist& gtiffKwl,
                   std::vector<ossim_float64>& floats) const;

   /**
    * @brief Extracts float values from lookup line and puts in floats array.
    * @param lookup Line to parse like:
    * "0.00138888888889 0.00138888888889, 0.0"
    * @param floats Array to stuff.  Will have size of zero on failure.
    */
   bool getFloats(const ossimString& line,
                  std::vector<ossim_float64>& floats) const;

   /**
    * @brief Gets the datum code matching code from the keyword list as a
    * string.
    *
    * This checks for "geodetic_datum"(geotiff key 2050), then
    * "gcs_code"(geotiff key 2048) if first lookup faild.
    * 
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param datumCode The code to initialize.
    * @return true on success, false on error.
    */
   bool getDatumCode(const ossimString& gtiffPrefix,
                     const ossimKeywordlist& gtiffKwl,
                     ossimString& datumCode) const;

   /**
    * @brief Gets the datum code.
    * @param code Code from either "geodetic_datum"(geotiff key 2050),
    * or "gcs_code"(geotiff key 2048).
    * @param datumCode The code to initialize.
    * @return true on success, false on error.
    */
   bool getDatumCode(ossim_int32 code, ossimString& datumCode) const;

   /**
    * @brief Gets the pcs code from the keyword list as a string.
    *
    * If the lookup of "pcs_code" returns false then the "pcs_citation" is then
    * checked.
    * 
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param pcsCode The code to initialize.
    * @return true on success, false on error.
    */
   bool getPcsCode(const ossimString& gtiffPrefix,
                   const ossimKeywordlist& gtiffKwl,
                   ossimString& pcsCode) const;

   /**
    * @brief Gets units from keyword list as a string. This will be angular
    * or linear base on the model type.  The default is meters if not found.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param linearUnits The string to initialize with units.
    * @return true on success, false on error.
    */
   bool getUnits(const ossimString& gtiffPrefix,
                 const ossimKeywordlist& gtiffKwl,
                 ossimString& units) const;
   /**
    * @brief Gets the linear units from keyword list as a string.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param linearUnits The string to initialize with units.
    * @return true on success, false on error.
    */
   bool getLinearUnits(const ossimString& gtiffPrefix,
                       const ossimKeywordlist& gtiffKwl,
                       ossimString& linearUnits) const;

   /**
    * @brief Gets the vertical units from keyword list as a string.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param linearUnits The string to initialize with units.
    * @return true on success, false on error.
    */
   bool getVerticalUnits(const ossimString& gtiffPrefix,
                         const ossimKeywordlist& gtiffKwl,
                         ossimString& verticalUnits) const;

   /**
    * @brief Gets the units from keyword list as a string.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param angularUnit The string to initialize with units.
    * @return true on success, false on error.
    */
   bool getAngularUnits(const ossimString& gtiffPrefix,
                        const ossimKeywordlist& gtiffKwl,
                        ossimString& units) const;


   /**
    * @brief Gets the pixel type (point or area) from keyword list as a string.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param unit The string to initialize with pixel type.
    * @return true on success, false on error.
    */   
   bool getPixelType(const ossimString& gtiffPrefix,
                     const ossimKeywordlist& gtiffKwl,
                     ossimString& pixelType) const;

   /**
    * @brief Gets the model type from keyword list as a string.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param modelType The string to initialize with pixel type.
    * @return true on success, false on error.
    */   
   bool getModelType(const ossimString& gtiffPrefix,
                     const ossimKeywordlist& gtiffKwl,
                     ossimString& modeType) const;

   /**
    * @brief Gets the ossimProjection from keyword list as a string.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param ossimProj The string to initialize with pixel type.
    * @return true on success, false on error.
    */   
   bool getOssimProjectionName(const ossimString& gtiffPrefix,
                               const ossimKeywordlist& gtiffKwl,
                               ossimString& ossimProj) const;
   
   /**
    * @brief Gets the number of lines from keyword list.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @return Number of lines or 0 if lines not found.
    */   
   ossim_uint32 getLines(const ossimString& gtiffPrefix,
                         const ossimKeywordlist& gtiffKwl) const;

   /**
    * @brief Gets the number of samples from keyword list.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @return Number of lines or 0 if lines not found.
    */   
   ossim_uint32 getSamples(const ossimString& gtiffPrefix,
                           const ossimKeywordlist& gtiffKwl) const;

   /**
    * @brief Gets the first standard parallel from keyword list.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param value Initialized with result.
    */   
   bool getStdParallelOne(const ossimString& gtiffPrefix,
                          const ossimKeywordlist& gtiffKwl,
                          ossimString& value) const;

   /**
    * @brief Gets the second standard parallel from keyword list.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param value Initialized with result.
    */
   bool getStdParallelTwo(const ossimString& gtiffPrefix,
                          const ossimKeywordlist& gtiffKwl,
                          ossimString& value) const;
   
   /**
    * @brief Gets the false easting from keyword list.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param value Initialized with result.
    */
   bool getFalseEasting(const ossimString& gtiffPrefix,
                        const ossimKeywordlist& gtiffKwl,
                        ossimString& value) const;

   /**
    * @brief Gets the false northing from keyword list.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param value Initialized with result.
    */
   bool getFalseNorthing(const ossimString& gtiffPrefix,
                         const ossimKeywordlist& gtiffKwl,
                         ossimString& value) const;

   /**
    * @brief Gets the false easting/northing from keyword list.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param eastingNorthing Initialized with result.
    */
   bool getFalseEastingNorthing(const ossimString& gtiffPrefix,
                                const ossimKeywordlist& gtiffKwl,
                                ossimDpt& eastingNorthing) const;

   /**
    * @brief Gets the scale factor from keyword list.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param value Initialized with result.
    */
   bool getScaleFactor(const ossimString& gtiffPrefix,
                       const ossimKeywordlist& gtiffKwl,
                       ossim_float64& value) const;

   /**
    * @brief Gets the origin of latitude from keyword list.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param value Initialized with result.
    */
   bool getOriginLat(const ossimString& gtiffPrefix,
                     const ossimKeywordlist& gtiffKwl,
                     ossim_float64& value) const;

   /**
    * @brief Gets the central meridian from keyword list.
    * @param gtiffPrefix Prefix for gtiffKwl.
    * @param gtiffKwl The keyword list with all tiff keywords.
    * @param value Initialized with result.
    */
   bool getCentralMeridian(const ossimString& gtiffPrefix,
                           const ossimKeywordlist& gtiffKwl,
                           ossim_float64& value) const;

   /**
    * @brief Split tie points into sets of six tie sets.
    * @param tie points.
    * @param width Image width.
    * @param height Image height.
    * @param tieSet Initialized with result.
    */
   void getTieSets(const std::vector<ossim_float64>& ties,
                   ossim_uint32 width,
                   ossim_uint32 height,
                   ossimTieGptSet& tieSet) const;


   /**
    * @brief Check tie points to see if they are zero base or one based.
    * @param tie points.
    * @param width Image width.
    * @param height Image height.
    * @return true if one based, false if not.
    */
   bool hasOneBasedTiePoints(const std::vector<ossim_float64>& ties,
                             ossim_uint32 width,
                             ossim_uint32 height) const;
   std::ostream &printDigitalGlobe(std::ostream &out,
                                   const std::string &prefix) const;
   std::ostream &printPleiades(std::ostream &out,
                               const std::string &prefix) const;

   std::string m_connectionString;
   mutable std::shared_ptr<ossim::istream> m_inputStream;
   //ossimFilename          theFile;
   mutable ossimEndian* m_endian;
};

#endif /* End of "#ifndef ossimTiffInfo_HEADER" */
