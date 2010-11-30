/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#include <assert.h>
#include "AppConfig.h"
#include "bthread.h"

BThread::BThread()
{
   mThreadData = NULL;
   mThreadRunFunction = NULL;
}

BThread::BThread(void *threadData, void *threadRunFunction)
{
  mThreadData = threadData;
  mThreadRunFunction = threadRunFunction;
}

bool BThread::ThreadLaunch(int priority)
{
   assert (mThreadRunFunction != NULL);
   pthread_attr_t attr;
   if(pthread_attr_init(&attr) != 0)
      return false;
   if(priority != 0)
   {
      struct sched_param param;
      if(pthread_attr_getschedparam(&attr, &param) != 0)
         return false;
      param.sched_priority = priority;
      if(pthread_attr_setschedparam(&attr, &param) != 0)
         return false;
   }
#if defined(WIN_API)
   pthread_create(&mThreadID, &attr, (void *(__cdecl *)(void *))  mThreadRunFunction, 
#else
   pthread_create(&mThreadID, &attr, (void *(*)(void *))  mThreadRunFunction, 
#endif
      (void *) mThreadData);

   return true;
}

bool BThread::ThreadWait()
{
   pthread_join (mThreadID, NULL);

   return true;
}

bool BThread::ThreadDetach()
{
   pthread_detach (mThreadID);

   return true;
}

bool BThread::ThreadCancel()
{
   pthread_cancel (mThreadID);

   return true;
}

bool BThread::ThreadSetPriority(int priority)
{
   struct sched_param param;
   int policy;
   if(pthread_getschedparam(mThreadID, &policy, &param) != 0)
      return false;
   param.sched_priority = priority;
   if(pthread_setschedparam(mThreadID, policy, &param) != 0)
      return false;
   return true;
}
