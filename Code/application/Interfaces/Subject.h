/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef __SUBJECT_H
#define __SUBJECT_H

#include "TypeAwareObject.h"

#include <string>

class SafeSlot;
class Slot;

/**
 *  This macro simplifies and standardizes creation of signal methods. This
 *  macro will create a static method to use when attaching, detaching
 *  and notifying. The full body of the method is created. For example:
 *  @code
 *  SIGNAL_METHOD(MyClass, MySignal)
 *  @endcode
 *  will create the following method:
 *  @code
 *  static const std::string &signalMySignal() { ... }
 *  @endcode
 *
 *  @param type
 *             The class that the signal is emitted by
 *  @param name
 *             The unique name for the signal.
 */
#define SIGNAL_METHOD(type,name) \
   static const std::string &signal##name() \
   { \
      static std::string signalName(#type "::" #name);  \
      return signalName;\
   }

/**
 *  This macro simplifies specifying a particular signal. For example:
 *  @code
 *  notify(SIGNAL_NAME(MyClass, MySignal));
 *  @endcode
 *  notifies with the signal created with
 *  @code
 *  SIGNAL_METHOD(MyClass, MySignal)
 *  @endcode
 *
 *  @param type
 *             The class that the signal is emitted by
 *  @param name
 *             The unique name for the signal.
 */
#define SIGNAL_NAME(type,name) type::signal##name()

/**
 *  Base class for objects to send notification when their data changes
 *
 *  Generalized interface for objects that can notify observers
 *  when changes have been made. It provides a uniform means for
 *  letting other objects register an interest in the contents 
 *  of this object's contents.
 *
 *  Each %Subject has signals that it will emit under certain circumstances.
 *  When a signal is emitted, all slots attached to that signal on the %Subject
 *  will be called. For example:
 *  @code
 *  pSubject->attach(SIGNAL_NAME(Subject, Modified), Slot(pObj, &Obj::mySlot));
 *  @endcode
 *  This will cause pObj->mySlot(*pSubject, "Subject::Modified", v) to be called
 *  whenever pSubject emits SIGNAL_NAME(Subject, Modified).
 *
 *  Whenever a signal other than Subject::Modified or Subject::Deleted is
 *  emitted, Subject::Modified will also be emitted.
 *  This means that all slots attached to Subject::signalModified on a 
 *  %Subject of type MyClass will be called when that MyClass object 
 *  emits SIGNAL_NAME(MyClass, MySignal).
 *
 *  The notification of slots is done in two passes.
 *  First, all slots attached to the specific signal are notified in FIFO
 *  order (First In == first attached, and First Out == first notified).
 *  Then, assuming the specific signal is not Subject::Deleted or
 *  Subject::Modified, all slots attached to Subject::Modified
 *  are notified in FIFO order.

 *  @see    TypeAwareObject, Slot, SafeSlot, AutoSlot, Signal
 */
class Subject : public TypeAwareObject
{
public:
   /**
    *  Emitted when a %Subject is deleted.
    */
   SIGNAL_METHOD(Subject, Deleted)
   /**
    *  Emitted when a %Subject is modified. Anytime a signal other than deleted
    *  or modified is notified, signalModified will be notified as well.
    */
   SIGNAL_METHOD(Subject, Modified)

   /**
    *  Allow another object to register an interest in specific changes on 
    *  this %Subject.
    *
    *  @param   signal
    *           The name of the signal to attach to. If signal is empty, the 
    *           Slot will be called on all signals from this %Subject.
    *
    *  @param   slot
    *           An object that contains the object and method to call when the
    *           specified signal is notified. This can be a Slot, SafeSlot or
    *           AutoSlot.
    *
    *  @return   true if the Slot was valid and was not already attached to
    *           the signal on the subject and false otherwise.
    *
    *  @notify  This method will call Observer::attached on the object in
    *           the slot, if the object inherits Observer.
    */
    virtual bool attach(const std::string& signal, const Slot &slot) = 0;

   /**
    *  Allow a Slot to be deregistered from a signal on this %Subject.
    *
    *  @param   signal
    *           The name of the signal the Slot is attached to. If signal is
    *           empty, the specified Slot will be detached from all signals on
    *           this %Subject.
    *
    *  @param   slot
    *           The Slot to detach from the specified signal. If the Slot is 
    *           empty, all slots will be detached from the specified signal.
    *           If the slot was originally attached as a SafeSlot or AutoSlot, 
    *           it needs to be detached as same type.
    *
    *  @return   true if the Slot was attached to the specified signal on the
    *          subject.
    *
    *  @notify  This method will call Observer::detached on the object in
    *           the slot, if the object inherits Observer.
    */
   virtual bool detach(const std::string& signal, const Slot &slot) = 0;

protected:
   /**
    * This should not be deleted directly.  It should be deleted according to
    * the instructions provided for the relevant subclass.
    */
   virtual ~Subject() {}
};

#endif
