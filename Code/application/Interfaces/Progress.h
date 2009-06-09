/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef __PROGRESS_H
#define __PROGRESS_H

#include "ConfigurationSettings.h"
#include "EnumWrapper.h"
#include "Subject.h"

#include <string>

/**
 *  Specifies the state of the status message.
 *
 *  The reporting level indicates the circumstances surrounding the current
 *  progress status and displays the appropriate message in the progress dialog.
 */
enum ReportingLevelEnum
{
   NORMAL = 0,    /**< A brief status message is displayed. */
   WARNING,       /**< An extended status message is displayed with scrolling text as necessary. */
   ABORT,         /**< The user has aborted the process.\   The progress dialog indicates a zero
                       percent complete, and the Cancel button text changes to Close. */
   ERRORS         /**< An error has occurred in the process.\   The progress dialog indicates the
                       current percent complete, and the Cancel button text changes to Close. */
};

/**
 * @EnumWrapper ::ReportingLevelEnum.
 */
typedef EnumWrapper<ReportingLevelEnum> ReportingLevel;

/**
 *  %Progress reporting interface.
 *
 *  This object serves as a collector of progress/activity notifications.
 *  Whenever it receives an update, it notifies any clients that might
 *  have registered with the object so they can interrogate what the
 *  change was and render/react to it as appropriate.
 *
 *  This subclass of Subject will notify upon the following conditions:
 *  - The following method is called: updateProgress().
 *  - Everything else documented in Subject.
 *
 *  @see        Subject
 */
class Progress : public Subject
{
public:
   SETTING(AutoClose, Progress, bool, false)

   /**
    *  Indicate that some activity has taken place.
    *
    *  @param   text
    *           %Message indicating the nature of the activity being reported.
    *  @param   percent
    *           For partial completion status changes, this figure represents
    *           what percent (0-100) of the activity has cumulatively been
    *           completed.
    *  @param   gran
    *           Reporting level granularity, allowing clients to filter out
    *           activity changes they are not interested in. For example,
    *           most clients would ignore debugging mode notifications.
    *
    *  @notify  This method will notify Subject::signalModified.
    */
   virtual void updateProgress(const std::string& text, int percent, ReportingLevel gran) = 0;

   /**
    *  Report the activity that has taken place.
    *
    *  @param   text
    *           %Message indicating the nature of the activity being reported.
    *  @param   percent
    *           For partial completion status changes, this figure represents
    *           what percent (0-100) of the activity has cumulatively been
    *           completed.
    *  @param   gran
    *           Reporting level granularity, allowing clients to filter out
    *           activity changes they are not interested in. For example,
    *           most clients would ignore debugging mode notifications.
    */
   virtual void getProgress(std::string& text, int& percent, ReportingLevel& gran) const = 0;

protected:
   /**
    * This should be destroyed by calling UtilityServices::destroyProgress.
    */
   virtual ~Progress() {}
};

#endif
