/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef THREADSAFEPROGRESSADAPTER_H
#define THREADSAFEPROGRESSADAPTER_H

#include "Progress.h"
#include "ThreadSafeProgressImp.h"

class ThreadSafeProgressAdapter : public Progress, public ThreadSafeProgressImp
   THREADSAFEPROGRESSADAPTEREXTENSION_CLASSES
{
public:
   ThreadSafeProgressAdapter() {};
   virtual ~ThreadSafeProgressAdapter()
   {
      notify(SIGNAL_NAME(Subject, Deleted));
   }

   virtual const std::string& getObjectType() const
   {
      static std::string sType("ThreadSafeProgressAdapter");
      return sType;
   }

   virtual bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "ThreadSafeProgress"))
      {
         return true;
      }

      return ThreadSafeProgressImp::isKindOf(className);
   }

   /**
    *  Indicate that some activity has taken place.
    *
    *  @param   text
    *           Message indicating the nature of the activity being reported.
    *  @param   percent
    *           For partial completion status changes, this figure represents
    *           what percent (0-100) of the activity has cumulatively been
    *           completed.  A percent value of -1 will hide the progress
    *           dialog until a positive percentage is set.
    *  @param   gran
    *           Reporting level granularity, allowing clients to filter out
    *           activity changes they are not interested in. For example,
    *           most clients would ignore debugging mode notifications.
    */
   void updateProgress(const std::string& text, int percent, ReportingLevel gran)
   {
      ThreadSafeProgressImp::updateProgress(text, percent, gran);
   }

   /**
    *  Report the activity that has taken place.
    *
    *  @param   text
    *           Message indicating the nature of the activity being reported.
    *  @param   percent
    *           For partial completion status changes, this figure represents
    *           what percent (0-100) of the activity has cumulatively been
    *           completed.
    *  @param   gran
    *           Reporting level granularity, allowing clients to filter out
    *           activity changes they are not interested in. For example,
    *           most clients would ignore debugging mode notifications.
    */
   void getProgress(std::string& text, int& percent, ReportingLevel& gran) const
   {
      ThreadSafeProgressImp::getProgress(text, percent, gran);
   }

   THREADSAFEPROGRESSADAPTER_METHODS(ThreadSafeProgressImp)
};

#endif
