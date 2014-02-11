/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDPAGER_H
#define POINTCLOUDPAGER_H

#include "AppConfig.h"

class PointCloudDataRequest;
class PointDataBlock;

/**
 * Abstraction for a point cloud data pager.
 *
 * This is similar to a RasterPager but for point cloud data.
 * This is usually implemented by a point cloud pager.
 */
class PointCloudPager
{
public:
   /**
    * Access a block of point data at the given location.
    *
    * @param startIndex
    *        Index of the starting point requested.
    * @param numPoints
    *        The number of requested points in the data block. If there are not enough
    *        points to meet the request, the data block may contain fewer.
    * @param pOriginalRequest
    *        The PointCloudDataRequest used to create the accessor requesting this block.
    *        This must not be \c NULL but can be a default request.
    *
    * @return A pointer to the data block or \c NULL is the request is invalid, out of bounds, or can't
    *         meet the writable condition.
    */
   virtual PointDataBlock* getPointBlock(uint32_t startIndex, uint32_t numPoints, PointCloudDataRequest* pOriginalRequest) = 0;

   /**
    * Release the resources associated with the point data block.
    *
    * @param pBlock
    *        The previously allocated data block. If this is \c NULL, nothing will happen.
    */
   virtual void releasePointBlock(PointDataBlock* pBlock) = 0;

protected:
   /**
    *  Since the PointCloudPager interface is usually used in conjunction with the
    *  PlugIn and Executable interfaces, this should be destroyed by casting to
    *  the PlugIn interface and calling PlugInManagerServices::destroyPlugIn().
    */
   virtual ~PointCloudPager() {}
};

#endif
