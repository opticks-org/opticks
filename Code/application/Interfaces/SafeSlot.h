/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SAFE_SLOT_H
#define SAFE_SLOT_H

#include "AppVerify.h"
#include "Observer.h"
#include "Slot.h"

#include <memory>
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

class SlotInvalidator;

/**
 *  Subclass of Slot that allows the specification of an object whose destructor
 *  will cause the Slot to be automatically detached. This addresses the problem
 *  of stale pointers being left behind when an observer is destroyed while
 *  attached to a Subject. 
 *
 *  The SafeSlot constructor takes a third argument that is an 'invalidator.' 
 *  If this argument is \c NULL, the SafeSlot behaves exactly like a normal 
 *  Slot. If it is not \c NULL, the destructor of the invalidator object will 
 *  cause the SafeSlot to be invalidated. An invalid slot's update method will
 *  never be called, and it will be automatically removed from the Subject's
 *  list of slots.
 *
 *  A SafeSlot does not need to be detached when its invalidator goes away, 
 *  because the SafeSlot will be automatically destroyed.
 *
 *  @see    Slot
 */
class SafeSlot : public Slot
{
   friend class SubjectImpPrivate;
   friend class SlotInvalidator;

public:
   /**
    *  Creates an empty slot object.
    *
    *  The default constructor creates an empty slot object. The object will
    *  return \c false from isValid() and will test equal to other empty slots.
    */
   SafeSlot() : Slot()
   {
   }

   /**
    *  Creates an auto-detaching slot.
    *
    *  Creates an auto-detaching slot. Assuming neither of the first two input 
    *  arguments is \c NULL, the %SafeSlot will return \c true from isValid() and will 
    *  test equal to other SafeSlots created with the same arguments.
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
    *
    *  @param   pInvalidator
    *           If this is non- \c NULL, when pInvalidator is destroyed,
    *           this slot will automatically detach from the Subject it is attached to.
    *           If this is \c NULL, the SafeSlot will behave like a normal slot.
    */
   template<class T, typename Method>
   SafeSlot(T *pT, Method pMethod, SlotInvalidator *pInvalidator) : Slot()
   {
      if(VERIFYNR(pT != NULL && pMethod != NULL))
      {
         mpSlot = new SafeSlotValue<T>(pT, pMethod, pInvalidator);
      }
   }

   /**
    *  Creates a copy of the input slot.
    *
    *  @param   slot
    *           The %Slot to copy from.
    */
   SafeSlot(const Slot& slot) :
      Slot(slot)
   {
   }

   /**
    *  Destroys the %SafeSlot, detaching from the Subject if necessary.
    */
   ~SafeSlot()
   {
      invalidate();
   }

protected:
   /**
    * A class that extends the Slot::SlotWrapper class to add the ability to 
    * query an invalidator. Derived classes need to provide an implementation
    * of this method.
    */
   class SafeSlotWrapper
   {
   public:
      /**
       * Returns the invalidator object associated with this slot wrapper.
       *
       * @return  The invalidator the slot was created with.
       */
      virtual SlotInvalidator *getInvalidator() = 0;
   };

private:
   void invalidate()
   {
      delete mpSlot;
      mpSlot = NULL;
   }

   SlotInvalidator *getInvalidator()
   {
      SafeSlotWrapper *pWrapper = dynamic_cast<SafeSlotWrapper*>(mpSlot);
      if (pWrapper != NULL)
      {
         return pWrapper->getInvalidator();
      }
      return NULL;
   }

   template<class T>
   class SafeSlotValue : public SlotValue<T>, public SafeSlotWrapper
   {
   public:
      SafeSlotValue(T *pT, void (T::*pMethod)(Subject&,const std::string&,const boost::any&), 
         SlotInvalidator *pInvalidator) : SlotValue<T>(pT, pMethod), mpInvalidator(pInvalidator)
      {
      }

      SafeSlotValue(const SlotValue<T> &base, 
         SlotInvalidator *pInvalidator) : SlotValue<T>(base), mpInvalidator(pInvalidator)
      {
      }

      SlotWrapper* clone() const
      {
         return new SafeSlotValue<T>(*this, mpInvalidator);
      }

      const std::type_info &getType() const
      {
         return typeid(SafeSlotValue<T>);
      }

      SlotInvalidator *getInvalidator()
      {
         return mpInvalidator;
      }

      bool matches(const SlotWrapper& rhs) const
      {
         // Two SafeSlotValues test equal if the target and method are the 
         // same and they both have or both don't have an invalidator. If
         // they both have invalidators, the exact value of the invalidator
         // doesn't matter.
         const SafeSlotValue<T> *pRhs = dynamic_cast<const SafeSlotValue<T> *>(&rhs);
         if (mpInvalidator)
         {
            return pRhs != NULL && pRhs->mpInvalidator != NULL && SlotValue<T>::matches(rhs);
         }
         else // this has no invalidator
         {
            // A SafeSlotValue tests equal to a SlotValue if the 
            // SafeSlotValue has a NULL invalidator and the other two 
            // members are equal. We can't just call SlotValue<T>::matches
            // because it will do the type check which will fail.
            if (pRhs != NULL && pRhs->mpInvalidator != NULL)
            {
               return false;
            }
            else // neither has an invalidator
            {
               std::auto_ptr<const SlotValue<T> > pThisCopy(dynamic_cast<const SlotValue<T>*>(this->SlotValue<T>::clone()));
               const SlotValue<T>& rhsValue = static_cast<const SlotValue<T>&>(rhs);
               std::auto_ptr<const SlotValue<T> > pRhsCopy(dynamic_cast<const SlotValue<T>*>(
                  rhsValue.SlotValue<T>::clone()));
               return pThisCopy->matches(*(pRhsCopy.get()));
            }
         }
      }

   private:
      SlotInvalidator *mpInvalidator;
   };
};

/**
 *  This class provides an object that, when destroyed invalidates a slot.  It
 *  is for use with the auto-NULLing SafeSlot class.
 *  
 *  Typically, an observer object that has a slot will inherit from or contain
 *  a SlotInvalidator. The invalidator will be provided to the SafeSlot 
 *  constructor and ensures that the Subject the slot is attached to does not
 *  hold a stale pointer if the observer object is destroyed while still attached.
 *
 *  @see    SafeSlot, AutoSlot
 */
class SlotInvalidator
{
   friend class SubjectImpPrivate;
public:
   /**
    *  The SlotInvalidator constructor.
    */
   SlotInvalidator() : mpSlot(NULL) {}

   /**
    *  The SlotInvalidator destructor.
    */
   ~SlotInvalidator()
   {
      if (mpSlot)
      {
         mpSlot->invalidate();
      }
   }

private:
   void activate(SafeSlot *pSlot)
   {
      mpSlot = pSlot;
   }

   void deactivate()
   {
      mpSlot = NULL;
   }

   SafeSlot *mpSlot;
};

/**
 *  Subclass of SafeSlot for convenience when the observer and invalidator
 *  are the same object.
 *
 *  @see    SafeSlot
 */
class AutoSlot : public SafeSlot
{
public:
   /**
    *  Creates an auto-detaching slot.
    *
    *  Creates a slot. Assuming neither of the input arguments is \c NULL, the 
    *  slot will return \c true from isValid() and will test equal to other slots
    *  created with the same arguments. This slot will automatically detach
    *  from the subject it is attached to when the observer is destroyed.
    *
    *  @param   pObserver
    *           The object to call in Slot::update. It must inherit from 
    *           SlotInvalidator.
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
   AutoSlot (T *pObserver, Method pMethod) : SafeSlot(pObserver, pMethod, pObserver)
   {
   }
};

#endif
