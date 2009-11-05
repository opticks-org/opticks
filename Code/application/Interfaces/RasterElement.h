/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef RASTERELEMENT_H
#define RASTERELEMENT_H

#include "AppConfig.h"
#include "ComplexData.h"
#include "DataAccessor.h"
#include "DataElement.h"
#include "DimensionDescriptor.h"
#include "LocationType.h"

#include <string>
#include <vector>

class DataRequest;
class Georeference;
class Progress;
class RasterPager;
class Statistics;

/**
 *  A three-dimensional data set.
 *
 *  The RasterElement class is commonly used for three dimensional data sets that
 *  are processed by the user.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following method is called: updateData().
 *  - Everything else documented in DataElement.
 *
 *  @see   DataElement
 */
class RasterElement : public DataElement
{
public:
   /**
    *  Emitted with any<RasterElement*> when the associated terrain object is changed.
    */
   SIGNAL_METHOD(RasterElement, TerrainSet)

   /**
    *  Emitted when the RasterElement's georeferencing has been changed.
    */
   SIGNAL_METHOD(RasterElement, GeoreferenceModified);

   /**
    *  Emitted when the RasterElement's data has been changed.
    */
   SIGNAL_METHOD(RasterElement, DataModified);

   /**
    *  Returns an individual data value in the cube.
    *
    *  @param   column
    *           The column of the pixel to check.  This must be gotten from the 
    *           RasterDataDescriptor's column vector.
    *  @param   row
    *           The row of the pixel to check.  This must be gotten from the 
    *           RasterDataDescriptor's row vector.
    *  @param   band
    *           The band of the pixel to check.  A default-constructed value can be passed
    *           in if multiple bands are not present.  For a non-default value, it be gotten from the 
    *           RasterDataDescriptor's column vector.
    *  @param   component
    *           The complex data component for which to get the data value.  For non-complex
    *           data, this value is ignored.
    *
    *  @return  The data value at the given column, row, and band.  A value of 0.0 is
    *           returned if the pixel value could not be obtained.
    */
   virtual double getPixelValue(DimensionDescriptor column, DimensionDescriptor row,
      DimensionDescriptor band = DimensionDescriptor(), ComplexComponent component = COMPLEX_MAGNITUDE) const = 0;

   /**
    * Get a DataAccessor with the parameters contained within the given request.
    *
    * @param pRequest
    *        Requested access parameters.  If NULL, then the default parameters will
    *        be used.  This method takes ownership of the DataRequest object.
    *
    * @return A DataAccessor for the dataset.  If the request was invalid or
    *         unable to be filled, an invalid DataAccessor will be returned.
    *         Callers should check the returned object's DataAccessor::isValid().
    */
   virtual DataAccessor getDataAccessor(DataRequest *pRequest = NULL) = 0;

   /**
    * Get a DataAccessor with the parameters contained within the given request.
    *
    * This const overloaded method is exactly like the non-const version, except that
    * it cannot be used with a writable DataRequest.
    *
    * @param pRequest
    *        Requested access parameters.  If NULL, then the default parameters will
    *        be used.  This method takes ownership of the DataRequest object.
    *
    * @return A DataAccessor for the dataset.  If the request was invalid or
    *         unable to be filled, an invalid DataAccessor will be returned.
    *         Callers should check the returned object's DataAccessor::isValid().
    *
    * @see DataRequest::setWritable()
    */
   virtual DataAccessor getDataAccessor(DataRequest *pRequest = NULL) const = 0;

   /**
    *  Increments the Data Accessor to the next segment of memory.
    *
    *  The incrementDataAccessor() method steps to the next segment
    *  within the Matrix.  This notifies the Data Accessor that 
    *  the algorithm is complete with the current segment.
    *
    *  @see     DataAccessor
    */
   virtual void incrementDataAccessor(DataAccessorImpl& accessor) = 0;

   /**
    *  Notifies all observers of the object that its data has changed.
    *
    *  @notify  This method will notify RasterElement::signalDataModified.
    */
   virtual void updateData() = 0;

   /**
    *  Sanitize the data in the object.
    *
    *  This method will iterate over the dataset and replace all
    *  instances of floating point NaNs with the specified value.
    *
    *  @param   value
    *           The value to use for all instances of floating point NaNs.
    *           This value will be cast (via static_cast) to the underlying EncodingType of the dataset.
    *
    *  @return The number of data values which were sanitized.
    *         If this value is non-zero, RasterElement::updateData() will be called before returning.
    *
    *  @see RasterElement::updateData()
    */
   virtual uint64_t sanitizeData(double value = 0.0) = 0;

   /**
    *  Returns statistics for the given band data.
    *
    *  @param   band
    *           The band for which to get its statistics.
    *
    *  @return  A pointer to the Statistics object from which individual
    *           statistics values for the given band can be set or retrieved.
    *           If invalid, this will get the first band's statistics.
    */
   virtual Statistics* getStatistics(DimensionDescriptor band = DimensionDescriptor()) const = 0;

   /**
    * This method will create a new RasterElement which is a
    * chip of the object it is called on.  Its active row, column, and
    * band vectors will correctly refer to the same original and on-disk
    * numbers of the original cube.  The new cube will match the original
    * RasterElement's processing location (on-disk (not read-only) or in-memory).
    * Except for the name and parent, a RasterElement created with this function
    * should be indistinguishable from one chipped on import from the same file.
    *
    * The DimensionDescriptor vectors should be created by copying the descriptors
    * of the desired rows, columns, or bands from the DataDescriptor of the source RasterElement.
    * These must be in ascending order, without duplication.
    *
    *  @param   pParent
    *           The element to use for the parent of the created cube.
    *  @param   appendName
    *           What to append to the name of the RasterElement.
    *           ie. "_chip" would transform the name "A.sio" to "A_chip.sio".
    *           Passing an empty string as the appendName will result in using the RasterElement's
    *           name as the chipped name.
    *  @param   selectedRows
    *           The DimensionDescriptors (unmodified from this object) for the rows
    *           which should be included in this chip.
    *  @param   selectedColumns
    *           The DimensionDescriptors (unmodified from this object) for the rows
    *           which should be included in this chip.
    *  @param   selectedBands
    *           The DimensionDescriptors (unmodified from this object) for the rows
    *           which should be included in this chip.
    * 
    *  @return  A pointer to the created RasterElement
    *
    *  @see RasterElement::copy(), RasterElement::copyDataToChip()
    */
   virtual RasterElement *createChip(DataElement *pParent,
      const std::string &appendName, const std::vector<DimensionDescriptor> &selectedRows,
      const std::vector<DimensionDescriptor> &selectedColumns,
      const std::vector<DimensionDescriptor> &selectedBands = std::vector<DimensionDescriptor>()) const = 0;

   /**
    * This method will copy data from this RasterElement to the chip RasterElement.
    *
    * Only the cube data will be copied -- no other data from the DataDescriptor
    * will be copied.
    *
    * The DimensionDescriptor vectors should be created by copying the descriptors
    * of the desired rows, columns, or bands from the DataDescriptor of the source RasterElement.
    * These must be in ascending order, without duplication.
    *
    *  @param   pRasterChip
    *           The chip to copy data to.
    *  @param   selectedRows
    *           The DimensionDescriptors (unmodified from this object) for the rows
    *           which should be included in this chip.
    *  @param   selectedColumns
    *           The DimensionDescriptors (unmodified from this object) for the rows
    *           which should be included in this chip.
    *  @param   selectedBands
    *           The DimensionDescriptors (unmodified from this object) for the rows
    *           which should be included in this chip.
    *  @param   abort
    *           A flag which can be set externally to abort.
    *           Set this to true when abort is desired.
    *  @param   pProgress
    *           The progress object to report the current progress to.
    * 
    *  @return  True if the operation succeeded, false otherwise
    *
    *  @see RasterElement::copy(), RasterElement::createChip()
    */
   virtual bool copyDataToChip(RasterElement *pRasterChip, 
      const std::vector<DimensionDescriptor> &selectedRows,
      const std::vector<DimensionDescriptor> &selectedColumns,
      const std::vector<DimensionDescriptor> &selectedBands,
      bool &abort, Progress *pProgress = NULL) const = 0;

   /**
    *  Creates a new raster element with the same values as this element but without copying the raster data.
    *
    *  The method creates a new element based on the data contained in this
    *  element, which includes a copy of all data in the data descriptor. It is similar to
    *  RasterElement::copy() but does not copy the raw raster data so it is faster.
    *  This is useful if you are going to modify the raw raster data but otherwise would
    *  like a duplicate of the raster element.
    *
    *  @param   name
    *           The name for the created raster element, which can be the same
    *           as this object's name if the parent is different than this
    *           object's parent.
    *  @param   pParent
    *           The parent element for the created data element, which can be
    *           the same as this object's parent if the name is different
    *           than this object's name.
    *
    *  @return  A pointer to the new raster element.  \b NULL is returned if
    *           the element cannot be copied or if the given parent is the
    *           same as this object's parent and the given name is the same as this object's name.
    */
   virtual RasterElement* copyShallow(const std::string& name, DataElement* pParent) const = 0;

   /**
    *  Sets a terrain map of the RasterElement.
    *
    *  @param   pTerrain
    *           A pointer to an object containing terrain information of the data cube.
    *           It is assumed that the passed in RasterElement is coregistered with
    *           this RasterElement.
    *
    *  @notify  This method will notify signalTerrainSet() with any<RasterElement*>.
    */
   virtual void setTerrain(RasterElement* pTerrain) = 0;

   /**
    *  Gets a terrain map of the RasterElement.
    *
    *  NOTE: When the RasterElement is destroyed, it will destroy its associated terrain object.
    *        DO NOT attempt to set the same terrain object in two different RasterElements.
    *
    *  @return  A pointer to an object containing terrain information of the data cube. Returns NULL
    *           on RasterElements that have not had their terrain map set.
    */
   virtual const RasterElement* getTerrain() const = 0;

   /**
    *  Creates an empty temporary file.
    *
    *  This method creates an empty temporary file to be used for on-disk
    *  processing.  After creating the temp file, createMemoryMappedPager()
    *  is called to create a pager plug-in that uses the data
    *  parameters specificed in the data descriptor.  Therefore, there is
    *  no need to call createMemoryMappedPager() or setPager()
    *  after calling this method.
    *
    *  The size of the temporary file is the size of the subset specified in
    *  the data descriptor.  If no subset is specified, the size of the file
    *  equals the size of the data.  There are no header, trailer, preline,
    *  postline, preband, or postband bytes.
    *
    *  The created filename can be retrieved to copy data into the file by
    *  calling getTemporaryFilename().
    *
    *  @return  Returns \b true if the temporary file was created successfully;
    *           otherwise returns \b false.
    */
   virtual bool createTemporaryFile() = 0;

   /**
    *  Creates a default pager plug-in instance that will be used
    *  by this object to map data from the original file on disk into memory.
    *
    *  This method creates a default raster pager plug-in that is used to
    *  access the data using the data parameters specified in the file
    *  descriptor.  It is typically used in the read-only processing case.  To
    *  set a custom pager plug-in, call the setPager()
    *  method instead.
    *
    *  @return  Returns \b true if the default pager plug-in was
    *           successfully created; otherwise returns \b false.
    */
   virtual bool createMemoryMappedPager() = 0;

   /**
    *  Creates a default pager plug-in instance that will be used
    *  by this object to store data in memory.
    *
    *  This method creates a default raster pager plug-in that is used to
    *  access the data using the data parameters specified in the data
    *  descriptor.
    *
    *  This method does not usually need to be called from a plug-in.
    *  RasterElements with ProcessingLocation::IN_MEMORY automatically
    *  have an in-memory pager created for them.
    *
    *  @return  Returns \b true if the default pager plug-in was
    *           successfully created; otherwise returns \b false.
    */
   virtual bool createInMemoryPager() = 0;

   /**
    * If there is no pager set into the RasterElement, create a default
    * one.
    *
    * This method create an appropriate default pager, including blank space
    * to use for the data.
    *
    * @return True if there was already a pager or a default one was successfully
    *         created, false otherwise.
    */
   virtual bool createDefaultPager() = 0;

   /**
    *  Sets the raster pager plug-in instance that will be used by this
    *  object to access the data.
    *
    *  This method provides the means to set a custom pager plug-in
    *  that is used to access the data.  To create a default pager plug-in,
    *  call createMemoryMappedPager() instead.
    *
    *  @param   pPager
    *           The raster pager plug-in instance that should be used to
    *           page data into the application.
    *
    *  @return  Returns \b true if the given pager plug-in can be
    *           used by this object; otherwise returns \b false.
    */
   virtual bool setPager(RasterPager* pPager) = 0;

   /**
    * Returns the raster pager plug-in instance that will be used by this
    * object to access the data.
    *
    * @return Returns the raster pager plug-in instance used by this object.
    */
   virtual RasterPager* getPager() const = 0;

   /**
    *  Returns the filename that contains the data used for on-disk processing.
    *
    *  @return  The filename from which on-disk processing will occur.  An
    *           empty string is returned if there is no temporary file.
    */
   virtual const std::string& getTemporaryFilename() const = 0;

   /**
    *  Returns a read-only pointer to the data values in memory.
    *
    *  This method will return non-NULL only if the entire dataset can be
    *  accessed concurrently.  This will not be true for all RasterElements.
    *  This method should only be used to implement an optimized version of an
    *  algorithm.  The general case is to use a DataAccessor.
    *
    *  The calling object must interpret the handle according to the data type
    *  and interleave.  No conversions are available.
    *
    *  @return  A read-only pointer to the data if it can be accessed as a whole,
    *           NULL otherwise.
    *
    *  @see     RasterDataDescriptor::getDataType()
    */
   virtual const void* getRawData() const = 0;

   /**
    *  Returns a pointer to the data values in memory.
    *
    *  This method will return non-NULL only if the entire dataset can be
    *  accessed concurrently.  This will not be true for all RasterElements.
    *  This method should only be used to implement an optimized version of an
    *  algorithm.  The general case is to use a DataAccessor.
    *
    *  The calling object must interpret the handle according to the data type
    *  and interleave.  No conversions are available.
    *
    *  @return  A pointer to the data if it can be accessed as a whole,
    *           NULL otherwise.
    *
    *  @see     RasterDataDescriptor::getDataType()
    */
   virtual void* getRawData() = 0;

   /**
    *  Returns a geocoordinate corresponding to a given scene pixel location.
    *
    *  @param   pixel
    *           The scene pixel location as a LocationType.
    *  @param   quick
    *           Set this to true if less accurate results are acceptable
    *           in exchange for speed.
    *  @param   pAccurate
    *           Evaluation of the accuracy of the computed location as determined by the
    *           Georeference plug-in. When \c NULL, no accuracy check is performed.
    *
    *  @return  The corresponding geocoordinate as a LocationType.
    */
   virtual LocationType convertPixelToGeocoord(LocationType pixel, bool quick = false,
                                               bool* pAccurate = NULL) const = 0;

   /**
    *  Returns geocoordinates for multiple pixel locations.
    *
    *  This method uses the convertPixelToGeocoord() method to perform the
    *  conversion for each pixel location.
    *
    *  @param   pixels
    *           The pixel locations for which to get their geocoordinates.
    *  @param   quick
    *           Set this to true if less accurate results are acceptable
    *           in exchange for speed.
    *  @param   pAccurate
    *           Evaluation of the accuracy of the computed locations as determined by the
    *           Georeference plug-in. When \c NULL, no accuracy checks are performed.
    *
    *  @return  A vector containing the geocoordinates that correspond to
    *           each pixel location in the given vector.
    */
   virtual std::vector<LocationType> convertPixelsToGeocoords(const std::vector<LocationType>& pixels,
                                                              bool quick = false,
                                                              bool* pAccurate = NULL) const = 0;

   /**
    *  Returns a scene pixel location corresponding to a given geocoordinate.
    *
    *  @param   geocoord
    *           The geocoordinate as a LocationType.
    *  @param   quick
    *           Set this to true if less accurate results are acceptable
    *           in exchange for speed.
    *  @param   pAccurate
    *           Evaluation of the accuracy of the computed location as determined by the
    *           Georeference plug-in. When \c NULL, no accuracy check is performed.
    *
    *  @return  The corresponding pixel location as a LocationType.
    */
   virtual LocationType convertGeocoordToPixel(LocationType geocoord, bool quick = false,
                                               bool* pAccurate = NULL) const = 0;

   /**
    *  Returns pixel locations for multiple geocoordinates.
    *
    *  This method uses the convertGeocoordToPixel() method to perform the
    *  conversion for each geocoordinate.
    *
    *  @param   geocoords
    *           The geocoordinates for which to get the pixel locations.
    *  @param   quick
    *           Set this to true if less accurate results are acceptable
    *           in exchange for speed.
    *  @param   pAccurate
    *           Evaluation of the accuracy of the computed locations as determined by the
    *           Georeference plug-in. When \c NULL, no accuracy checks are performed.
    *
    *  @return  A vector containing the pixel locations that correspond to
    *           each geocoordinate in the given vector.
    */
   virtual std::vector<LocationType> convertGeocoordsToPixels(const std::vector<LocationType>& geocoords,
                                                              bool quick = false,
                                                              bool* pAccurate = NULL) const = 0;

   /**
    * Determine if the RasterElement has been georeferenced.
    *
    * @return True if the RasterElement is georeferenced, false otherwise.
    */
   virtual bool isGeoreferenced() const = 0;

   /**
    * Notify the RasterElement that the georeferencing data has been updated.
    *
    * This method is typically called by a Georeference plugin.
    *
    * @notify  This method will notify Raster::signalGeoreferenceModified.
    */
   virtual void updateGeoreferenceData() = 0;

   /**
    *  Set a plugin to perform all georeferencing computations.
    *
    * @param   pGeo
    *          The plugin to use for georeferencing.
    *
    * @notify  This method will notify Raster::signalGeoreferenceModified.
    */
   virtual void setGeoreferencePlugin(Georeference *pGeo) = 0;

   /**
    *  Get the plugin currently used.
    *
    * @return The plugin used.
    */
   virtual Georeference *getGeoreferencePlugin() const = 0;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~RasterElement() {}
};

#endif
