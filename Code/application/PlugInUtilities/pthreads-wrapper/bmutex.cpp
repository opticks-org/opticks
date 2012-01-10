/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#include <assert.h>
#include "bmutex.h"

BMutex::BMutex()
{
   mMutexID = NULL;
}

BMutex::~BMutex()
{
   if (mMutexID != NULL)
   {
      pthread_mutex_destroy (mMutexID);
      delete [] mMutexID;
      mMutexID = NULL;
   }

}
bool BMutex::MutexCreate()
{
   mMutexID = new pthread_mutex_t;
   return true;
}

bool BMutex::MutexInit()
{
   assert (mMutexID != NULL);

   pthread_mutex_init (mMutexID, NULL);
   return true;
}

bool BMutex::MutexLock()
{
   assert (mMutexID != NULL);

   pthread_mutex_lock (mMutexID);

   return true;
}

bool BMutex::MutexUnlock()
{
   assert (mMutexID != NULL);

   pthread_mutex_unlock (mMutexID);

   return true;
}

bool BMutex::MutexDestroy()
{
   assert (mMutexID != NULL);

   pthread_mutex_destroy (mMutexID);
   delete [] mMutexID;
   mMutexID = NULL;

   return true;
}
