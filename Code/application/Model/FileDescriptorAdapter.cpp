/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "FileDescriptorAdapter.h"

using namespace std;

FileDescriptorAdapter::FileDescriptorAdapter()
{}

FileDescriptorAdapter::~FileDescriptorAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& FileDescriptorAdapter::getObjectType() const
{
   static string sType("FileDescriptorAdapter");
   return sType;
}

bool FileDescriptorAdapter::isKindOf(const string& className) const
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
   FileDescriptorAdapter* pDescriptor = new FileDescriptorAdapter();
   if (pDescriptor != NULL)
   {
      *pDescriptor = *this;
   }

   return pDescriptor;
}
