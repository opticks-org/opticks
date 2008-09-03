/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SignatureAdapter.h"

using namespace std;

SignatureAdapter::SignatureAdapter(const DataDescriptorImp& descriptor, const string& id) :
   SignatureImp(descriptor, id)
{
}

SignatureAdapter::~SignatureAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& SignatureAdapter::getObjectType() const
{
   static string type("SignatureAdapter");
   return type;
}

bool SignatureAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Signature"))
   {
      return true;
   }

   return SignatureImp::isKindOf(className);
}
