/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SignatureSetAdapter.h"

using namespace std;

SignatureSetAdapter::SignatureSetAdapter(const DataDescriptorImp& descriptor, const string& id) :
   SignatureSetImp(descriptor, id)
{
}

SignatureSetAdapter::~SignatureSetAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& SignatureSetAdapter::getObjectType() const
{
   static string type("SignatureSetAdapter");
   return type;
}

bool SignatureSetAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "SignatureSet"))
   {
      return true;
   }

   return SignatureSetImp::isKindOf(className);
}
