/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATAREQUEST_H
#define DATAREQUEST_H

#include "DimensionDescriptor.h"
#include "TypesFile.h"

class RasterDataDescriptor;

/**
 * DataRequest is a class used to specify how to get access to data
 * through a DataAccessor.
 *
 * To use, create an instance with the ObjectFactory or a FactoryResource.
 * Set the fields for which the defaults are insufficient.  Pass in the
 * instance to RasterElement::getDataAccessor.
 *
 * @see RasterElement, RasterDataDescriptor, DataAccessor
 */
class DataRequest
{
public:
   /**
    * Creates and returns a copy of the object.
    *
    * @return A new DataRequest, which is a copy of the existing one.
    *         The caller is responsible for ensuring deletion of the
    *         returned object.
    */
   virtual DataRequest *copy() const = 0;

   /**
    * Determine whether the DataRequest is valid for the given 
    * RasterDataDescriptor.
    *
    * The validation will most likely fail if the DataRequest has not
    * had polish() called on it.
    *
    * Plug-ins generally do not need to call this function.
    *
    * @param pDescriptor
    *        The descriptor to validate against.
    *
    * @return True if the DataRequest is a valid request for the
    *         descriptor, false otherwise.
    */
   virtual bool validate(const RasterDataDescriptor *pDescriptor) const = 0;

   /**
    * Polish the DataRequest for the given RasterDataDescriptor.
    *
    * This function will apply any defaults to the actual value as
    * appropriate for the given descriptor.  This function should be called
    * before validate().
    *
    * The return values of accessor functions are not valid
    * until this function has been called.
    *
    * Plug-ins generally do not need to call this function, as the core 
    * will call it when appropriate.
    *
    * @param pDescriptor
    *        The descriptor from which to apply defaults.
    *
    * @return True if the operation succeeded, false otherwise.
    */
   virtual bool polish(const RasterDataDescriptor *pDescriptor) = 0;

   /**
    * Get the version required to support this request.
    *
    * @param pDescriptor
    *        The descriptor to use to determine required version.
    *
    * @return The smallest version number which can properly use this
    *         DataRequest.  Currently always returns 1.
    *
    * @see RasterPager::getSupportedRequestVersion()
    */
   virtual int getRequestVersion(const RasterDataDescriptor *pDescriptor) const = 0;

   /**
    * Get the requested interleave.
    *
    * This defaults to the interleave of the associated RasterElement.
    *
    * @return The requested interleave. If the interleave has not been set,
    *         either with polish() or setInterleaveFormat(), the return
    *         value is undefined.
    */
   virtual InterleaveFormatType getInterleaveFormat() const = 0;

   /**
    * Set the requested interleave.
    *
    * @param interleave
    *        The requested interleave.
    */
   virtual void setInterleaveFormat(InterleaveFormatType interleave) = 0;

   /**
    * Get the requested start row.
    *
    * This defaults to the first row in the RasterElement.
    *
    * @return The requested start row.
    *
    * @see setRows()
    */
   virtual DimensionDescriptor getStartRow() const = 0;

   /**
    * Get the requested stop row.
    *
    * This defaults to the last row in the RasterElement.
    *
    * @return The requested stop row.
    *
    * @see setRows()
    */
   virtual DimensionDescriptor getStopRow() const = 0;

   /**
    * Get the request concurrent rows.
    *
    * This defaults to 1.
    *
    * @return The requested number of concurrent rows.
    *
    * @see setRows()
    */
   virtual unsigned int getConcurrentRows() const = 0;

   /**
    * Set the start, stop, and concurrent rows.
    *
    * @param startRow
    *        The requested start row.  This may be an invalid DimensionDescriptor
    *        to apply the default.
    * @param stopRow
    *        The requested stop row.  This may be an invalid DimensionDescriptor
    *        to apply the default.
    * @param concurrentRows
    *        The requested number of concurrent rows.  This may be 0 to apply
    *        the default.
    *
    * @see getStartRow(), getStopRow(), getConcurrentRows()
    */
   virtual void setRows(DimensionDescriptor startRow, DimensionDescriptor stopRow, unsigned int concurrentRows = 0) = 0;

   /**
    * Get the requested start column.
    *
    * This defaults to the first column in the RasterElement.
    *
    * @return The requested start column.
    *
    * @see setColumns()
    */
   virtual DimensionDescriptor getStartColumn() const = 0;

   /**
    * Get the requested stop column.
    *
    * This defaults to the last column in the RasterElement.
    *
    * @return The requested stop column.
    *
    * @see setColumn()
    */
   virtual DimensionDescriptor getStopColumn() const = 0;
  
   /**
    * Get the request concurrent columns.
    *
    * This defaults to the number of columns between start and stop.
    *
    * @return The requested number of concurrent column.
    *
    * @see setColumn()
    */
   virtual unsigned int getConcurrentColumns() const = 0;

   /**
    * Set the start, stop, and concurrent column.
    *
    * @param startColumn
    *        The requested start column.  This may be an invalid DimensionDescriptor
    *        to apply the default.
    * @param stopColumn
    *        The requested stop column.  This may be an invalid DimensionDescriptor
    *        to apply the default.
    * @param concurrentColumns
    *        The requested number of concurrent columns.  This may be 0 to apply
    *        the default.
    *
    * @see getStartColumn(), getStopColumn(), getConcurrentColumns()
    */
   virtual void setColumns(DimensionDescriptor startColumn, DimensionDescriptor stopColumn, unsigned int concurrentColumns = 0) = 0;

   /**
    * Get the requested start band.
    *
    * This defaults to the first band in the RasterElement.
    *
    * @return The requested start band.
    *
    * @see setBands()
    */
   virtual DimensionDescriptor getStartBand() const = 0;

   /**
    * Get the requested stop band.
    *
    * This defaults to the last band in the RasterElement for BIP and BIL data, or
    * the requested start band for BSQ data.
    *
    * @return The requested stop band.
    *
    * @see setBands()
    */
   virtual DimensionDescriptor getStopBand() const = 0;
 
   /**
    * Get the request concurrent bands.
    *
    * This defaults to the number of bands between the start and stop.
    *
    * @return The requested number of concurrent bands.
    *
    * @see setBands()
    */
   virtual unsigned int getConcurrentBands() const = 0;

   /**
    * Set the start, stop, and concurrent bands.
    *
    * @param startBand
    *        The requested start band.  This may be an invalid DimensionDescriptor
    *        to apply the default.
    * @param stopBand
    *        The requested stop band.  This may be an invalid DimensionDescriptor
    *        to apply the default.
    * @param concurrentBands
    *        The requested number of concurrent bands.  This may be 0 to apply
    *        the default.
    *
    * @warning BSQ only supports accessing a single band at a time.
    *
    * @see getStartBand(), getStopBand(), getConcurrentBands()
    */
   virtual void setBands(DimensionDescriptor startBand, DimensionDescriptor stopBand, unsigned int concurrentBands = 0) = 0;

   /**
    * Get whether the request is for writable data.
    *
    * This defaults to false.
    *
    * @return True if the request is for writable data, false otherwise.
    *
    * @see setWritable()
    */
   virtual bool getWritable() const = 0;

   /**
    * Set whether the request is for writable data.
    *
    * It is undefined what will happen if data is written
    * from a DataAccessor requested with an unwritable DataRequest.
    * Depending on the circumstances, it may apply the written data,
    * ignore the written data, crash, or lead to other undesirable results.
    * \b Always set the request to writable if you want to write to a RasterElement.
    *
    * @param writable
    *        True if the request is for writable data, false otherwise.
    *
    * @see getWritable(), RasterElement::updateData()
    */
   virtual void setWritable(bool writable) = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~DataRequest() {}
};

#endif
