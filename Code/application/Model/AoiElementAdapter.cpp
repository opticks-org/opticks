/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AoiElementAdapter.h"

#include <string>
using namespace std;

AoiElementAdapter::AoiElementAdapter(const DataDescriptorImp& descriptor, const string& id) :
   AoiElementImp(descriptor, id)
{
}

AoiElementAdapter::~AoiElementAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& AoiElementAdapter::getObjectType() const
{
   static string sType("AoiElementAdapter");
   return sType;
}

bool AoiElementAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "AoiElement"))
   {
      return true;
   }

   return AoiElementImp::isKindOf(className);
}
