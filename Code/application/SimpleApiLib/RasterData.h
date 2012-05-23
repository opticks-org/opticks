/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIMPLEAPIDATAACCESSOR_H
#define SIMPLEAPIDATAACCESSOR_H

#include "AppConfig.h"

class BadValues;
class DataAccessorImpl;
class DataElement;
class RasterElement;

#ifdef __cplusplus
extern "C"
{
#endif
   /** \addtogroup simple_api */
   /*@{*/

   /**
    * @file RasterData.h
    * This file contains API utilities for accessing raster data.  Many of these methods are wrappers around methods in
    * RasterElement, RasterDataDescriptor, RasterUtilities, DataRequest, and DataAccessorImpl.
    */

   /**
    * Descriptor for data dimensions.  The values returned here reflect currently active values, not necessarily those
    * found in the file.  This can occur if the data set was subcubed or if the interleave was converted during import.
    */
   struct DataInfo
   {
      uint32_t numRows;          /**< The number of rows in the data. */
      uint32_t numColumns;       /**< The number of columns in the data. */
      uint32_t numBands;         /**< The number of bands in the data. */
      uint32_t interleaveFormat; /**< 0 -> BSQ, 1 -> BIP, 2 -> BIL. @see InterleaveFormatType */
      uint32_t encodingType;     /**< 0 -> char, 1 -> unsigned char, 2 -> short, 3 -> unsigned short,
                                      4 -> complex short, 5 -> int, 6 -> unsigned int, 7 -> float,
                                      8 -> complex float, 9 -> double.  @see EncodingType */
      uint32_t encodingTypeSize; /**< The number of bytes per element.  @see RasterUtilities::bytesInEncoding() */
      BadValues* pBadValues;     /**< The bad values instance for this data set or
                                      \c NULL if the bad values vary by band. */
   };

   /**
    * Obtain data dimensions for the given RasterElement.
    *
    * @param pElement
    *        The RasterElement to query.
    * @return A fully populated DataInfo for the given RasterElement.
    *         On failure, \c NULL is returned and getLastError() may be queried for information on the error.
    *         Ownership is transfered to the called and the memory must be freed by calling destroyDataInfo().
    *
    * @see getDataElement(), destroyDataInfo()
    */
   EXPORT_SYMBOL DataInfo* createDataInfo(DataElement* pElement);

   /**
    * Destroy a pointer obtained by calling createDataInfo().
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pDataInfo
    *        A DataInfo obtained by calling createDataInfo.
    *
    * @see createDataInfo()
    */
   EXPORT_SYMBOL void destroyDataInfo(DataInfo* pDataInfo);

   /**
    * Descriptor for creating a RasterElement.
    */
   struct RasterElementArgs
   {
      uint32_t numRows;          /**< The number of rows in the data. */
      uint32_t numColumns;       /**< The number of columns in the data. */
      uint32_t numBands;         /**< The number of bands in the data. */
      uint32_t interleaveFormat; /**< 0 -> BSQ, 1 -> BIP, 2 -> BIL.  @see InterleaveFormatType */
      uint32_t encodingType;     /**< 0 -> char, 1 -> unsigned char, 2 -> short, 3 -> unsigned short,
                                      4 -> complex short, 5 -> int, 6 -> unsigned int, 7 -> float,
                                      8 -> complex float, 9 -> double.  @see EncodingType */
      uint32_t location;         /**< 0 -> Prefer RAM but allow on-disk, 1 -> Only RAM, 2 -> Only on-disk */
      DataElement* pParent;      /**< The parent of this RasterElement or \c NULL if it has no parent. */
      BadValues* pBadValues;     /**< Pointer to the bad values for this data set or
                                      \c NULL if the bad values vary by band. */
   };

   /** 
    * Creates a RasterElement with the given parameters that can be immediately used.  This method
    * should only be used by plug-ins that need to programmatically create a RasterElement to store
    * results of an algorithm.  It should NOT be used by importers or exporters to create a RasterElement
    * It should also NOT be used to create a RasterElement that corresponds to a data file on the filesystem.
    *
    * @param pName
    *        A \c NULL terminated C-style string containing the name for the new RasterElement. This cannot be \c NULL.
    * @param args
    *        The structure containing information to process the request.
    * @return A RasterElement created with the given parameters that requires no additional initialization.
    *
    * @see destroyDataElement()
    */
   EXPORT_SYMBOL DataElement* createRasterElement(const char* pName, RasterElementArgs args);

   /** 
    * Creates a BadValues instance with the given parameters.
    *
    * @param pBadValuesStr
    *        A \c NULL terminated C-style string containing the string representation for the bad values.
    *        This cannot be \c NULL.
    * @return A BadValues instance created with the given bad values string that requires no additional initialization.
    *
    *@note    Examples of bad value strings:
    *             Lower threshold of -0.5, upper threshold +2.0, individual value of 0.0 is "<-0.5, 0.0, >2.0".
    *             Lower threshold of -0.5, upper threshold +2.0, range of +0.5 to +0.75 is "<-0.5, 0.5<>0.75, >2.0".
    */
   EXPORT_SYMBOL BadValues* createBadValues(const char* pBadValuesStr);

    /**
    * Determines if a data value is bad.
    *
    * @param pBadValues
    *        The BadValues to use to evaluate the quality of the data value.
    *        If pBadValues is \c NULL, all data values will be evaluated as good.
    * @param value
    *        The data value to be evaluated.
    * @return Returns 0 if the data value is bad or non-zero if it is good.
    *
    * @see BadValues::isBadValue()
    */
   EXPORT_SYMBOL int isDataValueBad(BadValues* pBadValues, double value);

   /**
    * Destroy a pointer obtained by calling createBadValues().
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pBadValues
    *        A BadValues obtained by calling createBadValues.
    *
    * @see createBadValues()
    */
   EXPORT_SYMBOL void destroyBadValues(BadValues* pBadValues);

   /**
    * Descriptor for data access.
    * Rows, columns, and bands are all 0-based and reflect active numbers.
    *
    * @see DataRequest
    */
   struct DataPointerArgs
   {
      uint32_t rowStart;         /**< The first row to access. */
      uint32_t rowEnd;           /**< The last row to access. */

      uint32_t columnStart;      /**< The first column to access. */
      uint32_t columnEnd;        /**< The last column to access. */

      uint32_t bandStart;        /**< The first band to access. */
      uint32_t bandEnd;          /**< The last band to access. */

      uint32_t interleaveFormat; /**< 0 -> BSQ, 1 -> BIP, 2 -> BIL.  @see InterleaveFormatType */
   };

   /**
    * Obtain a writable pointer to raw data in memory which must be destroyed by calling destroyDataPointer().
    * In some cases this method returns a pointer to the original data. In other cases a copy must be created.
    *
    * @param pElement
    *        The RasterElement to access.
    * @param pArgs
    *        The structure containing information to process the request or \c NULL to access the entire cube.
    * @param pOwn
    *        An output parameter which will contain 0 if the caller does not take ownership of the return value
    *        and a non-zero if the caller takes ownership of the return value. If this is non-zero, the caller
    *        must release the memory by calling destroyDataPointer(). If this is 0, the return value points to
    *        the original data and any modifications to this buffer will modify the original data.
    * @return A writable pointer to a three-dimensional matrix containing raster data.
    *         On failure, \c NULL is returned and getLastError() may be queried for information on the error.
    * @see getDataElement(), destroyDataPointer(), createDataAccessor(),
    *      updateDataElement(), RasterElement::getRawData()
    */
   EXPORT_SYMBOL void* createDataPointer(DataElement* pElement, DataPointerArgs* pArgs, int* pOwn);

   /**
    * Destroy a pointer obtained by calling createDataPointer().
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pData
    *        The value returned by createDataPointer().
    *
    * @see createDataPointer(), updateDataElement()
    */
   EXPORT_SYMBOL void destroyDataPointer(void* pData);

   /**
    * Copy data from a contiguous C array in a RasterElement.
    *
    * The caller should call updateRasterElement() to redisplay the data.
    *
    * @param pElement
    *        The RasterElement to access.
    * @param pArgs
    *        The structure containing information to process the request.
    *        This specifies the parameters for the DataAccessor which will
    *        be used to write to the RasterElement. If this is \c NULL and
    *        RasterElement::getRawData() returns a valid pointer, a single
    *        memcpy() will be used to replace the contents of the RasterElement.
    * @param pData
    *        A pointer to a three-dimensional matrix containing raster data.
    *        The format of this data must match the information in pArgs or the RasterElement
    *        parameters if pArgs is \c NULL. If necessary, the caller must reformat this data
    *        prior to copying it to the RasterElement.
    * @return a non-zero on failure or a zero on success.
    * @see updateDataElement()
    */
   EXPORT_SYMBOL int copyDataToRasterElement(DataElement* pElement, DataPointerArgs* pArgs, void* pData);

   /**
    * Notfiy %Opticks that a RasterElement's data has changed.
    *
    * @param pElement
    *        The RasterElement to update.
    */
   EXPORT_SYMBOL void updateRasterElement(DataElement* pElement);

   /**
    * Descriptor for data access.
    * Rows, columns, and bands are all 0-based and reflect active numbers.
    *
    * @see DataRequest
    */
   struct DataAccessorArgs
   {
      uint32_t rowStart;         /**< The first row to access. */
      uint32_t rowEnd;           /**< The last row to access. */
      uint32_t concurrentRows;   /**< The number of rows to access concurrently or 0 to use the default. */

      uint32_t columnStart;      /**< The first column to access. */
      uint32_t columnEnd;        /**< The last column to access. */
      uint32_t concurrentColumns;/**< The number of columns to access concurrently or 0 to use the default. */

      uint32_t bandStart;        /**< The first band to access. */
      uint32_t bandEnd;          /**< The last band to access. */
      uint32_t concurrentBands;  /**< The number of bands to access concurrently or 0 to use the default. */

      uint32_t interleaveFormat; /**< 0 -> BSQ, 1 -> BIP, 2 -> BIL.  @see InterleaveFormatType */
      uint32_t writable;         /**< 0 -> Do not request write access, Any other value -> Request write access. */
   };

   /**
    * Create a DataAccessor for accessing raster data which must be destroyed by calling destroyDataAccessor().
    *
    * @param pElement
    *        The RasterElement to access.
    *
    * @param pArgs
    *        The structure containing information to process the request or \c NULL to use default arguments.
    *        Default arguments are determined heuristically based on the data set.
    *
    * @return A newly-created DataAccessor which must be destroyed by calling destroyDataAccessor.
    *         On failure, \c NULL is returned and getLastError() may be queried for information on the error.
    *         Note this is actually a DataAccessorImpl since DataAccessor is used for reference counting in C++.
    *
    * @see getDataElement(), destroyDataAccessor()
    */
   EXPORT_SYMBOL DataAccessorImpl* createDataAccessor(DataElement* pElement, DataAccessorArgs* pArgs);

   /**
    * Destroy a DataAccessorImpl created with createDataAccessor().
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pAccessor
    *        The DataAccessorImpl to destroy.
    */
   EXPORT_SYMBOL void destroyDataAccessor(DataAccessorImpl* pAccessor);

    /**
    * Determines if the DataAccessor is valid.
    *
    * @param pAccessor
    *        The DataAccessorImpl to use.
    * @return Returns 0 if the data accessor is not valid or non-zero if it is valid.
    *
    * @see DataAccessor::isValid()
    */
   EXPORT_SYMBOL int isDataAccessorValid(DataAccessorImpl* pAccessor);

   /**
    * Gets a row of values.
    *
    * This method returns a pointer to the first element in an array that is
    * guaranteed to be at least (concurrent columns * number of bands) in size
    * for BIP data and (concurrent columns) in size for BSQ data.
    *
    * @param pAccessor
    *        The DataAccessorImpl to use.
    * @return Pointer to the raw block of data from the dataset.
    *         On failure, \c NULL is returned and getLastError() may be queried for information on the error.
    *
    * @see isValid(), DataAccessorImpl::getRow()
    */
   EXPORT_SYMBOL void* getDataAccessorRow(DataAccessorImpl* pAccessor);

   /**
    * Advances to a succeeding row in the dataset.
    *
    * @param pAccessor
    *        The DataAccessorImpl to use.
    * @param count
    *        The number of rows to advance.
    * @param resetColumn
    *        non-zero -> Reset the column to the beginning column of the accessor, 0 -> Maintain the current column.
    *
    * @see DataAccessorImpl::nextRow(int, bool)
    */
   EXPORT_SYMBOL void nextDataAccessorRow(DataAccessorImpl* pAccessor, uint32_t count, int resetColumn);

   /**
    * Gets a column of values.
    *
    * This method returns a pointer to the first element in an array
    * that is guaranteed to be at least the number of bands in size
    * for BIP data and one in size for BSQ data.
    *
    * @param pAccessor
    *        The DataAccessorImpl to use.
    * @return Pointer to the raw block of data from the data set.
    *         On failure, \c NULL is returned and getLastError() may be queried for information on the error.
    *
    * @see DataAccessorImpl::getColumn()
    */
   EXPORT_SYMBOL void* getDataAccessorColumn(DataAccessorImpl* pAccessor);

   /**
    * Advances to a succeeding column in the dataset.
    *
    * @param pAccessor
    *        The DataAccessorImpl to use.
    * @param count
    *        The number of columns to advance.
    *
    * @see DataAccessorImpl::nextColumn(int)
    */
   EXPORT_SYMBOL void nextDataAccessorColumn(DataAccessorImpl* pAccessor, uint32_t count);

   /**
    * Jumps to the specified pixel.
    *
    * @param pAccessor
    *        The DataAccessorImpl to use.
    * @param row
    *        The row to access.  This must be less than the total number of rows.
    * @param column
    *        The column to access.  This must be less than the total number of columns.
    *
    * @see DataAccessorImpl::toPixel(int, int)
    */
   EXPORT_SYMBOL void toDataAccessorPixel(DataAccessorImpl* pAccessor, uint32_t row, uint32_t column);

   /**
    * Returns the RasterElement associated with this DataAccessor.
    *
    * @param pAccessor
    *        The DataAccessor to use.
    * @return Returns the RasterElement associated with this class.
    *         On failure, \c NULL is returned and getLastError() may be queried for information on the error.
    *
    * @see DataAccessorImpl::getAssociatedRasterElement()
    */
   EXPORT_SYMBOL RasterElement* getDataAccessorRasterElement(DataAccessorImpl* pAccessor);

   /**
    * Access the size in bytes of a row of data.
    *
    * This includes the number of bytes of actual data in a row
    * and excludes preline and postline bytes.
    *
    * @note If using this information to access an entire row
    *       of data with getDataAccessorRow(), you must ensure that the DataAccessor's
    *       concurrent columns (and concurrent bands if the accessor is BIP)
    *       is equal to the number of columns in the data set.
    *
    * @param pAccessor
    *        The DataAccessorImpl to use.
    * @return The number of bytes in a row of data excluding preline and postline bytes.
    *
    * @see DataAccessorImpl::getRowSize()
    */
   EXPORT_SYMBOL uint32_t getDataAccessorRowSize(DataAccessorImpl* pAccessor);

   /*@}*/
#ifdef __cplusplus
}
#endif

#endif
