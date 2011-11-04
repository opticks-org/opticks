/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BTHREAD_H
#define BTHREAD_H

#include "thread.h"
#include "pthread.h"
#include "bmutex.h"

class BThread : public Thread
{
   public:
      BThread();
      BThread(void* threadData, void* threadRunFunction);
      virtual ~BThread() {};
      virtual bool ThreadInit() { return true; }
      /**
       * Start a thread
       *
       * @param priority
       *          set the initial priority of the thread
       *          if this is =0 then the default priority is used
       *          this can be changed later with ThreadSetPriority()
       *
       * @return true if successfull, false otherwise
       *
       * @see ThreadSetPriority(int priority)
       */
      virtual bool ThreadLaunch(int priority);
      virtual bool ThreadLaunch();
      virtual bool ThreadWait();
      virtual bool ThreadRegisterMutex(Mutex* mutexPointer) { return true; }
      virtual bool ThreadUnregisterMutex(Mutex* mutexPointer) { return true; }
      virtual bool ThreadSetRunFunction(void* threadRunFunction)
      {
         mThreadRunFunction = threadRunFunction;
         return true;
      }
      virtual bool ThreadSetThreadData(void* threadData)
      {
         mThreadData = threadData;
         return true;
      }
      virtual bool ThreadDetach();
      virtual bool ThreadCancel();
      /**
       * Set the thread's priority.
       * @param priority
       *    this is mostly system dependant but is relative
       *       <0 run at a lower priorty
       *       =0 no change to priority
       *       >0 run at a higher priority
       *          WARNING: running a thread at a higher priority than
       *                   the main thread can result in an apparent
       *                   lock up and slow response to use input...use great
       *                   care when raising thread priority. running a
       *                   computation intensive thread at a priority equal to
       *                   the main thread can also result in slow response so
       *                   it is advisable to lower these intensive threads' priority
       *
       * @return true if the priority was successfully changed, false otherwise
       */
      bool ThreadSetPriority(int priority);
      BThread& operator=(BThread* current)
      {
         this->mThreadData = current->mThreadData;
         this->mThreadRunFunction = current->mThreadRunFunction;
         this->mThreadID = current->mThreadID;
         return *this;
      };

   private:
      void* mThreadData;
      void* mThreadRunFunction;
      pthread_t mThreadID;
};

#endif
