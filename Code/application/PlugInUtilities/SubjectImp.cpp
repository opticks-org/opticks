/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include <boost/any.hpp>


#include "Slot.h"
#include "SubjectImp.h"
#include "SubjectImpPrivate.h"
#include "SubjectAdapter.h"

using namespace std;

SubjectImp::SubjectImp() : 
   mpImpPrivate(new SubjectImpPrivate)
{
}

SubjectImp::~SubjectImp()
{
   delete mpImpPrivate;
}

bool SubjectImp::attach(const std::string &signal, const Slot &slot)
{
   Subject* pSubject = dynamic_cast<Subject*>(this);
   if (pSubject == NULL)
   {
      return false;
   }

   return mpImpPrivate->attach(*pSubject, signal, slot);
}

bool SubjectImp::detach(const std::string &signal, const Slot &slot)
{
   Subject* pSubject = dynamic_cast<Subject*>(this);
   if (pSubject == NULL)
   {
      return false;
   }

   return mpImpPrivate->detach(*pSubject, signal, slot);
}

void SubjectImp::notify(const string &signal, const boost::any &data)
{
   Subject* pSubject = dynamic_cast<Subject*>(this);
   if (pSubject == NULL)
   {
      return;
   }

   mpImpPrivate->notify(*pSubject, signal, signal, data);
}

const string& SubjectImp::getObjectType() const
{
   static string type("SubjectImp");
   return type;
}

bool SubjectImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Subject"))
   {
      return true;
   }

   return false;
}

void Signal::SignalValue::update(Subject &subject, const std::string &signal, const boost::any &data) const
{
   SubjectImp* pSubjectImp = dynamic_cast<SubjectImp*>(&mSubject);
   if (NN(pSubjectImp))
   {
      if (mSignalName.empty())
      {
         pSubjectImp->notify(signal.c_str(), data);
      }
      else
      {
         pSubjectImp->notify(mSignalName.c_str(), data);
      }
   }
}

const list<SafeSlot>& SubjectImp::getSlots(const string &signal)
{
   static list<SafeSlot> emptyList;
   if (signal.empty())
   {
      return emptyList;
   }

   Subject* pSubject = dynamic_cast<Subject*>(this);
   if (pSubject == NULL)
   {
      return emptyList;
   }

   return mpImpPrivate->getSlots(signal);
}
