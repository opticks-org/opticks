/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppVerify.h"
#include "BitMaskImp.h"
#include "XercesIncludes.h"
#include "xmlbase.h"
#include "xmlreader.h"

#include <limits>
#include <memory.h>
#include <stdlib.h>
#include <sstream>

using namespace std;
XERCES_CPP_NAMESPACE_USE

// The number of bits in a int. Note that the code does assume 32 bits
// in places. This is only done where assuming 32 bits allows divide to be
// replaced with >> and the mod operator (%) with &.
#define LONG_BITS   (8 * sizeof(int))

static inline int countBits(unsigned int v);
static inline bool regionsOverlap(int r1x1, int r1y1, int r1x2, int r1y2,
                                  int r2x1, int r2y1, int r2x2, int r2y2);

/**
 *  Default Constructor.
 *
 *  Creates an empty BitMask.
 */
BitMaskImp::BitMaskImp() :
   mx1(0),
   my1(0),
   mx2(0),
   my2(0),
   mbbx1(0),
   mbby1(0),
   mbbx2(0),
   mbby2(0),
   mxSize(0),
   mySize(0),
   mSize(0),
   mCount(0),
   mOutside(false),
   mpMask(NULL),
   mpBuffer(NULL),
   mBufferX1(0),
   mBufferY1(0),
   mBufferX2(0),
   mBufferY2(0),
   mBufferNeedsUpdated(true)
{}

/**
 *  Copy Constructor.
 *
 *  Creates a BitMask that is an exact duplicate of the specified
 *  parameter. 
 *
 *  @param  rhs
 *          "Right Hand Side". The mask to copy from.
 */
BitMaskImp::BitMaskImp(const BitMaskImp& rhs) :
   mx1(rhs.mx1),
   my1(rhs.my1),
   mx2(rhs.mx2),
   my2(rhs.my2),
   mbbx1(rhs.mbbx1),
   mbby1(rhs.mbby1),
   mbbx2(rhs.mbbx2),
   mbby2(rhs.mbby2),
   mxSize(rhs.mxSize),
   mySize(rhs.mySize),
   mSize(rhs.mSize),
   mCount(rhs.mCount),
   mOutside(rhs.mOutside),
   mpMask(NULL),
   mpBuffer(NULL),
   mBufferX1(0),
   mBufferY1(0),
   mBufferX2(0),
   mBufferY2(0),
   mBufferNeedsUpdated(true)
{
   if (rhs.mpMask)
   {
      mpMask = new unsigned int*[mySize];
      mpMask[0] = new (nothrow) unsigned int[mSize];
      if (mpMask[0] == NULL)
      {
         delete [] mpMask;
         mpMask = NULL;
         throw bad_alloc();
      }

      memcpy(mpMask[0], rhs.mpMask[0], mSize * sizeof(unsigned int));

      for (int i = 1; i <= my2 - my1; ++i)
      {
         mpMask[i] = mpMask[i - 1] + mxSize;
      }
   }
}

BitMaskImp::BitMaskImp(const bool** pRegion, int x1, int y1, int x2, int y2) :
   mx1(0),
   my1(0),
   mx2(0),
   my2(0),
   mbbx1(0),
   mbby1(0),
   mbbx2(0),
   mbby2(0),
   mxSize(0),
   mySize(0),
   mSize(0),
   mCount(0),
   mOutside(false),
   mpMask(NULL),
   mpBuffer(NULL),
   mBufferX1(0),
   mBufferY1(0),
   mBufferX2(0),
   mBufferY2(0),
   mBufferNeedsUpdated(true)
{
   if (pRegion == NULL)
   {
      setRegion(x1, y1, x2, y2, DRAW);
   }
   else
   {
      growToInclude(x1, y1, x2, y2, mOutside);
      int offset = mx1 - x1;
      for (int row = y1; row <= y2; ++row)
      {
         const bool* pBuffer = &pRegion[row - y1][offset];
         const bool* pSafeLow = pRegion[row - y1];
         const bool* pSafeHigh = &pRegion[row - y1][x2 - x1 + 1];
         unsigned int* pRow = mpMask[row - y1];
         const bool* pStop = pSafeLow + 32;
         for (int col = mx1; col <= x2; col += 32, pStop += 32)
         {
            unsigned int values = 0;
            unsigned int mask = 0x80000000;

            pStop = min(pStop, pSafeHigh);

            while (pBuffer < pSafeLow)
            {
               ++pBuffer;
               mask >>= 1;
            }

            for (; pBuffer < pStop; ++pBuffer)
            {
               if (*pBuffer)
               {
                  values |= mask;
                  mCount++;
               }
               mask >>= 1;
            }
            *pRow = values;
            ++pRow;
         }
      }
   }
}

/**
 *  Destructor.
 *
 *  Frees the memory stored by the mask
 */
BitMaskImp::~BitMaskImp()
{
   if (mpMask)
   {
      delete [] mpMask[0];
      delete [] mpMask;
   }

   if (mpBuffer)
   {
      delete [] mpBuffer[0];
      delete [] mpBuffer;
   }
}

/**
 *  Assignment operator.
 *
 *  Sets the mask of the left side of the '=' to be an exact
 *  duplicate of the one on the right hand side of the '='. 
 *
 *  @param  rhs
 *          "Right Hand Side". The mask to copy from.
 */
BitMaskImp& BitMaskImp::operator=(const BitMaskImp& rhs)
{
   if (rhs.mpMask != mpMask)
   {
      if (mpMask)
      {
         delete [] mpMask[0];
         delete [] mpMask;
         mpMask = NULL;
      }

      mx1 = rhs.mx1;
      my1 = rhs.my1;
      mx2 = rhs.mx2;
      my2 = rhs.my2;

      mbbx1 = rhs.mbbx1;
      mbby1 = rhs.mbby1;
      mbbx2 = rhs.mbbx2;
      mbby2 = rhs.mbby2;

      mxSize = rhs.mxSize;
      mySize = rhs.mySize;
      mSize = rhs.mSize;

      mCount = rhs.mCount;

      if (rhs.mpMask)
      {
         mpMask = new unsigned int*[my2 - my1 + 1];
         mpMask[0] = new (nothrow) unsigned int[mSize];
         if (mpMask[0] == NULL)
         {
            delete[] mpMask;
            mpMask = NULL;
            throw bad_alloc();
         }

         memcpy(mpMask[0], rhs.mpMask[0], mSize * sizeof(unsigned int));

         for (int i = 1; i <= my2 - my1; ++i)
         {
            mpMask[i] = mpMask[i - 1] + mxSize;
         }
      }
      else
      {
         mpMask = NULL;
      }

      if (mpBuffer != NULL)
      {
         delete [] mpBuffer[0];
         delete [] mpBuffer;
         mpBuffer = NULL;
      }

      mBufferX1 = 0;
      mBufferY1 = 0;
      mBufferX2 = 0;
      mBufferY2 = 0;
      mBufferNeedsUpdated = true;
   }

   mOutside = rhs.mOutside;
   return *this;
}

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
void BitMaskImp::operator|=(const BitMaskImp& rhs)
{
   if (this == &rhs)
   {
      return;   // OR'ing with self
   }

   if (mCount == 0)
   {
      if (mOutside == false)   // empty mask OR'ed with any other mask is the other mask
      {
         *this = rhs;
      }
      else   // full mask OR'ed with anything is a full mask
      {
      }
      return;
   }

   if (rhs.mCount == 0)
   {
      if (rhs.mOutside == false) // OR'ing with an empty mask
      {
      }
      else   // OR'ing with a solid mask
      {
         *this = rhs;
      }
      return;
   }

   bool overlap = regionsOverlap (mx1, my1, mx2, my2, rhs.mx1, rhs.my1, rhs.mx2, rhs.my2);
   if (!overlap && (mOutside && rhs.mOutside))
   {
      if (mpMask)
      {
         delete [] mpMask[0];
         delete [] mpMask;
         mpMask = NULL;
      }

      mx1 = 0;
      my1 = 0;
      mx2 = 0;
      my2 = 0;
      mxSize = 0;
      mySize = 0;
      mSize = 0;
      mCount = 0;
      mOutside = true;

      return;
   }
   else
   {
      if (!overlap && mOutside)
      {
         return;
      }
      else
      {
         if (!overlap && rhs.mOutside)
         {
            *this = rhs;
            return;
         }
         else
         {
            unsigned int** lrhsMask = rhs.mpMask;
            unsigned int** lMask = NULL;

            growToInclude(rhs.mx1, rhs.my1, rhs.mx2, rhs.my2, mOutside | rhs.mOutside);

            int xOffset = (rhs.mx1 - mx1) / LONG_BITS;

            lMask = mpMask + rhs.my1 - my1;
            for (int y = rhs.my1; y <= rhs.my2; ++y)
            {
               for (int x = 0; x < rhs.mxSize; ++x)
               {
                  (*lMask)[x + xOffset] |= (*lrhsMask)[x];
               }

               lrhsMask++;
               lMask++;
            }

            mCount = computeCount();
            mx1 = min(mx1, rhs.mx1);
            mx2 = max(mx2, rhs.mx2);
            my1 = min(my1, rhs.my1);
            my2 = max(my2, rhs.my2);
            mbbx1 = min(mbbx1, rhs.mbbx1);
            mbbx2 = max(mbbx2, rhs.mbbx2);
            mbby1 = min(mbby1, rhs.mbby1);
            mbby2 = max(mbby2, rhs.mbby2);
         }
      }
   }
   mBufferNeedsUpdated = true;
}

/**
 *  In-place bitwise 'XOR' operator.
 *
 *  Merges two BitMasks. 
 *
 *  @param  rhs
 *          "Right Hand Side". The mask to merge from.
 */
void BitMaskImp::operator^=(const BitMaskImp& rhs)
{
   if (this == &rhs) // XOR'ing with self
   {
      clear();
      mOutside = false;
      return;
   }

   if (mCount == 0)
   {
      if (mOutside == false)   // empty mask XOR'ed with any other mask is the other mask
      {
         *this = rhs;
      }
      else   // full mask XOR'ed with anything is that mask inverted
      {
         *this = rhs;
         invert();
      }
      mBufferNeedsUpdated = true;
      return;
   }

   if (rhs.mCount == 0)
   {
      if (rhs.mOutside == false) // XOR'ing with an empty mask
      {
      }
      else   // XOR'ing with a solid mask
      {
         *this = rhs;
         invert();
      }
      mBufferNeedsUpdated = true;
      return;
   }

   unsigned int** lrhsMask = rhs.mpMask;
   unsigned int** lMask = NULL;

   growToInclude(rhs.mx1, rhs.my1, rhs.mx2, rhs.my2, mOutside ^ rhs.mOutside);

   int xOffset = (rhs.mx1 - mx1) / LONG_BITS;

   lMask = mpMask + rhs.my1 - my1;
   for (int y = rhs.my1; y <= rhs.my2; ++y)
   {
      for (int x = 0; x < rhs.mxSize; ++x)
      {
         (*lMask)[x + xOffset] ^= (*lrhsMask)[x];
      }

      lrhsMask++;
      lMask++;
   }

   mCount = computeCount();
   mBufferNeedsUpdated = true;
}

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
void BitMaskImp::operator&=(const BitMaskImp& rhs)
{
   int x;
   int y;

   if (this == &rhs)
   {
      return;   // AND'ing with self
   }

   if (mCount == 0)
   {
      if (mOutside == true)   // full mask AND'ed with any other mask is the other mask
      {
         *this = rhs;
      }
      else   // empty mask AND'ed with anything is an empty mask
      {
      }
      return;
   }

   if (rhs.mCount == 0 && rhs.mSize == 0)
   {
      if (rhs.mOutside == true) // AND'ing with a solid mask
      {
      }
      else   // AND'ing with an empty mask
      {
         *this = rhs;
      }

      return;
   }

   bool overlap = regionsOverlap (mx1, my1, mx2, my2, rhs.mx1, rhs.my1, rhs.mx2, rhs.my2);
   if (overlap || (mOutside && rhs.mOutside))
   {
      unsigned int** lrhsMask = NULL;
      unsigned int** lMask = NULL;
      unsigned int fill = 0xffffffff * rhs.mOutside;

      growToInclude (rhs.mx1, rhs.my1, rhs.mx2, rhs.my2, mOutside & rhs.mOutside);

      int xOffset = (rhs.mx1 - mx1) / LONG_BITS;

      int size = mxSize * sizeof(unsigned int);
      for (y = my1; y < rhs.my1; ++y)   // rows at the bottom with no overlap
      {
         for (x = 0; x < mxSize; ++x)
         {
            mpMask[y - my1][x] &= fill;
         }
      }

      if (mx1 < rhs.mx1)   // cols at the left with no overlap
      {
         size = xOffset * sizeof(unsigned int);
         lMask = mpMask + rhs.my1 - my1;
         for (y = rhs.my1; y <= rhs.my2; ++y)
         {
            for (x = 0; x < xOffset; ++x)
            {
               (*lMask)[x] &= fill;
            }

            lMask++;
         }
      }
      
      if (mx2 > rhs.mx2)   // cols at the right with no overlap
      {
         size = (mx2 - rhs.mx2) / LONG_BITS * sizeof(unsigned int);
         int pos = (rhs.mx2 + 1 - mx1) / LONG_BITS;
         lMask = mpMask + rhs.my1 - my1;
         for (y = rhs.my1; y <= rhs.my2; ++y, ++lMask)
         {
            for (x = 0; x < static_cast<int>(size / sizeof(unsigned int)); ++x)
            {
               (&(*lMask)[pos])[x] &= fill;
            }
         }
      }

      for (y = rhs.my1; y <= rhs.my2; ++y)   // overlap area
      {
         lMask = &mpMask[y - my1];
         lrhsMask = &rhs.mpMask[y - rhs.my1];
         for (x = 0; x < rhs.mxSize; ++x)
         {
            (*lMask)[x+xOffset] &= (*lrhsMask)[x];
         }
      }

      size = mxSize * sizeof(unsigned int);
      for ( ; y <= my2; ++y)   // rows at the top with no overlap
      {
         for (x = 0; x < mxSize; ++x)
         {
            mpMask[y-my1][x] &= fill;
         }
      }

      mCount = computeCount();
   }
   else // no overlap && one/both region(s) is/are 0 outside
   {
      if (!mOutside)   // no overlap && this.mOutside == false
      {
         if (!rhs.mOutside)   // no overlap && both Outsides == false
         {
            if (mpMask)
            {
               delete [] mpMask[0];
               delete [] mpMask;
               mpMask = NULL;
            }

            mx1 = 0;
            my1 = 0;
            mx2 = 0;
            my2 = 0;
            mxSize = 0;
            mySize = 0;
            mSize = 0;
            mCount = 0;
            mOutside = false;
         }
         return;
      }
      else // no overlap && (mOutside && !rhs.mOutside)
      {
         *this = rhs;
         return;
      }
   }
   mBufferNeedsUpdated = true;
}

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
bool BitMaskImp::operator==(const BitMaskImp& rhs) const
{
   if (mOutside == rhs.mOutside)
   {
      int left = min(mx1, rhs.mx1);
      int right = max(mx2, rhs.mx2);
      int bottom = min(my1, rhs.my1);
      int top = max(my2, rhs.my2);

      for (int x = left; x <= right; x += 32)
      {
         for (int y = bottom; y <= top; ++y)
         {
            if (getPixels(x, y) != rhs.getPixels(x, y))
            {
               return false;
            }
         }
      }

      return true;
   }

   return false;
}

/**
 *  clear member function.
 *
 *  Re-initializes a bitmask to have no bits set
 */
void BitMaskImp::clear()
{
   BitMaskImp temp;
   *this = temp;
}

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
void BitMaskImp::merge(const BitMask& rhs)
{
   *this |= *(dynamic_cast<const BitMaskImp*>(&rhs));
}

/**
 *  In-place bitwise 'XOR' operator.
 *
 *  Merges two BitMasks. 
 *
 *  @param  rhs
 *          "Right Hand Side". The mask to merge from.
 */
void BitMaskImp::toggle(const BitMask& rhs)
{
   *this ^= *(dynamic_cast<const BitMaskImp*>(&rhs));
}

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
void BitMaskImp::intersect(const BitMask& rhs)
{
   *this &= *(dynamic_cast<const BitMaskImp*>(&rhs));
}

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
bool BitMaskImp::compare(const BitMask& rhs) const
{
   return *this == *(dynamic_cast<const BitMaskImp*>(&rhs));
}

/**
 *  Mask inversion method.
 *
 *  Inverts all bits in the mask. All 1's become 0's and all 0's
 *  become 1's. It uses the ~ operator internally to perform the
 *  inversion operation
 */
void BitMaskImp::invert()
{
   int index = 0;
   for (int j = 0; j < mSize; ++j, ++index)
   {
      (*mpMask)[index] = ~(*mpMask)[index];
   }

   if (mSize != 0)
   {
      mCount = (mbbx2 - mbbx1 + 1) * (mbby2 - mbby1 + 1) - mCount;
   }

   mOutside = !mOutside;
   mBufferNeedsUpdated = true;
}

void BitMaskImp::setRegion(int x1, int y1, int x2, int y2, ModeType op)
{
   if (x1 > x2)
   {
      int temp = x1;
      x1 = x2;
      x2 = temp;
   }

   if (y1 > y2)
   {
      int temp = y1;
      y1 = y2;
      y2 = temp;
   }

   // setRegion only works if the BitMask is empty..instead of
   // fixing it and possibly introducing other bugs, when the
   // mask is not empty, we setregion on a new mask and merge the
   // two...this is a quick operation so there should be no noticable
   // loss of performance
   if (getCount() > 0)
   {
      BitMaskImp tmp;
      tmp.setRegion(x1, y1, x2, y2, op);
      merge(tmp);
      return;
   }
   int i;
   int x;
   int y;
   int leftX;
   int rightX;
   unsigned int mask = 0;
   unsigned int leftMask = 0xffffffff;
   unsigned int rightMask = 0xffffffff;

   int inX1 = x1;
   int inX2 = x2;

   if (x1 > mx2 || x2 < mx1 || y1 > my2 || y2 < my1 || mpMask == NULL)
   {
      if ((op == DRAW && mOutside == true) || (op == ERASE && mOutside == false))
      {
         return;
      }

      growToInclude(x1, y1, x2, y2, mOutside);
   }

   leftX = 32 * (x1 / 32);
   rightX = 32 * (x2 / 32 + 1) - 1;

   for (i = 0; i < x1 - leftX; ++i)
   {
      leftMask >>= 1;
   }
   leftMask ^= -1;

   for (i = 0; i < rightX - x2; ++i)
   {
      rightMask <<= 1;
   }
   rightMask ^= -1;

   switch (op)
   {
   case ERASE:
      mask = -1;
      leftMask ^= -1;
      rightMask ^= -1;
      // yes, I left the break out on purpose
   case DRAW:
      mask ^= -1;   // will be 0 if op==CLEAR and 0xffffffff if op==SET
      leftMask ^= -1;
      rightMask ^= -1;

      for (y = y1; y <= y2; ++y)
      {
         if (leftX == rightX - 31)
         {
            setPixels(leftX, y, leftMask & rightMask);
         }
         else
         {
            if (rightX - 31 > leftX + 32)
            {
               setPixels(leftX, y, leftMask);
               for (x = leftX + 32; x < rightX - 31; x += 32)
               {
                  setPixels(x, y, mask);
               }
               setPixels(rightX - 31, y, rightMask);
            }
            else
            {
               setPixels(leftX, y, leftMask);
               setPixels(rightX - 31, y, rightMask);
            }
         }
      }
      break;
   case TOGGLE:
      mask ^= -1;
      for (y = y1; y <= y2; ++y)
      {
         if (leftX == rightX - 31)
         {
            setPixels(leftX, y, getPixels(leftX, y) ^ leftMask & rightMask);
         }
         else
         {
            if (rightX - 31 > leftX + 32)
            {
               setPixels(leftX, y, getPixels(leftX, y) ^ leftMask);
               for (x = leftX + 32; x < rightX - 31; x += 32)
               {
                  setPixels(x, y, getPixels(leftX, y) ^ mask);
               }
               setPixels(rightX - 31, y, rightMask);
            }
            else
            {
               setPixels(leftX, y, getPixels(leftX, y) ^ leftMask);
               setPixels(rightX - 31, y, getPixels(leftX, y) ^ rightMask);
            }
         }
      }
      break;
   default:
      break;
   }
   mBufferNeedsUpdated = true;
}

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
bool BitMaskImp::isSubsetOf(const BitMask& source) const
{
   const BitMaskImp& sourceImp = dynamic_cast<const BitMaskImp&>(source);
   bool result = false;

   if (!mOutside || (mOutside && sourceImp.mOutside))
   {
      int left = min(mx1, sourceImp.mx1);
      int right = max(mx2, sourceImp.mx2);
      int bottom = min(my1, sourceImp.my1);
      int top = max(my2, sourceImp.my2);

      for (int x = left; x <= right; x += 32)
      {
         for (int y = bottom; y <= top; ++y)
         {
            if (((~getPixels(x, y)) | sourceImp.getPixels(x, y)) != 0xffffffff)
            {
               return false;
            }
         }
      }

      result = true;
   }

   return result;
}

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
void BitMaskImp::setPixel(int x, int y, bool value)
{
   if (x > mx2 || x < mx1 || y > my2 || y < my1 || mpMask == NULL)
   {
      if (value == mOutside)
      {
         return;
      }
      else
      {
         growToInclude(x, y, x, y, mOutside);
      }
   }

   if (value != mOutside)
   {
      mbbx1 = min(mbbx1, x);
      mbbx2 = max(mbbx2, x);
   }

   x -= mx1;
   y -= my1;

   int longIndex = x >> 5; // divide by 32
   int longShift = x & 0x1f;   // mod 32
   unsigned int longMask = 0x80000000 >> longShift;

   unsigned int* pMaskValue = &mpMask[y][longIndex];
   bool isSet = (*pMaskValue & longMask) != 0;
   if (value == true)
   {
      if (!isSet)
      {
         *pMaskValue |= longMask;
         mCount++;
      }
   }
   else
   {
      if (isSet)
      {
         *pMaskValue &= ~longMask;
         mCount--;
      }
   }

   mBufferNeedsUpdated = true;
   return;
}

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
bool BitMaskImp::getPixel(int x, int y) const
{
   if (x > mbbx2 || x < mbbx1 || y > mbby2 || y < mbby1 || mpMask == NULL)
   {
      return mOutside;
   }

   x -= mx1;
   y -= my1;

   int longIndex = x >> 5; // divide by 32
   int longShift = x & 0x1f;   // mod 32
   unsigned int longMask = 0x80000000 >> longShift;

   return ((mpMask[y][longIndex] & longMask) != 0);
}

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
unsigned int BitMaskImp::getPixels(int x, int y) const
{
   if (x > mx2 || x < mx1 || y > my2 || y < my1 || mpMask == NULL)
   {
      return mOutside * 0xffffffff;
   }

   x -= mx1;
   y -= my1;

   if (x & 0x1f)
   {
      x -= (x & 0x1f);
   }

   unsigned int values = mpMask[y][x / LONG_BITS];
   return values;
}

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
void BitMaskImp::setPixels(int x, int y, unsigned int values)
{
   if (x > mx2 || x < mx1 || y > my2 || y < my1 || mpMask == NULL)
   {
      if (values == mOutside * 0xffffffff)
      {
         return;
      }

      int rdx = x - x & 0x1f;
      growToInclude(x, y, x + 31, y, mOutside);
   }

   x -= mx1;
   y -= my1;

   unsigned int* pMaskValues = &mpMask[y][x / LONG_BITS];

   mCount += countBits (values) - countBits (*pMaskValues);

   *pMaskValues = values;
   mBufferNeedsUpdated = true;
}

void BitMaskImp::getBoundingBox(int& x1, int& y1, int& x2, int& y2) const
{
   x1 = mbbx1;
   y1 = mbby1;
   x2 = mbbx2;
   y2 = mbby2;
}

bool BitMaskImp::isOutsideSelected() const
{
   return mOutside;
}

void BitMaskImp::clipBoundingBox(int x1, int y1, int x2, int y2)
{
   bool bOutside = mOutside;

   BitMaskImp maskCopy(*this);
   clear();

   for (int i = y1; i <= y2; ++i)
   {
      for (int j = x1; j <= x2; ++j)
      {
         bool bSelected = maskCopy.getPixel(j, i);
         setPixel(j, i, bSelected);
      }
   }

   mOutside = bOutside;
}

/**
 *  getCount method.
 *
 *  Gets the number of bits that are set in the mask.
 *
 *  @return
 *         the number of bits that are set in the mask
 */
int BitMaskImp::getCount() const
{
   return mCount;
}

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
const bool** BitMaskImp::getRegion(int x1, int y1, int x2, int y2)
{
   int i;
   int j;
   int k;
   unsigned int values;
   unsigned int mask;
   bool* pBuffer = NULL;
   bool* pSafe = NULL;

   if ((mBufferNeedsUpdated == false) && (x1 == mBufferX1) && (y1 == mBufferY1) &&
      (x2 == mBufferX2) && (y2 == mBufferY2) && (mpBuffer != NULL))
   {
      return const_cast<const bool**>(mpBuffer);
   }

   mBufferX1 = x1;
   mBufferY1 = y1;
   mBufferX2 = x2;
   mBufferY2 = y2;

   int offset = x1 & 0x1f;

   if (mpBuffer)
   {
      delete [] mpBuffer[0];
      delete [] mpBuffer;
   }

   int left = x1 - (x1 & 0x1f);
   int right = x2 - (x2 & 0x1f) + 31;

   int xSize = right - left + 1;
   int ySize = y2 - y1 + 1;
   int totalSize = xSize * ySize;

   mpBuffer = new bool*[ySize];
   mpBuffer[0] = new (nothrow) bool[totalSize];
   if (mpBuffer[0] == NULL)
   {
      delete[] mpBuffer;
      mpBuffer = NULL;
      throw bad_alloc();
   }

   for (i = 1; i < ySize; ++i)
   {
      mpBuffer[i] = mpBuffer[i - 1] + xSize;
   }

   for (i = y1; i <= y2; ++i)
   {
      pBuffer = &mpBuffer[i - y1][-offset];
      pSafe = mpBuffer[i - y1];
      for (j = left; j <= right; j += 32)
      {
         values = getPixels (j, i);
         mask = 0x80000000;
         for (k = 0; k < 32; ++k, ++pBuffer)
         {
            if (pBuffer >= pSafe)
            {
               *pBuffer = ((values & mask) != 0);
            }
            mask >>= 1;
         }
      }
   }

   mBufferNeedsUpdated = false;
   return const_cast<const bool**>(mpBuffer);
}

/**
 *  computeCount member function.
 *
 *  Computes the number of set bits in the BitMask.
 *
 *  @return
 *         the number of set bits in the mask
 */
int BitMaskImp::computeCount() const
{
   unsigned int* pRowMask = NULL;
   int count = 0;

   for (int i = 0; i < mySize; ++i)
   {
      pRowMask = mpMask[i];
      for (int j = 0; j < mxSize; ++j)
      {
         if (pRowMask[j] == 0xffffffff)
         {
            count += LONG_BITS;
         }
         else
         {
            count += countBits(pRowMask[j]);
         }
      }
   }

   return count;
}

/**
 *  growToInclude method.
 *
 *  Expands the bitmask's mask to cover the area it previously covered
 *   plus the newly specified area. All new pixels are filled with the
 *   value specified by the 'fill' parameter.
 *
 *  @param  x1,y1
 *          The coordinate of the lower-left corner of the new region
 *  @param  x2,y2
 *          The coordinate of the upper-right corner of the new region
 *   @param   fill
 *         The value to fill newly allocated bits with.
 */
void BitMaskImp::growToInclude(int x1, int y1, int x2, int y2, bool fill)
{
   int i;
   int leftExtra;
   int rightExtra;
   int topExtra;
   int bottomExtra;
   unsigned int** lMask = NULL;

   int inX1 = x1;
   int inY1 = y1;
   int inX2 = x2;
   int inY2 = y2;

   x1 -= (x1 & 0x1f);
   x2 = x2 - (x2 & 0x1f) + 31;

   if (mx1 > x1)
   {
      leftExtra = (mx1 - x1) / LONG_BITS;
   }
   else
   {
      leftExtra = 0;
   }

   if (x2 > mx2)
   {
      rightExtra = (x2 - mx2) / LONG_BITS;
   }
   else
   {
      rightExtra = 0;
   }

   if (my1 > y1)
   {
      bottomExtra = my1 - y1;
   }
   else
   {
      bottomExtra = 0;
   }

   if (y2 > my2)
   {
      topExtra = y2 - my2;
   }
   else
   {
      topExtra = 0;
   }

   if (leftExtra <= 0 && rightExtra <= 0 && bottomExtra <= 0 && topExtra <= 0 && mpMask != NULL)
   {
      mOutside = fill;
      return;
   }

   int newx1;
   int newx2;
   int newy1;
   int newy2;

   if (mpMask != NULL)
   {
      newx1 = min(mx1, x1);
      newx2 = max(mx2, x2);
      newy1 = min(my1, y1);
      newy2 = max(my2, y2);
      mbbx1 = min(mbbx1, inX1);
      mbbx2 = max(mbbx2, inX2);
      mbby1 = min(mbby1, inY1);
      mbby2 = max(mbby2, inY2);
   }
   else
   {
      newx1 = x1;
      newx2 = x2;
      newy1 = y1;
      newy2 = y2;
      mbbx1 = inX1;
      mbbx2 = inX2;
      mbby1 = inY1;
      mbby2 = inY2;
   }

   int newxSize = (newx2 - newx1 + 1) / LONG_BITS;
   int newySize = newy2 - newy1 + 1;
   int newSize = newxSize * newySize;

   lMask = new unsigned int*[newySize];
   lMask[0] = new (nothrow) unsigned int[newSize];
   if (lMask[0] == NULL)
   {
      delete [] lMask;
      lMask = NULL;
      throw bad_alloc();
   }

   for (i = 1; i < newySize; ++i)
   {
      lMask[i] = lMask[i - 1] + newxSize;
   }

   memset(lMask[0], fill * 0xff, newSize * sizeof(unsigned int));

   bottomExtra = max(bottomExtra, 0);
   for (i = 0; i < mySize; ++i)
   {
      memcpy(&lMask[bottomExtra + i][leftExtra], mpMask[i], mxSize * sizeof(unsigned int));
   }

   if (mpMask)
   {
      delete [] mpMask[0];
      delete [] mpMask;
   }

   mpMask = lMask;

   mx1 = newx1;
   my1 = newy1;
   mx2 = newx2;
   my2 = newy2;

   mxSize = newxSize;
   mySize = newySize;
   mSize = newSize;

   mOutside = fill;

   if (fill)
   {
      mCount += (topExtra + bottomExtra) * newxSize * LONG_BITS;
      mCount += (leftExtra + rightExtra) * LONG_BITS;
   }
}

/**
 *  countBits function.
 *
 *  Computes the number of bits set in the unsigned int passed in as
 *   a parameter.
 *
 *  @param  v
 *          The value to be counted.
 *
 *  @return
 *         the number of bits set.
 */
static inline int countBits(unsigned int v)
{
   int count = 0;
   for (int i = 0; i < LONG_BITS; ++i)
   {
      if (v & 0x1)
      {
         count++;
      }

      v >>= 1;
   }

   return count;
}

/**
 *  regionsOverlap function.
 *
 *  Determines if two rectangular regions, specified by their ll and ur corners, overlap.
 *
 *  @param  r1x1,r1y1
 *          The lower left corner of region 1
 *  @param  r1x2,r1y2
 *          The upper right corner of region 1
 *  @param  r2x1,r2y1
 *          The lower left corner of region 2
 *  @param  r2x2,r2y2
 *          The upper right corner of region 2
 *
 *  @return
 *         true if the regions share any pixels
 *         false otherwise
 */
static inline bool regionsOverlap(int r1x1, int r1y1, int r1x2, int r1y2,
                                  int r2x1, int r2y1, int r2x2, int r2y2)
{
   return
      r1x1 > r2x2 ? false:   // r1 entirely to the right of r2
      r1x2 < r2x1 ? false:   // r1 entirely to the left of r2
      r1y1 > r2y2 ? false:   // r1 entirely above r2
      r1y2 < r2y1 ? false:   // r1 entirely below r2
      true;            // else, they overlap
}

bool BitMaskImp::toXml(XMLWriter* xml) const
{
   stringstream buf;
   DOMElement* rectElmnt(xml->addElement("rectangle"));
   buf << mx1 << " " << my1 << " " << mx2 << " " << my2;
   xml->addText(buf.str().c_str(), rectElmnt);

   buf.str("");
   DOMElement* bbElmnt(xml->addElement("boundingBox"));
   buf << mbbx1 << " " << mbby1 << " " << mbbx2 << " " << mbby2;
   xml->addText(buf.str().c_str(), bbElmnt);

   buf.str("");
   DOMElement* sizeElmnt(xml->addElement("size"));
   buf << mxSize << " " << mySize << " " << mCount;
   xml->addText(buf.str().c_str(), sizeElmnt);

   xml->addAttr("outside", (mOutside) ? "true" : "false");

   if (mpMask != NULL)
   {
      std::string checksum;
      XMLByte* b64repr = XmlBase::encodeBase64(mpMask[0], mxSize * mySize, NULL, &checksum);
      xml->pushAddPoint(xml->addElement("mask"));
      xml->addText(reinterpret_cast<char*>(b64repr));
      xml->popAddPoint();
      xml->addAttr("ecc", "ccitt:" + checksum);

      // Use ::operator delete() per Xerces documentation
      ::operator delete(b64repr);
   }

   return true;
}

bool BitMaskImp::fromXml(DOMNode* document, unsigned int version)
{
   string outsideVal(A(static_cast<DOMElement*>(document)->getAttribute(X("outside"))));
   string crcString(A(static_cast<DOMElement*>(document)->getAttribute(X("ecc"))));
   if (outsideVal == "1" || outsideVal == "t" || outsideVal == "true")
   {
      mOutside = true;
   }
   else
   {
      mOutside = false;
   }

   for (DOMNode* pChld = document->getFirstChild(); pChld != NULL; pChld = pChld->getNextSibling())
   {
      if (XMLString::equals(pChld->getNodeName(), X("rectangle")))
      {
         DOMNode* pGchld(pChld->getFirstChild());
         double a;
         double b;
         double c;
         double d;
         XmlReader::StrToQuadCoord(pGchld->getNodeValue(), a, b, c, d);
         mx1 = static_cast<int>(a);
         my1 = static_cast<int>(b);
         mx2 = static_cast<int>(c);
         my2 = static_cast<int>(d);
      }
      else if (XMLString::equals(pChld->getNodeName(), X("boundingBox")))
      {
         DOMNode* pGchld(pChld->getFirstChild());
         double a;
         double b;
         double c;
         double d;
         XmlReader::StrToQuadCoord(pGchld->getNodeValue(), a, b, c, d);
         mbbx1 = static_cast<int>(a);
         mbby1 = static_cast<int>(b);
         mbbx2 = static_cast<int>(c);
         mbby2 = static_cast<int>(d);
      }
      else if (XMLString::equals(pChld->getNodeName(), X("size")))
      {
         // we only need to read 3 coords by StrToQuadCoord still works well
         // as it handles cases where there are fewer than four coords (defaults them to 0.0)
         // so we just pass in a dummy that never actually gets read
         DOMNode* pGchld(pChld->getFirstChild());
         double a;
         double b;
         double c;
         double dummy;
         XmlReader::StrToQuadCoord(pGchld->getNodeValue(), a, b, c, dummy);
         mxSize = static_cast<int>(a);
         mySize = static_cast<int>(b);
         mCount = static_cast<int>(c);
         mSize = mxSize * mySize;
      }
      else if (XMLString::equals(pChld->getNodeName(), X("mask")))
      {
         mpMask = new (nothrow) unsigned int*[mySize];
         if (mpMask == NULL)
         {
            throw XmlReader::DomParseException("Can't create a new unsigned int array", pChld);
         }

         std::string checksum;
         if (crcString.find("ccitt:") != string::npos)
         {
            checksum = crcString.substr(crcString.find(":") + 1);
         }

         DOMNode* pGchld(pChld->getFirstChild());
         mpMask[0] = XmlBase::decodeBase64(reinterpret_cast<const XMLByte*>(A(pGchld->getNodeValue())), 0, checksum);
         if (mpMask[0] == NULL)
         {
            delete [] mpMask;
            mpMask = NULL;
            throw XmlReader::DomParseException("Can't decode the bitmask", pChld);
         }

         for (int i = 1; i < mySize; ++i)
         {
            mpMask[i] = mpMask[i - 1] + mxSize;
         }
      }
   }

   return true;
}

void BitMaskImp::getMinimalBoundingBox(int& x1, int& y1, int& x2, int& y2) const
{
   x1 = numeric_limits<int>::max();
   y1 = numeric_limits<int>::max();
   x2 = numeric_limits<int>::min();
   y2 = numeric_limits<int>::min();

   for (int y = my1; y <= my2; ++y)
   {
      for (int x = mx1; x < mx2 && x < x1; x += LONG_BITS)
      {
         unsigned long val = mpMask[(y - my1)][(x - mx1) / LONG_BITS];
         if (val != 0x00000000)
         {
            int i = 0;
            while (val != 0)
            {
               if (val & 0x80000000)
               {
                  x1 = min(x1, x + i);
                  y1 = min(y1, y);
                  break;
               }
               ++i;
               val <<= 1;
            }
         }
      }
   }

   for (int y = my2; y >= my1; --y)
   {
      for (int x = mx2; x > mx1 && x > x2; x -= LONG_BITS)
      {
         unsigned long val = mpMask[y - my1][(x - mx1) / LONG_BITS];
         if (val != 0x00000000)
         {
            int i = 0;
            while (val != 0)
            {
               if (val & 0x00000001)
               {
                  x2 = max(x2, x - i);
                  y2 = max(y2, y);
                  break;
               }
               ++i;
               val >>= 1;
            }
         }
      }
   }

   // set to defaults if nothing was found.
   // If any of these are triggered, the box is probably empty of any actual points.
   if (x1 == numeric_limits<int>::max())
   {
      x1 = 0;
   }
   if (y1 == numeric_limits<int>::max())
   {
      y1 = 0;
   }
   if (x2 == numeric_limits<int>::min())
   {
      x2 = 0;
   }
   if (y2 == numeric_limits<int>::min())
   {
      y2 = 0;
   }
}
