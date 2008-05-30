/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


//         ***** UNCLASSIFIED *****
//         Module Name: Thread
         
#ifndef _THREAD_DOT_H
#define _THREAD_DOT_H

#include <stdlib.h>
#include "mutex.h"

class Thread
{
   public:
      virtual bool ThreadInit () = 0; 
      virtual bool ThreadLaunch() = 0;
      virtual bool ThreadWait() = 0;
      virtual bool ThreadRegisterMutex (Mutex *mutexPointer) = 0;
      virtual bool ThreadUnregisterMutex (Mutex *mutexPointer) = 0;
      virtual bool ThreadSetRunFunction(void *threadRunFunction) = 0;
      virtual bool ThreadSetThreadData(void *threadData) = 0;
      virtual bool ThreadDetach() = 0;
      virtual bool ThreadCancel() = 0;
};


#endif
// Do not put anything below this line
