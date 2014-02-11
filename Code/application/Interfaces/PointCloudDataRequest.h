/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUDDATAREQUEST_H
#define POINTCLOUDDATAREQUEST_H

#include "TypesFile.h"

class PointCloudDataDescriptor;

/**
 * PointCloudDataRequest is a class used to specify how to get access to data
 * through a PointCloudAccessor.
 *
 * To use, create an instance with the ObjectFactory or a FactoryResource.
 * Set the fields for which the defaults are insufficient.  Pass in the
 * instance to PointCloudElement::getPointCloudAccessor.
 *
 * @see PointCloudElement, PointCloudDataDescriptor, PointCloudAccessor
 */
class PointCloudDataRequest
{
public:
   /**
    * Creates and returns a copy of the object.
    *
    * @return A new PointCloudDataRequest, which is a copy of the existing one.
    *         The caller is responsible for ensuring deletion of the
    *         returned object.
    */
   virtual PointCloudDataRequest *copy() const = 0;

   /**
    * Determine whether the PointCloudDataRequest is valid for the given 
    * PointCloudDataDescriptor.
    *
    * The validation will most likely fail if the PointCloudDataRequest has not
    * had polish() called on it.
    *
    * Plug-ins generally do not need to call this function.
    *
    * @param pDescriptor
    *        The descriptor to validate against.
    *
    * @return True if the PointCloudDataRequest is a valid request for the
    *         descriptor, false otherwise.
    */
   virtual bool validate(const PointCloudDataDescriptor *pDescriptor) const = 0;

   /**
    * Polish the PointCloudDataRequest for the given PointCloudDataDescriptor.
    *
    * This function will apply any defaults to the actual value as
    * appropriate for the given descriptor.  This function should be called
    * before validate().
    *
    * The return values of accessor functions are not valid
    * until this function has been called.
    *
    * Plug-ins generally do not need to call this function, as the core 
    * will call it when appropriate.
    *
    * @param pDescriptor
    *        The descriptor from which to apply defaults.
    *
    * @return True if the operation succeeded, false otherwise.
    */
   virtual bool polish(const PointCloudDataDescriptor *pDescriptor) = 0;

   /**
    * Get the version required to support this request.
    *
    * @param pDescriptor
    *        The descriptor to use to determine required version.
    *
    * @return The smallest version number which can properly use this
    *         PointCloudDataRequest. Currently always returns 1.
    *
    * @see RasterPager::getSupportedRequestVersion()
    */
   virtual int getRequestVersion(const PointCloudDataDescriptor *pDescriptor) const = 0;

   /**
    * Get the requested start X location.
    *
    * This defaults to the minimum X in the PointCloudDataDescriptor.
    *
    * @return The requested start X location.
    */
   virtual double getStartX() const = 0;

   /**
    * Get the requested stop X location.
    *
    * This defaults to the maximum X in the PointCloudDataDescriptor.
    *
    * @return The requested start X location.
    */
   virtual double getStopX() const = 0;

   /**
    * Get the requested start Y location.
    *
    * This defaults to the minimum Y in the PointCloudDataDescriptor.
    *
    * @return The requested start Y location.
    */
   virtual double getStartY() const = 0;

   /**
    * Get the requested stop Y location.
    *
    * This defaults to the maximum Y in the PointCloudDataDescriptor.
    *
    * @return The requested start Y location.
    */
   virtual double getStopY() const = 0;

   /**
    * Get the requested start Z location.
    *
    * This defaults to the minimum Z in the PointCloudDataDescriptor.
    *
    * @return The requested start Z location.
    */
   virtual double getStartZ() const = 0;

   /**
    * Get the requested stop Z location.
    *
    * This defaults to the maximum Z in the PointCloudDataDescriptor.
    *
    * @return The requested start Z location.
    */
   virtual double getStopZ() const = 0;

   /**
    * Set the requested start X location.
    *
    * This defaults to the minimum X in the PointCloudDataDescriptor.
    *
    * @param val
    *        The requested start X location.
    */
   virtual void setStartX(double val) = 0;

   /**
    * Set the requested stop X location.
    *
    * This defaults to the maximum X in the PointCloudDataDescriptor.
    *
    * @param val
    *        The requested stop X location.
    */
   virtual void setStopX(double val) = 0;

   /**
    * Set the requested start Y location.
    *
    * This defaults to the minimum Y in the PointCloudDataDescriptor.
    *
    * @param val
    *        The requested start Y location.
    */
   virtual void setStartY(double val) = 0;

   /**
    * Set the requested stop Y location.
    *
    * This defaults to the maximum Y in the PointCloudDataDescriptor.
    *
    * @param val
    *        The requested stop Y location.
    */
   virtual void setStopY(double val) = 0;

   /**
    * Set the requested start Z location.
    *
    * This defaults to the minimum Z in the PointCloudDataDescriptor.
    *
    * @param val
    *        The requested start Z location.
    */
   virtual void setStartZ(double val) = 0;

   /**
    * Set the requested stop Z location.
    *
    * This defaults to the maximum Z in the PointCloudDataDescriptor.
    *
    * @param val
    *        The requested stop Z location.
    */
   virtual void setStopZ(double val) = 0;

   /**
    * Set the start and stop values for X, Y, and Z
    *
    * @param startX
    *        The requested start X.
    * @param stopX
    *        The requested stop X.
    * @param startY
    *        The requested start Y.
    * @param stopY
    *        The requested stop Y.
    * @param startZ
    *        The requested start Z.
    * @param stopZ
    *        The requested stop Z.
    */
   virtual void setBoundingBox(double startX, double stopX, double startY, double stopY, double startZ, double stopZ) = 0;

   /**
    * Get whether the request is for writable data.
    *
    * This defaults to false.
    *
    * @return True if the request is for writable data, false otherwise.
    *
    * @see setWritable()
    */
   virtual bool getWritable() const = 0;

   /**
    * Set whether the request is for writable data.
    *
    * It is undefined what will happen if data is written
    * from a DataAccessor requested with an unwritable DataRequest.
    * Depending on the circumstances, it may apply the written data,
    * ignore the written data, crash, or lead to other undesirable results.
    * \b Always set the request to writable if you want to write to a PointCloudElement.
    *
    * @param writable
    *        True if the request is for writable data, false otherwise.
    *
    * @see getWritable(), PointCloudElement::updateData()
    */
   virtual void setWritable(bool writable) = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~PointCloudDataRequest() {}
};

#endif
