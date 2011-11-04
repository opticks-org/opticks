/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef ATTACHMENTPTR_H
#define ATTACHMENTPTR_H

#include "SafePtr.h"

#include <string>
#include <vector>
#include <utility>

/**
 * Smart pointer used to simplify Subject attachment lifetimes.
 *
 * This class handles a number of issues relating to the management of
 * Subject attachment lifetimes:
 *   - Detaching from the Subject upon destruction.
 *   - Changing attachments when the observed instance changed, including
 *     attachments made within subclasses of the observer.
 *   - Detecting when the Subject is deleted to prevent stale pointers.
 *
 * This class is intended to be used as a member variable, in place of a raw
 * pointer to the observed Subject.  If subclasses are to be allowed to attach
 * to the Subject, the variable should be made protected.
 *
 * Note that this class will not cause any kind of callback in the case
 * that its Subject has been deleted.  It will simply NULLify the pointer.
 * If you require any special handling for when the Subject has been deleted,
 * You should separately attach to SIGNAL_NAME(Subject, Deleted) with 
 * AttachmentPtr::addSignal().
 *
 * When used in conjunction with a SafeSlot, the attachments will additionally be 
 * automatically detached when the SafeSlot's invalidator object is destroyed.
 */
template<typename T>
class AttachmentPtr : public SafePtr<T>
{
public:
   /**
    * Default constructor.
    *
    * No Subject is observed.
    */
   AttachmentPtr() : SafePtr<T>(NULL)
   {
   }

   /**
    *  Construct with a subject to observe.
    *
    *  @param   pSubject
    *           The subject to observe.
    */
   AttachmentPtr(T* pSubject) :
      SafePtr<T>(pSubject)
   {
   }

   /**
    * Construct with a single signal/slot connection registered,
    * no Subject observed.
    *
    * @param signalName
    *        The signal to register interest in.
    * @param slot
    *        The slot to call when the signal is emitted.
    */
   AttachmentPtr(const std::string& signalName, const Slot& slot) : SafePtr<T>(NULL),
      mSignalSlots(1, std::make_pair(signalName, slot))
   {
   }

   /**
    * Construct with a subject to observe, and a single signal/slot
    * connection registered.
    *
    * @param pSubject
    *        The subject to observe.
    * @param signalName
    *        The signal to register interest in.
    * @param slot
    *        The slot to call when the signal is emitted.
    */
   AttachmentPtr(T* pSubject, const std::string& signalName, const Slot& slot) : SafePtr<T>(pSubject),
      mSignalSlots(1, std::make_pair(signalName, slot))
   {
      SafePtr<T>::attach(signalName, slot);
   }

   /**
    * Destructor.
    *
    * This will take care of detaching from any signals.
    */
   virtual ~AttachmentPtr()
   {
      reset(NULL);
   }

   /**
    * Add a new signal to be observed.
    *
    * The attachment will be made immediately if there is currently a valid
    * Subject.  The attachment will automatically be made whenever the
    * observed Subject is changed.
    *
    * @param signalName
    *        The signal to register interest in.
    * @param slot
    *        The slot to call when the signal is emitted.
    */
   void addSignal(const std::string& signalName, const Slot& slot)
   {
      mSignalSlots.push_back(std::make_pair(signalName, slot));

      SafePtr<T>::attach(signalName, slot);
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
      if (pSubject != SafePtr<T>::get())
      {
         for (std::vector<std::pair<std::string, Slot> >::const_iterator iter = mSignalSlots.begin();
            iter != mSignalSlots.end(); ++iter)
         {
            SafePtr<T>::detach(iter->first, iter->second);
         }
         SafePtr<T>::reset(pSubject);
         for (std::vector<std::pair<std::string, Slot> >::const_iterator iter = mSignalSlots.begin();
            iter != mSignalSlots.end(); ++iter)
         {
            SafePtr<T>::attach(iter->first, iter->second);
         }
      }
   }

private:
   AttachmentPtr(const AttachmentPtr&);
   AttachmentPtr& operator=(const AttachmentPtr&);

   std::vector<std::pair<std::string, Slot> > mSignalSlots;
};

#endif
