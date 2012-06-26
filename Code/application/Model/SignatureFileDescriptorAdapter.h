/*
 * The information in this file is
 * Copyright(c) 2011 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATUREFILEDESCRIPTORADAPTER_H
#define SIGNATUREFILEDESCRIPTORADAPTER_H

#include "SignatureFileDescriptor.h"
#include "SignatureFileDescriptorImp.h"

class SignatureFileDescriptorAdapter : public SignatureFileDescriptor, public SignatureFileDescriptorImp
   SIGNATUREFILEDESCRIPTORADAPTEREXTENSION_CLASSES
{
public:
   SignatureFileDescriptorAdapter();
   ~SignatureFileDescriptorAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   // FileDescriptor
   FileDescriptor* copy() const;

   SIGNATUREFILEDESCRIPTORADAPTER_METHODS(SignatureFileDescriptorImp)
};

#endif
