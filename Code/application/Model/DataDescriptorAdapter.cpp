/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataDescriptorAdapter.h"

using namespace std;

DataDescriptorAdapter::DataDescriptorAdapter(const string& name, const string& type, DataElement* pParent) :
   DataDescriptorImp(name, type, pParent)
{
}

/*
DataDescriptorAdapter::DataDescriptorAdapter(const string& name, const string& type, const vector<string>& parent) :
   DataDescriptorImp(name, type, parent)
{
}
*/

DataDescriptorAdapter::~DataDescriptorAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& DataDescriptorAdapter::getObjectType() const
{
   static string sType("DataDescriptorAdapter");
   return sType;
}

bool DataDescriptorAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "DataDescriptor"))
   {
      return true;
   }

   return DataDescriptorImp::isKindOf(className);
}
