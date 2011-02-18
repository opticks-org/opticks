/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef PROGRESSIMP_H
#define PROGRESSIMP_H

#include "SubjectImp.h"
#include "PlugIn.h"
#include "Progress.h"

#include <string>

class ProgressImp : public SubjectImp
{
public:
   ProgressImp();
   ProgressImp(std::string amProgressText, int amPercentComplete, ReportingLevel amGranularity);
   virtual ~ProgressImp();

   virtual const std::string& getObjectType() const;
   virtual bool isKindOf(const std::string& className) const;

   /**
    * Indicate that some activity has taken place.
    *
    * @param  text
    *         Message indicating the nature of the activity being reported.
    *
    * @param  percent
    *         For partial completion status changes, this figure represents
    *         what percent (0-100) of the activity has cumulatively been
    *         completed.  A percent value of -1 will hide the progress
    *         dialog until a positive percentage is set.
    *
    * @param  gran
    *         Reporting level granularity, allowing clients to filter out
    *         activity changes they are not interested in. For example,
    *         most clients would ignore debugging mode notifications.
    */
   void updateProgress(const std::string& text, int percent, ReportingLevel gran);

   /**
    * Report the activity that has taken place.
    *
    * @param  text
    *         Message indicating the nature of the activity being reported.
    *
    * @param  percent
    *         For partial completion status changes, this figure represents
    *         what percent (0-100) of the activity has cumulatively been
    *         completed.
    *
    * @param  gran
    *         Reporting level granularity, allowing clients to filter out
    *         activity changes they are not interested in. For example,
    *         most clients would ignore debugging mode notifications.
    */
   void getProgress(std::string& text, int& percent, ReportingLevel& gran) const;

   void setPlugIn(PlugIn* pPlugIn);
   PlugIn* getPlugIn() const;

private:
   std::string mProgressText;
   int mPercentComplete;
   ReportingLevel mGranularity;
   PlugIn* mpPlugIn;
};

#define PROGRESSADAPTEREXTENSION_CLASSES \
   SUBJECTADAPTEREXTENSION_CLASSES

#define PROGRESSADAPTER_METHODS(impClass) \
   SUBJECTADAPTER_METHODS(impClass)

#endif
