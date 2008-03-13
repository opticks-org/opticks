/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LIBRARYSIGNATUREIMP_H
#define LIBRARYSIGNATUREIMP_H

#include "SignatureImp.h"

class SignatureLibrary;

class LibrarySignatureImp : public SignatureImp
{
public:
   LibrarySignatureImp(const DataDescriptorImp& descriptor, const std::string& id, unsigned int index, const SignatureLibrary *pLib);
   ~LibrarySignatureImp();

   const DataVariant &getData(std::string name) const;
   void setData(std::string name, const DataVariant &data);
   const Units *getUnits(std::string name) const;

   DataElement* copy(const std::string& name, DataElement* pParent) const;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   static bool isKindOfElement(const std::string& className);
   static void getElementTypes(std::vector<std::string>& classList);

private:
   unsigned int mSignatureIndex;
   const SignatureLibrary *mpLibrary;
   std::string mOrdinateName;
};

#define LIBRARYSIGNATUREADAPTER_METHODS(impClass) \
   SIGNATUREADAPTER_METHODS(impClass)

#endif
