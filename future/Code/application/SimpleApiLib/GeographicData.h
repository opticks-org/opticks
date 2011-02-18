/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef GEOGRAPHICDATA_H__
#define GEOGRAPHICDATA_H__

#include "AppConfig.h"

#if !defined(WIN_API)
#include <stdint.h>
#endif

class GcpList;

#ifdef __cplusplus
extern "C"
{
#endif
   /** \addtogroup simple_api */
   /*@{*/

   /**
   * @file GeographicData.h
   * This file contains functions and type definitions for accessing geographic (GCP) and tie point data.
   */

   /**
    * The in-memory definition of a GCP point.
    *
    * This is exactly the same memory layout as the GcpPoint class. It is redefined
    * here since a class is not a valid C type definition.
    */
   struct Gcp
   {
      double mPixelX;            /*< The pixel X position (column) in the data set */
      double mPixelY;            /*< The pixel Y position (row) in the data set */
      double reserved1;          /*< This offset is reserved for future expansion. The value is undefined. */
      double mLatitude;          /*< The corresponding latitude for the GCP point */
      double mLongitude;         /*< The corresponding longitude for the GCP point */
      double reserved2;          /*< This offset is reserved for future expansion. The value is undefined. */
      double mRmsErrorLatitude;  /*< The RMS error for the latitude value.
                                     This is generally non-zero if georeferencing has occured. */
      double mRmsErrorLongitude; /*< The RMS error for the longitude value.
                                     This is generally non-zero if georeferencing has occured. */
      double reserved3;          /*< This offset is reserved for future expansion. The value is undefined. */
   };

   /**
    * Get the number of GCP points in a GcpList.
    *
    * @param pList
    *        Handle for the GcpList being accessed.
    *        This handle should be obtained using getDataElement().
    *
    * @return The number of GCP points in the specified GCP list. This will be
    *         0 if an error occurs but getLastError() must still be called as
    *         0 may be returned if no error occurs.
    */
   EXPORT_SYMBOL uint32_t getGcpCount(DataElement* pList);

   /**
    * Get a GCP point from a GcpList.
    *
    * @param pList
    *        Handle for the GcpList being accessed.
    *        This handle should be obtained using getDataElement().
    *
    * @param index
    *        The zero based index for the requested GcpPoint. This must be
    *        less than the size returned by getGcpCount()
    *
    * @return The GcpPoint at location index. This is undefined if an error occurs.
    */
   EXPORT_SYMBOL struct Gcp getGcpPoint(DataElement* pList, uint32_t index);

   /**
    * Get an array of all the GCP points in a GcpList.
    *
    * @param pList
    *        Handle for the GcpList being accessed.
    *        This handle should be obtained using getDataElement().
    *
    * @param pPoints
    *        Output parameter which will contain the array of GcpPoints in pList.
    *        This shall be pre-allocated and must be at least
    *        (sizeof(struct Gcp) * getGcpCount()) bytes long.
    *
    * @return The number of GcpPoints placed in pPoints. If an error occurs, this will be 0
    *         but getLastError() should be checked as 0 may be returned if no error occurs.
    */
   EXPORT_SYMBOL uint32_t getGcpPoints(DataElement* pList, struct Gcp* pPoints);

   /**
    * Set the GCP points in a GcpList, replacing any existing points.
    *
    * @param pList
    *        Handle for the GcpList being modified.
    *        This handle should be obtained using getDataElement().
    *
    * @param count
    *        The number of GCP points in pPoints.
    *
    * @param pPoints
    *        The array of GcpPoints to set in pList.
    *        This shall be at least (sizeof(struct Gcp) * count) bytes long.
    *        This may be \c NULL if count is 0.
    *
    * @return The number of GcpPoints placed in the GcpList. If an error occurs, this will be 0
    *         but getLastError() should be checked as 0 may be returned if no error occurs.
    */
   EXPORT_SYMBOL uint32_t setGcpPoints(DataElement* pList, uint32_t count, struct Gcp* pPoints);
      
   /*@}*/
#ifdef __cplusplus
}
#endif

#endif