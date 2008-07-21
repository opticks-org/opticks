/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DMUTEX_H
#define DMUTEX_H

#include "bmutex.h"
#include "bthread_signal.h"

/**
 * MTA is the Multi-Threaded Algorithm namespace.
 */
namespace mta
{
   /**
    * The DMutex is a resource equivalent to a BMutex.
    *
    * @see Resource
    */
   class DMutex : public BMutex
   {
   public:
      /**
       * Create and initialize the DMutex.
       */
      DMutex()
      {
         MutexCreate();
         MutexInit();
      }

      /**
       * Destroy the DMutex
       */
      ~DMutex()
      {
         MutexDestroy();
      }
   };

   /**
    * DThreadSignal is a Resource equivalent to a BThreadSignal.
    *
    * @see Resource
    */
   class DThreadSignal : public BThreadSignal
   {
   public:
      /**
       * Create and initialize the DThreadSignal.
       */
      DThreadSignal()
      {
         ThreadSignalCreate();
         ThreadSignalInit();
      }

      /**
       * Destroy the DThreadSignal.
       */
      ~DThreadSignal()
      {
         ThreadSignalDestroy();
      }
   };

   /**
    * MutexLock is a Resource that locks a BMutex.
    *
    * @see Resource
    */
   class MutexLock
   {
   public:
      /**
       * Create a lock for a mutex.
       *
       * @param  mutex
       *         The %BMutex to lock.
       */
      MutexLock(BMutex &mutex) : mMutex(mutex)
      {
         mMutex.MutexLock();
      }

      /**
       * Unlocks the BMutex.
       */
      ~MutexLock()
      {
         mMutex.MutexUnlock();
      }

   private:
      BMutex& mMutex;
   };
}

#endif
