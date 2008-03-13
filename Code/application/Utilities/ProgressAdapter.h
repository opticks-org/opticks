/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef __PROGRESSADAPTER_H
#define __PROGRESSADAPTER_H

#include "Progress.h"
#include "ProgressImp.h"

class ProgressAdapter : public Progress, public ProgressImp
{
public:
   ProgressAdapter() {};
   virtual ~ProgressAdapter()
   {
   notify(SIGNAL_NAME(Subject, Deleted));
   }

   virtual const std::string& getObjectType() const
   {
      static std::string type("ProgressAdapter");
      return type;
   }

   virtual bool isKindOf(const std::string& className) const
   {
      if ((className == getObjectType()) || (className == "Progress"))
      {
         return true;
      }

      return ProgressImp::isKindOf(className);
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
      ProgressImp::updateProgress(text, percent, gran);
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
      ProgressImp::getProgress(text, percent, gran);
   }

   bool attach(const std::string &signal, const Slot &slot)
   {
      return ProgressImp::attach(signal, slot);
   }
   bool detach(const std::string &signal, const Slot &slot)
   {
      return ProgressImp::detach(signal, slot);
   }
};

#endif
