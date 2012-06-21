/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SignatureDataDescriptorAdapter.h"

SignatureDataDescriptorAdapter::SignatureDataDescriptorAdapter(const std::string& name, const std::string& type,
   DataElement* pParent) :
   SignatureDataDescriptorImp(name, type, pParent)
{}

SignatureDataDescriptorAdapter::SignatureDataDescriptorAdapter(const std::string& name, const std::string& type,
   const std::vector<std::string>& parent) :
   SignatureDataDescriptorImp(name, type, parent)
{}

SignatureDataDescriptorAdapter::~SignatureDataDescriptorAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const std::string& SignatureDataDescriptorAdapter::getObjectType() const
{
   static std::string sType("SignatureDataDescriptorAdapter");
   return sType;
}

bool SignatureDataDescriptorAdapter::isKindOf(const std::string& className) const
{
   if ((className == getObjectType()) || (className == "SignatureDataDescriptor"))
   {
      return true;
   }

   return SignatureDataDescriptorImp::isKindOf(className);
}
