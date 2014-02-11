/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PointCloudElementAdapter.h"

using namespace std;

PointCloudElementAdapter::PointCloudElementAdapter(const DataDescriptorImp& descriptor, const string& id) :
   PointCloudElementImp(descriptor, id)
{
}

PointCloudElementAdapter::~PointCloudElementAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& PointCloudElementAdapter::getObjectType() const
{
   static string sType("PointCloudElementAdapter");
   return sType;
}

bool PointCloudElementAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PointCloudElement"))
   {
      return true;
   }

   return PointCloudElementImp::isKindOf(className);
}
