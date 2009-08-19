/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERDATADESCRIPTOR_H
#define RASTERDATADESCRIPTOR_H

#include "DataDescriptor.h"
#include "DimensionDescriptor.h"
#include "TypesFile.h"

#include <vector>

class Units;

/**
 *  Describes a raster data element.
 *
 *  In addition to the anscillary information stored in the DataDescriptor base
 *  class, this class contains information pertinent to raster data elements.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following methods are called: setDataType(), setBadValues(),
 *    setRows(), setColumns(), setBands(), setXPixelSize(), setYPixelSize(),
 *    setUnits(), and setInterleaveFormat()
 *  - All notifications documented in DataDescriptor.
 *
 *  @see        RasterDataDescriptorExt1, RasterElement
 */
class RasterDataDescriptor : public DataDescriptor
{
public:
   /**
    *  Sets the data type of the values in the data set.
    *
    *  @param   dataType
    *           The data type of the values in the data set.
    *
    *  @notify  This method notifies Subject::signalModified if the given
    *           data type is different than the current data type.
    */
   virtual void setDataType(EncodingType dataType) = 0;

   /**
    *  Returns the data type of the values in the data set.
    *
    *  @return  The data type.
    */
   virtual EncodingType getDataType() const = 0;

   /**
    *  Returns the number of bytes used for each pixel element value.
    *
    *  This is a convenience method that returns the number of bytes used for
    *  each pixel element as determined by the data type.
    *
    *  @return  The number of bytes.
    *
    *  @see     getDataType()
    */
   virtual unsigned int getBytesPerElement() const = 0;

   /**
    *  Sets bad values that should be ignored in the data statistics.
    *
    *  This method is intended to be used when creating a data descriptor that
    *  will be used to create a RasterElement.  When the RasterElement is
    *  created, the bad values that were set in the data descriptor by calling
    *  this method will be set into the Statistics objects in the raster
    *  element.  After the raster element is created, the bad values should be
    *  changed by calling Statistics::setBadValues() instead of this method.
    *
    *  @param   badValues
    *           The bad values that should be ignored.
    *
    *  @notify  This method notifies Subject::signalModified if the given
    *           bad values are different than the current bad values.
    *
    *  @see     Statistics::setBadValues()
    */
   virtual void setBadValues(const std::vector<int>& badValues) = 0;

   /**
    *  Returns the bad values that should be ignored in the data statistics.
    *
    *  @return  The bad values as integers that should be ignored.
    */
   virtual const std::vector<int>& getBadValues() const = 0;

   /**
    *  Sets the interleave format of the data values.
    *
    *  @param   format
    *           The interleave format in which the values in the data set are
    *           stored.
    */
   virtual void setInterleaveFormat(InterleaveFormatType format) = 0;

   /**
    *  Returns the interleave format of the data values.
    *
    *  @return  The interleave format in which the values in the data set are
    *           stored.
    */
   virtual InterleaveFormatType getInterleaveFormat() const = 0;

   /**
    *  Sets the rows for the data element that are available for processing.
    *
    *  This method sets the rows of the data element that are available for
    *  processing.  It is typically called in the following scenarios:
    *  - <b>Importing</b> - An importer would call this method to set the
    *    default rows to import, where the given vector is identical to or a
    *    subset of the rows vector in the RasterFileDescriptor contained
    *    in this data descriptor.  It would then called by the core after the
    *    user identifies any subset to load before Executable::execute() is
    *    called on the importer.
    *  - <b>Creating a new raster element</b> - Between calls to
    *    ModelServices::createDataDescriptor() and
    *    ModelServices::createElement() an object would call this method to
    *    set the rows describing the data in the element.
    *
    *  These rows differ from the rows in a corresponding
    *  RasterFileDescriptor in that they contain only those row objects
    *  that have been imported.
    *
    *  @param   rows
    *           A vector of DimensionDescriptors containing one
    *           instance for each row of the data set that is available for
    *           processing.
    *
    *  @notify  This method notifies Subject::signalModified if the given
    *           rows are different than the current rows.
    *
    *  @see     DimensionDescriptor
    */
   virtual void setRows(const std::vector<DimensionDescriptor>& rows) = 0;

   /**
    *  Returns the rows for the data element that are available for processing.
    *
    *  @return  A vector of DimensionDescriptors containing one
    *           instance for each row of the data set that is available for
    *           processing.
    */
   virtual const std::vector<DimensionDescriptor>& getRows() const = 0;

   /**
    *  Returns the uniform skip factor between the on-disk numbers of
    *  DimensionDescriptors returned by getRows().  This is a convenience method.
    *
    *  @return the uniform skip factor. 0 means there is no skipping
    *
    *  @see RasterUtilities::determineSkipFactor()
    */
   virtual unsigned int getRowSkipFactor() const = 0;

   /**
    *  Returns the row object containing a given original number.
    *
    *  @param   originalNumber
    *           The zero-based original number for which to get the row object.
    *
    *  @return  The row object that has the given original number.  An invalid 
    *           DimensionDescriptor is returned if a row object does not exist 
    *           with the given original number.  This could occur if that particular 
    *           row has not been imported.
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
    *           with the given on-disk number.  This could occur if the element 
    *           was created from an algorithm instead of an import, where there 
    *           would be no on-disk representation of the data.
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
    *  Returns the number of rows available for processing.
    *
    *  This is a convenience method that returns getRows().size().
    *
    *  @return  The number of rows available for processing.
    */
   virtual unsigned int getRowCount() const = 0;

   /**
    *  Sets the columns for the data element that are available for processing.
    *
    *  This method sets the columns of the data element that are available for
    *  processing.  It is typically called in the following scenarios:
    *  - <b>Importing</b> - An importer would call this method to set the
    *    default columns to import, where the given vector is identical to or
    *    a subset of the columns vector in the RasterFileDescriptor
    *    contained in this data descriptor.  It would then called by the core
    *    after the user identifies any subset to load before Executable::execute()
    *    is called on the importer.
    *  - <b>Creating a new raster element</b> - Between calls to
    *    ModelServices::createDataDescriptor() and
    *    ModelServices::createElement() an object would call this method to
    *    set the columns describing the data in the element.
    *
    *  These columns differ from the columns in a corresponding
    *  RasterFileDescriptor in that they contain only those column objects
    *  that have been imported.
    *
    *  @param   columns
    *           A vector of DimensionDescriptors containing one
    *           instance for each column of the data set that is available for
    *           processing.
    *
    *  @notify  This method notifies Subject::signalModified if the given
    *           columns are different than the current columns.
    *
    *  @see     DimensionDescriptor
    */
   virtual void setColumns(const std::vector<DimensionDescriptor>& columns) = 0;

   /**
    *  Returns the columns for the data element that are available for
    *  processing.
    *
    *  @return  A vector of DimensionDescriptors containing one
    *           instance for each column of the data set that is available for
    *           processing.
    */
   virtual const std::vector<DimensionDescriptor>& getColumns() const = 0;

   /**
    *  Returns the uniform skip factor between the on-disk numbers of
    *  DimensionDescriptors returned by getColumns().  This is a convenience method.
    *
    *  @return the uniform skip factor. 0 means there is no skipping
    *
    *  @see RasterUtilities::determineSkipFactor()
    */
   virtual unsigned int getColumnSkipFactor() const = 0;

   /**
    *  Returns the column object containing a given original number.
    *
    *  @param   originalNumber
    *           The zero-based original number for which to get the column
    *           object.
    *
    *  @return  The column object that has the given original number.  An invalid 
    *           DimensionDescriptor is returned if a column object does not exist 
    *           with the given original number.  This could occur if that particular 
    *           column has not been imported.
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
    *           with the given on-disk number.  This could occur if the element 
    *           was created from an algorithm instead of an import, where there would be
    *           no on-disk representation of the data.
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
    *  Returns the number of columns available for processing.
    *
    *  This is a convenience method that returns getColumns().size().
    *
    *  @return  The number of columns available for processing.
    */
   virtual unsigned int getColumnCount() const = 0;

   /**
    *  Sets the bands for the data element that are available for processing.
    *
    *  This method sets the bands of the data element that are available for
    *  processing.  It is typically called in the following scenarios:
    *  - <b>Importing</b> - An importer would call this method to set the
    *    default bands to import, where the given vector is identical to or a
    *    subset of the bands vector in the RasterFileDescriptor contained
    *    in this data descriptor.  It would then called by the core after the
    *    user identifies any subset to load before Executable::execute() is
    *    called on the importer.
    *  - <b>Creating a new sensor data element</b> - Between calls to
    *    ModelServices::createDataDescriptor() and
    *    ModelServices::createElement() an object would call this method to
    *    set the bands describing the data in the element.
    *
    *  These bands differ from the bands in a corresponding
    *  RasterFileDescriptor in that they contain only those band objects
    *  that have been imported.
    *
    *  @param   bands
    *           A vector of DimensionDescriptors containing one
    *           instance for each band of the data set that is available for
    *           processing.
    *
    *  @notify  This method notifies Subject::signalModified if the given
    *           bands are different than the current bands.
    *
    *  @see     DimensionDescriptor
    */
   virtual void setBands(const std::vector<DimensionDescriptor>& bands) = 0;

   /**
    *  Returns the bands for the data element that are available for
    *  processing.
    *
    *  @return  A vector of DimensionDescriptors containing one
    *           instance for each band of the data set that is available for
    *           processing.
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
    *           with the given original number.  This could occur if that 
    *           particular band has not been imported.
    */
   virtual DimensionDescriptor getOriginalBand(unsigned int originalNumber) const = 0;

   /**
    *  Returns the band object containing a given on-disk number.
    *
    *  @param   onDiskNumber
    *           The zero-based on-disk number for which to get the band object.
    *
    *  @return  The band object that has the given on-disk number.   An invalid 
    *           DimensionDescriptor is returned if a band object does not exist 
    *           with the given on-disk number.  This could occur if the element
    *           was created from an algorithm instead of an import, where there
    *           would be no on-disk representation of the data.
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
    *  Returns the number of bands available for processing.
    *
    *  This is a convenience method that returns getBands().size().
    *
    *  @return  The number of bands available for processing.
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
    *  This method serves as a default for the view that can be associated
    *  with a model element.  Once a view is created, call the
    *  Layer::setXScaleFactor() and Layer::setXOffset() method to change the
    *  display pixel size.
    *
    *  @param   pixelSize
    *           The pixel size for each column in the data set.  If this value
    *           is negative or zero, this method does nothing.
    *
    *  @notify  This method notifies Subject::signalModified if the given
    *           column pixel size is different than the current column pixel
    *           size.
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
    *  This method serves as a default for the view that can be associated
    *  with a model element.  Once a view is created, call the
    *  Layer::setYScaleFactor() and Layer::setYOffset() method to change the
    *  display pixel size.
    *
    *  @param   pixelSize
    *           The pixel size for each row in the data set.  If this value
    *           is negative or zero, this method does nothing.
    *
    *  @notify  This method notifies Subject::signalModified if the given
    *           row pixel size is different than the current row pixel size.
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
    *
    *  @notify  This method notifies Subject::signalModified if the given
    *           units object is different than the current units object.
    */
   virtual void setUnits(const Units* pUnits) = 0;

   /**
    *  Returns a pointer to the element's units object.
    *
    *  @return  A pointer to the element's units object.
    */
   virtual Units* getUnits() = 0;

   /**
    *  Returns read-only access to the element's units object.
    *
    *  @return  A const pointer to the element's units object.  The units
    *           represented by the returned pointer should not be modified.  To
    *           modify the values, call the non-const version of getUnits().
    */
   virtual const Units* getUnits() const = 0;

  /**
    *  Sets the band that is initially displayed in a RasterLayer.
    *
    *  This method sets a band to be initially displayed when a raster layer is
    *  created.
    *
    *  @param   eColor
    *           The color in which to display the given band.
    *  @param   band
    *           The band object to display in the given color.
    *
    *  @see     setDisplayMode()
    */
   virtual void setDisplayBand(RasterChannelType eColor, DimensionDescriptor band) = 0;

   /**
    *  Returns the band object that is initially displayed in a RasterLayer.
    *
    *  @param   eColor
    *           The color in which to get the initially displayed band.
    *
    *  @return  A pointer to the band object that is initially displayed in the
    *           given color.  \b NULL is returned if no band has been set to
    *           be initially displayed for the given color.
    */
   virtual DimensionDescriptor getDisplayBand(RasterChannelType eColor) const = 0;

   /**
    *  Sets the initial display mode for the data in a RasterLayer.
    *
    *  @param   displayMode
    *           The initial display mode.
    *
    *  @see     setDisplayBand()
    */
   virtual void setDisplayMode(DisplayMode displayMode) = 0;

   /**
    *  Returns the initial display mode for the data in a RasterLayer.
    *
    *  @return  The initial display mode.
    */
   virtual DisplayMode getDisplayMode() const = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyDataDescriptor.
    */
   virtual ~RasterDataDescriptor() {}
};

/**
 * Extends capability of the RasterDataDescriptor interface.
 *
 * This class provides additional capability for the RasterDataDescriptor interface
 * class.  A pointer to this class can be obtained by performing a dynamic cast
 * on a pointer to RasterDataDescriptor.
 *
 * @warning A pointer to this class can only be used to call methods contained
 *          in this extension class and cannot be used to call any methods in
 *          RasterDataDescriptor.
 *
 * @see RasterDataDescriptor
 */
class RasterDataDescriptorExt1
{
public:
   /**
    *  Sets the valid data types for the data set. The current data type may or may not be valid.
    *  By default, all Encoding Types are valid.
    *
    *  @param   dataType
    *           The valid data types for the data set.
    *
    *  @notify  This method notifies Subject::signalModified if the given
    *           valid data types are different than the current valid data types.
    *
    *  @see RasterDataDescriptor::setDataType()
    */
   virtual void setValidDataTypes(const std::vector<EncodingType>& validDataTypes) = 0;

   /**
    *  Returns the valid data types for the data set. The current data type may or may not be valid.
    *  By default, all Encoding Types are valid.
    *
    *  @return  The valid data types.
    *
    *  @see RasterDataDescriptor::getDataType()
    */
   virtual const std::vector<EncodingType>& getValidDataTypes() const = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyDataDescriptor.
    */
   virtual ~RasterDataDescriptorExt1() {}
};

#endif
