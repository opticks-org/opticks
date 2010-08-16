/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATA_ACCESSOR_H
#define DATA_ACCESSOR_H

class DataAccessorImpl;

/**
 *  Provides safe deletion of the DataAccessorImpl.
 *
 *  A plug-in developer does not use this class directly.  This class is used 
 *  to safely delete the DataAccessorImpl class.  When the DataAccessor loses scope,
 *  the DataAccessorImpl class is deleted.
 *
 *  @see        DataAccessor, DataAccessorImpl
 */
class DataAccessorDeleter
{
public:
   /**
    * Provides support for deleting the DataAcessorImpl.
    *
    * @param  pImpl
    *         A pointer to the managed DataAcessorImpl instance.
    */
   virtual void operator()(DataAccessorImpl* pImpl) = 0;
};

/**
 *  Provides reference counting for the DataAccessorImpl.
 *
 *  A plug-in developer does not use this class directly.   The %DataAccessor exists
 *  for the sole purpose of managing the lifespan of the DataAccessorImpl.  This
 *  class is used to create a wrapper around the DataAccessorImpl class to provide
 *  safe reference counting.  When the %DataAccessor loses scope the DataAccessorImpl
 *  class is deleted, causing its associated RasterPager to release its held RasterPage.
 *  If the RasterPager has already been deleted (e.g.: by deleting its RasterElement),
 *  the destruction of the DataAccessorImpl will cause undefined behavior. Therefore,
 *  the deletion of a RasterElement must occur after the deletion of its associated %DataAccessors.
 *
 *  @see        DataAccessorDeleter, DataAccessorImpl
 */
class DataAccessor
{
public:
   /**
    *  Creates a %DataAccessor.
    *
    *  This class is not called directly by a plug-in developer.  This creates an
    *  instance of a class that manages the lifespan of the DataAccessorImpl.
    *
    *  @param   pDeleter
    *           A class that manages the deletion of the DataAccessorImpl.
    *  @param   pImpl
    *           The DataAccessorImpl class to manage.
    */
   DataAccessor(DataAccessorDeleter *pDeleter, DataAccessorImpl*pImpl);

   /**
    *  Default copy constructor.
    */
   DataAccessor(const DataAccessor &da);

   /**
    *  Destructor for the DataAccessor.
    */
   ~DataAccessor();

   /**
    *  The equals operator.
    *
    *  @param   rhs
    *           The DataAccessor from which to set this accessor's values.
    */
   DataAccessor &operator=(const DataAccessor &rhs);

   /**
    *  Provides access to the real DataAccessorImpl.
    *
    *  The overloaded operator-> allows us to continue to use pointer notation to 
    *  directly access the internally held pointer to the DataAccessorImpl. 
    *  See also the Boost smart pointers. You will see the same thing.
    *
    *  @return  Returns a pointer to the DataAccessorImpl.
    */
   inline DataAccessorImpl* operator->() { return mpImpl; }

   /**
    *  Determines if the DataAccessor references a valid DataAccessorImpl.
    *
    *  @return  Returns true if the DataAccessorImpl is valid.
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

   DataAccessorDeleter *mpDeleter;
   DataAccessorImpl *mpImpl;
};

#endif
