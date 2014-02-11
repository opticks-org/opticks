/*
 * The information in this file is
 * Copyright(c) 2013 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef POINTCLOUD_ACCESSOR_H
#define POINTCLOUD_ACCESSOR_H

class PointCloudAccessorImpl;

/**
 *  Provides safe deletion of the PointCloudAccessorImpl.
 *
 *  A plug-in developer does not use this class directly.  This class is used 
 *  to safely delete the PointCloudAccessorImpl class.  When the PointCloudAccessor loses scope,
 *  the PointCloudAccessorImpl class is deleted.
 *
 *  @see        PointCloudAccessor, PointCloudAccessorImpl
 */
class PointCloudAccessorDeleter
{
public:
   /**
    * Provides support for deleting the PointCloudAccessorImpl.
    *
    * @param  pImpl
    *         A pointer to the managed PointCloudAccessorImpl instance.
    */
   virtual void operator()(PointCloudAccessorImpl* pImpl) = 0;
};

/**
 *  Provides reference counting for the PointCloudAccessorImpl.
 *
 *  A plug-in developer does not use this class directly.   The %PointCloudAccessor exists
 *  for the sole purpose of managing the lifespan of the PointCloudAccessorImpl.  This
 *  class is used to create a wrapper around the PointCloudAccessorImpl class to provide
 *  safe reference counting.  When the %PointCloudAccessor loses scope the PointCloudAccessorImpl
 *  class is deleted, causing its associated pager to release its held PointCloudPage.
 *  If the PointCloudPager has already been deleted (e.g.: by deleting its PointCloudElement),
 *  the destruction of the PointCloudAccessorImpl will cause undefined behavior. Therefore,
 *  the deletion of a PointCloudElement must occur after the deletion of its associated %PointCloudAccessors.
 *
 *  @see        PointCloudAccessorDeleter, PointCloudAccessorImpl
 */
class PointCloudAccessor
{
public:
   /**
    *  Creates a %PointCloudAccessor.
    *
    *  This class is not called directly by a plug-in developer.  This creates an
    *  instance of a class that manages the lifespan of the PointCloudAccessorImpl.
    *
    *  @param   pDeleter
    *           A class that manages the deletion of the PointCloudAccessorImpl.
    *  @param   pImpl
    *           The PointCloudAccessorImpl class to manage.
    */
   PointCloudAccessor(PointCloudAccessorDeleter* pDeleter, PointCloudAccessorImpl* pImpl);

   /**
    *  Default copy constructor.
    */
   PointCloudAccessor(const PointCloudAccessor& da);

   /**
    *  Destructor for the PointCloudAccessor.
    */
   ~PointCloudAccessor();

   /**
    *  The equals operator.
    *
    *  @param   rhs
    *           The PointCloudAccessor from which to set this accessor's values.
    */
   PointCloudAccessor& operator=(const PointCloudAccessor& rhs);

   /**
    *  Provides access to the real PointCloudAccessorImpl.
    *
    *  The overloaded operator-> allows us to continue to use pointer notation to 
    *  directly access the internally held pointer to the PointCloudAccessorImpl. 
    *  See also the Boost smart pointers. You will see the same thing.
    *
    *  @return  Returns a pointer to the PointCloudAccessorImpl.
    */
   inline PointCloudAccessorImpl* operator->() { return mpImpl; }

   /**
    *  Determines if the PointCloudAccessor references a valid PointCloudAccessorImpl.
    *
    *  @return  Returns true if the PointCloudAccessorImpl is valid.
    */
   bool isValid() const;

private:
   /**
    *  Increases the number of references by one.
    */
   void incrementRefCount();

   /**
    *  Decreases the number of references by one.
    */
   void decrementRefCount();

   PointCloudAccessorDeleter* mpDeleter;
   PointCloudAccessorImpl* mpImpl;
};

#endif
