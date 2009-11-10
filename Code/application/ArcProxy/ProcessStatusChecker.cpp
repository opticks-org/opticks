/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ProcessStatusChecker.h"

#include <QtCore/QTimer>

#if defined(ESRI_WINDOWS)
#include <Windows.h>
#endif

ProcessStatusChecker::ProcessStatusChecker(QObject *pParent, long pid, 
                                           unsigned int msec) : QObject(pParent), mPid(pid)
{
   QTimer *pTimer = new QTimer(this);

   connect(pTimer, SIGNAL(timeout()), this, SLOT(timeout()));
   pTimer->start(msec);
}

ProcessStatusChecker::~ProcessStatusChecker()
{
}

void ProcessStatusChecker::timeout()
{
#pragma message("Write this function for unix (leckels)")
#if defined(ESRI_WINDOWS)
   HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, mPid);
   if (proc == NULL)
   {
      emit processExited();
   }

   DWORD exitCode = STILL_ACTIVE;
   if (GetExitCodeProcess(proc, &exitCode) == 0 || exitCode != STILL_ACTIVE)
   {
      emit processExited();
   }
   
   CloseHandle(proc);
#endif
}
