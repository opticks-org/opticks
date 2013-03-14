/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SubjectImpPrivate.h"
#include "SafeSlot.h"
#include "Subject.h"

#include <QtCore/QtGlobal>

#include <algorithm>
#include <vector>

using namespace std;

SubjectImpPrivate::SubjectImpPrivate() : mpSubject(NULL), mSignalsEnabled(true)
{
}

SubjectImpPrivate::~SubjectImpPrivate()
{
}

bool SubjectImpPrivate::attach(Subject& subject, const string& signal, const Slot& slot)
{
   if (slot == SafeSlot())
   {
      return false;
   }

   if (mpSubject == NULL)
   {
      mpSubject = &subject;
   }

   MapType::iterator pSlotVec = mSlots.find(signal);
   if (pSlotVec != mSlots.end())
   {
      list<SafeSlot>& slotVec = pSlotVec->second;
      list<SafeSlot>::iterator pSlot;
      for (pSlot = slotVec.begin(); pSlot != slotVec.end(); ++pSlot)
      {
         if (*pSlot == slot)
         {
            return false;
         }
      }
   }

   mSlots[signal].push_back(slot);
   SafeSlot& mappedSlot(mSlots[signal].back());
   SlotInvalidator* pInvalidator = mappedSlot.getInvalidator();
   if (pInvalidator)
   {
      pInvalidator->activate(&mappedSlot);
   }

   try
   {
      slot.callAttachMethod(subject, signal);
   }
   catch (boost::bad_any_cast &exc)
   {
      string msg = "Bad cast while calling processing attached method for signal " + signal + "\n" + exc.what();
      VERIFYRV_MSG(false, true, msg.c_str());
   }

   return true;
}

bool SubjectImpPrivate::detach(Subject& subject, const string& signal, const Slot& slot)
{
   bool success = true;
   MapType::iterator pSlotVec = mSlots.find(signal);
   if (pSlotVec != mSlots.end())
   {
      list<SafeSlot>& slotVec = pSlotVec->second;
      if (slot == SafeSlot())
      {
         bool shouldCallDetachMethod = true;
         list<SafeSlot>::iterator pSlot;
         for (pSlot = slotVec.begin(); pSlot != slotVec.end(); ++pSlot)
         {
            SlotInvalidator* pInvalidator = pSlot->getInvalidator();
            if (pInvalidator)
            {
               pInvalidator->deactivate();
            }
            SafeSlot slotCopy = *pSlot;
            *pSlot = SafeSlot();
            if (shouldCallDetachMethod)
            {
               try
               {
                  slotCopy.callDetachMethod(subject, signal);
               }
               catch (boost::bad_any_cast &exc)
               {
                  string msg = "Bad cast while calling processing detached method for signal " + signal +
                     "\n" + exc.what();
                  VERIFYNR_MSG(false, msg.c_str());
                  shouldCallDetachMethod = false;
               }
            }
         }
      }
      else
      {
         list<SafeSlot>::iterator pSlot = find(slotVec.begin(), slotVec.end(), slot);
         if (pSlot != slotVec.end())
         {
            SlotInvalidator* pInvalidator = pSlot->getInvalidator();
            if (pInvalidator)
            {
               pInvalidator->deactivate();
            }
            SafeSlot slotCopy = *pSlot;
            *pSlot = SafeSlot();
            try
            {
               slotCopy.callDetachMethod(subject, signal);
            }
            catch (boost::bad_any_cast &exc)
            {
               string msg = "Bad cast while calling processing detached method for signal " + signal +
                  "\n" + exc.what();
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
   PopRecursion(vector<string>& recursions, const string& recursion) : mRecursions(recursions) 
   {
      mRecursions.push_back(recursion);
   }
   ~PopRecursion() 
   { 
      mRecursions.pop_back();
   }
private:
   PopRecursion& operator=(const PopRecursion& rhs);

   vector<string>& mRecursions;
};

void SubjectImpPrivate::notify(Subject& subject, const string& signal, const string& originalSignal,
                               const boost::any& data)
{
   if (signal.empty())
   {
      return;
   }

   bool signalIsDeleted = false;
   bool deletedFlagSet = false; // only want to do the signal name check if necessary
   if (!mSignalsEnabled)
   {
      signalIsDeleted = (signal == SIGNAL_NAME(Subject, Deleted));
      if (!signalIsDeleted)
      {
         return;
      }
      deletedFlagSet = true;
   }

   // notify slots attached to signal
   MapType::iterator pSlotVec = mSlots.find(signal);
   if (pSlotVec != mSlots.end())
   {
      list<SafeSlot>& slotVec = pSlotVec->second;

      if (!slotVec.empty())
      {
         PopRecursion popper(mRecursions, signal);

         // Keep a (unique) vector of Slots which have been notified to ensure that no Slot is notified more than once
         // For efficiency, only check the vector when Slots have been added during notification
         // This prevents an infinite loop when a Slot does a detach/attach to a signal
         unsigned int slotNum = 0;
         const unsigned int numOriginalSlots = slotVec.size();
         vector<SafeSlot> notifiedSlots;
         notifiedSlots.reserve(numOriginalSlots);
         for (list<SafeSlot>::iterator pSlot = slotVec.begin(); pSlot != slotVec.end(); ++pSlot, ++slotNum)
         {
            try
            {
               SafeSlot slotCopy = *pSlot;
               if (slotNum < numOriginalSlots ||
                  find(notifiedSlots.begin(), notifiedSlots.end(), slotCopy) == notifiedSlots.end())
               {
                  notifiedSlots.push_back(slotCopy);
                  slotCopy.update(subject, signal, data);
               }
            }
            catch (boost::bad_any_cast &exc)
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

const list<SafeSlot>& SubjectImpPrivate::getSlots(const string& signal)
{
   static list<SafeSlot> emptyList;

   if (signal.empty())
   {
      return emptyList;
   }

   MapType::iterator pSlotVec = mSlots.find(signal);
   if (pSlotVec != mSlots.end())
   {
      list<SafeSlot>& slotVec = pSlotVec->second;
      removeEmptySlots(signal, slotVec);
      return slotVec;
   }
   else
   {
      return emptyList;
   }
}

void SubjectImpPrivate::removeEmptySlots(const string& recursion, list<SafeSlot>& slotVec)
{
   if (count(mRecursions.begin(), mRecursions.end(), recursion) == 0)
   {
      for (list<SafeSlot>::iterator pSlot = slotVec.begin(); pSlot != slotVec.end(); )
      {
         if (pSlot->isValid() == false)
         {
            pSlot = slotVec.erase(pSlot);
         }
         else
         {
            ++pSlot;
         }
      }
   }
}

void SubjectImpPrivate::enableSignals(bool enabled)
{
   mSignalsEnabled = enabled;
}

bool SubjectImpPrivate::signalsEnabled() const
{
   return mSignalsEnabled;
}
