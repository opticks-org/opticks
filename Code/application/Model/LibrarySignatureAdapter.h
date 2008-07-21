/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LIBRARYSIGNATUREADAPTER_H
#define LIBRARYSIGNATUREADAPTER_H

#include "Signature.h"
#include "LibrarySignatureImp.h"

class SignatureLibrary;

class LibrarySignatureAdapter : public Signature, public LibrarySignatureImp LIBRARYSIGNATUREADAPTEREXTENSION_CLASSES
{
public:
   LibrarySignatureAdapter(const DataDescriptorImp& descriptor, const std::string& id, unsigned int index, const SignatureLibrary *pLib);
   ~LibrarySignatureAdapter();

   // TypeAwareObject
   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   LIBRARYSIGNATUREADAPTER_METHODS(LibrarySignatureImp)
};

#endif
