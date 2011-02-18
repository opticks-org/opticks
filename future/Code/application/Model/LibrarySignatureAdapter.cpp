/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "LibrarySignatureAdapter.h"

using namespace std;

LibrarySignatureAdapter::LibrarySignatureAdapter(const DataDescriptorImp& descriptor, const string& id,
                                                 unsigned int index, const SignatureLibrary* pLib) :
   LibrarySignatureImp(descriptor, id, index, pLib)
{
}

LibrarySignatureAdapter::~LibrarySignatureAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& LibrarySignatureAdapter::getObjectType() const
{
   static string sType("LibrarySignatureAdapter");
   return sType;
}

bool LibrarySignatureAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "LibrarySignature"))
   {
      return true;
   }

   return LibrarySignatureImp::isKindOf(className);
}
