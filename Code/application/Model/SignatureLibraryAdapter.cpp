/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SignatureLibraryAdapter.h"

using namespace std;

SignatureLibraryAdapter::SignatureLibraryAdapter(const DataDescriptorImp& descriptor, const string& id) :
   SignatureLibraryImp(descriptor, id)
{
}

SignatureLibraryAdapter::~SignatureLibraryAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& SignatureLibraryAdapter::getObjectType() const
{
   static string sType("SignatureLibraryAdapter");
   return sType;
}

bool SignatureLibraryAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "SignatureLibrary"))
   {
      return true;
   }

   return SignatureLibraryImp::isKindOf(className);
}
