/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "RasterElementAdapter.h"

using namespace std;

RasterElementAdapter::RasterElementAdapter(const DataDescriptorImp& descriptor, const string& id) :
   RasterElementImp(descriptor, id)
{
}

RasterElementAdapter::~RasterElementAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& RasterElementAdapter::getObjectType() const
{
   static string type("RasterElementAdapter");
   return type;
}

bool RasterElementAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "RasterElement"))
   {
      return true;
   }

   return RasterElementImp::isKindOf(className);
}
