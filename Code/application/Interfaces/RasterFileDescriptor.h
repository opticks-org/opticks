/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERDATAFILEDESCRIPTOR_H
#define RASTERDATAFILEDESCRIPTOR_H

#include "FileDescriptor.h"
#include "DimensionDescriptor.h"
#include "GcpList.h"

#include <list>
#include <vector>

class Units;

/**
 *  Describes how a raster data element is stored in a file on disk.
 *
 *  In addition to the information stored in the FileDescriptor base class,
 *  this class contains information pertinent to how raster data elements
 *  are stored in files on disk.
 *
 * This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setHeaderBytes(), setTrailerBytes(),
 *    setPrelineBytes(), setPostlineBytes(), setPrebandBytes(),
 *    setPostbandBytes(), setBitsPerElement(), setInterleaveFormat(),
 *    setRows(), setColumns(), setBands(), setXPixelSize(), setYPixelSize(),
 *    setUnits(), setGcps(), setBandFiles()
 *  - Everything else documented in FileDescriptor.
 *
 *  @see        RasterElement, RasterDataDescriptor
 */
class RasterFileDescriptor : public FileDescriptor
{
public:
   /**
    *  Sets the number of file header bytes.
    *
    *  @param   bytes
    *           The number of bytes allocated for header information in the
    *           data set file.
    */
   virtual void setHeaderBytes(unsigned int bytes) = 0;

   /**
    *  Returns the number of file header bytes.
    *
    *  @return  The number of bytes allocated for header information in the
    *           data set file.
    */
   virtual unsigned int getHeaderBytes() const = 0;

   /**
    *  Sets the number of file trailer bytes.
    *
    *  @param   bytes
    *           The number of bytes allocated for trailer information in the
    *           data set file.
    */
   virtual void setTrailerBytes(unsigned int bytes) = 0;

   /**
    *  Returns the number of file trailer bytes.
    *
    *  @return  The number of bytes allocated for trailer information in the
    *           data set file.
    */
   virtual unsigned int getTrailerBytes() const = 0;

   /**
    *  Sets the number of preline bytes.
    *
    *  Preline bytes refer to the number of bytes preceding each line of data
    *  values in the data file.  Preline bytes apply to files with data stored
    *  in any interleave format.
    *
    *  @param   bytes
    *           The number of bytes allocated preceding each line of data
    *           values in the data set file.
    */
   virtual void setPrelineBytes(unsigned int bytes) = 0;

   /**
    *  Returns the number of preline bytes.
    *
    *  Preline bytes refer to the number of bytes preceding each line of data
    *  values in the data file.  Preline bytes apply to files with data stored
    *  in any interleave format.
    *
    *  @return  The number of bytes allocated preceding each line of data
    *           values in the data set file.
    */
   virtual unsigned int getPrelineBytes() const = 0;

   /**
    *  Sets the number of postline bytes.
    *
    *  Postline bytes refer to the number of bytes following each line of data
    *  values in the data file.  Postline bytes apply to files with data stored
    *  in any interleave format.
    *
    *  @param   bytes
    *           The number of bytes allocated following each line of data
    *           values in the data set file.
    */
   virtual void setPostlineBytes(unsigned int bytes) = 0;

   /**
    *  Returns the number of postline bytes.
    *
    *  Postline bytes refer to the number of bytes following each line of data
    *  values in the data file.  Postline bytes apply to files with data stored
    *  in any interleave format.
    *
    *  @return  The number of bytes allocated following each line of data
    *           values in the data set file.
    */
   virtual unsigned int getPostlineBytes() const = 0;

   /**
    *  Sets the number of preband bytes.
    *
    *  Preband bytes refer to the number of bytes preceding each band of data
    *  values in the data file.  Preband bytes apply only to files with data
    *  stored in the BSQ interleave format with all bands stored in a single
    *  file.
    *
    *  @param   bytes
    *           The number of bytes allocated preceding each band of data
    *           values in the data set file.
    *
    *  @see     InterleaveFormatType
    */
   virtual void setPrebandBytes(unsigned int bytes) = 0;

   /**
    *  Returns the number of preband bytes.
    *
    *  Preband bytes refer to the number of bytes preceding each band of data
    *  values in the data file.  Preband bytes apply only to files with data
    *  stored in the BSQ interleave format with all bands stored in a single
    *  file.
    *
    *  @return  The number of bytes allocated preceding each band of data
    *           values in the data set file.
    *
    *  @see     InterleaveFormatType
    */
   virtual unsigned int getPrebandBytes() const = 0;

   /**
    *  Sets the number of postband bytes.
    *
    *  Postband bytes refer to the number of bytes following each band of data
    *  values in the data file.  Postband bytes apply only to files with data
    *  stored in the BSQ interleave format with all bands stored in a single
    *  file.
    *
    *  @param   bytes
    *           The number of bytes allocated following each band of data
    *           values in the data set file.
    *
    *  @see     InterleaveFormatType
    */
   virtual void setPostbandBytes(unsigned int bytes) = 0;

   /**
    *  Returns the number of postband bytes.
    *
    *  Postband bytes refer to the number of bytes following each band of data
    *  values in the data file.  Postband bytes apply only to files with data
    *  stored in the BSQ interleave format with all bands stored in a single
    *  file.
    *
    *  @return  The number of bytes allocated following each band of data
    *           values in the data set file.
    *
    *  @see     InterleaveFormatType
    */
   virtual unsigned int getPostbandBytes() const = 0;

   /**
    *  Sets the interleave format of the data.
    *
    *  @param   format
    *           The interleave format in which the values in the data set are
    *           stored in the file on disk.
    */
   virtual void setInterleaveFormat(InterleaveFormatType format) = 0;

   /**
    *  Returns the interleave format of the data.
    *
    *  @return  The interleave format in which the values in the data set are
    *           stored in the file on disk.
    */
   virtual InterleaveFormatType getInterleaveFormat() const = 0;

   /**
    *  Sets the filenames for each band of a BSQ multiple-file data set.
    *
    *  Band files apply only to data sets stored in the BSQ interleave format
    *  with each band stored in a separate file.
    *
    *  This is a convenience method that calls the
    *  setBandFiles(const std::vector<const Filename*>&) method.
    *
    *  @param   bandFiles
    *           The filenames for each band of data of a BSQ multiple-file data
    *           set.
    *
    *  @see     InterleaveFormatType
    */
   virtual void setBandFiles(const std::vector<std::string>& bandFiles) = 0;

   /**
    *  Sets the filenames for each band of a BSQ multiple-file data set.
    *
    *  Band files apply only to data sets stored in the BSQ interleave format
    *  with each band stored in a separate file.
    *
    *  @param   bandFiles
    *           The filenames for each band of data of a BSQ multiple-file data
    *           set.
    *
    *  @see     InterleaveFormatType
    */
   virtual void setBandFiles(const std::vector<const Filename*>& bandFiles) = 0;

   /**
    *  Returns the filenames for each band of a BSQ multiple-file data set.
    *
    *  Band files apply only to data sets stored in the BSQ interleave format
    *  with each band stored in a separate file.
    *
    *  @return  The filenames for each band of data of a BSQ multiple-file data
    *           set.
    *
    *  @see     InterleaveFormatType
    */
   virtual const std::vector<const Filename*>& getBandFiles() const = 0;

   /**
    *  Sets the number of bits used for each pixel element value.
    *
    *  @param   numBits
    *           The number of bits per element.
    */
   virtual void setBitsPerElement(unsigned int numBits) = 0;

   /**
    *  Returns the number of bits used for each pixel element value.
    *
    *  @return  The number of bits per element.
    */
   virtual unsigned int getBitsPerElement() const = 0;

   /**
    *  Sets the rows for the data as they are stored in the file on disk.
    *
    *  These rows may differ from the rows in a corresponding
    *  RasterDataDescriptor in that they contain row objects for each row in
    *  the file on disk regardless of the imported rows.
    *
    *  @param   rows
    *           A vector of DimensionDescriptors containing one
    *           instance for each row of data in the file on disk.
    *
    *  @see     DimensionDescriptor
    */
   virtual void setRows(const std::vector<DimensionDescriptor>& rows) = 0;

   /**
    *  Returns the rows for the data as they are stored in the file on disk.
    *
    *  @return  A vector of DimensionDescriptors containing one
    *           instance for each row of data in the file on disk.
    */
   virtual const std::vector<DimensionDescriptor>& getRows() const = 0;

   /**
    *  Returns the row object containing a given original number.
    *
    *  @param   originalNumber
    *           The zero-based original number for which to get the row object.
    *
    *  @return  The row object that has the given original number.  An invalid
    *           DimensionDescriptor is returned if a row object does not exist
    *           with the given original number.
    */
   virtual DimensionDescriptor getOriginalRow(unsigned int originalNumber) const = 0;

   /**
    *  Returns the row object containing a given on-disk number.
    *
    *  @param   onDiskNumber
    *           The zero-based on-disk number for which to get the row object.
    *
    *  @return  The row object that has the given on-disk number.  An invalid 
    *           DimensionDescriptor is returned if a row object does not exist 
    *           with the given on-disk number.
    */
   virtual DimensionDescriptor getOnDiskRow(unsigned int onDiskNumber) const = 0;

   /**
    *  Returns the row object containing a given active number.
    *
    *  @param   activeNumber
    *           The zero-based active number for which to get the row object.
    *
    *  @return  The row object that has the given active number.  An invalid 
    *           DimensionDescriptor is returned if a row object does not exist 
    *           with the given active number.
    */
   virtual DimensionDescriptor getActiveRow(unsigned int activeNumber) const = 0;

   /**
    *  Returns the number of rows as they are stored in the file on disk.
    *
    *  This is a convenience method that returns getRows().size().
    *
    *  @return  The number of rows as they are stored in the file on disk.
    */
   virtual unsigned int getRowCount() const = 0;

   /**
    *  Sets the columns for the data as they are stored in the file on disk.
    *
    *  These columns may differ from the columns in a corresponding
    *  RasterDataDescriptor in that they contain columns objects for each column
    *  in the file on disk regardless of the imported columns.
    *
    *  @param   columns
    *           A vector of DimensionDescriptors containing one
    *           instance for each column of data in the file on disk.
    *
    *  @see     DimensionDescriptor
    */
   virtual void setColumns(const std::vector<DimensionDescriptor>& columns) = 0;

   /**
    *  Returns the columns for the data as they are stored in the file on disk.
    *
    *  @return  A vector of DimensionDescriptors containing one
    *           instance for each column of data in the file on disk.
    */
   virtual const std::vector<DimensionDescriptor>& getColumns() const = 0;

   /**
    *  Returns the column object containing a given original number.
    *
    *  @param   originalNumber
    *           The zero-based original number for which to get the column
    *           object.
    *
    *  @return  The column object that has the given original number.  An invalid 
    *           DimensionDescriptor is returned if a column object does not exist 
    *           with the given original number.
    */
   virtual DimensionDescriptor getOriginalColumn(unsigned int originalNumber) const = 0;

   /**
    *  Returns the column object containing a given on-disk number.
    *
    *  @param   onDiskNumber
    *           The zero-based on-disk number for which to get the column
    *           object.
    *
    *  @return  The column object that has the given on-disk number.  An invalid 
    *           DimensionDescriptor is returned if a column object does not exist 
    *           with the given on-disk number.
    */
   virtual DimensionDescriptor getOnDiskColumn(unsigned int onDiskNumber) const = 0;

   /**
    *  Returns the column object containing a given active number.
    *
    *  @param   activeNumber
    *           The zero-based active number for which to get the column
    *           object.
    *
    *  @return  The column object that has the given active number.  An invalid 
    *           DimensionDescriptor is returned if a column object does not exist 
    *           with the given active number.
    */
   virtual DimensionDescriptor getActiveColumn(unsigned int activeNumber) const = 0;

   /**
    *  Returns the number of columns as they are stored in the file on disk.
    *
    *  This is a convenience method that returns getColumns().size().
    *
    *  @return  The number of columns as they are stored in the file on disk.
    */
   virtual unsigned int getColumnCount() const = 0;

   /**
    *  Sets the bands for the data as they are stored in the file on disk.
    *
    *  These bands may differ from the bands in a corresponding
    *  RasterDataDescriptor in that they contain band objects for each band in
    *  the file on disk regardless of the imported bands.
    *
    *  @param   bands
    *           A vector of DimensionDescriptors containing one
    *           instance for each band of data in the file on disk.
    *
    *  @see     DimensionDescriptor
    */
   virtual void setBands(const std::vector<DimensionDescriptor>& bands) = 0;

   /**
    *  Returns the bands for the data as they are stored in the file on disk.
    *
    *  @return  A vector of DimensionDescriptors containing one
    *           instance for each band of data in the file on disk.
    */
   virtual const std::vector<DimensionDescriptor>& getBands() const = 0;

   /**
    *  Returns the band object containing a given original number.
    *
    *  @param   originalNumber
    *           The zero-based original number for which to get the band
    *           object.
    *
    *  @return  The band object that has the given original number.  An invalid 
    *           DimensionDescriptor is returned if a band object does not exist 
    *           with the given original number.
    */
   virtual DimensionDescriptor getOriginalBand(unsigned int originalNumber) const = 0;

   /**
    *  Returns the band object containing a given on-disk number.
    *
    *  @param   onDiskNumber
    *           The zero-based on-disk number for which to get the band object.
    *
    *  @return  The band object that has the given on-disk number.  An invalid 
    *           DimensionDescriptor is returned if a band object does not exist 
    *           with the given on-disk number.
    */
   virtual DimensionDescriptor getOnDiskBand(unsigned int onDiskNumber) const = 0;

   /**
    *  Returns the band object containing a given active number.
    *
    *  @param   activeNumber
    *           The zero-based active number for which to get the band object.
    *
    *  @return  The band object that has the given active number.  An invalid 
    *           DimensionDescriptor is returned if a band object does not exist 
    *           with the given active number.
    */
   virtual DimensionDescriptor getActiveBand(unsigned int activeNumber) const = 0;

   /**
    *  Returns the number of bands as they are stored in the file on disk.
    *
    *  This is a convenience method that returns getBands().size().
    *
    *  @return  The number of bands as they are stored in the file on disk.
    */
   virtual unsigned int getBandCount() const = 0;

   /**
    *  Sets the pixel size of each column in the data set.
    *
    *  By default, each row and column has a pixel size of 1.0, thereby
    *  producing a size ratio of 1.0, which indicates that the pixel appears
    *  as a square in the view.  Calling this method with a value other than
    *  1.0 allows for non-square pixels, which may represent the true nature
    *  of the data.
    *
    *  @param   pixelSize
    *           The pixel size for each column in the data set.
    */
   virtual void setXPixelSize(double pixelSize) = 0;

   /**
    *  Returns the pixel size of each column in the data set.
    *
    *  @return  The column pixel size.
    *
    *  @see     setXPixelSize()
    */
   virtual double getXPixelSize() const = 0;

   /**
    *  Sets the pixel size of each row in the data set.
    *
    *  By default, each row and column has a pixel size of 1.0, thereby
    *  producing a size ratio of 1.0, which indicates that the pixel appears
    *  as a square in the view.  Calling this method with a value other than
    *  1.0 allows for non-square pixels, which may represent the true nature
    *  of the data.
    *
    *  @param   pixelSize
    *           The pixel size for each row in the data set.
    */
   virtual void setYPixelSize(double pixelSize) = 0;

   /**
    *  Returns the pixel size of each row in the data set.
    *
    *  @return  The row pixel size.
    *
    *  @see     setYPixelSize()
    */
   virtual double getYPixelSize() const = 0;

   /**
    *  Sets the units the values in the data set.
    *
    *  @param   pUnits
    *           The units of the values in the data set.
    */
   virtual void setUnits(const Units* pUnits) = 0;

   /**
    *  Returns a pointer to the data's units object.
    *
    *  @return  A pointer to the data's units object.
    */
   virtual Units* getUnits() = 0;

   /**
    *  Returns read-only access to the data's units object.
    *
    *  @return  A const pointer to the data's units object.  The units
    *           represented by the returned pointer should not be modified.  To
    *           modify the values, call the non-const version of getUnits().
    */
   virtual const Units* getUnits() const = 0;

   /**
    *  Sets GCPs associated with the data in the file on disk.
    *
    *  GCPs can be used to represent geocoordinate information for a particular
    *  data set.
    *
    *  On import, an importer will typically set the GCPs after reading
    *  geocoordinate information from a file.  Then, after importing is
    *  complete, if a SpatialDataView is created the GCPs are used to create a
    *  GcpList element and corresponding GcpLayer that is automatically
    *  displayed in the view.
    *
    *  On export, an object executing the exporter can set the geocoordinate
    *  information that the exporter should use when saving the data to disk.
    *
    *  @param   gcps
    *           The GCPs associated with the data.
    */
   virtual void setGcps(const std::list<GcpPoint>& gcps) = 0;

   /**
    *  Returns the GCPs associated with the data.
    *
    *  @return  The GCPs associated with the data.
    */
   virtual const std::list<GcpPoint>& getGcps() const = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~RasterFileDescriptor() {}
};

#endif
