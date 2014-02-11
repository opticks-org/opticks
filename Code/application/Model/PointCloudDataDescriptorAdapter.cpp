/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "PointCloudDataDescriptorAdapter.h"

using namespace std;

PointCloudDataDescriptorAdapter::PointCloudDataDescriptorAdapter(const string& name, const string& type,
   DataElement* pParent) :
   PointCloudDataDescriptorImp(name, type, pParent)
{}

PointCloudDataDescriptorAdapter::PointCloudDataDescriptorAdapter(const string& name, const string& type,
   const vector<string>& parent) :
   PointCloudDataDescriptorImp(name, type, parent)
{}

PointCloudDataDescriptorAdapter::~PointCloudDataDescriptorAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& PointCloudDataDescriptorAdapter::getObjectType() const
{
   static string sType("PointCloudDataDescriptorAdapter");
   return sType;
}

bool PointCloudDataDescriptorAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "PointCloudDataDescriptor"))
   {
      return true;
   }

   return PointCloudDataDescriptorImp::isKindOf(className);
}
