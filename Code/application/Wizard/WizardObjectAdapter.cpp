/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "WizardObjectAdapter.h"

using namespace std;

WizardObjectAdapter::WizardObjectAdapter()
{
}

WizardObjectAdapter::~WizardObjectAdapter()
{
   notify(SIGNAL_NAME(Subject, Deleted));
}

// TypeAwareObject
const string& WizardObjectAdapter::getObjectType() const
{
   static string type("WizardObjectAdapter");
   return type;
}

bool WizardObjectAdapter::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "WizardObject"))
   {
      return true;
   }

   return WizardObjectImp::isKindOf(className);
}
