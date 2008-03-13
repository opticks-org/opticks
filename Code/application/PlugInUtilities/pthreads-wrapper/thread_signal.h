/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 


#ifndef _THREAD_SIGNAL_DOT_H
#define _THREAD_SIGNAL_DOT_H

class ThreadSignal
{
   public:
      virtual bool ThreadSignalCreate () = 0; 
      virtual bool ThreadSignalInit () = 0;
      virtual bool ThreadSignalDestroy () = 0;
      virtual bool ThreadSignalWait (void *) = 0;
      virtual bool ThreadSignalActivate () = 0;
};

#endif
