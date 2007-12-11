/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SubjectImpPrivate.h"
#include "Slot.h"
#include "Subject.h"

#include <QtCore/QtGlobal>

using namespace std;

SubjectImpPrivate::SubjectImpPrivate() : 
   mpSlotsWaitingForDetach(NULL),
   mpActiveSlot(NULL)
{
}

SubjectImpPrivate::~SubjectImpPrivate()
{
   if (mpSlotsWaitingForDetach != NULL)
   {
      delete mpSlotsWaitingForDetach;
   }
}

bool SubjectImpPrivate::attach(Subject &subject, const string &signal, const Slot &slot)
{
   if (slot == Slot())
   {
      return false;
   }

   MapType::iterator pSlotVec=mSlots.find(signal);
   if (pSlotVec != mSlots.end())
   {
      list<Slot> &slotVec = pSlotVec->second;
      list<Slot>::iterator pSlot;
      for (pSlot=slotVec.begin(); pSlot!=slotVec.end(); ++pSlot)
      {
         if (*pSlot == slot) 
         {
            return false;
         }
      }
   }

   mSlots[signal].push_back(slot);

   try
   {
      slot.callAttachMethod(subject, signal);
   }
   catch(boost::bad_any_cast &exc)
   {
      std::string msg = "Bad cast while calling processing attached method for signal " + signal + "\n" + exc.what();
      VERIFYRV_MSG(false, true, msg.c_str());
   }

   return true;
}

bool SubjectImpPrivate::detach(Subject &subject, const string &signal, const Slot &slot)
{
   if ((&slot == mpActiveSlot) || 
      (mpActiveSlot!=NULL && mActiveSignal == signal &&
      (slot==Slot() || slot == *mpActiveSlot)))
   {
      //Slot is attempting to detach itself while
      //it is being notified by this Subject.
      //queue the delete request
      if (mpSlotsWaitingForDetach == NULL)
      {
         mpSlotsWaitingForDetach = new std::list<Slot>();
      }
      mpSlotsWaitingForDetach->push_back(slot);
      return true;
   }

   MapType::iterator pSlotVec = mSlots.find(signal);
   if (pSlotVec != mSlots.end())
   {
      list<Slot> &slotVec = pSlotVec->second;
      if (slot == Slot())
      {
         list<Slot>::iterator pSlot;
         for (pSlot=slotVec.begin(); pSlot!=slotVec.end(); ++pSlot)
         {
            Slot slotCopy = *pSlot;
            slotVec.erase(pSlot);
            try
            {
               slotCopy.callDetachMethod(subject, signal);              
            }
            catch(boost::bad_any_cast &exc)
            {
               std::string msg = "Bad cast while calling processing detached method for signal " + signal + "\n" + exc.what();
               VERIFYRV_MSG(false, true, msg.c_str());
            }
         }
      }
      else
      {
         list<Slot>::iterator pSlot = find(slotVec.begin(), slotVec.end(), slot);
         if (pSlot != slotVec.end())
         {
            Slot slotCopy = *pSlot;
            slotVec.erase(pSlot);
            try
            {
               slotCopy.callDetachMethod(subject, signal);
            }
            catch(boost::bad_any_cast &exc)
            {
               std::string msg = "Bad cast while calling processing detached method for signal " + signal + "\n" + exc.what();
               VERIFYRV_MSG(false, true, msg.c_str());
            }
         }
         else
         {
            return false;
         }
      }
   }

   return true;
}

void SubjectImpPrivate::notify(Subject &subject, const string &signal, const boost::any &data)
{
   if (signal.empty())
   {
      return;
   }

   // notify slots attached to signal
   MapType::iterator pSlotVec = mSlots.find(signal);
   if (pSlotVec != mSlots.end())
   {
      list<Slot> &slotVec = pSlotVec->second;
      list<Slot>::iterator pSlot;
      mActiveSignal = signal;
      for (pSlot=slotVec.begin(); 
         pSlot!=slotVec.end(); 
         ++pSlot)
      {
         mpActiveSlot = &*pSlot;
         try
         {
            pSlot->update(subject, signal, data);
         }
         catch(boost::bad_any_cast &exc)
         {
            std::string msg = "Bad cast while calling processing signal " + mActiveSignal + "\n" + exc.what();
            VERIFYNRV_MSG(false, msg.c_str());
         }
         mpActiveSlot = NULL;
      }
   }

   detachWaitingSlots(subject, signal);

   if (signal != SIGNAL_NAME(Subject, Modified) && signal != SIGNAL_NAME(Subject, Deleted))
   {
      // notify slots attached to all signals
      pSlotVec = mSlots.find(SIGNAL_NAME(Subject, Modified));
      if (pSlotVec != mSlots.end())
      {
         list<Slot> &slotVec = pSlotVec->second;
         list<Slot>::iterator pSlot;
         mActiveSignal = SIGNAL_NAME(Subject, Modified);
         for (pSlot=slotVec.begin(); pSlot!=slotVec.end(); ++pSlot)
         {
            mpActiveSlot = &*pSlot;
            try
            {
               pSlot->update(subject, SIGNAL_NAME(Subject, Modified), data);
            }
            catch(boost::bad_any_cast &exc)
            {
               std::string msg = "Bad cast while calling processing signal " + mActiveSignal + "\n" + exc.what();
               VERIFYNRV_MSG(false, msg.c_str());
            }
            mpActiveSlot = NULL;
         }
      }

      detachWaitingSlots(subject, SIGNAL_NAME(Subject, Modified));
   }

   mActiveSignal.clear();
}

void SubjectImpPrivate::detachWaitingSlots(Subject &subject, const string &signal)
{
   while (mpSlotsWaitingForDetach != NULL)
   {
      //now that we have finished notifying slots,
      //detach any that requested to be detached while
      //they were being notified.
      std::list<Slot>* pSlotsWaitingForDetach = mpSlotsWaitingForDetach;
      mpSlotsWaitingForDetach = NULL;

      list<Slot>::iterator pSlot;
      for (pSlot = pSlotsWaitingForDetach->begin(); pSlot != pSlotsWaitingForDetach->end(); ++pSlot)
      {
         detach(subject, signal, *pSlot);
      }
      delete pSlotsWaitingForDetach;
   }
}

const list<Slot>& SubjectImpPrivate::getSlots(const string &signal)
{
   static list<Slot> emptyList;

   if (signal.empty())
   {
      return emptyList;
   }

   MapType::iterator pSlotVec = mSlots.find(signal);
   if (pSlotVec != mSlots.end())
   {
      list<Slot> &slotVec = pSlotVec->second;
      return slotVec;
   }
   else
   {
      return emptyList;
   }
}
