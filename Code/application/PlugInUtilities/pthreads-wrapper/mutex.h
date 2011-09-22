/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef MUTEX_H
#define MUTEX_H

#include <stdlib.h>

class Mutex
{
   public:
      virtual bool MutexCreate() = 0;
      virtual bool MutexInit() = 0;
      virtual bool MutexLock() = 0;
      virtual bool MutexUnlock() = 0;
      virtual bool MutexDestroy() = 0;
};

#endif
