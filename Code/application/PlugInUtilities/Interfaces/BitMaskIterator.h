/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BITMASKITERATOR_H
#define BITMASKITERATOR_H

#include "LocationType.h"

class BitMask;
class RasterElement;

/**
 * Traverses selected pixels within a BitMask.
 * This class provides a convenient way to traverse the selected pixels within a
 * given area of a BitMask object.  The BitMaskIterator extents can be set to either
 * custom values or the extents of a RasterElement.
 *  \image html BitMaskIterator.JPG
 *  \image html BitMaskIteratorExample1.jpg
 *  \image html BitMaskIteratorExample2.jpg
 *
 * The iterator incorporates the outside flag of the BitMask when determining
 * whether a pixel is selected.  The getPixel(int,int) const method is provided to
 * query the selected state of any pixel within the extents. A call to getPixel() for
 * pixel locations outside the extents of the BitMasKIterator will return false.
 *
 *  \image html BitMaskIteratorExample3.jpg
 *  \image html BitMaskIteratorExample4.jpg
 *
 * This class is intended to be used in place of directly traversing the
 * BitMask.
 *
 * @warning    The iterator does not assume ownership over the BitMask.  It is
 *             intended to be used in place of directly traversing the BitMask.
 *             Also for efficiency purposes, this class assumes that the BitMask
 *             is valid (i.e. not deleted) throughout the lifetime of the
 *             iterator.
 */
class BitMaskIterator
{
public:
   /**
    * Constructs a BitMaskIterator with given extents.
    *
    * All parameters are zero-based and inclusive.  Upon construction, the iterator's pixel
    * location is set to the first selected pixel within the iterator's bounding box (the
    * intersection of the iterator extents and the BitMask minimal bounding box). If no pixels within
    * the iterator's bounding box are selected, the pixel location is set to (-1, -1), which is
    * equivalent to BitMaskIterator::end().
    *
    * @param   pBitMask
    *          The BitMask to traverse. \c NULL is a valid value for the BitMask.
    *          Use of a \c NULL BitMask allows algorithms to be designed with
    *          a single code path, e.g., BitMaskIterator bit(pAoi, 0, 0, 100, 100) where pAoi can be
    *          NULL or a valid pointer.
    * @param   x1
    *          The first column of the area over which the iterator may operate. The intersection between
    *          this area and the BitMask determines the area to be traversed. This
    *          value must be non-negative and less than or equal to the maximum value that the int
    *          type will support on the platform. This is enforced by setting
    *          larger values to that maximum value.
    * @param   y1
    *          The first row of the area over which the iterator may operate. The intersection between
    *          this area and the BitMask determines the area to be traversed. This
    *          value must be non-negative and less than or equal to the maximum value that the int
    *          type will support on the platform. This is enforced by setting
    *          larger values to that maximum value.
    * @param   x2
    *          The last column of the area over which the iterator may operate. The intersection between
    *          this area and the BitMask determines the area to be traversed. This
    *          value must be non-negative and less than or equal to the maximum value that the int
    *          type will support on the platform. This is enforced by setting
    *          larger values to that maximum value.
    * @param   y2
    *          The last row of the area over which the iterator may operate. The intersection between
    *          this area and the BitMask determines the area to be traversed. This
    *          value must be non-negative and less than or equal to the maximum value that the int
    *          type will support on the platform. This is enforced by setting
    *          larger values to that maximum value.
    */
   BitMaskIterator(const BitMask* pBitMask, unsigned int x1, unsigned int y1,
                   unsigned int x2, unsigned int y2);

   /**
    * Constructs a BitMaskIterator using the extents from a given RasterElement.
    *
    * Upon construction, the iterator's pixel location is set to the first selected pixel within
    * the iterator's bounding box (the intersection of the iterator extents and the BitMask minimal
    * bounding box). If no pixels within the iterator's bounding box are selected, the pixel location
    * is set to (-1, -1), which is equivalent to BitMaskIterator::end().
    *
    * @param   pBitMask
    *          The BitMask to traverse. \c NULL is a valid value for the BitMask.
    *          Use of a \c NULL BitMask allows algorithms to be designed with
    *          a single code path, e.g., BitMaskIterator bit(pAoi, pRaster) where pAoi can be NULL or a valid pointer.
    * @param   pRasterElement
    *          The RasterElement supplying the extents that will define the area over which the iterator may operate.
    *          The extents are defined by the number of rows and columns in the
    *          RasterElement, i.e., (0, 0) to (num cols - 1, num rows - 1).  %Any offset to other elements is ignored.
    */
   BitMaskIterator(const BitMask* pBitMask, const RasterElement* pRasterElement);

   /**
    * Resets the pixel location to the first pixel.
    *
    * This method re-initializes the state of the BitMaskIterator to point at
    * the first selected pixel of the BitMask within the extents provided when
    * the iterator was constructed (the iterator's bounding box).
    *
    * @see     nextPixel()
    */
   void firstPixel();

   /**
    * Gets the number of selected pixels.
    *
    * To get the number of selected pixels, this method must iterate over the
    * entire extents, which may be slow based on the size of the extents.  After
    * this method is called once, the selected pixel count is stored internally,
    * so subsequent calls to getCount() are much faster.
    *
    * @return  Returns the number of selected pixels within the extents.
    */
   int getCount() const;

   /**
    * Queries whether a given pixel is selected.
    *
    * @param   col
    *          The zero-based column number of the pixel to query.
    * @param   row
    *          The zero-based row number of the pixel to query.
    *
    * @return  Returns \c true if the given pixel is selected or if the BitMask is \c NULL
    *          and the pixel is contained within the iterator's extents; otherwise returns \c false.
    *          Also returns \c false if the given pixel is outside the iterator's extents.
    *
    * @see     getPixelLocation()
    */
   bool getPixel(int col, int row) const;

   /**
    * Gets the current pixel location.
    *
    * @param   pixelLocation
    *          Populated with the current pixel location.  When an iterator is
    *          constructed, its pixel location is the first selected pixel. If
    *          no pixels are selected, the pixel location is (-1, -1),
    *          which is equivalent to BitMaskIterator::end().
    */
   void getPixelLocation(LocationType& pixelLocation) const;

   /**
    * Gets the bounding box over which the iterator iterates.
    *
    * If the outside flag in the underlying BitMask is on, this method populates
    * the given parameters with the iterator extents set in the constructor.  All
    * parameter values are zero-based and inclusive.
    *
    * @param   x1
    *          The starting column for iteration.
    * @param   y1
    *          The starting row for iteration.
    * @param   x2
    *          The ending column for iteration.
    * @param   y2
    *          The ending row for iteration.
    *
    * @see     getNumSelectedRows() getNumSelectedColumns()
    */
   void getBoundingBox(int& x1, int& y1, int& x2, int& y2) const;

   /**
    * Advances the pixel location to the next selected pixel.
    *
    * If there are no more selected pixels within the extents, the state of the
    * iterator is equivalent to BitMaskIterator::end().
    *
    * @see     operator++(), operator++(int)
    */
   void nextPixel();

   /**
    * Advances the pixel location to the next selected pixel. This overloads the prefix increment operator.
    *
    * @return  Returns \c true if the iterator advanced to the next selected
    *          pixel or the BitMask is \c NULL and the pixel is contained within the extents of the iterator.
    *          If there are no more selected pixels within the iterator extents, the state of the
    *          iterator is equivalent to BitMaskIterator::end() and \c false is returned.
    *
    * @see     nextPixel(), operator++(int)
    */
   bool operator++();

   /**
    * Advances the pixel location to the next selected pixel. This overloads the postfix increment operator.
    * A dummy parameter is used to differentiate between the signatures of the postfix (var++) operator
    * and the prefix (++var) operator.
    *
    * @return  Returns \c true if the iterator advanced to the next selected
    *          pixel or the BitMask is \c NULL and the pixel is contained within the extents of the iterator.
    *          If there are no more selected pixels within the iterator extents, the state of the
    *          iterator is equivalent to BitMaskIterator::end() and \c false is returned.
    *
    * @see     nextPixel(), operator++()
    */
   bool operator++(int);

   /**
    * Creates an iterator that is ready to start traversing the BitMask.
    *
    * @return  Returns a valid iterator with the conditions set to the initial
    *          conditions of this iterator.
    */
   BitMaskIterator begin();

   /**
    * Creates an iterator that has finished traversing the BitMask.
    *
    * @return  Returns an invalid iterator with invalid conditions to represent an iterator
    *          that has finished traversing the BitMask.
    */
   BitMaskIterator end();

   /**
   * Gets the number of rows contained within the intersection of the iterator extents and the minimal
   * bounding box for the BitMask selected pixels. If the BitMask Outside flag is set, this will be equal
   * to the number of rows in the iterator extents.
    *
    * @return  Returns the number of rows spanning the range of the BitMaskIterator bounding box.
    *
    * @see     getBoundingBox()
    */
   int getNumSelectedRows() const;

   /**
    * Gets the number of columns contained within the intersection of the iterator extents and the minimal
    * bounding box for the BitMask selected pixels. If the BitMask Outside flag is set, this will be equal
    * to the number of columns in the iterator extents.
    *
    * @return  Returns the number of columns spanning the range of the BitMaskIterator bounding box.
    *
    * @see     getBoundingBox()
    */
   int getNumSelectedColumns() const;

   /**
    * Gets the distance in pixels from the beginning of the iterator extents to the start of the iterator bounding box.
    * For an unrotated, rectangular BitMap, it will be the distance between the beginning of the iterator extents
    * and the first selected pixel in the BitMask.
    *
    * @return  Returns the offset in (column, row) coordinates from the beginning of the iterator
    *          extents to the start of the iterator bounding box. A default constructed
    *          LocationType indicating no offset is returned in the following cases:
    *             - The BitMask is \c NULL.
    *             - The method BitMask::isOutsideSelected() returns \c true.
    *
    * @see     getBoundingBox()
    */
   LocationType getOffset() const;

   /**
    * Determines if the iterator will iterate over every every pixel within the iterator extents, i.e., every pixel
    * within the iterator extends is selected.
    *
    * @return  Returns \c true if every pixel within the iterator extents is selected; otherwise
    *          returns \c false.
    *
    * @see     getBoundingBox()
    */
   bool useAllPixels() const;

   /**
    * Queries whether the current pixel is selected.
    *
    * @return  Returns \c true if the current pixel is selected or the BitMask is \c NULL
    *           and the pixel is contained within the iterator extents; otherwise returns \c false.
    *
    * @see     getPixel(int,int) const
    */
   bool operator*() const;

   /**
    * Compares the position of this iterator with that of another.
    *
    * @param   other
    *          The iterator with which to compare its position.
    *
    * @return  Returns \c true if the position of the given iterator matches
    *          the position of this iterator; otherwise returns \c false.
    *
    * @see     operator!=()
    */
   bool operator==(const BitMaskIterator& other) const;

   /**
    * Compares the position of this iterator with that of another.
    *
    * @param   other
    *          The iterator with which to compare its position.
    *
    * @return  Returns \c true if the position of the given iterator does not
    *          match the position of this iterator; otherwise returns \c false.
    *
    * @see     operator==()
    */
   bool operator!=(const BitMaskIterator& other) const;

   /**
    * Gets the number of rows contained within the full extents of the iterator. This method just returns
    * either the number of rows in the extents passed to the constructor or the number of rows in the
    * raster element passed to the constructor. It should not be used in iterating over the BitMask passed
    * to the constructor. Use getNumSelectedRows() instead.
    *
    * @return  Returns the number of rows within the iterator extents.
    *
    * @see     getNumSelectedRows(), getNumSelectedColumns()
    */
   int getNumRows() const;

   /**
    * Gets the number of columns contained within the full extents of the iterator. This method just returns
    * either the number of columns in the extents passed to the constructor or the number of columns in the
    * raster element passed to the constructor. It should not be used in iterating over the BitMask passed
    * to the constructor. Use getNumSelectedColumns() instead.
    *
    * @return  Returns the number of columns within the iterator extents.
    *
    * @see     getNumSelectedRows(), getNumSelectedColumns()
    */
   int getNumColumns() const;

   /**
    * Gets the starting row in pixel coordinates of the bounding box over which the iterator iterates.
    *
    * @return  Returns the starting row for iteration.  This will be the number in pixel
    *          coordinates of the first row in the iterator bounding box if the Outside flag in
    *          the underlying BitMask is off. If the Outside flag is on, this method returns
    *          the number of the first row in the full extents of the iterator.
    *          The return value is zero-based and inclusive.
    *
    * @see     getBoundingBox(), getBoundingBoxStartColumn()
    */
   int getBoundingBoxStartRow() const;

   /**
    * Gets the starting column in pixel coordinates of the bounding box over which the iterator iterates.
    *
    * @return  Returns the starting column for iteration.  This will be the number in pixel
    *          coordinates of the first column in the iterator bounding box if the Outside flag in
    *          the underlying BitMask is off. If the Outside flag is on, this method returns
    *          the number of the first column in the full extents of the iterator.
    *          The return value is zero-based and inclusive.
    *
    * @see     getBoundingBox(), getBoundingBoxStartRow()
    */
   int getBoundingBoxStartColumn() const;

   /**
    * Gets the ending row in pixel coordinates of the bounding box over which the iterator iterates.
    *
    * @return  Returns the ending row for iteration.  This will be the number in pixel
    *          coordinates of the last row in the iterator bounding box if the Outside flag in
    *          the underlying BitMask is off. If the Outside flag is on, this method returns
    *          the number of the last row in the full extents of the iterator.
    *          The return value is zero-based and inclusive.
    *
    * @see     getBoundingBox(), getBoundingBoxEndColumn()
    */
   int getBoundingBoxEndRow() const;

   /**
    * Gets the ending column in pixel coordinates of the bounding box over which the iterator iterates.
    *
    * @return  Returns the ending column for iteration.  This will be the number in pixel
    *          coordinates of the last column in the iterator bounding box if the Outside flag in
    *          the underlying BitMask is off. If the Outside flag is on, this method returns
    *          the number of the last column in the full extents of the iterator.
    *          The return value is zero-based and inclusive.
    *
    * @see     getBoundingBox(), getBoundingBoxEndRow()
    */
   int getBoundingBoxEndColumn() const;

   /**
    * Gets the distance in pixels from the beginning of the iterator extents to the first row in
    * the iterator bounding box.
    *
    * @return  Returns the offset in row coordinates from the beginning of the
    *          iterator extents to the first row in the iterator bounding box. A value of zero
    *          indicating no offset is returned in the following cases:
    *             - The BitMask is \c NULL.
    *             - The method BitMask::isOutsideSelected() returns \c true.
    *
    * @see     getOffset(), getColumnOffset()
    */
   int getRowOffset() const;

   /**
    * Gets the distance in pixels from the beginning of the iterator extents to the first column
    * in the iterator bounding box.
    *
    * @return  Returns the offset in column coordinates from the beginning of the
    *          iterator extents to the first column in the iterator bounding box. A value of zero
    *          indicating no offset is returned in the following cases:
    *             - The BitMask is \c NULL.
    *             - The method BitMask::isOutsideSelected() returns \c true.
    *
    * @see     getOffset(), getRowOffset()
    */
   int getColumnOffset() const;

   /**
    * Gets the row of the current pixel.
    *
    * @return  Returns the row of the current pixel location.  When an iterator is
    *          constructed, its pixel row location is the row of the first selected
    *          pixel. If no pixels are selected, the pixel row and column are -1,
    *          which is equivalent to BitMaskIterator::end().
    *
    * @see     getPixelColumnLocation()
    */
   int getPixelRowLocation() const;

   /**
    * Gets the column of the current pixel.
    *
    * @return  Returns the column of the current pixel location.  When an iterator is
    *          constructed, its pixel column location is the column of the first selected
    *          pixel. If no pixels are selected, the pixel row and column are -1,
    *          which is equivalent to BitMaskIterator::end().
    *
    * @see     getPixelRowLocation()
    */
   int getPixelColumnLocation() const;

private:
   BitMaskIterator(BitMaskIterator, bool);
   bool getPixel() const;
   void computeCount();

   const BitMask* mpBitMask;
   int mX1;
   int mY1;
   int mX2;
   int mY2;
   int mCurrentPixelX; // Begin state = first selected pixel, end state = -1
   int mCurrentPixelY; // Begin state = first selected pixel, end state = -1
   int mFirstPixelX;
   int mFirstPixelY;
   unsigned int mCurrentPixelCount;
   int mPixelCount;
   int mMinX;
   int mMinY;
   int mMaxX;
   int mMaxY;
};

#endif
