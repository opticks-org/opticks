/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PointCloudFileDescriptorAdapter.h"

using namespace std;

PointCloudFileDescriptorAdapter::PointCloudFileDescriptorAdapter()
{}

PointCloudFileDescriptorAdapter::~PointCloudFileDescriptorAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& PointCloudFileDescriptorAdapter::getObjectType() const
{
   static string sType("PointCloudFileDescriptorAdapter");
   return sType;
}

bool PointCloudFileDescriptorAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PointCloudFileDescriptor"))
   {
      return true;
   }

   return PointCloudFileDescriptorImp::isKindOf(className);
}

// FileDescriptor
FileDescriptor* PointCloudFileDescriptorAdapter::copy() const
{
   PointCloudFileDescriptorAdapter* pDescriptor = new PointCloudFileDescriptorAdapter();
   if (pDescriptor != NULL)
   {
      *pDescriptor = *this;
   }

   return pDescriptor;
}
