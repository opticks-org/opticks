/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ProgressImp.h"

using namespace std;

ProgressImp::ProgressImp() :
   mPercentComplete(-1),
   mGranularity(NORMAL),
   mpPlugIn(NULL)
{
}

ProgressImp::ProgressImp(string amProgressText, int amPercentComplete, ReportingLevel amGranularity) :
   mProgressText(amProgressText),
   mPercentComplete(amPercentComplete),
   mGranularity(amGranularity),
   mpPlugIn(NULL)
{
   if (mPercentComplete > 100)
   {
      mPercentComplete = 100;
   }
}

ProgressImp::~ProgressImp()
{
}

const string& ProgressImp::getObjectType() const
{
   static string sType = "ProgressImp";
   return sType;
}

bool ProgressImp::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Progress"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}

void ProgressImp::updateProgress(const string& text, int percent, ReportingLevel gran)
{
   mProgressText = text;
   mPercentComplete = percent;
   mGranularity = gran;

   notify(SIGNAL_NAME(Subject, Modified));
}

void ProgressImp::getProgress(string& text, int& percent, ReportingLevel& gran) const
{
   text = mProgressText;
   percent = mPercentComplete;
   gran = mGranularity;
}

void ProgressImp::setPlugIn(PlugIn* pPlugIn)
{
   mpPlugIn = pPlugIn;
}

PlugIn* ProgressImp::getPlugIn() const
{
   return mpPlugIn;
}
