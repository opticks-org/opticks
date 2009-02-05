/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataElementAdapter.h"
#include "DataDescriptor.h"

using namespace std;

DataElementAdapter::DataElementAdapter(const DataDescriptorImp& descriptor, const string& id) :
   DataElementImp(descriptor, id)
{
}

DataElementAdapter::~DataElementAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& DataElementAdapter::getObjectType() const
{
   static string sType("DataElementAdapter");
   return sType;
}

bool DataElementAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "DataElement"))
   {
      return true;
   }

   return DataElementImp::isKindOf(className);
}
