/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BMUTEX_H
#define BMUTEX_H

#include "mutex.h"
#include "pthread.h"

class BMutex : public Mutex
{
   public:
      BMutex();
      virtual ~BMutex();
      virtual bool MutexCreate(); 
      virtual bool MutexInit();
      virtual bool MutexLock();
      virtual bool MutexUnlock();
      virtual bool MutexDestroy();
      pthread_mutex_t* GetMutexID() { return mMutexID; }

   private:
      pthread_mutex_t *mMutexID;
};

#endif
