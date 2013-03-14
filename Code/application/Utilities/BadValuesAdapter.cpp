/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "BadValuesAdapter.h"

BadValuesAdapter::BadValuesAdapter()
{}

BadValuesAdapter::~BadValuesAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const std::string& BadValuesAdapter::getObjectType() const
{
   static std::string sType("BadValuesAdapter");
   return sType;
}

bool BadValuesAdapter::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "BadValuesAdapter"))
   {
      return true;
   }

   return BadValuesImp::isKindOf(className);
}
