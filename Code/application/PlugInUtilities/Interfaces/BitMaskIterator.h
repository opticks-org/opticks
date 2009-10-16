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
 *
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
    *          The BitMask to traverse.
    * @param   x1
    *          The first column of the area of the BitMask to be traversed. This
    *          value must be non-negative and this is enforced by setting any negative
    *          values to 0.
    * @param   y1
    *          The first row of the area of the BitMask to be traversed. This
    *          value must be non-negative and this is enforced by setting any negative
    *          values to 0.
    * @param   x2
    *          The last column of the area of the BitMask to be traversed. This
    *          value must be non-negative and this is enforced by setting any negative
    *          values to 0.
    * @param   y2
    *          The last row of the area of the BitMask to be traversed. This
    *          value must be non-negative and this is enforced by setting any negative
    *          values to 0.
    */
   BitMaskIterator(const BitMask* pBitMask, int x1, int y1, int x2, int y2);

   /**
    * Constructs a BitMaskIterator taking extents from a given RasterElement.
    *
    * All parameters are zero-based and inclusive.  Upon construction, the pixel
    * location is the first selected pixel. If no pixels are selected, the pixel
    * location is (-1, -1), which is equivalent to end().
    *
    * @param   pBitMask
    *          The BitMask to traverse.
    * @param   pRasterElement
    *          The RasterElement over which the BitMask will be traversed.  The
    *          extents are defined as the number of rows and columns in the
    *          RasterElement.  %Any offset to other elements is ignored.
    */
   BitMaskIterator(const BitMask* pBitMask, RasterElement* pRasterElement);

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
    * @return  The number of selected pixels within the extents.
    */
   int getCount();

   /**
    * Queries whether the current pixel is selected.
    *
    * @return  Returns \c true if the current pixel is selected; otherwise
    *          returns \c false.
    *
    * @see     getPixel(int,int) const, getPixelLocation(), operator*()
    */
   bool getPixel() const;

   /**
    * Queries whether a given pixel is selected.
    *
    * @param   col
    *          The zero-based column number of the pixel to query.
    * @param   row
    *          The zero-based row number of the pixel to query.
    *
    * @return  Returns \c true if the given pixel is selected; otherwise returns
    *          \c false.  Also returns \c false if the given pixel is outside
    *          the extents.
    *
    * @see     getPixel(), getPixelLocation()
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
    *          The column of the lower-left corner.
    * @param   y1
    *          The row of the lower-left corner.
    * @param   x2
    *          The column of the upper-right corner.
    * @param   y2
    *          The row of the upper-right corner.
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
    *          pixel.  If there are no more selected pixels within the extents,
    *          the state of the iterator is equivalent to end() and \c false is
    *          returned.
    *
    * @see     nextPixel(), operator++(int)
    */
   bool operator++();

   /**
    * Advances the pixel location to the next selected pixel.
    *
    * @return  Returns \c true if the iterator advanced to the next selected
    *          pixel.  If there are no more selected pixels within the extents,
    *          the state of the iterator is equivalent to end() and \c false is
    *          returned.
    *
    * @see     nextPixel(), operator++()
    */
   bool operator++(int);

   /**
    * Creates an iterator that is ready to start traversing the BitMask.
    *
    * @return  A valid iterator with the conditions set to the initial
    *          conditions of this iterator.
    */
   BitMaskIterator begin();

   /**
    * Creates an iterator that has finished traversing the BitMask.
    *
    * @return  An invalid iterator with invalid conditions to represent the end
    *          of the BitMask.
    */
   BitMaskIterator end();

   /**
    * Queries whether the current pixel is selected.
    *
    * @return  Returns \c true if the current pixel is selected; otherwise
    *          returns \c false.
    *
    * @see     getPixel(int,int) const, getPixel()
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

private:
   BitMaskIterator(BitMaskIterator, bool);
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
};

#endif
