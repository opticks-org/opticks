/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROGRESSBRIEFCONSOLE_H
#define PROGRESSBRIEFCONSOLE_H

#include "Progress.h"
#include "SubjectImp.h"

#include <string>

/**
 *  Progress reporting interface.
 *
 *  This object serves as a collector of progress/activity notifications.
 *  Whenever it receives an update, it notifies any clients that might
 *  have registered with the object so they can interrogate what the
 *  change was and render/react to it as appropriate.
 */
class ProgressBriefConsole : public Progress, SubjectImp
{
public:
   ProgressBriefConsole(bool veryBrief);
   virtual ~ProgressBriefConsole();

   /**
    *  Indicate that some activity has taken place.
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
   void updateProgress(const std::string& text, int percent, ReportingLevel gran);

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
   void getProgress(std::string& text, int& percent, ReportingLevel& gran) const;

   const std::string& getObjectType() const;
   bool isKindOf(const std::string& className) const;

   SUBJECTADAPTER_METHODS(SubjectImp)

private:
   std::string mMessage;
   bool mVeryBrief;
   int mCounter;
};

#endif
