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

#include <algorithm>
#include <vector>

using namespace std;

SubjectImpPrivate::SubjectImpPrivate() : mpSubject(NULL)
{
}

SubjectImpPrivate::~SubjectImpPrivate()
{
}

bool SubjectImpPrivate::attach(Subject &subject, const string &signal, const Slot &slot)
{
   if (slot == Slot())
   {
      return false;
   }

   if (mpSubject == NULL)
   {
      mpSubject = &subject;
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
      string msg = "Bad cast while calling processing attached method for signal " + signal + "\n" + exc.what();
      VERIFYRV_MSG(false, true, msg.c_str());
   }

   return true;
}

bool SubjectImpPrivate::detach(Subject &subject, const string &signal, const Slot &slot)
{
   bool success = true;
   MapType::iterator pSlotVec = mSlots.find(signal);
   if (pSlotVec != mSlots.end())
   {
      list<Slot> &slotVec = pSlotVec->second;
      if (slot == Slot())
      {
         bool shouldCallDetachMethod = true;
         list<Slot>::iterator pSlot;
         for (pSlot=slotVec.begin(); pSlot!=slotVec.end(); ++pSlot)
         {
            Slot slotCopy = *pSlot;
            *pSlot = Slot();
            if (shouldCallDetachMethod)
            {
               try
               {
                  slotCopy.callDetachMethod(subject, signal);
               }
               catch(boost::bad_any_cast &exc)
               {
                  string msg = "Bad cast while calling processing detached method for signal " + signal + "\n" + exc.what();
                  VERIFYNR_MSG(false, msg.c_str());
                  shouldCallDetachMethod = false;
               }
            }
         }
      }
      else
      {
         list<Slot>::iterator pSlot = find(slotVec.begin(), slotVec.end(), slot);
         if (pSlot != slotVec.end())
         {
            Slot slotCopy = *pSlot;
            *pSlot = Slot();
            try
            {
               slotCopy.callDetachMethod(subject, signal);
            }
            catch(boost::bad_any_cast &exc)
            {
               string msg = "Bad cast while calling processing detached method for signal " + signal + "\n" + exc.what();
               VERIFYNR_MSG(false, msg.c_str());
            }
         }
         else
         {
            success = false;
         }
      }

      removeEmptySlots(signal, slotVec);
   }

   return success;
}

class PopRecursion
{
public:
   PopRecursion(vector<string> &recursions, const string &recursion) : mRecursions(recursions) 
   {
      mRecursions.push_back(recursion);
   }
   ~PopRecursion() 
   { 
      mRecursions.pop_back(); 
   }
private:
   vector<string> &mRecursions;
};

void SubjectImpPrivate::notify(Subject &subject, const string &signal, const string &originalSignal, const boost::any &data)
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

      // Scope the recursion popper
      {
         PopRecursion popper(mRecursions, signal);

         // Keep a (unique) vector of Slots which have been notified to ensure that no Slot is notified more than once
         // For efficiency, only check the vector when Slots have been added during notification
         // This prevents an infinite loop when a Slot does a detach/attach to a signal
         unsigned int slotNum = 0;
         const unsigned int numOriginalSlots = slotVec.size();
         vector<Slot> notifiedSlots;
         notifiedSlots.reserve(numOriginalSlots);
         for (list<Slot>::iterator pSlot = slotVec.begin(); pSlot != slotVec.end(); ++pSlot, ++slotNum)
         {
            try
            {
               Slot slotCopy = *pSlot;
               if (slotNum < numOriginalSlots ||
                  find(notifiedSlots.begin(), notifiedSlots.end(), slotCopy) == notifiedSlots.end())
               {
                  notifiedSlots.push_back(slotCopy);
                  slotCopy.update(subject, signal, data);
               }
            }
            catch(boost::bad_any_cast &exc)
            {
               string msg = "Bad cast while calling processing signal " + originalSignal + "\n" + exc.what();
               VERIFYNRV_MSG(false, msg.c_str());
            }
         }
      }

      removeEmptySlots(signal, slotVec);
   }

   if (signal != SIGNAL_NAME(Subject, Modified) && signal != SIGNAL_NAME(Subject, Deleted))
   {
      string effectiveSignal = originalSignal + " as " + SIGNAL_NAME(Subject, Modified);
      notify(subject, SIGNAL_NAME(Subject, Modified), effectiveSignal, data);
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
      removeEmptySlots(signal, slotVec);
      return slotVec;
   }
   else
   {
      return emptyList;
   }
}

void SubjectImpPrivate::removeEmptySlots(const string &recursion, list<Slot> &slotVec)
{
   if (count(mRecursions.begin(), mRecursions.end(), recursion) == 0)
   {
      slotVec.erase(remove(slotVec.begin(), slotVec.end(), Slot()), slotVec.end());
   }
}
