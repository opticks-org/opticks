/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BTHREAD_SIGNAL_H
#define BTHREAD_SIGNAL_H

#include "thread_signal.h"
#include "bmutex.h"

class BThreadSignal : public ThreadSignal
{
   public:
      BThreadSignal();
      virtual ~BThreadSignal();
      virtual bool ThreadSignalCreate(); 
      virtual bool ThreadSignalInit();
      virtual bool ThreadSignalDestroy();
      virtual bool ThreadSignalWait(void *mutexData);
      virtual bool ThreadSignalActivate();

   private:
      pthread_cond_t *mThreadSignalID;
};

#endif
