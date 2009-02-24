/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef _SYSTEMSERVICESIMP_H
#define _SYSTEMSERVICESIMP_H

#include "AppConfig.h"
#include "SystemServices.h"
#include <string>

#if defined(WIN_API)
#include <windows.h>
#endif

/**
 *  Provides access to system dependant services like the syslog/eventlog
 */
class SystemServicesImp : public SystemServices
{
public:
   static SystemServicesImp* instance();

   /**
    *  Write an message to the syslog/eventlog
    *  The Info, Warning, and Error varients write
    *  messages marked with the specified severity level
    *
    *  @param   message
    *           The message to write to the log
    *
    */
   virtual void WriteLogInfo(std::string message);
   virtual void WriteLogWarning(std::string message);
   virtual void WriteLogError(std::string message);

   virtual ~SystemServicesImp();

protected:
   SystemServicesImp();

private:
   static SystemServicesImp* SystemServicesImp::singleton;

   bool mLoggingActive;
#if defined(WIN_API)
   HANDLE mpHandle;
#endif

   class Deleter
   {
   public:
      ~Deleter()
      {
         if (SystemServicesImp::instance())
         {
            delete SystemServicesImp::instance();
         }
      }
      friend class SystemServicesImp;
   };
   static Deleter deleter;
   friend class Deleter;
};

#endif
