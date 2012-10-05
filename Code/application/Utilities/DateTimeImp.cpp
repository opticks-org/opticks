/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppConfig.h"
#include "DateTimeImp.h"
#include "TimeUtilities.h"

#include <locale>
#include <sstream>
#include <time.h>

using namespace std;

DateTimeImp::DateTimeImp() :
   mTime(0),
   mOnlyDateIsValid(false)
{
}

DateTimeImp::DateTimeImp(const DateTimeImp& rhs)
{
   *this = rhs;
}

DateTimeImp::DateTimeImp(const time_t& fromTime) :
   mTime(static_cast<unsigned int>(fromTime) + TimeUtilities::TimeScaleOffset),
   mOnlyDateIsValid(false)
{
}

DateTimeImp::DateTimeImp(const string& fromTime) :
   mTime(0),
   mOnlyDateIsValid(false)
{
   set(fromTime);
}

DateTimeImp::~DateTimeImp()
{
}

DateTimeImp& DateTimeImp::operator =(const DateTimeImp& rhs)
{
   if (this != &rhs)
   {
      mTime = rhs.mTime;
      mOnlyDateIsValid = rhs.mOnlyDateIsValid;
   }

   return *this;
}

bool DateTimeImp::operator ==(const DateTimeImp& rhs) const
{
   if (mOnlyDateIsValid != rhs.mOnlyDateIsValid)
   {
      return false;
   }

   return (difftime(mTime, rhs.mTime) == 0.0);
}

bool DateTimeImp::operator !=(const DateTimeImp& rhs) const
{
   if (mOnlyDateIsValid != rhs.mOnlyDateIsValid)
   {
      return true;
   }

   return (difftime(mTime, rhs.mTime) != 0.0);
}

string DateTimeImp::getFormattedLocal(const string& fmt) const
{
   // Calculate a time zone offset for the member time
   int64_t timezoneOffset = 0;

   time_t utc1970Time = 0;
   if (mTime >= TimeUtilities::TimeScaleOffset)
   {
      utc1970Time = getStructured();
   }
   else
   {
      // Select an arbitrary date not in daylight savings time
      DateTimeImp tempDate;
      tempDate.set(2000, 1, 1);

      utc1970Time = tempDate.getStructured();
   }

   struct tm* pLocalTime = localtime(&utc1970Time);
   if (pLocalTime != NULL)
   {
      int64_t local1970Time = TimeUtilities::timeStructToSecondsFrom1940(*pLocalTime) - TimeUtilities::TimeScaleOffset;
      timezoneOffset = local1970Time - utc1970Time;
   }

   struct tm timeStruct = TimeUtilities::secondsFrom1940ToTimeStruct(static_cast<unsigned int>(mTime + timezoneOffset));
   char buff[255];

   strftime(buff, sizeof(buff), fmt.c_str(), &timeStruct);
   return string(buff);
}

string DateTimeImp::getFormattedUtc(const string& fmt) const
{
   struct tm timeStruct = TimeUtilities::secondsFrom1940ToTimeStruct(mTime);
   char buff[255];

   strftime(buff, sizeof(buff), fmt.c_str(), &timeStruct);
   return string(buff);
}

time_t DateTimeImp::getStructured() const
{
   return mTime - TimeUtilities::TimeScaleOffset;
}

double DateTimeImp::getSecondsSince(const DateTime& other) const
{
   const DateTimeImp* pTemp = dynamic_cast<const DateTimeImp*>(&other);
   if (pTemp == NULL)
   {
      return 0.0;
   }
   return difftime(mTime, pTemp->mTime);
}

void DateTimeImp::setStructured(time_t fromTime)
{
   mTime = static_cast<unsigned int>(fromTime) + TimeUtilities::TimeScaleOffset;
   mOnlyDateIsValid = false;
}

void DateTimeImp::setToCurrentTime()
{
   time_t lTime = mTime;
   time(&lTime);

   mTime = static_cast<unsigned int>(lTime + TimeUtilities::TimeScaleOffset);
   mOnlyDateIsValid = false;
}

bool DateTimeImp::set(unsigned short year, unsigned short month, unsigned short day,
                      unsigned short hour, unsigned short min, unsigned short second)
{
   static unsigned char sMonthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
   mTime = 0;

   // Check for invalid dates
   if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31 ||
       hour > 23 || min > 59 || second > 59)
   {
      return false;
   }

   if ((month == 2) && (day == 29))
   {
      if ((year % 4) == 0)
      {
         if ((year % 100) == 0)
         {
            if ((year % 400) != 0)
            {
               return false;
            }
         }
      }
      else
      {
         return false;
      }
   }
   else
   {
      if (day > sMonthDays[month - 1])
      {
         return false;
      }
   }

   struct tm tStruct;
   tStruct.tm_year = year - 1900;
   tStruct.tm_mon = month - 1;
   tStruct.tm_mday = day;
   tStruct.tm_hour = hour;
   tStruct.tm_min = min;
   tStruct.tm_sec = second;
   tStruct.tm_isdst = 0;
   tStruct.tm_wday = 0;
   tStruct.tm_yday = 0;

   mTime = TimeUtilities::timeStructToSecondsFrom1940(tStruct);
   mOnlyDateIsValid = false;
   return true;
}

bool DateTimeImp::set(unsigned short year, unsigned short month, unsigned short day)
{
   bool success = set(year, month, day, 0, 0, 0);
   if (success == true)
   {
      mOnlyDateIsValid = true;

   }
   return success;
}

bool DateTimeImp::set(const std::string &fromTime)
{
   stringstream ft(fromTime);
   unsigned short year = 0;
   unsigned short month = 0;
   unsigned short day = 0;
   unsigned short hour = 0;
   unsigned short min = 0;
   unsigned short sec = 0;
   char dummy = '\0';
   ft >> year >> dummy
      >> month >> dummy
      >> day;
   if (ft.fail())
   {
      return false;
   }
   bool haveTime = false;
   ft >> dummy;
   if (!ft.fail())
   {
      haveTime = true;
      ft >> hour >> dummy
      >> min >> dummy
      >> sec >> dummy;
      if (ft.fail())
      {
         return false;
      }
   }

   if (haveTime)
   {
      return set(year, month, day, hour, min, sec);
   }
   else
   {
      return set(year, month, day);
   }
}

bool DateTimeImp::isTimeValid() const
{
   if (mTime != 0)
   {
      return !mOnlyDateIsValid;
   }

   return false;
}

bool DateTimeImp::isValid() const
{
   return (mTime != 0);
}

int DateTimeImp::getMonth(const string& month)
{
   static const char* const sShortMonthNames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
   static const char* const sLongMonthNames[] = { "January", "February", "March", "April", "May", "June",
      "July", "August", "September", "October", "November", "December" };

   for (int i = 0; i < 12; i++)
   {
      if ((month == sShortMonthNames[i]) || (month == sLongMonthNames[i]))
      {
         return i + 1;
      }
   }

   return 0;
}

int DateTimeImp::getDay(const string& day)
{
   static const char* const sShortDayNames[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
   static const char* const sLongDayNames[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
      "Friday", "Saturday" };

   for (int i = 0; i < 7; i++)
   {
      if ((day == sShortDayNames[i]) || (day == sLongDayNames[i]))
      {
         return i + 1;
      }
   }

   return 0;
}
