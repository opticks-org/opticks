/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AnyAdapter.h"

using namespace std;

AnyAdapter::AnyAdapter(const string& dataType, const DataDescriptorImp& descriptor, const string& id) :
   AnyImp(dataType, descriptor, id)
{
}

AnyAdapter::~AnyAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& AnyAdapter::getObjectType() const
{
   return AnyImp::getObjectType();
}

bool AnyAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Any"))
   {
      return true;
   }

   return AnyImp::isKindOf(className);
}
