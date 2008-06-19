/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataElementGroupAdapter.h"

using namespace std;

DataElementGroupAdapter::DataElementGroupAdapter(const DataDescriptorImp& descriptor, const string& id) :
   DataElementGroupImp(descriptor, id)
{
}

DataElementGroupAdapter::~DataElementGroupAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& DataElementGroupAdapter::getObjectType() const
{
   static string type("DataElementGroupAdapter");
   return type;
}

bool DataElementGroupAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "DataElementGroup"))
   {
      return true;
   }

   return DataElementGroupImp::isKindOf(className);
}
