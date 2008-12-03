/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataAccessor.h"
#include "DataAccessorImpl.h"

DataAccessor::DataAccessor(DataAccessorDeleter* pDeleter, DataAccessorImpl* pImpl) :
   mpDeleter(pDeleter),
   mpImpl(pImpl)
{
   incrementRefCount();
}

DataAccessor::DataAccessor(const DataAccessor& da) :
   mpDeleter(da.mpDeleter),
   mpImpl(da.mpImpl)
{
   incrementRefCount();
}

DataAccessor::~DataAccessor() 
{
   decrementRefCount();
}

DataAccessor& DataAccessor::operator=(const DataAccessor& rhs)
{
   if (this != &rhs)
   {
      decrementRefCount();
      mpDeleter = rhs.mpDeleter;
      mpImpl = rhs.mpImpl;
      incrementRefCount();
   }

   return *this;
}

bool DataAccessor::isValid() const
{
   return (mpImpl != NULL && mpImpl->isValid());
}

void DataAccessor::incrementRefCount()
{
   if (mpImpl != NULL)
   {
      mpImpl->incrementRefCount();
   }
}

void DataAccessor::decrementRefCount()
{
   if (mpImpl != NULL)
   {
      int newRefCount = mpImpl->decrementRefCount();
      if (mpDeleter != 0 && newRefCount == 0)
      {
         (*mpDeleter)(mpImpl);
         mpImpl = NULL;
         mpDeleter = NULL;
      }
   }
}
