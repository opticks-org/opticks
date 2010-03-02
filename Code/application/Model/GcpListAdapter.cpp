/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

/**
  * The documentation for this class is in GcpList.h
  */
#include "GcpListAdapter.h"

using namespace std;

GcpListAdapter::GcpListAdapter(const DataDescriptorImp& descriptor, const string& id) :
   GcpListImp(descriptor, id)
{
}

GcpListAdapter::~GcpListAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& GcpListAdapter::getObjectType() const
{
   static string sType("GcpListAdapter");
   return sType;
}

bool GcpListAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "GcpList"))
   {
      return true;
   }

   return GcpListImp::isKindOf(className);
}
