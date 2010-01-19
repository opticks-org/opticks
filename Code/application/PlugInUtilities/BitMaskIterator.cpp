/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "BitMask.h"
#include "BitMaskIterator.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"

#include <limits>

using namespace std;

BitMaskIterator::BitMaskIterator(BitMaskIterator, bool) :
   mpBitMask(NULL),
   mX1(0),
   mY1(0),
   mX2(0),
   mY2(0),
   mCurrentPixelX(-1),
   mCurrentPixelY(-1),
   mFirstPixelX(-1),
   mFirstPixelY(-1),
   mCurrentPixelCount(0),
   mPixelCount(-1)
{}

BitMaskIterator::BitMaskIterator(const BitMask* pBitMask,
                                 unsigned int x1, unsigned int y1,
                                 unsigned int x2, unsigned int y2) :
   mpBitMask(pBitMask),
   mX1(min(min(x1, x2), static_cast<unsigned int>(numeric_limits<int>::max()))),
   mY1(min(min(y1, y2), static_cast<unsigned int>(numeric_limits<int>::max()))),
   mX2(min(max(x1, x2), static_cast<unsigned int>(numeric_limits<int>::max()))),
   mY2(min(max(y1, y2), static_cast<unsigned int>(numeric_limits<int>::max()))),
   mCurrentPixelX(-1),
   mCurrentPixelY(0),
   mFirstPixelX(-1),
   mFirstPixelY(-1),
   mCurrentPixelCount(0),
   mPixelCount(-1),
   mMinX(mX1),
   mMinY(mY1),
   mMaxX(mX2),
   mMaxY(mY2)
{
   if (mpBitMask != NULL)
   {
      int bbx1 = 0;
      int bby1 = 0;
      int bbx2 = 0;
      int bby2 = 0;

      mpBitMask->getMinimalBoundingBox(bbx1, bby1, bbx2, bby2);
      if (mpBitMask->isOutsideSelected() == false)
      {
         mX1 = max(bbx1, mMinX);
         mY1 = max(bby1, mMinY);
         mX1 = min(mX1, mMaxX);
         mY1 = min(mY1, mMaxY);
         mX2 = max(bbx2, mMinX);
         mY2 = max(bby2, mMinY);
         mX2 = min(mX2, mMaxX);
         mY2 = min(mY2, mMaxY);
      }
   }
   firstPixel();
}

BitMaskIterator::BitMaskIterator(const BitMask* pBitMask, const RasterElement* pRasterElement) :
   mpBitMask(pBitMask),
   mX1(0),
   mY1(0),
   mX2(0),
   mY2(0),
   mCurrentPixelX(-1),
   mCurrentPixelY(0),
   mFirstPixelX(-1),
   mFirstPixelY(-1),
   mCurrentPixelCount(0),
   mPixelCount(-1),
   mMinX(0),
   mMinY(0),
   mMaxX(-1),
   mMaxY(-1)
{
   if (mpBitMask == NULL || pRasterElement == NULL)
   {
      return;
   }
   const RasterDataDescriptor* pDescriptor = 
      dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }

   mMaxY = pDescriptor->getRowCount() - 1;
   mMaxX = pDescriptor->getColumnCount() - 1;
   if (mpBitMask->isOutsideSelected())
   {
      mX1 = mMinX;
      mY1 = mMinY;
      mX2 = mMaxX;
      mY2 = mMaxY;
   }
   else
   {
      mpBitMask->getMinimalBoundingBox(mX1, mY1, mX2, mY2);
      mX1 = max(mX1, mMinX);
      mY1 = max(mY1, mMinY);
      mX1 = min(mX1, mMaxX);
      mY1 = min(mY1, mMaxY);
      mX2 = max(mX2, mMinX);
      mY2 = max(mY2, mMinY);
      mX2 = min(mX2, mMaxX);
      mY2 = min(mY2, mMaxY);
   }
   firstPixel();
}

inline bool BitMaskIterator::getPixel() const
{
   if (mpBitMask == NULL)
   {
      return true;
   }
   if (mCurrentPixelX < mX1 || mCurrentPixelX > mX2 ||
       mCurrentPixelY < mY1 || mCurrentPixelY > mY2)
   {
      return false;
   }
   else
   {
      return mpBitMask->getPixel(mCurrentPixelX, mCurrentPixelY);
   }
}

bool BitMaskIterator::getPixel(int col, int row) const
{
   if (mpBitMask == NULL)
   {
      return true;
   }
   if (col < mX1 || col > mX2 || row < mY1 || row > mY2)
   {
      return false;
   }
   else
   {
      return mpBitMask->getPixel(col, row);
   }
}

void BitMaskIterator::nextPixel()
{
   while (mCurrentPixelY <= mY2)
   {
      if (mCurrentPixelX == mX2)
      {
         ++mCurrentPixelY;
         mCurrentPixelX = mX1;
      }
      else
      {
         ++mCurrentPixelX;
      }
      if (getPixel())
      {
         ++mCurrentPixelCount;
         if (mFirstPixelX == -1 && mFirstPixelY == -1)
         {
            mFirstPixelX = mCurrentPixelX;
            mFirstPixelY = mCurrentPixelY;
         }
         return;
      }
   }
   mCurrentPixelY = -1;
   mCurrentPixelX = -1;
}

void BitMaskIterator::getPixelLocation(LocationType& pixelLocation) const
{
   pixelLocation.mX = mCurrentPixelX;
   pixelLocation.mY = mCurrentPixelY;
}

bool BitMaskIterator::operator==(const BitMaskIterator& other) const
{
   return((mCurrentPixelX == other.mCurrentPixelX) &&
          (mCurrentPixelY == other.mCurrentPixelY));
}

bool BitMaskIterator::operator!=(const BitMaskIterator& other) const
{
   return((mCurrentPixelX != other.mCurrentPixelX) ||
          (mCurrentPixelY != other.mCurrentPixelY));
}

bool BitMaskIterator::operator*() const
{
   return getPixel();
}

bool BitMaskIterator::operator++()
{
   nextPixel();
   return getPixel();
}

bool BitMaskIterator::operator++(int)
{
   nextPixel();
   return getPixel();
}

BitMaskIterator BitMaskIterator::begin()
{
   return BitMaskIterator(*this);
}

void BitMaskIterator::firstPixel()
{
   if (mFirstPixelX == -1 && mFirstPixelY == -1)
   {
      mCurrentPixelX = -1;
      mCurrentPixelY = 0;
      mCurrentPixelCount = 0;
      nextPixel();
   }
   else
   {
      mCurrentPixelX = mFirstPixelX;
      mCurrentPixelY = mFirstPixelY;
      mCurrentPixelCount = 1;
   }
}

BitMaskIterator BitMaskIterator::end()
{
   return BitMaskIterator(*this, true);
}

int BitMaskIterator::getCount() const
{
   if (mPixelCount == -1)
   {
      const_cast<BitMaskIterator*>(this)->computeCount();
   }
   return mPixelCount;
}

inline void BitMaskIterator::computeCount()
{
   if (mpBitMask == NULL)
   {
      mPixelCount = getNumRows() * getNumColumns();
      return;
   }
   mPixelCount = mCurrentPixelCount;
   unsigned int saveCurrentPixelCount = mCurrentPixelCount;
   int savePixelX = mCurrentPixelX;
   int savePixelY = mCurrentPixelY;
   nextPixel();
   while ((*this) != end())
   {
      ++mPixelCount;
      nextPixel();
   }
   mCurrentPixelX = savePixelX;
   mCurrentPixelY = savePixelY;
   mCurrentPixelCount = saveCurrentPixelCount;
}

void BitMaskIterator::getBoundingBox(int& x1, int& y1, int& x2, int& y2) const
{
   x1 = mX1;
   y1 = mY1;
   x2 = mX2;
   y2 = mY2;
}

int BitMaskIterator::getNumSelectedRows() const
{
   int x1 = 0;
   int x2 = 0;
   int y1 = 0;
   int y2 = 0;
   getBoundingBox(x1, y1, x2, y2);
   return (y2 - y1 + 1);
}

int BitMaskIterator::getNumSelectedColumns() const
{
   int x1 = 0;
   int x2 = 0;
   int y1 = 0;
   int y2 = 0;
   getBoundingBox(x1, y1, x2, y2);
   return (x2 - x1 + 1);
}

LocationType BitMaskIterator::getOffset() const
{
   if (mpBitMask == NULL)
   {
      return LocationType(0, 0);
   }
   if (mpBitMask->isOutsideSelected())
   {
      return LocationType(0, 0);
   }
   return LocationType(mX1 - mMinX, mY1 - mMinY);
}

bool BitMaskIterator::useAllPixels() const
{
   return (getCount() == getNumRows() * getNumColumns());
}

int BitMaskIterator::getNumRows() const
{
   return mMaxY - mMinY + 1;
}

int BitMaskIterator::getNumColumns() const
{
   return mMaxX - mMinX + 1;
}

int BitMaskIterator::getBoundingBoxStartRow() const
{
   return mY1;
}

int BitMaskIterator::getBoundingBoxStartColumn() const
{
   return mX1;
}

int BitMaskIterator::getBoundingBoxEndRow() const
{
   return mY2;
}

int BitMaskIterator::getBoundingBoxEndColumn() const
{
   return mX2;
}

int BitMaskIterator::getRowOffset() const
{
   if (mpBitMask == NULL)
   {
      return 0;
   }
   if (mpBitMask->isOutsideSelected())
   {
      return 0;
   }
   return mY1 - mMinY;
}

int BitMaskIterator::getColumnOffset() const
{
   if (mpBitMask == NULL)
   {
      return 0;
   }
   if (mpBitMask->isOutsideSelected())
   {
      return 0;
   }
   return mX1 - mMinX;
}

int BitMaskIterator::getPixelRowLocation() const
{
   return mCurrentPixelY;
}

int BitMaskIterator::getPixelColumnLocation() const
{
   return mCurrentPixelX;
}
