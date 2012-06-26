/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "UnitsAdapter.h"

UnitsAdapter::UnitsAdapter()
{}

UnitsAdapter::~UnitsAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const std::string& UnitsAdapter::getObjectType() const
{
   static std::string sType("UnitsAdapter");
   return sType;
}

bool UnitsAdapter::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "Units"))
   {
      return true;
   }

   return UnitsImp::isKindOf(className);
}
