/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "RasterFileDescriptorAdapter.h"
#include "ObjectResource.h"

RasterFileDescriptorAdapter::RasterFileDescriptorAdapter()
{}

RasterFileDescriptorAdapter::~RasterFileDescriptorAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const std::string& RasterFileDescriptorAdapter::getObjectType() const
{
   static std::string sType("RasterFileDescriptorAdapter");
   return sType;
}

bool RasterFileDescriptorAdapter::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "RasterFileDescriptor"))
   {
      return true;
   }

   return RasterFileDescriptorImp::isKindOf(className);
}

// FileDescriptor
FileDescriptor* RasterFileDescriptorAdapter::copy() const
{
   FactoryResource<RasterFileDescriptor> pFileDescriptor;
   if (pFileDescriptor->clone(this) == false)
   {
      return NULL;
   }

   return pFileDescriptor.release();
}
