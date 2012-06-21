/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "FileDescriptorAdapter.h"
#include "ObjectResource.h"

FileDescriptorAdapter::FileDescriptorAdapter()
{}

FileDescriptorAdapter::~FileDescriptorAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const std::string& FileDescriptorAdapter::getObjectType() const
{
   static std::string sType("FileDescriptorAdapter");
   return sType;
}

bool FileDescriptorAdapter::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "FileDescriptor"))
   {
      return true;
   }

   return FileDescriptorImp::isKindOf(className);
}

// FileDescriptor
FileDescriptor* FileDescriptorAdapter::copy() const
{
   FactoryResource<FileDescriptor> pFileDescriptor;
   if (pFileDescriptor->clone(this) == false)
   {
      return NULL;
   }

   return pFileDescriptor.release();
}
