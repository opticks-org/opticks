/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef _BITMASKIMP_
#define _BITMASKIMP_

#include <stdio.h>
#include "BitMask.h"
#include "xmlwriter.h"

/**
 * BitMask Implementation class
 *
 * Defines the data members and interface for handling 2-d bitmasks.
 *
 *  @see     BitMask, AOI, AOIImp, AOIAdapter
 */
class BitMaskImp : public BitMask
{
private:
   int mx1,my1;      // the pixel coordinate of the lower left corner of the bitmask
   int mx2,my2;      // the pixel coordinate of the upper right corner of the bitmask
   int mbbx1, mbby1;   //   the pixel coordinate of the lower left corner of the bounding box
   int mbbx2, mbby2;   //   the pixel coordinate of the upper right corner of the bounding box
   int mxSize;         // the number of unsigned longs per row
   int mySize;         // the number of rows
   int mSize;         // mxSize * mySize
   int mCount;         // the number of pixels set in the bitmask
   bool mOutside;      // the value of bits outside the mask
   unsigned int **mMask;// the actual bitmask
   bool **mBuffer;      // a buffer for the results of the getRegion method
   int mBufferX1,mBufferY1;   // the pixel coordinate of the lower left corner of the buffer region
   int mBufferX2,mBufferY2;   // the pixel coordinate of the upper right corner of the buffer region
   bool mBufferNeedsUpdated;

   /**
    *  computeCount member function.
    *
    *  Computes the number of set bits in the BitMask.
    *
    *  @return
    *         the number of set bits in the mask
    */
   int computeCount () const;

    /**
     *  Resize method.
     *
     *  Resizes a bitmask.
     *
     *  @param  rows
     *          The new number of rows. Must be >0.
     *  @param  cols
     *          The new number of columns. Must be >0.
     *  @param  fill
     *          A flag indicating whether or not to set newly added pixels to
    *         1 or not. True means all new pixels in the mask are initialized
    *         to 1, false means they are initialized to 0.
    *
    *  @return
    *         true if valid parameters were provided for rows and cols
    *         false otherwise
     */
   void growToInclude (int x1, int y1, int x2, int y2, bool fill);

public:
    /**
     *  Default Constructor.
     *
     *  Creates a BitMask.
     *
     */
   BitMaskImp ();

    /**
     *  Copy Constructor.
     *
     *  Creates a BitMask that is an exact duplicate of the specified
     *  parameter. 
     *
     *  @param  rhs
     *          "Right Hand Side". The mask to copy from.
     */
   BitMaskImp (const BitMaskImp &rhs);

    /**
     *  Custom Constructor.
     *
     *  Creates a BitMask that contains all of the pixels specified in
     *  the region.
     *
     *  @param  pRegion
     *          The 2-d array of bools specifying the state for each pixel. Its
     *          size must be y2-y1+1 rows by x2-x1+1 columns. If this parameter
     *          is NULL, all specified pixels will be turned on (equivalent to creating
     *          an empty mask and calling setRegion on it).
     *  @param  x1
     *          The column of the lower-left corner.
     *  @param  y1
     *          The row of the lower-left corner.
     *  @param  x2
     *          The column of the upper-right corner.
     *  @param  y2
     *          The row of the upper-right corner.
     */
   BitMaskImp (const bool **pRegion, int x1, int y1, int x2, int y2);

    /**
     *  Assignment operator.
     *
     *  Sets the mask of the left side of the '=' to be an exact
     *  duplicate of the one on the right hand side of the '='.
     *
     *  @param  rhs
     *          "Right Hand Side". The mask to copy from.
     */
   BitMaskImp &operator= (const BitMaskImp &rhs);

    /**
     *  Destructor.
     *
     *  Frees the memory stored by the mask
     */
   ~BitMaskImp ();

    /**
     *  In-place bitwise 'OR' operator.
     *
     *  Merges two BitMasks. Upon return, the bitmask 'OR'ed with
    *  the parameter bitmask will have all bits set that it had
    *  originally, plus all that are set in the parameter bitmask.
    *  This can be looked at as the union of the two masks.
     *
     *  @param  rhs
     *          "Right Hand Side". The mask to merge from.
     */
   void operator|= (const BitMaskImp &rhs);

    /**
     *  In-place bitwise 'XOR' operator.
     *
     *  Merges two BitMasks.
     *
     *  @param  rhs
     *          "Right Hand Side". The mask to merge from.
     */
   void operator^= (const BitMaskImp &rhs);

    /**
     *  In-place bitwise 'AND' operator.
     *
     *  Merges two BitMasks. Upon return, the bitmask 'AND'ed with
    *  the parameter bitmask will have all bits set that were originally
    *  set in both it and the parameter bitmask. This can be looked at
    *  as the intersection of the two masks.
     *
     *  @param  rhs
     *          "Right Hand Side". The mask to merge from.
     */
   void operator&= (const BitMaskImp &rhs);

    /**
     *  Equivalence operator.
     *
     *  Compares two bitmasks.
     *
     *  @param  rhs
     *          "Right Hand Side". The mask to compare with.
    *
    *  @return
    *         true if the two masks are identical in content
    *         false otherwise
     */
   bool operator== (const BitMaskImp &rhs) const;

   /**
    *  clear member function.
    *
    *  Re-initializes a bitmask to have no bits set
    */
   virtual void clear ();

    /**
     *  In-place bitwise 'OR' operator.
     *
     *  Merges two BitMasks. Upon return, the bitmask 'OR'ed with
    *  the parameter bitmask will have all bits set that it had
    *  originally, plus all that are set in the parameter bitmask.
    *  This can be looked at as the union of the two masks.
     *
     *  @param  rhs
     *          "Right Hand Side". The mask to merge from.
     */
   virtual void merge (const BitMask &rhs);

    /**
     *  In-place bitwise 'XOR' operator.
     *
     *  Merges two BitMasks. 
     *
     *  @param  rhs
     *          "Right Hand Side". The mask to merge from.
     */
   virtual void toggle (const BitMask &rhs);

    /**
     *  In-place bitwise 'AND' operator.
     *
     *  Merges two BitMasks. Upon return, the bitmask 'AND'ed with
    *  the parameter bitmask will have all bits set that were originally
    *  set in both it and the parameter bitmask. This can be looked at
    *  as the intersection of the two masks.
     *
     *  @param  rhs
     *          "Right Hand Side". The mask to merge from.
     */
   virtual void intersect (const BitMask &rhs);

    /**
     *  Equivalence operator.
     *
     *  Compares two bitmasks.
     *
     *  @param  rhs
     *          "Right Hand Side". The mask to compare with.
    *
    *  @return
    *         true if the two masks are identical in content
    *         false otherwise
     */
   virtual bool compare (const BitMask &rhs) const;

    /**
     *  Mask inversion method.
     *
     *  Inverts all bits in the mask. All 1's become 0's and all 0's
    *  become 1's. It uses the ~ operator internally to perform the
    *  inversion operation
     */
   virtual void invert ();

    /**
     *  Subset comparison method.
     *
     *  Compares two bitmasks.
     *
     *  @param  source
     *          The mask to compare with.
    *
    *  @return
    *         true if all
    *            bits set in the left-hand-side mask are also set in
    *            the parameter mask. Ensures that ~lhs | source == 1 
    *            for all bits in the masks.
    *         false otherwise
     */
   virtual bool isSubsetOf (const BitMask &source) const;

   /**
   */
   virtual void setRegion (int x1, int y1, int x2, int y2, ModeType op);

   /**
    *  SetPixel method.
    *
    *  Sets a specified bit on or off.
    *
    *  @param  x
    *          The column of the pixel to set.
    *  @param  y
    *          The row of the pixel to set.
    *  @param  value
    *          A flag indicating whether or not to set pixel to 1 or not. 
    *         True means set the pixel to 1, false means set to 0.
    */
   virtual void setPixel (int x, int y, bool value);

   /**
    *  GetPixel method.
    *
    *  Gets a specified bit.
    *
    *  @param  x
    *          The column of the pixel to get.
    *  @param  y
    *          The row of the pixel to get.
    *
    *  @return
    *         the value of the pixel specified
    */
   virtual bool getPixel (int x, int y) const;

   virtual void getBoundingBox (int &x1, int &y1, int &x2, int &y2) const;
   bool isOutsideSelected() const;

   /**
    *  clipBoundingBox method.
    *
    *  Clips the bounding box of set pixels in the bitmap.  The values of all
    *  pixels outside the bitmap are set to false.  If the current bounding
    *  box is completely contained in the given area, nothing happens.
    *
    *  @param  x1
    *          The column of the lower-left corner.
    *  @param  y1
    *          The row of the lower-left corner.
    *  @param  x2
    *          The column of the upper-right corner.
    *  @param  y2
    *          The row of the upper-right corner.
    */
   virtual void clipBoundingBox (int x1, int y1, int x2, int y2);

   /**
     *  getCount method.
     *
     *  Gets the number of bits that are set in the mask.
     *
    *  @return
    *         the number of bits that are set in the mask
     */
   virtual int getCount () const;

   /**
     *  getRegion method.
     *
     *  Gets all of the bits in a rectangular region. They are returned as a 2D array 
    *   of bools. This
    *   array is owned by the BitMask and should not be modified or deleted. The
    *   array will remain unchanged until the next call to getRegion.
     *
     *  @param  x1,y1
     *          The coordinate of the lower-left corner of the region to get
     *  @param  x2,y2
     *          The coordinate of the upper-right corner of the region to get
    *
    *  @return
    *         a 2D array of bools representing the region specified
     */
   virtual const bool **getRegion (int x1, int y1, int x2, int y2);

   /**
    *  GetPixels method.
    *
    *  Gets pixels 32 at a time.
    *
    *  @param  x
    *          The column of the pixels to get. Must be a multiple of 32.
    *  @param  y
    *          The row of the starting pixel.
    *
    *  @return
    *         the 32 bits specified, packed into an unsigned int
    */
   virtual unsigned int getPixels (int x, int y) const;

   /**
    *  SetPixels method.
    *
    *  Sets pixels 32 at a time.
    *
    *  @param  x
    *          The column of the pixels to set. Must be a multiple of 32.
    *  @param  y
    *          The row of the starting pixel.
    *   @param   values
    *         An unsigned int containing the states for 32 bits, packed
    *         into an unsigned int
    */
   virtual void setPixels (int x, int y, unsigned int values);

    /**
     * Capture the object state to an archive.
     *
     * @param  xml
     *         Pointer to an XMLWriter object
     *
     * @return True if the serialization completed without error.
     */
    virtual bool toXml(XMLWriter* xml) const;
    virtual bool fromXml(DOMNode* document, unsigned int version);

   /**
    *   Gets the actual bounding box of set pixels in the bitmap.
    *
    *   If points have been removed from the BitMask, getBoundingBox() may not
    *   be the minimum box needed to enclose the pixels.  This method will
    *   always be the minimum.  If no pixels are set in the bitmap, it sets 
    *   the values to (0,0);(0,0).
    *
    *  @param  x1
    *          The column of the lower-left corner.
    *  @param  y1
    *          The row of the lower-left corner.
    *  @param  x2
    *          The column of the upper-right corner.
    *  @param  y2
    *          The row of the upper-right corner.
    */
    virtual void getMinimalBoundingBox(int &x1, int &y1, int &x2, int &y2) const;
};

#endif
