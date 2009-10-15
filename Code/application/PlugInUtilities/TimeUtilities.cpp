/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "TimeUtilities.h"

static const unsigned int secondsPerDay = 24 * 3600;
static const unsigned int daysPerMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

unsigned int TimeUtilities::timeStructToSecondsFrom1940(const struct tm& timeStruct)
{
   if (timeStruct.tm_year < 40 || timeStruct.tm_year > 175)
   {
      return 0;
   }

   unsigned int years = timeStruct.tm_year + 1900 - 1940;
   unsigned int daysThisYear = 0;
   int i;
   for (i = 0; i < timeStruct.tm_mon; ++i)
   {
      daysThisYear += daysPerMonth[i];
   }
   daysThisYear += (timeStruct.tm_mday - 1);

   if (timeStruct.tm_mon > 1 && isLeapYear(years + 1940))
   {
      daysThisYear++;
   }

   int leapYears = 0;
   for (i = 1940; i < static_cast<int>(years + 1940); ++i)
   {
      if (isLeapYear(i))
      {
         leapYears++;
      }
   }
   unsigned int days = years * 365 + leapYears + daysThisYear;
   unsigned int seconds = days * secondsPerDay + 
      timeStruct.tm_hour * 3600 + timeStruct.tm_min * 60 +
      timeStruct.tm_sec;

   return seconds;
}

struct tm TimeUtilities::secondsFrom1940ToTimeStruct(unsigned int seconds)
{
   struct tm timeStruct;
   unsigned int hourSeconds = seconds % 60;     // 60 == seconds / minute
   unsigned int minutes = (seconds / 60) % 60;  // 60 == minutes / hour
   unsigned int hours = (seconds / 3600) % 24;  // 24 == hours / day;
   unsigned int days = (seconds / secondsPerDay) % 365;
   unsigned int year;
   timeStruct.tm_hour = hours;
   timeStruct.tm_min = minutes;
   timeStruct.tm_sec = hourSeconds;
   unsigned int totalDays = seconds / secondsPerDay;
   timeStruct.tm_wday = (1 + totalDays) % 7;    // 1 Jan 1940 is a Monday, hence the '+1'
   int i;
   int leapYears = 0; // used in day-of-week computation
   for (i = 1940; totalDays > 0; ++i)
   {
      unsigned int daysThisYear = 365;
      if (isLeapYear(i))
      {
         daysThisYear++;
         leapYears++;
      }
      if (daysThisYear > totalDays)
      {
         break;
      }
      else
      {
         totalDays -= daysThisYear;
      }
   }
   if (isLeapYear(i) && totalDays > (31 + 29))
   {
      leapYears++;
   }
   year = i;
   timeStruct.tm_year = i - 1900;
   timeStruct.tm_yday = totalDays;
   timeStruct.tm_isdst = 0;
   int monthDaySum = 0;
   for (i = 0; i < 12; ++i)
   {
      unsigned int daysThisMonth = daysPerMonth[i];
      if (i == 1 && isLeapYear(year))
      {
         daysThisMonth++;
      }
      if (totalDays >= daysThisMonth)
      {
         totalDays -= daysThisMonth;
      }
      else
      {
         break;
      }
   }
   timeStruct.tm_mday = totalDays + 1;
   timeStruct.tm_mon = i;

   return timeStruct;
}

bool TimeUtilities::isLeapYear(int year)
{
   return (year % 4 == 0 && (year % 400 == 0 || year % 100 != 0));
}
