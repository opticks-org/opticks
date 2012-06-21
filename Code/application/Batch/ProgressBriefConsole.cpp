/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "ProgressBriefConsole.h"

#include <iostream>
using namespace std;

ProgressBriefConsole::ProgressBriefConsole(bool veryBrief) :
   mVeryBrief(veryBrief),
   mCounter(0)
{
}

ProgressBriefConsole::~ProgressBriefConsole()
{
   cout << endl;
}

void ProgressBriefConsole::updateProgress(const std::string& text, int percent, ReportingLevel gran)
{
   if ((percent < 0) || (percent > 100) || (gran == ERRORS) || (gran == ABORT) || (gran == WARNING))
   {
      cout << "\r";

      if (gran == ERRORS)
      {
         cout << "ERROR - ";
      }
      else if (gran == ABORT)
      {
         cout << "ABORT - ";
      }
      else if (gran == WARNING)
      {
         cout << "WARN  - ";
      }
      else
      {
         cout << "INFO  - ";
      }

      cout << text << endl;
      return;
   }

   if (mVeryBrief == false)
   {
      switch (mCounter)
      {
         case 0:
            cout << "\rProcessing -";
            break;
         case 10:
            cout << "\rProcessing \\";
            break;
         case 20:
            cout << "\rProcessing |";
            break;
         case 30:
            cout << "\rProcessing /";
            break;
         default:
            break;
      }

      mCounter++;
      if (mCounter >= 40)
      {
         mCounter = 0;
      }

      cout.flush();
   }
}

void ProgressBriefConsole::getProgress(string& text, int& percent, ReportingLevel& gran) const
{
   text = mMessage;
}

const string& ProgressBriefConsole::getObjectType() const
{
   static string sType("ProgressBriefConsole");
   return sType;
}

bool ProgressBriefConsole::isKindOf(const string& className) const
{
   if ((className == getObjectType()) || (className == "Progress"))
   {
      return true;
   }

   return SubjectImp::isKindOf(className);
}
