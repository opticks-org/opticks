/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIGNATURELIBRARYADAPTER_H
#define SIGNATURELIBRARYADAPTER_H

#include "SignatureLibrary.h"
#include "SignatureLibraryImp.h"

class SignatureLibraryAdapter : public SignatureLibrary, public SignatureLibraryImp
   SIGNATURELIBRARYADAPTEREXTENSION_CLASSES
{
public:
   SignatureLibraryAdapter(const DataDescriptorImp& descriptor, const std::string& id);
   ~SignatureLibraryAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SIGNATURELIBRARYADAPTER_METHODS(SignatureLibraryImp)

private:
   SignatureLibraryAdapter(const SignatureLibraryAdapter& rhs);
   SignatureLibraryAdapter& operator=(const SignatureLibraryAdapter& rhs);
};

#endif
