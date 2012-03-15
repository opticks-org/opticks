/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SignatureFileDescriptorAdapter.h"
#include "ObjectResource.h"

SignatureFileDescriptorAdapter::SignatureFileDescriptorAdapter()
{}

SignatureFileDescriptorAdapter::~SignatureFileDescriptorAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const std::string& SignatureFileDescriptorAdapter::getObjectType() const
{
   static std::string sType("SignatureFileDescriptorAdapter");
   return sType;
}

bool SignatureFileDescriptorAdapter::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "SignatureFileDescriptor"))
   {
      return true;
   }

   return SignatureFileDescriptorImp::isKindOf(className);
}

// FileDescriptor
FileDescriptor* SignatureFileDescriptorAdapter::copy() const
{
   FactoryResource<SignatureFileDescriptor> pFileDescriptor;
   if (pFileDescriptor->clone(this) == false)
   {
      return NULL;
   }

   return pFileDescriptor.release();
}
