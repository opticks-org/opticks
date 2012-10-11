/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATUREDATADESCRIPTORADAPTER_H
#define SIGNATUREDATADESCRIPTORADAPTER_H

#include "SignatureDataDescriptor.h"
#include "SignatureDataDescriptorImp.h"

#include <string>
#include <vector>

class SignatureDataDescriptorAdapter : public SignatureDataDescriptor, public SignatureDataDescriptorImp
   SIGNATUREDATADESCRIPTORADAPTEREXTENSION_CLASSES
{
public:
   SignatureDataDescriptorAdapter(const std::string& name, const std::string& type, DataElement* pParent);
   SignatureDataDescriptorAdapter(const std::string& name, const std::string& type,
      const std::vector<std::string>& parent);
   virtual ~SignatureDataDescriptorAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SIGNATUREDATADESCRIPTORADAPTER_METHODS(SignatureDataDescriptorImp)
};

#endif
