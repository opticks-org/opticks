/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SLOT_H
#define SLOT_H

#include "AppVerify.h"
#include "Observer.h"

#include <string>
#include <sstream>
#include <typeinfo>

/**
 *  \cond INTERNAL
 */
namespace boost
{
   class any;
}
/// \endcond
class Subject;

/**
 *  Class for specifying which method to call in response to a call to notify
 *  by a Subject.
 *
 *  Typically, only the constructors of this class will be used outside of the
 *  underlying implementation of the Subject class. 
 *
 *  When a Subject calls 'notify' with a particular signal name, all slots that
 *  are attached to that signal on the Subject will have their update method
 *  called. The slot's update method will in turn call the method on the 
 *  object provided via the slot's constructor. Slot methods need to have the
 *  following signature:
 *
 *  @code
 *  void T::method(Subject &subject, const std::string &signal, const boost::any &v);
 *  @endcode
 *
 *  When called, the arguments will be as follows:
 *  subject: the Subject notifying with the signal
 *  signal: the name of the signal the Subject notified
 *  v: data provided to the notify method by the Subject
 *
 *  When a %Slot is attached to a signal on a Subject, if the slot method is on
 *  an object derived from Observer, the object's attached method will be 
 *  called. Similarly, when a %Slot is detached from an object derived from
 *  Observer, the object's detached method will be called.
 *
 *  @see    Subject::attach, Subject::detach, Observer::attached, Observer::detached
 */
class Slot
{
   friend class SubjectImpPrivate;
   friend class SafeSlot;

public:
   /**
    *  Creates an empty Slot object.
    *
    *  The default constructor creates an empty %Slot object. The object will
    *  return false from isValid and will test equal to other empty %Slots.
    */
   Slot() : mpSlot(NULL)
   {
   }

   /**
    *  Creates a slot.
    *
    *  Creates a slot. Assuming neither of the input arguments is NULL, the 
    *  %Slot will return true from isValid and will test equal to other %Slots
    *  created with the same arguments.
    *
    *  @param   pT
    *           The object to call in Slot::update.
    *
    *  @param   pMethod
    *           The method to call on the object in Slot::update. The method 
    *           should not be virtual. Virtual slot methods may cause 
    *           Subject::detach to fail. If a virtual slot is needed, make the
    *           slot method non-virtual and have it call a virtual method. On
    *           notify, the slot method will be called as follows:
    *           @code
    *           (pT->*pMethod)(subject, signal, v);
    *           @endcode
    *           pMethod needs to have the following signature:
    *           @code
    *           T::method(Subject &subject, const std::string &signal, const boost::any &v);
    *           @endcode
    */
   template<class T, typename Method>
   Slot(T *pT, Method pMethod) : mpSlot(NULL)
   {
      if(VERIFYNR(pT != NULL && pMethod != NULL))
      {
         mpSlot = new SlotValue<T>(pT, pMethod);
      }
   }

   /**
    *  Creates a copy of the input slot.
    *
    *  @param   slot
    *           The %Slot to copy from.
    */
   Slot(const Slot& slot) :
      mpSlot(NULL)
   {
      if (slot.isValid() == true)
      {
         mpSlot = slot.mpSlot->clone();
      }
   }

   /**
    *  Copies the state of one slot into another slot.
    *
    *  @param   rhs
    *           The %Slot to copy from: on the right-hand-side of the =.
    *
    *  @return   a reference to the slot assigned to.
    */
   Slot& operator=(const Slot& rhs)
   {
      SlotWrapper *pSlot = NULL;
      if (rhs.isValid())
      {
         pSlot = rhs.mpSlot->clone();
      }
      if (isValid())
      {
         delete mpSlot;
      }
      mpSlot = pSlot;
      return *this;
   }

   /**
    *  Indicates if the %Slot is a valid %Slot.
    *
    *  @return  false if the %Slot was created via the default constructor or
    *             copied from an invalid %Slot, or true otherwise.
    */
   bool isValid() const
   {
      return mpSlot != NULL;
   }

   /**
    *  Calls the method on the Slot.
    *
    *  @param   subject
    *            The Subject that called notify.
    *
    *  @param   signal
    *            The name of the signal the Subject is notifying.
    *
    *  @param   data
    *            The ancillary data the Subject provided to its notification.
    */
   void update(Subject &subject, const std::string &signal, const boost::any &data) const
   {
      if (isValid())
      {
         mpSlot->update(subject, signal, data);
      }
   }

   /**
    *  Calls the attached method on the observer, if it derives from Observer.
    *
    *  This method is typically only called by Subject::attach.
    *
    *  @param   subject
    *            The Subject that the slot is being attached to.
    *
    *  @param   signal
    *            The name of the signal the %Slot is being attached to.
    */
   void callAttachMethod(Subject &subject, const std::string &signal) const
   {
      if (isValid())
      {
         mpSlot->callAttachMethod(subject, signal, *this);
      }
   }

   /**
    *  Calls the detached method on the observer, if it derives from Observer.
    *
    *  This method is typically only called by Subject::detach.
    *
    *  @param   subject
    *            The Subject that the slot is being detached from.
    *
    *  @param   signal
    *            The name of the signal the %Slot is being detached from.
    */
   void callDetachMethod(Subject &subject, const std::string &signal) const
   {
      if (isValid())
      {
         mpSlot->callDetachMethod(subject, signal, *this);
      }
   }

   /**
    *  Indicates if two %Slots are equivalent.
    *
    *  @param   rhs
    *            The slot on the right-hand-side of the comparison.
    *
    *  @return  true if the two Slots are both invalid or if they were
    *            constructed with the same arguments, or false otherwise.
    */
   bool operator==(const Slot& rhs) const
   {
      if (!isValid())
      {
         return (!rhs.isValid());
      }
      else if (!rhs.isValid())
      {
         return false;
      }
      else // both are valid
      {
         return mpSlot->matches(*rhs.mpSlot);
      }
   }

   /**
    *  Gets the typeid of the object the %Slot was constructed with.
    *
    *  @return  the typeid of the object the %Slot was constructed with or
    *             typeid(void) if the %Slot is invalid.
    */
   const std::type_info &getType() const
   {
      if (isValid())
      {
         return mpSlot->getType();
      }

      return typeid(void);
   }

   /**
    *  Destroys the %Slot.
    */
   ~Slot()
   {
      if (mpSlot)
      {
         delete mpSlot;
         mpSlot = NULL;
      }
   }

protected:
   /**
    *  This class provides an abstract interface that implements the details of
    *  the slot and hides the type information behind a generic interface.
    */
   class SlotWrapper
   {
   public:
      /**
       *  Calls the method on the Slot.
       *
       *  @param   subject
       *            The Subject that called notify.
       *
       *  @param   signal
       *            The name of the signal the Subject is notifying.
       *
       *  @param   data
       *            The ancillary data the Subject provided to its notification.
       */
      virtual void update(Subject &subject, const std::string &signal, const boost::any &data) const = 0;

      /**
       *  Creates a copy of the object.
       *
       *  @return  A copy of the object.
       */
      virtual SlotWrapper* clone() const = 0;

      /**
       *  Indicates if the supplied object matches this object.
       *
       *  @param rhs
       *             The object to compare with.
       *
       *  @return \c true if the two objects match and false otherwise.
       */
      virtual bool matches(const SlotWrapper& rhs) const = 0;

      /**
       *  Returns the type information of the concrete type of the object.
       *
       *  @return  The type information of the concrete object.
       */
      virtual const std::type_info &getType() const = 0;

      /**
       *  Calls the attached method on the observer, if it derives from Observer.
       *
       *  This method is typically only called by Subject::attach.
       *
       *  @param   subject
       *            The Subject that the slot is being attached to.
       *
       *  @param   signal
       *            The name of the signal the %Slot is being attached to.
       *
       *  @param   slot
       *            The slot that is being attached.
       */
      virtual void callAttachMethod(Subject &subject, const std::string &signal, const Slot &slot) = 0;

      /**
       *  Calls the detached method on the observer, if it derives from Observer.
       *
       *  This method is typically only called by Subject::detach.
       *
       *  @param   subject
       *            The Subject that the slot is being detached from.
       *
       *  @param   signal
       *            The name of the signal the %Slot is being detached from.
       *
       *  @param   slot
       *            The slot that is being detached.
       */
      virtual void callDetachMethod(Subject &subject, const std::string &signal, const Slot &slot) = 0;
   };

   SlotWrapper *mpSlot;

private:
   template<class T>
   class SlotValue : public SlotWrapper
   {
   public:
      SlotValue(T *pT, void (T::*pMethod)(Subject&,const std::string&,const boost::any&)) : mpTarget(pT), mpMethod(pMethod)
      {
      }

      virtual ~SlotValue() {};

      virtual void update(Subject& subject, const std::string& signal, const boost::any& data) const
      {
         (mpTarget->*mpMethod)(subject, signal.c_str(), data);
      }

      virtual void callAttachMethod(Subject& subject, const std::string& signal, const Slot& slot)
      {
         Observer *pObs = dynamic_cast<Observer*>(mpTarget);
         if (pObs != NULL)
         {
            pObs->attached(subject, signal, slot);
         }
      }

      virtual void callDetachMethod(Subject& subject, const std::string& signal, const Slot& slot)
      {
         Observer *pObs = dynamic_cast<Observer*>(mpTarget);
         if (pObs != NULL)
         {
            pObs->detached(subject, signal, slot);
         }
      }

      virtual SlotWrapper* clone() const
      {
         return new SlotValue<T>(mpTarget, mpMethod);
      }

      virtual const std::type_info& getType() const
      {
         return typeid(SlotValue<T>);
      }

      virtual bool matches(const SlotWrapper& rhs) const
      {
         if (getType() == rhs.getType())
         {
            const SlotValue<T>& rhsValue = static_cast<const SlotValue<T>&>(rhs);
            return (mpTarget == rhsValue.mpTarget && mpMethod == rhsValue.mpMethod);
         }
         else
         {
            return false;
         }
      }

   private:
      T *mpTarget;
      void (T::*mpMethod)(Subject&,const std::string&,const boost::any&);
   };
};

/**
 *  Class for specifying a signal to re-emit.
 *
 *  Only the constructors of this class will be used outside of the
 *  underlying implementation if the Subject class. 
 *
 *  When a Subject calls 'notify' with a particular signal name, all Signals 
 *  that are attached to that signal on the Subject will have their update method
 *  called. The Signal's update method will in turn call notify on it's Subject
 *  using the Signal's signal name.
 *
 *  For example:
 *  @code
 *  pSubject->attach(SIGNAL_NAME(Subject, Modified), Signal(pObj, SIGNAL_NAME(Subject, Modified)));
 *  @endcode
 *  This causes pSubject->notify(SIGNAL_NAME(Subject, Modified, any)); to result in
 *  pObj->notify(SIGNAL_NAME(Subject, Modified), v) being called. Remember that NULL
 *  is a wildcard for signal names. Using NULL as the first argument to attach
 *  in the case above will cause all signals to be forwarded. Using NULL for 
 *  second argument to the Signal ctor will cause the name of the original
 *  signal to be re-used when the signal is re-emited. E.g.
 *  @code
 *  pSubject->attach(NULL, Signal(pObj, NULL));
 *  pSubject->notify("MySignal", any(myData));
 *  @endcode
 *  This causes pObj->notify("MySignal", any(myData)) to be called.
 *
 *  @see    Subject::attach, Subject::detach
 */
class Signal : public Slot
{
   friend class SubjectImp;

public:
   /**
    *  Creates a %Signal.
    *
    *  @param   pSubject
    *           The Subject to re-emit the %Signal from.
    *
    *  @param   signalName
    *           The name to re-emit the signal as. If NULL, the signal will be
    *           re-emited with its original name.
    */
   Signal(Subject* pSubject, const std::string& signalName) : Slot()
   {
      if (pSubject != NULL)
      {
         mpSlot = new SignalValue(*pSubject, signalName);
      }
   }

private:
   class SignalValue : public SlotWrapper
   {
   public:
      SignalValue(Subject& subject, const std::string& signalName) :
         mSubject(subject),
         mSignalName(signalName)
      {
      }

      virtual ~SignalValue() {};

      virtual void update(Subject& subject, const std::string& signal, const boost::any& data) const;

      virtual SlotWrapper *clone() const
      {
         return new SignalValue(mSubject, mSignalName);
      }

      virtual bool matches(const SlotWrapper& rhs) const
      {
         if (getType() == rhs.getType())
         {
            const SignalValue& rhsValue = static_cast<const SignalValue&>(rhs);
            return (&mSubject == &rhsValue.mSubject && mSignalName == rhsValue.mSignalName);
         }
         else
         {
            return false;
         }
      }

      virtual const std::type_info& getType() const
      {
         return typeid(SignalValue);
      }

      virtual void callAttachMethod(Subject& subject, const std::string& signal, const Slot& slot)
      {
      }

      virtual void callDetachMethod(Subject& subject, const std::string& signal, const Slot& slot)
      {
      }

   private:
      SignalValue& operator=(const SignalValue& rhs);

      Subject& mSubject;
      std::string mSignalName;
   };
};

#endif
