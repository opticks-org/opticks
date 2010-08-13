/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIMPLEAOI_H__
#define SIMPLEAOI_H__

#include "AppConfig.h"

#ifdef __cplusplus
extern "C"
{
#endif
   /** \addtogroup simple_api */
   /*@{*/

   /**
    * @file SimpleAoi.h
    * This file contains API utilities for accessing AOI data as a bitmask.
    */

   /**
    * Get the value at a particular location in an AOI.
    *
    * The indices are relative to the origin of the AOI. Negative indices are possible and will
    * always return the value of BitMask::isOutsideSelected().
    *
    * @param pElement
    *        The AoiElement to access.
    * @param x
    *        The column requested.
    * @param y
    *        The row requested.
    * @return A zero if the requested location is not active in the AOI or a non-zero if the requested location
    *         is active in the AOI.
    */
   EXPORT_SYMBOL int getAoiValue(DataElement* pElement, int32_t x, int32_t y);

   /**
    * Set the value at a particular location in an AOI.
    *
    * The indices are relative to the origin of the AOI.
    *
    * @param pElement
    *        The AoiElement to access.
    * @param x
    *        The column to set.
    * @param y
    *        The row to set.
    * @param value
    *        The new value. A zero clears the point from the AOI and a non-zero adds the point to the AOI.
    * @return A zero if successful, a non-zero if an error occurs.
    */
   EXPORT_SYMBOL int setAoiValue(DataElement* pElement, uint32_t x, uint32_t y, int value);

   /**
    * Access the minimal bounding box required to contain the AOI's unique data.
    *
    * @param pElement
    *        The AoiElement to access.
    * @param pX1
    *        The start column.
    * @param pY1
    *        The start row.
    * @param pX2
    *        The end column.
    * @param pY2
    *        The end row.
    * @return Non-zero if an error occurred or zero otherwise.
    */
   EXPORT_SYMBOL int getAoiMinimalBoundingBox(DataElement* pElement,
      int32_t* pX1, int32_t* pY1, int32_t* pX2, int32_t* pY2);

   /**
    * Get an iterator over an AOI.
    *
    * @param pElement
    *        The AoiElement to access.
    * @param pRaster
    *        A raster element whose size will determine the bounding box.
    * @return A BitMaskIterator.
    */
   EXPORT_SYMBOL BitMaskIterator* createAoiIteratorOverRaster(DataElement* pElement, DataElement* pRaster);

   /**
    * Get an iterator over an AOI.
    *
    * @param pElement
    *        The AoiElement to access.
    * @param x1
    *        The starting X value for the bounding box.
    * @param y1
    *        The starting Y value for the bounding box.
    * @param x2
    *        The ending X value for the bounding box.
    * @param y2
    *        The ending Y value for the bounding box.
    * @return A BitMaskIterator.
    */
   EXPORT_SYMBOL BitMaskIterator* createAoiIteratorOverBoundingBox(DataElement* pElement,
      int32_t x1, int32_t y1, int32_t x2, int32_t y2);

   /**
    * Free an AOI iterator.
    *
    * Suitable for use as a cleanup callback.
    *
    * @param pIter
    *        The BitMaskIterator to free. No error checking is performed on this value
    *        and a \c NULL will cause a NOOP.
    */
   EXPORT_SYMBOL void freeAoiIterator(BitMaskIterator* pIter);

   /**
    * Advance an AOI iterator to the next selected pixel.
    *
    * @param pIter
    *        The BitMaskIterator to advance.
    * @return Zero if the end of the iterator has been reached or an error occurred.
    *         Non-zero if the new iterator position is valid.
    */
   EXPORT_SYMBOL int nextAoiIterator(BitMaskIterator* pIter);
   
   /**
    * Access the current AOI iterator position.
    *
    * @param pIter
    *        The BitMaskIterator to query.
    * @param pX
    *        Output parameter which will contain the current X location.
    * @param pY
    *        Output parameter which will contain the current Y location.
    * @return Non-zero if an error occurred or zero otherwise.
    */
   EXPORT_SYMBOL int getAoiIteratorLocation(BitMaskIterator* pIter, int32_t* pX, int32_t* pY);

   /*@}*/
#ifdef __cplusplus
}
#endif

#endif
