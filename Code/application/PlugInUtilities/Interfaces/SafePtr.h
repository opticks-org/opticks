/*
 * The information in this file is
 * Copyright(c) 2009 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SAFE_PTR_H
#define SAFE_PTR_H

#include "Slot.h"
#include "Subject.h"

/**
 * Smart pointer used to automatically NULLify when the subject is deleted.
 *
 * This class handles a number of issues relating to the management of
 * Subject attachment lifetimes:
 *   - Detaching from the Subject upon destruction.
 *   - Detecting when the Subject is deleted to prevent stale pointers.
 *
 * This class is intended to be used as a member variable, in place of a raw
 * pointer to the Subject.  If subclasses are to be allowed to attach
 * to the Subject, the variable should be made protected.
 *
 * Note that this class will not cause any kind of callback when
 * its Subject has been deleted.  It will simply NULLify the pointer.
 * If you require any special handling for when the Subject has been deleted,
 * You should separately attach to SIGNAL_NAME(Subject, Deleted).
 */
template<typename T>
class SafePtr
{
public:
   /**
    * Default constructor.
    *
    * No Subject is observed.
    */
   SafePtr() : mpSubject(NULL)
   {
   }

   /**
    *  Construct with a subject to observe.
    *
    *  @param   pSubject
    *           The subject to observe.
    */
   SafePtr(T* pSubject) :
      mpSubject(pSubject)
   {
      attachToDeleted();
   }

   /**
    * Destructor.
    *
    * This will take care of detaching from any signals.
    */
   virtual ~SafePtr()
   {
      detachFromDeleted();
      mpSubject = NULL;
   }

   /**
    * Change what Subject to observe.
    *
    * This will automatically detach from the old Subject, and attach to
    * the new one.
    *
    * @param pSubject
    *        The subject to begin observing.
    */
   void reset(T* pSubject = NULL)
   {
      if (pSubject != mpSubject)
      {
         detachFromDeleted();
         mpSubject = pSubject;
         attachToDeleted();
      }
   }

   /**
    * Get the observed Subject.
    *
    * @return The observed Subject.
    */
   T* get()
   {
      return mpSubject;
   }

   /**
    * @copydoc SafePtr::get()
    */
   const T* get() const
   {
      return mpSubject;
   }

   /**
    * @copydoc SafePtr::get()
    */
   T* operator->()
   {
      return get();
   }

   /**
    * @copydoc SafePtr::get()
    */
   const T* operator->() const
   {
      return get();
   }

protected:
   /**
    * Attaches the specified slot to the specified signal on the current
    * Subject. If there is no current Subject, it does nothing.
    *
    * @param signal
    *            The name of the signal to attach to
    *
    * @param slot
    *            The slot to attach to the signal
    */
   void attach(const std::string& signal, const Slot& slot)
   {
      if (mpSubject != NULL)
      {
         mpSubject->attach(signal, slot);
      }
   }

   /**
    * Detaches the specified slot from the specified signal on the current
    * Subject. If there is no current Subject, it does nothing.
    *
    * @param signal
    *            The name of the signal to detach from
    *
    * @param slot
    *            The slot to detach from the signal
    */
   void detach(const std::string& signal, const Slot& slot)
   {
      if (mpSubject != NULL)
      {
         mpSubject->detach(signal, slot);
      }
   }

private:
   SafePtr(const SafePtr&);
   SafePtr& operator=(const SafePtr&);

   void attachToDeleted()
   {
      attach(SIGNAL_NAME(Subject, Deleted), Slot(this, &SafePtr::subjectDeleted));
   }

   void detachFromDeleted()
   {
      detach(SIGNAL_NAME(Subject, Deleted), Slot(this, &SafePtr::subjectDeleted));
   }

   void subjectDeleted(Subject& subject, const std::string& signal, const boost::any& data)
   {
      if (&subject == mpSubject)
      {
         mpSubject = NULL;
      }
   }

   T* mpSubject;
};

#endif
