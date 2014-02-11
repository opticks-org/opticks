/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTDATABLOCK_H
#define POINTDATABLOCK_H

#include "AppConfig.h"

/**
 * An abstraction for accessing a block of point cloud data.
 *
 * This is typically implemented by an underlying pager.
 */
class PointDataBlock
{
public:
   /**
    * Access a pointer to the block's raw data.
    *
    * @return a pointer to the raw data.
    */
   virtual void* getRawData() = 0;

   /**
    * Access the number of points in the data block.
    *
    * @return the number of points in this block.
    */
   virtual uint32_t getNumPoints() = 0;
};

#endif
