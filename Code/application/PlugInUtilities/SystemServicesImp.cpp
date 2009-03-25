/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */


#include "AppConfig.h"
#include "AppVersion.h"
#include "ApplicationServices.h"
#include "SystemServicesImp.h"
#if defined(WIN_API)
#include "EventMessages.h"
#endif

using namespace std;

SystemServicesImp* SystemServicesImp::singleton = NULL;

SystemServicesImp* SystemServicesImp::instance()
{
   if (singleton == NULL)
   {
      singleton = new SystemServicesImp;
   }
   return singleton;
}

/************ UNIX Version of SystemServices ****************/

#if defined(UNIX_API)

#include <syslog.h>
#include <strings.h>

SystemServicesImp::SystemServicesImp() : mLoggingActive(true)
{
   int facility(LOG_USER);

   if (mLoggingActive)
   {
      openlog(APP_NAME, LOG_PID, facility);
   }
}

SystemServicesImp::~SystemServicesImp()
{
   if (mLoggingActive)
   {
      closelog();
   }
}

void SystemServicesImp::WriteLogInfo(string message)
{
   if (mLoggingActive)
   {
      syslog(LOG_INFO, message.c_str());
   }
}

void SystemServicesImp::WriteLogWarning(string message)
{
   if (mLoggingActive)
   {
      syslog(LOG_WARNING, message.c_str());
   }
}

void SystemServicesImp::WriteLogError(string message)
{
   if (mLoggingActive)
   {
      syslog(LOG_ERR, message.c_str());
   }
}

#endif // UNIX_API

/********** Windows Version of SystemServices ***************/

#if defined(WIN_API)

SystemServicesImp::SystemServicesImp() : mLoggingActive(true)
{
   mpHandle = RegisterEventSource(NULL,TEXT(APP_NAME));
}

SystemServicesImp::~SystemServicesImp()
{
   DeregisterEventSource(mpHandle);
}

void SystemServicesImp::WriteLogInfo(string message)
{
   if (mpHandle != NULL)
   {
      PCTSTR aInsertions[] = {message.c_str()};
      ReportEvent(mpHandle,EVENTLOG_INFORMATION_TYPE,0,APP_MESSAGE,NULL,1,0,aInsertions,NULL);
   }
}

void SystemServicesImp::WriteLogWarning(string message)
{
   if (mpHandle != NULL)
   {
      PCTSTR aInsertions[] = {message.c_str()};
      ReportEvent(mpHandle,EVENTLOG_WARNING_TYPE,0,APP_MESSAGE,NULL,1,0,aInsertions,NULL);
   }
}

void SystemServicesImp::WriteLogError(string message)
{
   if (mpHandle != NULL)
   {
      PCTSTR aInsertions[] = {message.c_str()};
      ReportEvent(mpHandle,EVENTLOG_ERROR_TYPE,0,APP_MESSAGE,NULL,1,0,aInsertions,NULL);
   }
}

#endif // WIN_API
