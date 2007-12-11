/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include "SubjectAdapter.h"

using namespace std;

SubjectAdapter::SubjectAdapter()
{
}

SubjectAdapter::~SubjectAdapter()
{
}

bool SubjectAdapter::attach(const std::string &signal, const Slot &slot)
{
   return SubjectImp::attach(signal, slot);
}

bool SubjectAdapter::detach(const std::string &signal, const Slot &slot)
{
   return SubjectImp::detach(signal, slot);
}

const string& SubjectAdapter::getObjectType() const
{
   static string type("SubjectAdapter");
   return type;
}

bool SubjectAdapter::isKindOf(const string& name) const
{
   if ((name == getObjectType()) || (name == "Subject") || (name == "TypeAwareObject"))
   {
      return true;
   }

   return SubjectImp::isKindOf(name);
}
