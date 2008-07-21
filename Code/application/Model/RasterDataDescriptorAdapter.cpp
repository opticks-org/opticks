/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "RasterDataDescriptorAdapter.h"

using namespace std;

RasterDataDescriptorAdapter::RasterDataDescriptorAdapter(const string& name, const string& type,
   DataElement* pParent) :
   RasterDataDescriptorImp(name, type, pParent)
{
}

RasterDataDescriptorAdapter::RasterDataDescriptorAdapter(const string& name, const string& type,
   const vector<string>& parent) :
   RasterDataDescriptorImp(name, type, parent)
{
}

RasterDataDescriptorAdapter::~RasterDataDescriptorAdapter()
{
}

// TypeAwareObject
const string& RasterDataDescriptorAdapter::getObjectType() const
{
   static string type("RasterDataDescriptorAdapter");
   return type;
}

bool RasterDataDescriptorAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "RasterDataDescriptor"))
   {
      return true;
   }

   return RasterDataDescriptorImp::isKindOf(className);
}
