/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#include <assert.h>
#include "bthread_signal.h"

BThreadSignal::BThreadSignal()
{
   mThreadSignalID = NULL;
}

BThreadSignal::~BThreadSignal()
{
   if (mThreadSignalID != NULL)
   {
      pthread_cond_destroy (mThreadSignalID);
      delete [] mThreadSignalID;
      mThreadSignalID = NULL;
   }
}

bool BThreadSignal::ThreadSignalCreate()
{
   mThreadSignalID = new pthread_cond_t;
   return true;
}

bool BThreadSignal::ThreadSignalInit()
{
   assert (mThreadSignalID != NULL);

   pthread_cond_init (mThreadSignalID, NULL);
   return true;
}

bool BThreadSignal::ThreadSignalDestroy()
{
   assert (mThreadSignalID != NULL);

   pthread_cond_destroy (mThreadSignalID);
   delete [] mThreadSignalID;
   mThreadSignalID = NULL;

   return true;
}

bool BThreadSignal::ThreadSignalActivate()
{
   assert (mThreadSignalID != NULL);

   pthread_cond_signal(mThreadSignalID);

   return true;
}

bool BThreadSignal::ThreadSignalWait(void *mutexData)
{
   assert (mThreadSignalID != NULL);
   assert (mutexData != NULL);

   BMutex *data = (BMutex *) mutexData;

   pthread_cond_wait(mThreadSignalID, data->GetMutexID());

   return true;
}
