/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "TimeStruct.h"

static const unsigned int secondsPerDay = 24 * 3600;
static const unsigned int daysPerMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
// return 0 == Midnight, 1Jan, 1940
unsigned int TimeStruct::ToSam(struct tm* pTimeStruct)
{
   if (pTimeStruct->tm_year < 40 || pTimeStruct->tm_year > 175)
   {
      return 0;
   }

   unsigned int years = pTimeStruct->tm_year + 1900 - 1940;
   unsigned int daysThisYear = 0;
   int i;
   for (i = 0; i < pTimeStruct->tm_mon; ++i)
   {
      daysThisYear += daysPerMonth[i];
   }
   daysThisYear += (pTimeStruct->tm_mday - 1);

   if (pTimeStruct->tm_mon > 1 && IsLeapYear(years+1940))
   {
      daysThisYear++;
   }

   int leapYears = 0;
   for (i = 1940; i < static_cast<int>(years + 1940); ++i)
   {
      if (IsLeapYear(i))
      {
         leapYears++;
      }
   }
   unsigned int days = years * 365 + leapYears + daysThisYear;
   unsigned int seconds = days * secondsPerDay + 
      pTimeStruct->tm_hour * 3600 + pTimeStruct->tm_min * 60 +
      pTimeStruct->tm_sec;

   return seconds; // seconds after midnight, 1Jan, 1940
}

struct tm* TimeStruct::FromSam(unsigned int sam)
{
   static struct tm timeStruct;
   unsigned int seconds = sam % 60; // 60 == seconds / minute
   unsigned int minutes = (sam / 60) % 60; // 60 == minutes / hour
   unsigned int hours = (sam / 3600) % 24; // 24 == hours / day;
   unsigned int days = (sam / secondsPerDay) % 365;
   unsigned int year;
   timeStruct.tm_hour = hours;
   timeStruct.tm_min = minutes;
   timeStruct.tm_sec = seconds;
   unsigned int totalDays = sam / secondsPerDay;
   timeStruct.tm_wday = (1+totalDays) % 7; // 1Jan,1940 is a Monday, hence the '+1'
   int i;
   int leapYears = 0; // used in day-of-week computation
   for (i = 1940; totalDays > 0; ++i)
   {
      unsigned int daysThisYear = 365;
      if (IsLeapYear(i))
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
   if (IsLeapYear(i) && totalDays > (31 + 29))
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
      if (i == 1 && IsLeapYear(year))
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
   timeStruct.tm_mday = totalDays+1;
   timeStruct.tm_mon = i;

   return &timeStruct;
}

bool TimeStruct::IsLeapYear(int year)
{
   return (year % 4 == 0 && (year % 400 == 0 || year % 100 != 0));
}
