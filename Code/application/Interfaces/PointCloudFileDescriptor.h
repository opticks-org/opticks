/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDFILEDESCRIPTOR_H
#define POINTCLOUDFILEDESCRIPTOR_H

#include "FileDescriptor.h"

/**
 * A description of a point cloud data file.
 */
class PointCloudFileDescriptor : public FileDescriptor
{
public:
   /**
    * Access the number of points contained in the file.
    *
    * @return the number of points in the file.
    */
   virtual uint32_t getPointCount() const = 0;

   /**
    * Set the number of points in a file.
    *
    * This is typically called prior to creation of the element.
    * Changing this value after element creation may not propagate
    * the new value to the element.
    *
    * @param pointTotal
    *        The new number of points.
    */
   virtual void setPointCount(uint32_t pointTotal) = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~PointCloudFileDescriptor() {}
};

#endif
