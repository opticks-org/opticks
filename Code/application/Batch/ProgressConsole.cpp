/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#include <iostream>
#include "ProgressConsole.h"

using namespace std;

ProgressConsole::ProgressConsole()
{
   mMessage = "";
   mPercentDone = 0;
   mReportLevel = NORMAL;
}

ProgressConsole::~ProgressConsole()
{
   cout << endl;
}

void ProgressConsole::updateProgress(const string& text, int percent, ReportingLevel status)
{
   char number[50];
   bool same = false;
   bool overwrite = false;

   if (mMessage.compare(text) == 0)
   {
      same = true;
   }

   if ((same) && (percent == mPercentDone)) return;

   if ((percent < 0) || (percent > 100) || (status == ERRORS) || (status == ABORT) || (status == WARNING))
   {
      if (!mMessage.empty())
      {
         cout << "\r      - " << mMessage.c_str() << endl;
      }

      if (status == ERRORS)
      {
         cout << "ERROR - ";
      }
      else if (status == ABORT)
      {
         cout << "ABORT - ";
      }
      else if (status == WARNING)
      {
         cout << "WARN  - ";
      }
      else
      {
         cout << "INFO  - ";
      }

      cout << text << endl;

      mMessage.erase();
      return;
   }

   if ((same) && ((percent > mPercentDone) || (percent == 100)))
   {
      overwrite = true;
   }

   if (overwrite)
   {
      cout << "\r";
   }
   else
   {
      if (!mMessage.empty())
      {
         cout << "\r      - " << mMessage.c_str() << endl;
      }
   }

   sprintf(number, "%4d%% - ", percent);
   cout << number << text;

   if (percent == 100)
   {
      cout << endl;
      mMessage.erase();
   }
   else
   {
      mMessage = text;
   }

   mPercentDone = percent;
   mReportLevel = status;
}

void ProgressConsole::getProgress(string& text, int& percent, ReportingLevel& status) const
{
   text = mMessage;
   percent = mPercentDone;
   status = mReportLevel;
}

const string& ProgressConsole::getObjectType() const
{
   static string type("ProgressConsole");
   return type;
}

bool ProgressConsole::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Progress"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}
