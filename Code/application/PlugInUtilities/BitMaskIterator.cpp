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

BitMaskIterator::BitMaskIterator(const BitMask * pBitMask, int x1, int y1, int x2, int y2) :
   mpBitMask(pBitMask),
   mX1(x1),
   mY1(y1),
   mX2(x2),
   mY2(y2),
   mCurrentPixelX(-1),
   mCurrentPixelY(0),
   mFirstPixelX(-1),
   mFirstPixelY(-1),
   mCurrentPixelCount(0),
   mPixelCount(-1)
{
   mX1 = max(mX1, 0);
   mY1 = max(mY1, 0);
   mX2 = max(mX2, 0);
   mY2 = max(mY2, 0);
   firstPixel();
}

BitMaskIterator::BitMaskIterator(const BitMask *pBitMask, RasterElement* pRasterElement) :
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
   mPixelCount(-1)
{
   if (pBitMask == NULL || pRasterElement == NULL)
   {
      return;
   }
   const RasterDataDescriptor* pDescriptor = 
      dynamic_cast<const RasterDataDescriptor*>(pRasterElement->getDataDescriptor());
   if (pDescriptor == NULL)
   {
      return;
   }
   int numRows = pDescriptor->getRowCount() - 1;
   int numColumns = pDescriptor->getColumnCount() - 1;
   if (mpBitMask->isOutsideSelected())
   {
      mX1 = 0;
      mY1 = 0;
      mX2 = numColumns;
      mY2 = numRows;
   }
   else
   {
      pBitMask->getMinimalBoundingBox(mX1, mY1, mX2, mY2);
      mX1 = max(mX1, 0);
      mY1 = max(mY1, 0);
      mX1 = min(mX1, static_cast<int>(numColumns));
      mY1 = min(mY1, static_cast<int>(numRows));
      mX2 = max(mX2, 0);
      mY2 = max(mY2, 0);
      mX2 = min(mX2, static_cast<int>(numColumns));
      mY2 = min(mY2, static_cast<int>(numRows));
   }
   firstPixel();
}

inline bool BitMaskIterator::getPixel() const
{
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

inline bool BitMaskIterator::getPixel(int col, int row) const
{
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

int BitMaskIterator::getCount()
{
   if (mPixelCount == -1)
   {
      computeCount();
   }
   return mPixelCount;
}

inline void BitMaskIterator::computeCount()
{
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
