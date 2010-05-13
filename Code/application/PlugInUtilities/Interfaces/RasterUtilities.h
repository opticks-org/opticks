/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERUTILITIES_H
#define RASTERUTILITIES_H

#include "AppConfig.h"
#include "DimensionDescriptor.h"
#include "EnumWrapper.h"
#include "TypesFile.h"

#if defined(WIN_API)
#include <windows.h>
#include <float.h>
#define FINITE _finite
#define isnanf(x) (_finite(x) == 0)
#elif defined(SOLARIS)
   #include <ieeefp.h>
   #define FINITE finite
#elif defined(LINUX)
   #include <ieee754.h>
   #include <math.h>
   #define FINITE finite
#endif

#include <limits>
#include <string>
#include <vector>

class DataDescriptor;
class DataElement;
class DynamicObject;
class FileDescriptor;
class Progress;
class RasterDataDescriptor;
class RasterDataDescriptor;
class RasterElement;
class RasterFileDescriptor;

/**
 * This namespace contains a number of convenience functions
 * for dealing with raster datasets.  This includes creation of
 * DataDescriptors, FileDescriptors, and DataElements.
 */
namespace RasterUtilities
{
   /**
    * Returns a vector of DimensionDescriptors, suitable for use in
    * a RasterDataDescriptor or RasterFileDescriptor.
    *
    * @param count
    *        The number of DimensionDescriptors to generate.
    * @param setOriginalNumbers
    *        if \c true, the original numbers will be set from 0 to count-1
    *        matching their index location within the vector.
    * @param setActiveNumbers
    *        if \c true, the active numbers will be set from 0 to count-1
    *        matching their index location within the vector.
    * @param setOnDiskNumbers
    *        if \c true, the on-disk numbers will be set from 0 to count-1
    *        matching their index location within the vector.
    *
    * @return A vector of size count, containing the requested
    *         DimensionDescriptors.
    */
   std::vector<DimensionDescriptor> generateDimensionVector(unsigned int count,
      bool setOriginalNumbers = true, bool setActiveNumbers = false, bool setOnDiskNumbers = false);

   /**
    * Determine the load skip factor if any between each DimensionDescriptor.  This
    * function will determine the uniform skip factor between each DimensionDescriptor::getOnDiskNumber()
    * which represents any skip factor used to load the data.
    *
    * @param values
    *        The DimensionDescriptors to determine the skip factor of.
    * @param skipFactor
    *        This will be set to the calculated skip factor.  A skipFactor of 0, indicates
    *        the on-disk numbers are the following: 0, 1, 2, 3, 4, 5, 6.  A skip factor of 1,
    *        indicates the on-disk numbers are the following: 0, 2, 4, 6.
    *
    * @return true, if a uniform skip factor could be determined, false otherwise.
    */
   bool determineSkipFactor(const std::vector<DimensionDescriptor>& values, unsigned int& skipFactor);

   /**
    * Determine the export skip factor if any between each DimensionDescriptor.  This
    * function will determine the uniform skip factor between each DimensionDescriptor::getActiveNumber()
    * which represents any skip factor used when exporting the data.
    *
    * @param values
    *        The DimensionDescriptors to determine the skip factor of.
    * @param skipFactor
    *        This will be set to the calculated skip factor.  A skipFactor of 0, indicates
    *        the active numbers are the following: 0, 1, 2, 3, 4, 5, 6.  A skip factor of 1,
    *        indicates the active numbers are the following: 0, 2, 4, 6.
    *
    * @return true, if a uniform skip factor could be determined, false otherwise.
    */
   bool determineExportSkipFactor(const std::vector<DimensionDescriptor>&values, unsigned int& skipFactor);

   /**
    * Returns a subset DimensionDescriptor vector.
    *
    * @param origValues
    *        The DimensionDescriptor vector that should be subset.
    * @param start
    *        The DimensionDescriptor that specifies the start of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the start of the subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the beginning of the origVector will be used.
    * @param stop
    *        The DimensionDescriptor that specifies the end of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the end of the subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the end of the origVector will be used.
    * @param skipFactor
    *        Specifies the number of DimensionDescriptors that should be skipped
    *        while creating the subset.  For example, a skipFactor of 0 would include
    *        the following entries of the vector: 0, 1, 2, 3, 4, 5, 6. For example, a 
    *        skipFactor of 1 would include the following entries of the vector: 0, 2, 4, 6.
    *
    * @return A new DimensionDescriptor vector that has been subset as specified.
    */
   std::vector<DimensionDescriptor> subsetDimensionVector(const std::vector<DimensionDescriptor>& origValues,
      const DimensionDescriptor& start, const DimensionDescriptor& stop, unsigned int skipFactor = 0);

   /**
    * Returns a FileDescriptor to go with a passed in DataDescriptor.
    *
    * @param pDd
    *        The DataDescriptor to generate a FileDescriptor for.
    *        If a RasterDataDescriptor, the DimensionDescriptors
    *        will be modified to set on-disk numbers.
    * @param filename
    *        The filename for the FileDescriptor
    * @param datasetLocation
    *        The location within the file which contains the dataset.
    * @param endian
    *        The endianness of the file.
    *
    * @return A newly created FileDescriptor, of an appropriate subclass
    *         to match pDd.  It has the appropriate fields copied from
    *         the source DataDescriptor.  Any special fields (like GCPs,
    *         preline bytes, etc) must be set manually.  The returned
    *         FileDescriptor is not set into the DataDescriptor.
    */
   FileDescriptor *generateFileDescriptor(DataDescriptor *pDd, 
      const std::string &filename, const std::string &datasetLocation, 
      EndianType endian);   


   /**
    * Returns a FileDescriptor to go with a passed in DataDescriptor and sets
    * the FileDescriptor onto the provided DataDescriptor.
    *
    * @param pDd
    *        The DataDescriptor to generate a FileDescriptor for.
    *        If a RasterDataDescriptor, the DimensionDescriptors
    *        will be modified to set on-disk numbers.
    * @param filename
    *        The filename for the FileDescriptor
    * @param datasetLocation
    *        The location within the file which contains the dataset.
    * @param endian
    *        The endianness of the file.
    *
    * @return A newly created FileDescriptor, of an appropriate subclass
    *         to match pDd.  It has the appropriate fields copied from
    *         the source DataDescriptor.  Any special fields (like GCPs,
    *         preline bytes, etc) must be set manually.  The returned
    *         FileDescriptor is set into the DataDescriptor.
    */
   FileDescriptor *generateAndSetFileDescriptor(DataDescriptor *pDd, 
      const std::string &filename, const std::string &datasetLocation, 
      EndianType endian);

   /**
    * Returns a FileDescriptor that can be provided to an exporter
    * to export the data represented by the given DataDescriptor.
    *
    * @param pDd
    *        The DataDescriptor to generate a suitable FileDescriptor
    *        necessary for export of.
    * @param filename
    *        The name of the file that the data should be exported to.
    *
    * @return A newly created FileDescriptor, of an appropriate subclass
    *         to match pDd.  The returned FileDescriptor is not set into the
    *         DataDescriptor and is only suitable for an exporter to use.
    */
   FileDescriptor* generateFileDescriptorForExport(const DataDescriptor* pDd,
      const std::string &filename);

   /**
    * Returns a FileDescriptor that can be provided to an exporter
    * to export the data that is some subset of the given DataDescriptor.
    *
    * @param pDd
    *        The DataDescriptor to generate a suitable FileDescriptor
    *        necessary for export of.
    * @param filename
    *        The name of the file that the data should be exported to.
    * @param startRow
    *        The DimensionDescriptor that specifies the starting row of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the start of the row subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the first row will be used.
    * @param stopRow
    *        The DimensionDescriptor that specifies the ending row of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the end of the row subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the last row will be used.
    * @param rowSkipFactor
    *        Specifies the number of rows that should be skipped
    *        while creating the subset.  For example, a skipFactor of 0 would include
    *        the following row positions: 0, 1, 2, 3, 4, 5, 6. For example, a 
    *        skipFactor of 1 would include the following row positions: 0, 2, 4, 6.
    * @param startCol
    *        The DimensionDescriptor that specifies the starting column of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the start of the column subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the first column will be used.
    * @param stopCol
    *        The DimensionDescriptor that specifies the ending column of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the end of the column subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the last column will be used.
    * @param colSkipFactor
    *        Specifies the number of columns that should be skipped
    *        while creating the subset.  For example, a skipFactor of 0 would include
    *        the following column positions: 0, 1, 2, 3, 4, 5, 6. For example, a 
    *        skipFactor of 1 would include the following column positions: 0, 2, 4, 6.
    * @param subsetBands
    *        Specifies the list of bands that should be included in the subset.
    *        The vector must be a subset of the DataDescriptor::getBands() value. An
    *        empty vector can be provided, in which case no bands will be removed
    *        during the subset.
    * @return A newly created FileDescriptor, of an appropriate subclass
    *         to match pDd.  The returned FileDescriptor only includes
    *         the specified subset and is not set into the
    *         DataDescriptor and is only suitable for an exporter to use.
    */
   FileDescriptor* generateFileDescriptorForExport(const DataDescriptor* pDd,
      const std::string &filename, const DimensionDescriptor& startRow,
      const DimensionDescriptor& stopRow,
      unsigned int rowSkipFactor,
      const DimensionDescriptor& startCol,
      const DimensionDescriptor& stopCol,
      unsigned int colSkipFactor,
      const std::vector<DimensionDescriptor>& subsetBands = std::vector<DimensionDescriptor>());

   /**
    * Returns a FileDescriptor that can be provided to an exporter
    * to export the data that is some subset of the given DataDescriptor.
    *
    * @param pDd
    *        The DataDescriptor to generate a suitable FileDescriptor
    *        necessary for export of.
    * @param filename
    *        The name of the file that the data should be exported to.
    * @param startRow
    *        The DimensionDescriptor that specifies the starting row of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the start of the row subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the first row will be used.
    * @param stopRow
    *        The DimensionDescriptor that specifies the ending row of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the end of the row subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the last row will be used.
    * @param rowSkipFactor
    *        Specifies the number of rows that should be skipped
    *        while creating the subset.  For example, a skipFactor of 0 would include
    *        the following row positions: 0, 1, 2, 3, 4, 5, 6. For example, a 
    *        skipFactor of 1 would include the following row positions: 0, 2, 4, 6.
    * @param startCol
    *        The DimensionDescriptor that specifies the starting column of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the start of the column subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the first column will be used.
    * @param stopCol
    *        The DimensionDescriptor that specifies the ending column of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the end of the column subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the last column will be used.
    * @param colSkipFactor
    *        Specifies the number of columns that should be skipped
    *        while creating the subset.  For example, a skipFactor of 0 would include
    *        the following column positions: 0, 1, 2, 3, 4, 5, 6. For example, a 
    *        skipFactor of 1 would include the following column positions: 0, 2, 4, 6.
    * @param startBand
    *        The DimensionDescriptor that specifies the starting band of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the start of the band subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the first band will be used.
    * @param stopBand
    *        The DimensionDescriptor that specifies the ending band of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the end of the band subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the last band will be used.
    * @param bandSkipFactor
    *        Specifies the number of band that should be skipped
    *        while creating the subset.  For example, a skipFactor of 0 would include
    *        the following band positions: 0, 1, 2, 3, 4, 5, 6. For example, a 
    *        skipFactor of 1 would include the following band positions: 0, 2, 4, 6.
    * @return A newly created FileDescriptor, of an appropriate subclass
    *         to match pDd.  The returned FileDescriptor only includes
    *         the specified subset and is not set into the
    *         DataDescriptor and is only suitable for an exporter to use.
    */
   FileDescriptor* generateFileDescriptorForExport(const DataDescriptor* pDd,
      const std::string &filename, const DimensionDescriptor& startRow,
      const DimensionDescriptor& stopRow,
      unsigned int rowSkipFactor,
      const DimensionDescriptor& startCol,
      const DimensionDescriptor& stopCol,
      unsigned int colSkipFactor,
      const DimensionDescriptor& startBand,
      const DimensionDescriptor& stopBand,
      unsigned int bandSkipFactor);

   /**
    * Generate a populated RasterDataDescriptor to match the given 
    * parameters. The new RasterDataDescriptor will inherit the
    * classification settings of the parent DataElement unless the
    * parent element is \c NULL, in which case the classification will
    * be set to the system's highest level.
    *
    * @param name
    *        The name for the new RasterDataDescriptor.
    * @param pParent
    *        The parent element for the new RasterDataDescriptor.
    * @param rows
    *        The number of rows for the new RasterDataDescriptor.
    * @param columns
    *        The number of columns for the new RasterDataDescriptor.
    * @param bands
    *        The number of bands for the new RasterDataDescriptor.
    * @param interleave
    *        The interleave for the new RasterDataDescriptor.
    * @param encoding
    *        The encoding for the new RasterDataDescriptor.
    * @param location
    *        The processing location for the new RasterDataDescriptor.
    *
    * @return A fully populated RasterDataDescriptor, without a
    *         FileDescriptor.  Note that original numbers for 
    *         rows, columns, and bands will go from 0 to n.
    */
   RasterDataDescriptor* generateRasterDataDescriptor(const std::string& name, DataElement* pParent,
      unsigned int rows, unsigned int columns, unsigned int bands, InterleaveFormatType interleave,
      EncodingType encoding, ProcessingLocation location);

   /**
    * Generate a populated RasterDataDescriptor to match the given 
    * parameters assuming the data only has one band. The new
    * RasterDataDescriptor will inherit the classification settings
    * of the parent DataElement unless the parent element is \c NULL, in
    * which case the classification will be set to the system's highest level.
    *
    * @param name
    *        The name for the new RasterDataDescriptor.
    * @param pParent
    *        The parent element for the new RasterDataDescriptor.
    * @param rows
    *        The number of rows for the new RasterDataDescriptor.
    * @param columns
    *        The number of columns for the new RasterDataDescriptor.
    * @param encoding
    *        The encoding for the new RasterDataDescriptor.
    * @param location
    *        The processing location for the new RasterDataDescriptor.
    *
    * @return A fully populated RasterDataDescriptor, without a
    *         FileDescriptor.  Note that original numbers for 
    *         rows, and columns will go from 0 to n.
    */
   RasterDataDescriptor* generateRasterDataDescriptor(const std::string& name, DataElement* pParent,
      unsigned int rows, unsigned int columns, EncodingType encoding, ProcessingLocation location);

   /**
    * Generate a populated RasterDataDescriptor with RasterFileDescriptor
    * to match the given RasterElement without any chipping.
    *
    * This method will retain any interesting original DimensionDescriptor
    * numbers, inherit the classification from the original element but not copy any metadata.
    *
    * @param pOrigElement
    *        The RasterElement to copy.  There must be a valid RasterFileDescriptor
    *        attached to this element.
    *
    * @return A fully populated RasterDataDescriptor with RasterFileDescriptor.
    *         The original numbers for rows and columns will match those for
    *         pOrigElement, but without any chipping.  The parent of
    *         this descriptor is pOrigElement.
    */
   RasterDataDescriptor *generateUnchippedRasterDataDescriptor(
      const RasterElement *pOrigElement);

   /**
    * Modifies the provided DataDescriptor to include only the subset specified.
    *
    * @param pDd
    *        The DataDescriptor that should be modified to only include the
    *        specified subset.
    * @param startRow
    *        The DimensionDescriptor that specifies the starting row of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the start of the row subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the first row will be used.
    * @param stopRow
    *        The DimensionDescriptor that specifies the ending row of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the end of the row subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the last row will be used.
    * @param rowSkipFactor
    *        Specifies the number of rows that should be skipped
    *        while creating the subset.  For example, a skipFactor of 0 would include
    *        the following row positions: 0, 1, 2, 3, 4, 5, 6. For example, a 
    *        skipFactor of 1 would include the following row positions: 0, 2, 4, 6.
    * @param startCol
    *        The DimensionDescriptor that specifies the starting column of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the start of the column subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the first column will be used.
    * @param stopCol
    *        The DimensionDescriptor that specifies the ending column of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the end of the column subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the last column will be used.
    * @param colSkipFactor
    *        Specifies the number of columns that should be skipped
    *        while creating the subset.  For example, a skipFactor of 0 would include
    *        the following column positions: 0, 1, 2, 3, 4, 5, 6. For example, a 
    *        skipFactor of 1 would include the following column positions: 0, 2, 4, 6.
    * @param subsetBands
    *        Specifies the list of bands that should be included in the subset.
    *        The vector must be a subset of the DataDescriptor::getBands() value. An
    *        empty vector can be provided, in which case no bands will be removed
    *        during the subset.
    */
   void subsetDataDescriptor(DataDescriptor* pDd,
      const DimensionDescriptor& startRow,
      const DimensionDescriptor& stopRow,
      unsigned int rowSkipFactor,
      const DimensionDescriptor& startCol,
      const DimensionDescriptor& stopCol,
      unsigned int colSkipFactor,
      const std::vector<DimensionDescriptor>& subsetBands = std::vector<DimensionDescriptor>());

   /**
    * Modifies the provided DataDescriptor to include only the subset specified.
    *
    * @param pDd
    *        The DataDescriptor that should be modified to only include the
    *        specified subset.
    * @param startRow
    *        The DimensionDescriptor that specifies the starting row of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the start of the row subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the first row will be used.
    * @param stopRow
    *        The DimensionDescriptor that specifies the ending row of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the end of the row subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the last row will be used.
    * @param rowSkipFactor
    *        Specifies the number of rows that should be skipped
    *        while creating the subset.  For example, a skipFactor of 0 would include
    *        the following row positions: 0, 1, 2, 3, 4, 5, 6. For example, a 
    *        skipFactor of 1 would include the following row positions: 0, 2, 4, 6.
    * @param startCol
    *        The DimensionDescriptor that specifies the starting column of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the start of the column subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the first column will be used.
    * @param stopCol
    *        The DimensionDescriptor that specifies the ending column of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the end of the column subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the last column will be used.
    * @param colSkipFactor
    *        Specifies the number of columns that should be skipped
    *        while creating the subset.  For example, a skipFactor of 0 would include
    *        the following column positions: 0, 1, 2, 3, 4, 5, 6. For example, a 
    *        skipFactor of 1 would include the following column positions: 0, 2, 4, 6.
    * @param startBand
    *        The DimensionDescriptor that specifies the starting band of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the start of the band subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the first band will be used.
    * @param stopBand
    *        The DimensionDescriptor that specifies the ending band of the subset.
    *        The DimensionDescriptor::getOriginalNumber() will be used to determine
    *        the end of the band subset in the vector.
    *        A default constructed DimensionDescriptor can be provided, in which
    *        case the last band will be used.
    * @param bandSkipFactor
    *        Specifies the number of band that should be skipped
    *        while creating the subset.  For example, a skipFactor of 0 would include
    *        the following band positions: 0, 1, 2, 3, 4, 5, 6. For example, a 
    *        skipFactor of 1 would include the following band positions: 0, 2, 4, 6.
    */
   void subsetDataDescriptor(DataDescriptor* pDd,
      const DimensionDescriptor& startRow,
      const DimensionDescriptor& stopRow,
      unsigned int rowSkipFactor,
      const DimensionDescriptor& startCol,
      const DimensionDescriptor& stopCol,
      unsigned int colSkipFactor,
      const DimensionDescriptor& startBand,
      const DimensionDescriptor& stopBand,
      unsigned int bandSkipFactor);

   /** 
    * Creates a RasterElement with the given parameters and that 
    * assumes a single band that can be immediately used. This method
    * should only be used by plug-ins that need to programmatically create a RasterElement to store
    * results of an algorithm.  It should NOT be used by importers or exporters to create a RasterElement.
    * It should also NOT be used to create a RasterElement that corresponds to a data file on the filesystem.
    * The created element will inherit the parent's classification unless the parent is \c NULL, in which case
    * the classification will be set to the system's highest level of classification. Use the DataElement convenience
    * method copyClassification or setClassification if the parent is \c NULL or if you require different
    * classification settings.
    *
    * @param name
    *        The name for the new RasterDataDescriptor.
    * @param rows
    *        The number of rows for the new RasterDataDescriptor.
    * @param columns
    *        The number of columns for the new RasterDataDescriptor.
    * @param encoding
    *        The encoding for the new RasterDataDescriptor.
    * @param inMemory
    *        If true, the data for the RasterElement will be fully contained in RAM.  If false,
    *        the data for the RasterElement will be fully contained on the filesystem and will
    *        be paged into memory as required.
    * @param pParent
    *        The parent element for the new RasterDataDescriptor.
    *
    * @return A RasterElement created with the given parameters
    *         and that assumes a single band that requires no additional initialization.
    *
    * @see DataElement::copyClassification, DataElement::setClassification
    */
   RasterElement* createRasterElement(const std::string& name, unsigned int rows, unsigned int columns,
      EncodingType encoding, bool inMemory = true, DataElement* pParent = 0 );

   /** 
    * Creates a RasterElement with the given parameters that can be immediately used.  This method
    * should only be used by plug-ins that need to programmatically create a RasterElement to store
    * results of an algorithm.  It should NOT be used by importers or exporters to create a RasterElement.
    * It should also NOT be used to create a RasterElement that corresponds to a data file on the filesystem.
    * The created element will inherit the parent's classification unless the parent is \c NULL, in which case
    * the classification will be set to the system's highest level of classification. Use the DataElement convenience
    * method copyClassification or setClassification if the parent is \c NULL or if you require different
    * classification settings.
    *
    * @param name
    *        The name for the new RasterDataDescriptor.
    * @param rows
    *        The number of rows for the new RasterDataDescriptor.
    * @param columns
    *        The number of columns for the new RasterDataDescriptor.
    * @param bands
    *        The number of bands for the new RasterDataDescriptor.
    * @param encoding
    *        The encoding for the new RasterDataDescriptor.
    * @param interleave
    *        The interleave for the new RasterDataDescriptor.
    * @param inMemory
    *        If true, the data for the RasterElement will be fully contained in RAM.  If false,
    *        the data for the RasterElement will be fully contained on the filesystem and will
    *        be paged into memory as required.
    * @param pParent
    *        The parent element for the new RasterDataDescriptor.
    *
    * @return A RasterElement created with the given parameters that requires no additional initialization.
    *
    * @see DataElement::copyClassification, DataElement::setClassification
    */
   RasterElement* createRasterElement(const std::string& name, unsigned int rows, unsigned int columns,
      unsigned int bands, EncodingType encoding, InterleaveFormatType interleave = BIP, bool inMemory = true,
      DataElement* pParent = 0);

   /**
    * Determine the number of bytes in a single element of a
    * given EncodingType.
    *
    * @param encoding
    *        EncodingType to find the size of.
    *
    * @return The size in bytes of encoding.
    */
   size_t bytesInEncoding(EncodingType encoding);

   /**
    * Returns the band names for the given descriptor.  This
    * method will query the #SPECIAL_METADATA_NAME / #BAND_METADATA_NAME / #NAMES_METADATA_NAME 
    * and #SPECIAL_METADATA_NAME / #BAND_NAME_PREFIX_METADATA_NAME
    * keys of the metadata to determine the correct band names.
    *
    * @param pDescriptor
    *        the descriptor to return the band names for.
    * 
    * @return the band names for the given descriptor or empty vector if not found.
    */
   std::vector<std::string> getBandNames(const RasterDataDescriptor* pDescriptor);

   /**
    * Returns the band name for the given descriptor and band.  This
    * method will query the #SPECIAL_METADATA_NAME / #BAND_METADATA_NAME / #NAMES_METADATA_NAME 
    * and #SPECIAL_METADATA_NAME / #BAND_NAME_PREFIX_METADATA_NAME
    * keys of the metadata to determine the correct band name.  This
    * method should not be called for every band in a RasterDataDescriptor,
    * for that please call getBandNames().
    *
    * @param pDescriptor
    *        the descriptor to return the band name for.
    * @param band
    *        the individual band to return the band name for.
    * 
    * @return the band name for the given descriptor and band or empty string if not found.
    */
   std::string getBandName(const RasterDataDescriptor* pDescriptor, DimensionDescriptor band);

   /**
   * Returns whether or not raster image can be displayed in true color.  This
   * method will query the #SPECIAL_METADATA_NAME / #BAND_METADATA_NAME / 
   * #CENTER_WAVELENGTHS_METADATA_NAME key of the metadata to 
   * determine if necessary wavelengths are present to display a true color image.
   *
   * @param pDescriptor
   *        the descriptor to return the ability to display true true for.
   * 
   * @return \c True if the raster image can be displayed in true color and \c false otherwise.
   */
   bool canBeDisplayedInTrueColor(const RasterDataDescriptor* pDescriptor);


   /**
   * Returns whether or not raster image is a subcube.
   *
   * @param pDescriptor
   *        the descriptor of the raster image to be checked.
   * @param checkBands
   *        If \c true, include the bands in the comparison.  If \c false, don't include the bands
   * 
   * @return True if the raster image DimensionDescriptors for Rows, Columns and Bands are equal
   * to the DimensionDescriptors for the Rows, Columns and Bands of the RasterFileDescriptor 
   * and the checkBands parameter is true or if the raster image DimensionDescriptors for Rows and 
   * Columns are equal to the DimensionDescriptors for the Rows and Columns of the RasterFileDescriptor 
   * and the checkBands parameter is false.
   */
   bool isSubcube(const RasterDataDescriptor* pDescriptor, bool checkBands);


   /**
   * Sets the display bands to true color equivalents.  This method will query 
   * the #SPECIAL_METADATA_NAME / #BAND_METADATA_NAME / 
   * #CENTER_WAVELENGTHS_METADATA_NAME key of the metadata to 
   * determine the best bands to use for displaying a true color image.
   *
   * @param pDescriptor
   *        the descriptor to return the ability to display true true for.
   * 
   * @return True if the raster image display bands were set to display in true color 
   * and false otherwise.
   */
   bool setDisplayBandsToTrueColor(RasterDataDescriptor* pDescriptor);

   /**
   * Finds the closest match in vector of values to a value within the tolerance specified.  
   * This method will return the index in vector of the closest match. If there is no 
   * match within the tolerance, the method will return -1.
   *
   * @param values
   *        the list of values to search for the closest match.
   * @param value
   *        the value for which to find the closest match.
   * @param tolerance
   *        the maximum difference allowed to consider as a valid match.
   * @param startAt
   *        the index at which to start search. Defaults to first element.
   * @return the index of best match or -1 if no match found.
   */
   int findBestMatch(const std::vector<double> &values, double value, 
                     double tolerance, int startAt = 0);

   /**
    * Returns a vector of RasterChannelType enum values.
    *
    * @return a vector of RasterChannelType enum values.
    */
   std::vector<RasterChannelType> getVisibleRasterChannels();

   /**
    * Chip the metadata in-place.
    *
    * The vector metadata with the #SPECIAL_METADATA_NAME / #BAND_METADATA_NAME, 
    * #ROW_METADATA_NAME, and #COLUMN_METADATA_NAME
    * DynamicObjects will be chipped such that they can still be
    * indexed with active numbers.
    *
    * This method will automatically be called when using one
    * of the standard ways to chip (RasterElement::createChip, chipping
    * on import using the default behavior in RasterElementImporterShell).
    * Plug-ins should only call it when implementing their own chipping behavior.
    *
    * Metadata chipping is supported for vectors of the signed and unsigned variants
    * of char, int, long, int64_t, as well as float, double, bool, and std::string.
    *
    *  @param   pMetadata
    *           The DynamicObject that contains
    *           #SPECIAL_METADATA_NAME / #BAND_METADATA_NAME, 
    *           #ROW_METADATA_NAME, and #COLUMN_METADATA_NAME that will
    *           be chipped by this method.  This DynamicObject will
    *           be modified in-place.  This DynamicObject will most likely
    *           come from DataDescriptor::getMetadata().
    *  @param   selectedRows
    *           The DimensionDescriptors (unmodified from the RasterElement) for the rows
    *           which should be included in this chip.
    *  @param   selectedColumns
    *           The DimensionDescriptors (unmodified from the RasterElement) for the rows
    *           which should be included in this chip.
    *  @param   selectedBands
    *           The DimensionDescriptors (unmodified from the RasterElement) for the rows
    *           which should be included in this chip.
    *
    * @return True if the operation succeeded, false otherwise.
    */
   bool chipMetadata(DynamicObject* pMetadata, 
      const std::vector<DimensionDescriptor> &selectedRows,
      const std::vector<DimensionDescriptor> &selectedColumns,
      const std::vector<DimensionDescriptor> &selectedBands);

   /**
    * Checks whether a floating-point value is a NaN.
    *
    *  @param   value
    *           The value to test.
    *
    * @return True if value is NaN, false otherwise.
    */
   template<typename T>
   inline bool isBad(T value)
   {
      return false;
   }

   /**
    * Checks whether a floating-point value is a NaN.
    *
    *  @param   value
    *           The value to test.
    *
    * @return True if value is NaN, false otherwise.
    */
   template<>
   inline bool isBad<double>(double value)
   {
      return isnanf(value);
   }

   /**
    * Checks whether a floating-point value is a NaN.
    *
    *  @param   value
    *           The value to test.
    *
    * @return True if value is NaN, false otherwise.
    */
   template<>
   inline bool isBad<float>(float value)
   {
      return (FINITE(value) == 0);
   }

   /**
    *  Sanitize the given data.
    *
    *  This method will iterate over the given data and replace all
    *  instances of floating point NaNs with the specified value.
    *
    *  @param   pData
    *           Pointer to the data to be sanitized.
    *
    *  @param   count
    *           The number of data values to be sanitized.
    *
    *  @param   value
    *           The value to use for all instances of floating point NaNs.
    *           This value will be cast (via static_cast) to T.
    *
    *  @return The number of data values which were sanitized.
    */
   template <typename T>
   inline uint64_t sanitizeData(T* pData, uint64_t count, double value = 0.0)
   {
      uint64_t badValueCount = 0;

      if (pData != NULL)
      {
         for (uint64_t index = 0; index < count; ++index)
         {
            if (isBad(pData[index]))
            {
               pData[index] = static_cast<T>(value);
               ++badValueCount;
            }
         }
      }

      return badValueCount;
   }

   /**
    *  Sanitize the given data.
    *
    *  This method will iterate over the given data and replace all
    *  instances of floating point NaNs with the specified value.
    *
    *  @param   pData
    *           Pointer to the data to be sanitized.
    *
    *  @param   count
    *           The number of data values to be sanitized.
    *
    *  @param   type
    *           The EncodingType of pData.
    *
    *  @param   value
    *           The value to use for all instances of floating point NaNs.
    *           This value will be cast (via static_cast) to the given EncodingType.
    *
    *  @return The number of data values which were sanitized.
    */
   inline uint64_t sanitizeData(void* pData, uint64_t count, EncodingType type, double value = 0.0)
   {
      switch (type)
      {
         case FLT8COMPLEX:
            count *= 2;  // fall through
         case FLT4BYTES:
            return sanitizeData(reinterpret_cast<float*>(pData), count, value);
         case FLT8BYTES:
            return sanitizeData(reinterpret_cast<double*>(pData), count, value);
         default:
            return 0;
      }
   }

   /**
    *  Calculate size of data file.
    *
    *  This function will calculate the size of the file described in
    *  the given RasterFileDescriptor. For a BSQ multiple file data set, it will
    *  calculate the required size for an individual file, not the total
    *  of all the files. 
    *
    *  @param pDescriptor
    *         The RasterFileDescriptor to use in calculation of file size.
    *
    *  @return The calculated file size. For BSQ multiple files, it will return
    *          calculated size that each of the files should require.
    *          Will return -1 if errors occurred during calculation, e.g.
    *          if pDescriptor is NULL.
    */
   int64_t calculateFileSize(const RasterFileDescriptor* pDescriptor);

   /**
    * Types of interpolation.
    */
   enum InterpolationTypeEnum
   {
      NEAREST_NEIGHBOR, /**< Duplicate the nearest neighbor */
      BILINEAR, /**< Bilinear interpolation */
      BICUBIC /**< Bicubic interpolation */
   };

   /**
    * @EnumWrapper RasterUtilities::InterpolationTypeEnum.
    */
   typedef EnumWrapper<InterpolationTypeEnum> InterpolationType;

   /**
    *  Rotate a data set.
    *
    *  The original dimensions of the data set are maintained. This means that data clipping and padding may occur.
    *
    *  @param pDst
    *         Destination RasterElement. Must be initialized to the same params as pSrc.
    *  @param pSrc
    *         RasterElement to rotate.
    *  @param angle
    *         Rotate by this angle. In radians.
    *  @param defaultValue
    *         Pixels which do not map to anything in the original data set will be set to this value.
    *         This value will be added to the bad values list if it is not already there.
    *  @param interp
    *         Interpolation type. Only NEAREST_NEIGHBOR is currently supported.
    *  @param pProgress
    *         Report progress.
    *  @param pAbort
    *         If not \c NULL, check this value during the rotation. If the value becomes \c true, abort.
    *  @return \c True if successful, \c false on error.
    */
   bool rotate(RasterElement* pDst, const RasterElement* pSrc, double angle, int defaultValue,
               InterpolationType interp = NEAREST_NEIGHBOR, Progress* pProgress = NULL, bool* pAbort = NULL);
}

#endif
