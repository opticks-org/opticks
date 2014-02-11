/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PointCloudAccessor.h"
#include "PointCloudAccessorImpl.h"

PointCloudAccessor::PointCloudAccessor(PointCloudAccessorDeleter* pDeleter, PointCloudAccessorImpl* pImpl) :
   mpDeleter(pDeleter),
   mpImpl(pImpl)
{
   incrementRefCount();
}

PointCloudAccessor::PointCloudAccessor(const PointCloudAccessor& da) :
   mpDeleter(da.mpDeleter),
   mpImpl(da.mpImpl)
{
   incrementRefCount();
}

PointCloudAccessor::~PointCloudAccessor() 
{
   decrementRefCount();
}

PointCloudAccessor& PointCloudAccessor::operator=(const PointCloudAccessor& rhs)
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

bool PointCloudAccessor::isValid() const
{
   return (mpImpl != NULL && mpImpl->isValid());
}

void PointCloudAccessor::incrementRefCount()
{
   if (mpImpl != NULL)
   {
      mpImpl->incrementRefCount();
   }
}

void PointCloudAccessor::decrementRefCount()
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
