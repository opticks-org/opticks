/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ThreadSafeProgressImp.h"

using namespace std;

ThreadSafeProgressImp::ThreadSafeProgressImp() : mpMutex(new BMutex)
{
   mpMutex->MutexCreate();
   mpMutex->MutexInit();
}

ThreadSafeProgressImp::ThreadSafeProgressImp(string amProgressText,
                                             int amPercentComplete,
                                             ReportingLevel amGranularity)
                  : ProgressImp(amProgressText, amPercentComplete, amGranularity),
                    mpMutex(new BMutex)
{
   mpMutex->MutexCreate();
   mpMutex->MutexInit();
}

ThreadSafeProgressImp::~ThreadSafeProgressImp()
{
   mpMutex->MutexDestroy();
   delete mpMutex;
}

const string& ThreadSafeProgressImp::getObjectType() const
{
   static string sType = "ThreadSafeProgressImp";
   return sType;
}

bool ThreadSafeProgressImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "ThreadSafeProgress"))
   {
      return true;
   }

   return ProgressImp::isKindOf(className);
}

//  Purpose:    Method for clients to indicate they have accomplished
//            something.
//  Comments:    This method needs to be thread safe so multiple threads
//            can have their calls sequenced properly. For each call,
//            the attributes are updated with the values from the
//            arguments, and the Notify method (inherited from Subject)
//            is called to inform all attached Observers of the change.
void ThreadSafeProgressImp::updateProgress(const string& text, int percent, ReportingLevel gran)
{
   mpMutex->MutexLock();
   ProgressImp::updateProgress(text, percent, gran);
   mpMutex->MutexUnlock();
}

//  Purpose:    Method for clients to learn what has been accomplished.
//  Comments:    This method needs to be thread safe so multiple threads
//            can have their calls sequenced properly. For each call,
//            the attributes update the values of the
//            arguments.
void ThreadSafeProgressImp::getProgress(string& text, int& percent, ReportingLevel& gran) const
{
   mpMutex->MutexLock();
   ProgressImp::getProgress(text, percent, gran);
   mpMutex->MutexUnlock();
}

void ThreadSafeProgressImp::setPlugIn(PlugIn* pPlugIn)
{
   mpMutex->MutexLock();
   ProgressImp::setPlugIn(pPlugIn);
   mpMutex->MutexUnlock();
}

PlugIn* ThreadSafeProgressImp::getPlugIn() const
{
   PlugIn* pPlugIn = NULL;
   mpMutex->MutexLock();
   pPlugIn = ProgressImp::getPlugIn();
   mpMutex->MutexUnlock();
   return pPlugIn;
}
