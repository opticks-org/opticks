/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDELEMENT_H
#define POINTCLOUDELEMENT_H

#include "DataElement.h"
#include "PointCloudAccessor.h"
#include "TypesFile.h"

class PointCloudDataRequest;
class PointCloudPager;
class PointDataBlock;

/**
 * A data element containing point cloud data.
 *
 * Stores non-grid point data with 3 dimensional spatial points (x,y,z/height) along
 * with optional intensity and classification data. Auxiliary data can also be
 * associated with the data points.
 *
 * Examples of point cloud data include LIDAR and range imaging data.
 */
class PointCloudElement : public DataElement
{
public:
   /**
    * Signalled when raw point data has been modified.
    * The any contains the update mask.
    */
   SIGNAL_METHOD(PointCloudElement, DataModified);

   /**
    * Update mask values.
    */
   static const uint32_t UPDATE_ALL = 0xffffffff;               /* default value indicates all data may be updated */
   
   static const uint32_t UPDATE_X = 0x00000001;                 /* X locations updated */
   static const uint32_t UPDATE_Y = 0x00000002;                 /* Y locations updated */
   static const uint32_t UPDATE_Z = 0x00000004;                 /* Z locations updated */
   static const uint32_t UPDATE_LOCATION = 0x00000007;          /* X, Y, and Z locations updated */

   static const uint32_t UPDATE_INTENSITY = 0x00000010;         /* Intensity values updated */
   static const uint32_t UPDATE_CLASSIFICATION = 0x00000020;    /* Classification values updated */
   static const uint32_t UPDATE_AUX = 0x10000000;               /* Ancillary data updated. This not only indicates ancillary info #0 have been updated
                                                                   but can be or'd with an index number to indicate that particular index has been updated */
   static const uint32_t UPDATE_ALL_AUX = 0x1fffffff;           /* All ancillary data fields updated. */

   /**
    * Indicates that the caller has modified the raw point cloud data.
    *
    * Signals DataModified
    */
   virtual void updateData(uint32_t updateMask) = 0;

   /**
    * Sets the point cloud pager plug-in instance that will be used by this
    * object to access the data.
    *
    * This method provides the means to set a custom pager plug-in
    * that is used to access the data.
    *
    *  @param pPager
    *         The point cloud pager plug-in instance that should be used to
    *         page data into the application.
    *
    *  @return Returns \b true if the given pager plug-in can be
    *          used by this object; otherwise returns \b false.
    */
   virtual bool setPager(PointCloudPager* pPager) = 0;

   /**
    * Returns the point cloud pager plug-in instance that will be used by this
    * object to access the data.
    *
    * @return Returns the point cloud pager plug-in instance used by this object.
    */
   virtual PointCloudPager* getPager() const = 0;

   /**
    * Creates a default pager plug-in instance that will be used
    * by this object to store data in memory.
    *
    * This method creates a default point cloud pager plug-in that is used to
    * access the data using the data parameters specified in the data
    * descriptor.
    *
    * @return Returns \b true if the default pager plug-in was
    *         successfully created; otherwise returns \b false.
    */
   virtual bool createInMemoryPager() = 0;

   /**
    * If there is no pager set into the PointCloudElement, create a default
    * one.
    *
    * This method creates an appropriate default pager based on the
    * ProcessingLocation, including blank space to use for the data via
    * ModelServices::getMemoryBlock() if ProcessingLocation::IN_MEMORY.
    *
    * @return True if there was already a pager or a default one was successfully
    *         created, false otherwise.
    */
   virtual bool createDefaultPager() = 0;

   /**
    * Get a PointCloudAccessor with the parameters contained within the given request.
    *
    * @warning The returned PointCloudAccessor must be deleted prior to the deletion of this PointCloudElement.
    *
    * @param pRequest
    *        Requested access parameters.  If NULL, then the default parameters will
    *        be used.  This method takes ownership of the PointCloudDataRequest object.
    *
    * @return A PointCloudAccessor for the dataset.  If the request was invalid or
    *         unable to be filled, an invalid PointCloudDataAccessor will be returned.
    *         Callers should check the returned object's PointCloudAccessor::isValid().
    */
   virtual PointCloudAccessor getPointCloudAccessor(PointCloudDataRequest *pRequest=NULL) = 0;

   /**
    * Get a read-only PointCloudAccessor with the parameters contained within the given request.
    *
    * @warning The returned PointCloudAccessor must be deleted prior to the deletion of this PointCloudElement.
    *
    * @param pRequest
    *        Requested access parameters.  If NULL, then the default parameters will
    *        be used.  This method takes ownership of the PointCloudDataRequest object.
    *
    * @return A PointCloudAccessor for the dataset. If the request was writable, invalid or
    *         unable to be filled, an invalid PointCloudDataAccessor will be returned.
    *         Callers should check the returned object's PointCloudAccessor::isValid().
    */
   virtual PointCloudAccessor getPointCloudAccessor(PointCloudDataRequest *pRequest=NULL) const = 0;

   /**
    * Access the number of elements in the underlying memory block used by this element.
    *
    * In most cases, this will equal the number of points in the PointCloudDataElement.
    * In cases where the data are stored in optimized tree representations such as a Kd-tree, this may
    * be larger than the point count, often the next power of two larger.
    *
    * @return the number of array elements in the data block.
    */
   virtual uint32_t getArrayCount() = 0;

   typedef uint32_t pointIdType;
   typedef unsigned char validPointType;

protected:
   /**
    * This should be destroyed by calling ModelServices::destroyElement.
    */
   virtual ~PointCloudElement() {}
};

#endif
