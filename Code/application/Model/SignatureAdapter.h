/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATUREADAPTER_H
#define SIGNATUREADAPTER_H

#include "Signature.h"
#include "SignatureImp.h"

class SignatureAdapter : public Signature, public SignatureImp SIGNATUREADAPTEREXTENSION_CLASSES
{
public:
   SignatureAdapter(const DataDescriptorImp& descriptor, const std::string& id);
   ~SignatureAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SIGNATUREADAPTER_METHODS(SignatureImp)
};

#endif
