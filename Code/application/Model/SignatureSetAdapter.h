/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATURESETADAPTER_H
#define SIGNATURESETADAPTER_H

#include "SignatureSet.h"
#include "SignatureSetImp.h"

class SignatureSetAdapter : public SignatureSet, public SignatureSetImp
{
public:
   SignatureSetAdapter(const DataDescriptorImp& descriptor, const std::string& id);
   ~SignatureSetAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SIGNATURESETADAPTER_METHODS(SignatureSetImp)
};

#endif
