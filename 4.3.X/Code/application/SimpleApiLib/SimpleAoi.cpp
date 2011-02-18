/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElement.h"
#include "BitMask.h"
#include "BitMaskIterator.h"
#include "RasterElement.h"
#include "SimpleAoi.h"
#include "SimpleApiErrors.h"
#include <memory>

extern "C"
{
   int getAoiValue(DataElement* pElement, int32_t x, int32_t y)
   {
      AoiElement* pAoi = dynamic_cast<AoiElement*>(pElement);
      if (pAoi == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      const BitMask* pMask = pAoi->getSelectedPoints();
      setLastError(SIMPLE_NO_ERROR);
      return pMask->getPixel(x, y);
   }

   int setAoiValue(DataElement* pElement, uint32_t x, uint32_t y, int value)
   {
      AoiElement* pAoi = dynamic_cast<AoiElement*>(pElement);
      if (pAoi == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      if (value != 0)
      {
         pAoi->addPoint(LocationType(x, y));
      }
      else
      {
         pAoi->removePoint(LocationType(x, y));
      }
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   int getAoiMinimalBoundingBox(DataElement* pElement, int32_t* pX1, int32_t* pY1, int32_t* pX2, int32_t* pY2)
   {
      AoiElement* pAoi = dynamic_cast<AoiElement*>(pElement);
      if (pAoi == NULL || pX1 == NULL || pY1 == NULL || pX2 == NULL || pY2 == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      const BitMask* pMask = pAoi->getSelectedPoints();
      int x1, y1, x2, y2;
      pMask->getMinimalBoundingBox(x1, y1, x2, y2);
      *pX1 = x1;
      *pY1 = y1;
      *pX2 = x2;
      *pY2 = y2;
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }

   BitMaskIterator* createAoiIteratorOverRaster(DataElement* pElement, DataElement* pRaster)
   {
      AoiElement* pAoi = dynamic_cast<AoiElement*>(pElement);
      RasterElement* pRasterElement = dynamic_cast<RasterElement*>(pRaster);
      if (pAoi == NULL || pRasterElement == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      std::auto_ptr<BitMaskIterator> pIter(new BitMaskIterator(pAoi->getSelectedPoints(), pRasterElement));
      if (pIter.get() == NULL)
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pIter.release();
   }

   BitMaskIterator* createAoiIteratorOverBoundingBox(DataElement* pElement,
      int32_t x1, int32_t y1, int32_t x2, int32_t y2)
   {
      AoiElement* pAoi = dynamic_cast<AoiElement*>(pElement);
      if (pAoi == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return NULL;
      }
      std::auto_ptr<BitMaskIterator> pIter(new BitMaskIterator(pAoi->getSelectedPoints(), x1, y1, x2, y2));
      if (pIter.get() == NULL)
      {
         setLastError(SIMPLE_OTHER_FAILURE);
         return NULL;
      }
      setLastError(SIMPLE_NO_ERROR);
      return pIter.release();
   }

   void freeAoiIterator(BitMaskIterator* pIter)
   {
      delete pIter;
   }

   int nextAoiIterator(BitMaskIterator* pIter)
   {
      if (pIter == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 0;
      }
      setLastError(SIMPLE_NO_ERROR);
      return (*pIter)++;
   }

   int getAoiIteratorLocation(BitMaskIterator* pIter, int32_t* pX, int32_t* pY)
   {
      if (pIter == NULL || pX == NULL || pY == NULL)
      {
         setLastError(SIMPLE_BAD_PARAMS);
         return 1;
      }
      LocationType loc;
      pIter->getPixelLocation(loc);
      if (loc.mX == -1 && loc.mY == -1)
      {
         setLastError(SIMPLE_NOT_FOUND);
         return 1;
      }
      *pX = static_cast<int32_t>(loc.mX);
      *pY = static_cast<int32_t>(loc.mY);
      setLastError(SIMPLE_NO_ERROR);
      return 0;
   }
};
