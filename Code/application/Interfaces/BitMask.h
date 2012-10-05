/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef _BITMASK_
#define _BITMASK_

#include "Serializable.h"
#include "TypesFile.h"

/**
 *  Mask for selecting indiviual pixel locations
 *
 *  The BitMask class provides a means of selecting individual pixels within
 *  a scene.  A pixel coordinate is marked as either 'on' or 'off'.  The bounding
 *  box contains the outer most coordinates of selected pixels.  The BitMask is
 *  underlying part of an AOI to represent selected pixel locations in a scene.
 *
 *  @see        AOI
 */
class BitMask : public Serializable
{
public:
   /**
    *  clear member function.
    *
    *  Re-initializes a bitmask to have no bits set
    */
   virtual void clear () = 0;

   /**
    *  In-place bitwise 'OR' operator.
    *
    *  Merges two BitMasks. Upon return, the bitmask 'OR'ed with
    *  the parameter bitmask will have all bits set that it had
    *  originally, plus all that are set in the parameter bitmask.
    *  This can be looked at as the union of the two masks.
    *
    *  @param   rhs
    *           "Right Hand Side". The mask to merge from.
    */
   virtual void merge (const BitMask& rhs) = 0;

   /**
    *  In-place bitwise 'AND' operator.
    *
    *  Merges two BitMasks. Upon return, the bitmask 'AND'ed with
    *  the parameter bitmask will have all bits set that were originally
    *  set in both it and the parameter bitmask. This can be looked at
    *  as the intersection of the two masks.
    *
    *  @param   rhs
    *           "Right Hand Side". The mask to merge from.
    */
   virtual void intersect (const BitMask& rhs) = 0;

   /**
    *  Equivalence operator.
    *
    *  Compares two bitmasks.
    *
    *  @param   rhs
    *           "Right Hand Side". The mask to compare with.
    *
    *  @return  true if the two masks are identical in content
    *         ` false otherwise
    */
   virtual bool compare (const BitMask& rhs) const = 0;

   /**
    *  Mask inversion method.
    *
    *  Inverts all bits in the mask. All 1's become 0's and all 0's
    *  become 1's. It uses the ~ operator internally to perform the
    *  inversion operation
    */
   virtual void invert () = 0;

   /**
    *  Subset comparison method.
    *
    *  Compares two bitmasks.
    *
    *  @param   source
    *           The mask to compare with.
    *
    *  @return  true if all bits set in the left-hand-side mask are also
    *           set in the parameter mask. Ensures that ~lhs | source == 1 
    *           for all bits in the masks. false otherwise.
    */
   virtual bool isSubsetOf (const BitMask &source) const = 0;

   /**
    *  setPixel method.
    *
    *  Sets a specified bit on or off.
    *
    *  @param   x
    *           The column of the pixel to set.
    *  @param   y
    *           The row of the pixel to set.
    *  @param   value
    *           A flag indicating whether or not to set pixel to 1 or not. 
    *           True means set the pixel to 1, false means set to 0.
    */
   virtual void setPixel (int x, int y, bool value) = 0;

   /**
    *  getPixel method.
    *
    *  Gets a specified bit.
    *
    *  @param   x
    *           The column of the pixel to get.
    *  @param   y
    *           The row of the pixel to get.
    *
    *  @return  the value of the pixel specified
    */
   virtual bool getPixel (int x, int y) const = 0;

   /**
    *  getBoundingBox method.
    *
    *  Gets the bounding box of set pixels in the bitmap. If no pixels are set
    *  in the bitmap, it sets the values to (0,0);(0,0).
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
   virtual void getBoundingBox (int &x1, int &y1, int &x2, int &y2) const = 0;

   /**
    *  Access the state of points outside the bounding box.
    *
    *  @return If this is true, than all points outside of the bounding box are selected.
    *          If this is false, that all points outside of the bounding box are not selected.
    *
    *  @see getBoundingBox(), getPixel()
    */
   virtual bool isOutsideSelected() const = 0;

   /**
    *  getCount method.
    *
    *  Gets the number of bits that are set in the mask.
    *
    *  @return  the number of bits that are set in the mask
    */
   virtual int getCount () const = 0;

   /**
    *  getRegion method.
    *
    *  Gets all of the bits in a rectangular region. They are returned as a 2D array 
    *  of bools. This
    *  array is owned by the BitMask and should not be modified or deleted. The
    *  array will remain unchanged until the next call to getRegion.
    *
    *  @param   x1,y1
    *           The coordinate of the lower-left corner of the region to get
    *  @param   x2,y2
    *           The coordinate of the upper-right corner of the region to get
    *
    *  @return  a 2D array of bools representing the region specified
    */
   virtual const bool **getRegion (int x1, int y1, int x2, int y2) = 0;

   /**
    *  GetPixels method.
    *
    *  Gets pixels 32 at a time.
    *
    *  @param   x
    *           The column of the pixels to get. Must be a multiple of 32.
    *  @param   y
    *           The row of the starting pixel.
    *
    *  @return  the 32 bits specified, packed into an unsigned int
    */
   virtual unsigned int getPixels (int x, int y) const = 0;

   /**
    *  SetPixels method.
    *
    *  Sets pixels 32 at a time.
    *
    *  @param   x
    *           The column of the pixels to set. Must be a multiple of 32.
    *  @param   y
    *           The row of the starting pixel.
    *  @param   values
    *           An unsigned int containing the states for 32 bits, packed
    *           into an unsigned int
    */
   virtual void setPixels (int x, int y, unsigned int values) = 0;

   /**
    *  Changes a region of a bitmask.
    *
    *  If op is DRAW, if fills the region.  If op is ERASE, it clears the region.
    *  If op is TOGGLE, it toggles all pixels in the region.
    *
    *  @param   x1
    *           The lower left x-coordinate of the region to change.
    *  @param   y1
    *           The lower left y-coordinate of the region to change.
    *  @param   x2
    *           The upper right x-coordinate of the region to change
    *  @param   y2
    *           The upper right y-coordinate of the region to change
    *  @param   op
    *           The drawing mode
    */
   virtual void setRegion (int x1, int y1, int x2, int y2, ModeType op) = 0;

   /**
    *  Merges two BitMasks.
    *
    *  In-place bitwise 'XOR' operator.
    *
    *  @param   rhs
    *           "Right Hand Side". The mask to merge from.
    */
   virtual void toggle (const BitMask& rhs) = 0;

   /**
    *   Gets the actual bounding box of set pixels in the bitmap.
    *
    *   If points have been removed from the BitMask, BitMask::getBoundingBox() may not
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
    virtual void getMinimalBoundingBox(int &x1, int &y1, int &x2, int &y2) const = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~BitMask() {}
};

#endif // _BITMASK_
