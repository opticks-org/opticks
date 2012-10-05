/*
 * The information in this file is
 * Copyright(c) 2012 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "GeoreferenceDescriptorAdapter.h"

GeoreferenceDescriptorAdapter::GeoreferenceDescriptorAdapter()
{}

GeoreferenceDescriptorAdapter::~GeoreferenceDescriptorAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

const std::string& GeoreferenceDescriptorAdapter::getObjectType() const
{
   static std::string sType("GeoreferenceDescriptorAdapter");
   return sType;
}

bool GeoreferenceDescriptorAdapter::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "GeoreferenceDescriptor"))
   {
      return true;
   }

   return GeoreferenceDescriptorImp::isKindOf(className);
}
