/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SUBJECT_IMP_PRIVATE_H
#define SUBJECT_IMP_PRIVATE_H

#include "TypesFile.h"

#include <boost/any.hpp>
#include <list>
#include <map>
#include <string>
#include <vector>

class Slot;
class Subject;

class SubjectImpPrivate
{
   typedef std::map<std::string,std::list<Slot> > MapType;

public:
   SubjectImpPrivate();
   virtual ~SubjectImpPrivate();
   void detachWaitingSlots(Subject& subject, const std::string &signal);
   virtual bool attach(Subject &subject, const std::string &signal, const Slot &slot);
   virtual bool detach(Subject &subject, const std::string &signal, const Slot &slot);
   void notify(Subject &subject, const std::string &signal, const boost::any &data=boost::any());
   const std::list<Slot>& getSlots(const std::string & signal);

private:
   MapType mSlots;
   std::list<Slot> *mpSlotsWaitingForDetach;
   Slot *mpActiveSlot;
   std::string mActiveSignal;
};

#endif
