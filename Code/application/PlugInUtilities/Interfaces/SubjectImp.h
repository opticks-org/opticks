/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef __SUBJECTIMP_H
#define __SUBJECTIMP_H

#include "SafeSlot.h"
#include "TypesFile.h"

#include <boost/any.hpp>
#include <list>
#include <map>
#include <string>
#include <vector>

class SubjectImpPrivate;

/**
 *  Implementation of the interface for generalized registration of
 *  objects intereseted in mutations of the subclasses of this interface.
 */
class SubjectImp
{
   friend class Signal::SignalValue;

public:
   SubjectImp();
   virtual ~SubjectImp();

   virtual bool attach(const std::string &signal, const Slot &slot);
   virtual bool detach(const std::string &signal, const Slot &slot);

   /**
    *  Inherited obligation from TypeAwareObject. Allow observers to
    *  determine the type of mutating object notifying them.
    *
    *  @return  The name of the subclass of this interface.
    */
   virtual const std::string& getObjectType() const;

   /**
    *  Inherited obligation from TypeAwareObject. Allow observers to
    *  determine the type of mutating object notifying them.
    *
    *  @param   className
    *           Name of the class which is being inquired about.
    *
    *  @return  True if the subclass matches of this interface matches or
    *           is a parent of the class identified in the argument.
    */
   virtual bool isKindOf(const std::string& className) const;

   /**
    *  Gets the list of Slots attached to the specified signal.
    *
    *  @param   signal
    *           The signal to get the slots for.
    *
    *  @return  A (possibly empty) list containing slots attached to the 
    *           specified signal.
    */
   const std::list<SafeSlot>& getSlots(const std::string &signal);

protected:
   void notify(const std::string &signal, const boost::any &data=boost::any());

private:
   SubjectImpPrivate *mpImpPrivate;
};

#define SUBJECTADAPTEREXTENSION_CLASSES

#define SUBJECTADAPTER_METHODS(impClass) \
   bool attach(const std::string &signal, const Slot &slot) \
   { \
      return impClass::attach(signal, slot); \
   } \
   bool detach(const std::string &signal, const Slot &slot) \
   { \
      return impClass::detach(signal, slot); \
   }

#endif   // __SUBJECTIMP_H
