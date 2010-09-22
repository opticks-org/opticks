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
 * This class provides a convenient way to traverse selected pixels within a
 * given area of a BitMask object.  The iteration extents can be either custom
 * values or the extents of a RasterElement.
 *
 * The iterator incorporates the outside flag of the BitMask when determining
 * whether a pixel is selected.  The getPixel(int,int) const method is provided to
 * query the selected state of any pixel within the extents.
 *
 * This class is intended to be used in place of directly traversing the
 * BitMask.
 *
 *  \image html BitMaskIterator.JPG
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
    * All parameters are zero-based and inclusive.  Upon construction, the pixel
    * location is the first selected pixel. If no pixels are selected, the pixel
    * location is (-1, -1), which is equivalent to end().
    *
    * @param   pBitMask
    *          The BitMask to traverse. \c NULL is a valid value for the BitMask.
    *          Use of a \c NULL BitMask may allow algorithms to be designed with
    *          a single code path.
    * @param   x1
    *          The first column of the area of the BitMask to be traversed. This
    *          value must be less than or equal to the maximum value that the int
    *          type will support on the platform and this is enforced by setting
    *          larger values to that maximum value.
    * @param   y1
    *          The first row of the area of the BitMask to be traversed. This
    *          value must be less than or equal to the maximum value that the int
    *          type will support on the platform and this is enforced by setting
    *          larger values to that maximum value.
    * @param   x2
    *          The last column of the area of the BitMask to be traversed. This
    *          value must be less than or equal to the maximum value that the int
    *          type will support on the platform and this is enforced by setting
    *          larger values to that maximum value.
    * @param   y2
    *          The last row of the area of the BitMask to be traversed. This
    *          value must be less than or equal to the maximum value that the int
    *          type will support on the platform and this is enforced by setting
    *          larger values to that maximum value.
    */
   BitMaskIterator(const BitMask* pBitMask, unsigned int x1, unsigned int y1,
                   unsigned int x2, unsigned int y2);

   /**
    * Constructs a BitMaskIterator taking extents from a given RasterElement.
    *
    * Upon construction, the pixel location is the first selected pixel.
    * If no pixels are selected, the pixel location is (-1, -1), which is
    * equivalent to end().
    *
    * @param   pBitMask
    *          The BitMask to traverse. \c NULL is a valid value for the BitMask.
    *          Use of a \c NULL BitMask may allow algorithms to be designed with
    *          a single code path.
    * @param   pRasterElement
    *          The RasterElement over which the BitMask will be traversed.  The
    *          extents are defined as the number of rows and columns in the
    *          RasterElement.  %Any offset to other elements is ignored.
    */
   BitMaskIterator(const BitMask* pBitMask, const RasterElement* pRasterElement);

   /**
    * Resets the pixel location to the first pixel.
    *
    * This method re-initializes the state of the BitMaskIterator to point at
    * the first selected pixel of the BitMask within the extents provided when
    * the iterator was constructed.
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
    *          and the pixel is contained within the extents; otherwise returns \c false.
    *          Also returns \c false if the given pixel is outside the extents.
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
    *          which is equivalent to end().
    */
   void getPixelLocation(LocationType& pixelLocation) const;

   /**
    * Gets the extents over which the iterator iterates.
    *
    * If the outside flag in the underlying BitMask is on, this method populates
    * the given parameters with the extents set in the constructor.  All
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
    * iterator is equivalent to end().
    *
    * @see     operator++(), operator++(int)
    */
   void nextPixel();

   /**
    * Advances the pixel location to the next selected pixel.
    *
    * @return  Returns \c true if the iterator advanced to the next selected
    *          pixel or the BitMask is \c NULL and the pixel is contained within the extents.
    *          If there are no more selected pixels within the extents, the state of the
    *          iterator is equivalent to end() and \c false is returned.
    *
    * @see     nextPixel(), operator++(int)
    */
   bool operator++();

   /**
    * Advances the pixel location to the next selected pixel.
    *
    * @return  Returns \c true if the iterator advanced to the next selected
    *          pixel or the BitMask is \c NULL and the pixel is contained within the extents.
    *          If there are no more selected pixels within the extents, the state of the
    *          iterator is equivalent to end() and \c false is returned.
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
    * @return  Returns an invalid iterator with invalid conditions to represent the end
    *          of the BitMask.
    */
   BitMaskIterator end();

   /**
    * Gets the number of rows contained within the extents over which the iterator iterates.
    *
    * @return  Returns the number of rows spanning the range of the bounding box.
    *
    * @see     getBoundingBox()
    */
   int getNumSelectedRows() const;

   /**
    * Gets the number of columns contained within the extents over which the iterator iterates.
    *
    * @return  Returns the number of columns spanning the range of the bounding box.
    *
    * @see     getBoundingBox()
    */
   int getNumSelectedColumns() const;

   /**
    * Gets the distance in pixels from the beginning of the extents to the first selected pixel in the BitMask.
    *
    * @return  Returns the offset in (column, row) coordinates from the beginning of the
    *          extents to the first selected pixel in the BitMask. A default constructed
    *          LocationType indicating no offset is returned in the following cases:
    *             - The BitMask is \c NULL.
    *             - The method BitMask::isOutsideSelected() returns \c true.
    *
    * @see     getBoundingBox()
    */
   LocationType getOffset() const;

   /**
    * Determines if the entire extents must be iterated over.
    *
    * @return  Returns \c true if every pixel within the extents is selected; otherwise
    *          returns \c false.
    *
    * @see     getBoundingBox()
    */
   bool useAllPixels() const;

   /**
    * Queries whether the current pixel is selected.
    *
    * @return  Returns \c true if the current pixel is selected or the BitMask is \c NULL
    *           and the pixel is contained within the extents; otherwise returns \c false.
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
    * Gets the number of rows contained within the extents.
    *
    * @return  Returns the number of rows within the extents.
    *
    * @see     getNumSelectedRows(), getNumSelectedColumns()
    */
   int getNumRows() const;

   /**
    * Gets the number of columns contained within the extents.
    *
    * @return  Returns the number of columns within the extents.
    *
    * @see     getNumSelectedRows(), getNumSelectedColumns()
    */
   int getNumColumns() const;

   /**
    * Gets the starting row of the extents over which the iterator iterates.
    *
    * @return  Returns the starting row for iteration.  If the outside flag in
    *          the underlying BitMask is on, this method returns the given parameter
    *          of the extents set in the constructor.  The return value is zero-based
    *          and inclusive.
    *
    * @see     getBoundingBox(), getBoundingBoxStartColumn()
    */
   int getBoundingBoxStartRow() const;

   /**
    * Gets the starting column of the extents over which the iterator iterates.
    *
    * @return  Returns the starting column for iteration.  If the outside flag in
    *          the underlying BitMask is on, this method returns the given parameter
    *          of the extents set in the constructor.  The return value is zero-based
    *          and inclusive.
    *
    * @see     getBoundingBox(), getBoundingBoxStartRow()
    */
   int getBoundingBoxStartColumn() const;

   /**
    * Gets the ending row of the extents over which the iterator iterates.
    *
    * @return  Returns the ending row for iteration.  If the outside flag in
    *          the underlying BitMask is on, this method returns the given parameter
    *          of the extents set in the constructor.  The return value is zero-based
    *          and inclusive.
    *
    * @see     getBoundingBox(), getBoundingBoxEndColumn()
    */
   int getBoundingBoxEndRow() const;

   /**
    * Gets the ending column of the extents over which the iterator iterates.
    *
    * @return  Returns the ending column for iteration.  If the outside flag in
    *          the underlying BitMask is on, this method returns the given parameter
    *          of the extents set in the constructor.  The return value is zero-based
    *          and inclusive.
    *
    * @see     getBoundingBox(), getBoundingBoxEndRow()
    */
   int getBoundingBoxEndColumn() const;

   /**
    * Gets the distance in pixels from the beginning of the extents to the first selected pixel in the BitMask.
    *
    * @return  Returns the offset in row coordinates from the beginning of the
    *          extents to the first selected pixel in the BitMask. A value of zero
    *          indicating no offset is returned in the following cases:
    *             - The BitMask is \c NULL.
    *             - The method BitMask::isOutsideSelected() returns \c true.
    *
    * @see     getOffset(), getColumnOffset()
    */
   int getRowOffset() const;

   /**
    * Gets the distance in pixels from the beginning of the extents to the first selected pixel in the BitMask.
    *
    * @return  Returns the offset in column coordinates from the beginning of the
    *          extents to the first selected pixel in the BitMask. A value of zero
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
    *          which is equivalent to end().
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
    *          which is equivalent to end().
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
