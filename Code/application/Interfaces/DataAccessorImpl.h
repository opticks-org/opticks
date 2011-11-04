/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATA_ACCESSOR_IMPL_H
#define DATA_ACCESSOR_IMPL_H

#include "AppConfig.h"
#include "RasterElement.h"
#include "RasterPager.h"
#include "DataRequest.h"
#include "TypesFile.h"
#include "ObjectResource.h"
#include <exception>
#include <stdexcept>

class RasterPage;

typedef double (*convertToDouble)(const void*, int, ComplexComponent);
typedef int64_t (*convertToInteger)(const void*, int, ComplexComponent);

/**
 * Provides a generic interface to the dataset.
 *
 * This class provides an efficient, generic interface to the raw dataset.  To 
 * provide efficient access to the raw data, range checking can not occur 
 * within this class.  It is up to the user of the DataAcessor to make 
 * sure the bounds of the dataset are not exceeded.  This class provides 
 * row-major access to the data.  All of the standard interleaves (BIP, 
 * BIL, or BSQ) are row-major, so this class doesn't provide column-major access. 
 *
 * @code 
 * The following is the in the order for the dataset:
 *    BIP: ( Maj, Mid, Min = Row, Col, Band )
 *    BIL: ( Maj, Mid, Min = Row, Band, Col  (NOT IMPLEMENTED))
 *    BSQ: ( Maj, Mid, Min = Band, Row, Col )
 * @endcode
 *
 * This class is not instantiated by a plug-in directly.  The plug-in developer
 * can get access to this class through the RasterElement::getDataAccessor() 
 * method.  One example is to call getDataAccessor() with a DataRequest with 
 * concurrentColumns == total columns, concurrentBands == 1, and 
 * the default interleave.
 *
 * @code
 *   for each row
 *      for each column
 *         value = *getColumn()
 *         // Do something useful with the value.
 *       nextColumn()
 *    nextRow()
 * @endcode
 *
 * @see      RasterElement::getDataAccessor()
 */
class DataAccessorImpl
{
public:
   /**
    *  DataAcessorImpl constructor.
    *
    *  This is the constructor for the data accessor.  This constructor should
    *  not be called directly from a plug-in.  Use RasterElement::getDataAccessor()
    *  to get a DataAccessor instance.
    *
    *  @param   pPage
    *           A pointer to a page of the raw dataset.  The size and ownership 
    *           of the data pointed to by mpPage is dependent on the interleave, 
    *           the concurrent elements and whether the data is loaded into memory 
    *           or not.
    *  @param   pRequest
    *           The DataRequest for this accessor.  DataAccessorImpl takes ownership
    *           of the request.
    *  @param   concurrentRows
    *           The number of rows provided by the RasterPager.
    *  @param   interLineBytes
    *           The number of bytes following a row of data in the dataset. This includes bytes that come before the next line.
    *  @param   concurrentColumns
    *           The number of columns provided by the RasterPager.
    *  @param   concurrentBands
    *           The number of bands provided by the RasterPager.
    *  @param   elementSize
    *           The size of the individual type referenced in the dataset 'matrix'.  For 
    *           example, sizeof(long), sizeof(float), etc.
    *  @param   pRasterElement
    *           This is a reference to the RasterElement class "owning" this DataAccessor.
    */
   DataAccessorImpl(char *pPage, DataRequest *pRequest, size_t concurrentRows,
      size_t interLineBytes, size_t concurrentColumns,
      size_t concurrentBands, 
      size_t elementSize, RasterElement* pRasterElement) :
      mbValid(true),
      mpPage(pPage),
      mpRasterElement(pRasterElement),
      mpRasterPage(NULL),
      mpRasterPager(NULL),
      mpRequest(pRequest),
      mConcurrentRows(concurrentRows),
      mConcurrentColumns(concurrentColumns),
      mConcurrentBands(concurrentBands),
      mCurrentRow(0),
      mCurrentColumn(0),
      mRowOffset(0),
      mColumnOffset(0),
      mRefCount(0),
      mConvertToDoubleFunc(NULL),
      mConvertToIntegerFunc(NULL)
   {
      if (pPage == NULL || mpRasterElement == NULL || mpRequest.get() == NULL)
      {
         throw std::logic_error("DataAccessorImpl constructor received NULL pointer");
      }
      mAccessorRow = mpRequest->getStartRow().getActiveNumber();
      mAccessorColumn = mpRequest->getStartColumn().getActiveNumber();
      mAccessorBand = mpRequest->getStartBand().getActiveNumber();
      updateDataSizes(elementSize, interLineBytes);
   }

   /**
    *  DataAcessorImpl destructor.
    */
   ~DataAccessorImpl()
   {
      if ( (mpRasterPager) && (mpRasterPage) )
      {
         mpRasterPager->releasePage(mpRasterPage);
      }
   }

   /**
    *  Gets a row of values.
    *
    *  This method returns a pointer to the first element in an array that is
    *  guaranteed to be at least (concurrent columns * number of bands) in size
    *  for BIP data and (concurrent columns) in size for BSQ data.
    *
    *  @return  Pointer to the raw block of row data from the dataset.
    */
   inline void *getRow()
   { 
      return &mpPage[mRowOffset];
   }

   /**
    *  Advances to the next row in the dataset.
    *
    *  This method increments to the next row based on the interleave
    *  BIP, BSQ, or BIL.
    *
    *  @param   resetColumn
    *           Whether or not to reset the column to the beginning column of the accessor.
    */
   inline void nextRow(bool resetColumn=true) 
   {
      ++mCurrentRow; 
      mRowOffset += mRowSize; 
      if (resetColumn) 
      {
         mCurrentColumn = mColumnOffset = 0;
      } 
      updateIfNeeded();
   }

   /**
    *  Advances to a succeeding row in the dataset.
    *
    *  This method increments to a later row based on the interleave
    *  BIP, BSQ, or BIL.
    *
    *  @param   count
    *           The number of rows to advance
    *  @param   resetColumn
    *           Whether or not to reset the column to the beginning column of the accessor.
    */
   inline void nextRow(int count, bool resetColumn=true) 
   { 
      if (count==1) 
      { 
         nextRow(); 
      } 
      else 
      {
         mCurrentRow+=count; 
         mRowOffset += mRowSize*count; 
         if (resetColumn) 
         {
            mCurrentColumn=mColumnOffset=0;
         } 
         updateIfNeeded(); 
      } 
   }

   /**
    *  Gets a column of values.
    *
    *  This method returns a pointer to the first element in an array
    *  that is guaranteed to be at least the number of bands in size
    *  for BIP data and one in size for BSQ data.
    *
    *  @return  Pointer to the raw block of column data from the data set.
    */
   inline void *getColumn() 
   { 
      return &mpPage[mRowOffset + mColumnOffset]; 
   }

   /**
    * Returns the data for the current column position as a double.
    *
    * @param iIndex
    *        The number of elements past the current column position to access.
    *        No bounds checking is performed on the provided index.
    * @param component
    *        For complex data, this specifies the component of the complex
    *        data that should be returned. If an invalid enum value is used
    *        for complex data, a value of 0 will be returned.  For non-complex data, this
    *        value is ignored.
    * @return The value at the given iIndex past the current column, returned as a double.
    */
   inline double getColumnAsDouble(int iIndex = 0, ComplexComponent component = COMPLEX_MAGNITUDE) const
   {
      return mConvertToDoubleFunc(&mpPage[mRowOffset + mColumnOffset], iIndex, component);
   }

   /**
    * Returns the data for the current column position as an integer.
    * This method will truncate the value if the underlying data is stored in floating point.
    *
    * @param iIndex
    *        The number of elements past the current column position to access.
    *        No bounds checking is performed on the provided index.
    * @param component
    *        For complex data, this specifies the component of the complex
    *        data that should be returned. If an invalid enum value is used
    *        for complex data, a value of 0 will be returned.  For non-complex data, this
    *        value is ignored.
    * @return The value at the given iIndex past the current column, returned as an integer.
    */
   int64_t getColumnAsInteger(int iIndex = 0, ComplexComponent component = COMPLEX_MAGNITUDE) const
   {
      return mConvertToIntegerFunc(&mpPage[mRowOffset + mColumnOffset], iIndex, component);
   }

   /**
    *  Advances to the next column in the dataset.
    *
    *  This method increments to the next column based on the interleave
    *  BIP, BSQ, or BIL.
    */
   inline void nextColumn() 
   { 
      ++mCurrentColumn; 
      mColumnOffset += mColumnSize; 
   }

   /**
    *  Advances to a succeeding column in the dataset.
    *
    *  This method increments to a later column based on the interleave
    *  BIP, BSQ, or BIL.
    *
    *  @param   count
    *           The number of columns to advance.
    */
   inline void nextColumn(int count) 
   { 
      if (count==1) 
      { 
         nextColumn(); 
      } 
      else 
      {
         mCurrentColumn+=count; 
         mColumnOffset += mColumnSize*count; 
      } 
   }

   /**
    *  Jumps to the specified pixel in the current band.
    *
    *  This method updates the current offset to the given pixel at row, column
    *  in the current band.
    *
    *  @param   row
    *           The row to access in the current band.  This must be non-negative
    *           and less than the total number of rows.
    *  @param   column
    *           The column to access in the current band.  This must be 
    *           non-negative and less than the total number of columns.
    */
   inline void toPixel(int row, int column) 
   { 
      mCurrentRow = row-mAccessorRow; 
      mCurrentColumn = column-mAccessorColumn; 
      mRowOffset = mCurrentRow * mRowSize;
      mColumnOffset = mCurrentColumn * mColumnSize;
      updateIfNeeded(); 
   }

   /**
    *  Returns the RasterElement associated with this DataAccessor.
    *
    *  @return  Returns the RasterElement associated with this class.
    */
   inline RasterElement *getAssociatedRasterElement()
   {
      return mpRasterElement;
   }

   /**
    *  Returns whether this is a valid data accessor.
    *
    *  A valid data accessor means that calls to getRow() and getColumn() will point
    *  to a valid location in memory.  This method can be called between nextRow()
    *  and getRow() or getColumn() calls.
    *
    *  @return  Returns whether this data accessor is valid, and subsequent calls to
    *           getRow() or getColumn() are valid pointers in memory.
    *
    *  @see     DataAccessor::isValid()
    */
   inline bool isValid() const 
   { 
      return mbValid; 
   }

   /**
    *  Increases the number of users of this data accessor.
    *
    *  This is simple reference counting mechanism to track the number
    *  of users of the data accessor.
    *
    *  @return  Returns the current number of users of this data accessor.
    */
   inline int incrementRefCount() 
   { 
      mRefCount++;
      return mRefCount; 
   }

   /**
    *  Decreases the number of users of this data accessor.
    *
    *  This is simple reference counting mechanism to track the number
    *  of users of the data accessor.
    *
    *  @return  Returns the current number of users of this data accessor.
    */
   inline int decrementRefCount()
   { 
      mRefCount--; 
      return mRefCount; 
   }

   /**
    *  Access the size in bytes of a row of data.
    *
    *  This includes the number of bytes of actual data in a row
    *  and excludes preline and postline bytes.
    *
    *  @note If using this information to access an entire row
    *        of data with getRow(), you must ensure that the DataAccessor's
    *        concurrent columns (and concurrent bands if the accessor is BIP)
    *        is equal to the number of columns in the data set.
    *
    *  @return The number of bytes in a row of data excluding preline and postline bytes.
    */
   inline size_t getRowSize() const
   {
      return mRowSize - mInterlineBytes;
   }

   /**
    *  Access the number of columns available concurrently.
    *
    *  @return The number of concurrent columns.
    *
    *  @see getRow()
    */
   inline size_t getConcurrentColumns() const
   {
      return mConcurrentColumns;
   }

private:
   friend class RasterElementImp;

   /**
    *  Jumps to the next column and row.
    *
    *  This method enables the DataAccessor to update the view into the dataset
    *  or read in the next block of data, if necessary.
    */
   inline void updateIfNeeded()
   {
      if (mCurrentRow >= mConcurrentRows)
      {
         if (mpRasterElement == NULL)
         {
            throw std::logic_error("DataAccessor back-pointer to data cube has become corrupted");
         }
         mpRasterElement->incrementDataAccessor(*this);
         mCurrentRow = 0;
         mRowOffset = 0;
      }
   }

   /**
    *  Calculate the column and row size depending on the interleave format
    *  and number of concurrentColumns, concurrentRows and concurrentBands.
    *
    *  @param elementSize
    *         the number of bytes needed to represent a single pixel in memory.
    *
    *  @param interLineBytes
    *         the number of bytes between each line that do not contain
    *         raw data. This includes bytes which come before the next line.
    */
   void updateDataSizes(size_t elementSize, size_t interLineBytes)
   {
      mInterlineBytes = interLineBytes;
      switch(mpRequest->getInterleaveFormat())
      {
      case BIP:
         mColumnSize = elementSize * mConcurrentBands;
         mRowSize = mColumnSize * mConcurrentColumns + interLineBytes;
         break;
      case BIL:
         mColumnSize = elementSize;
         mRowSize = mColumnSize * mConcurrentColumns * mConcurrentBands + interLineBytes;
         break;
      case BSQ:
         mColumnSize = elementSize;
         mRowSize = mColumnSize * mConcurrentColumns + interLineBytes;
         break;
      default:
         throw std::logic_error("DataAccessorImpl constructor received unknown interleave");
      }
   }

   bool mbValid;
   char *mpPage;
   RasterElement *mpRasterElement;
   RasterPage* mpRasterPage;
   RasterPager* mpRasterPager;
   FactoryResource<DataRequest> mpRequest;
   size_t mConcurrentRows;             // Number of rows currently available in memory at once.
   size_t mConcurrentColumns;          // Number of columns currently available in memory at once.
   size_t mConcurrentBands;            // Number of bands currently available in memory at once.
   size_t mCurrentRow;                 // Current processing row
   size_t mCurrentColumn;              // Current processing column
   size_t mRowSize;                    // Size of a full row
   size_t mColumnSize;                 // Size of a full column
   size_t mRowOffset;                  // Row offset into the cube
   size_t mColumnOffset;               // Column offset into the cube
   size_t mInterlineBytes;             // Number of bytes of non-data between rows

   size_t mAccessorColumn;
   size_t mAccessorRow;
   size_t mAccessorBand;

   int mRefCount;
   convertToDouble mConvertToDoubleFunc;
   convertToInteger mConvertToIntegerFunc;
};

#endif
