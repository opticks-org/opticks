/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 
#ifndef TIMEUTILITIES_H
#define TIMEUTILITIES_H

#include <time.h>

/**
 * Convenience functions when using time-based objects.
 */
namespace TimeUtilities
{
   /**
    * The number of seconds between midnight 1 Jan 1940 and midnight 1 Jan 1970.
    */
   const long TimeScaleOffset = 946771200;

   /**
    * Converts a time struct to seconds from 1940.
    *
    * @param   timeStruct
    *          The time struct from which to get seconds.
    *
    * @return  Returns the number of seconds after midnight 1 Jan 1940.  To
    *          convert this value to a time_t (seconds from 1970) object,
    *          simply subtract TimeUtilities::TimeScaleOffset.
    */
   unsigned int timeStructToSecondsFrom1940(const struct tm& timeStruct);

   /**
    * Converts seconds from 1940 to a time struct.
    *
    * @param   seconds
    *          The number of seconds after midnight 1 Jan 1940 from which to get
    *          a time struct.  To convert a time_t (seconds from 1970) to this
    *          value, simply add TimeUtilities::TimeScaleOffset.
    *
    * @return  The time struct based on the given seconds from 1940.
    */
   struct tm secondsFrom1940ToTimeStruct(unsigned int seconds);

   /**
    * Queries whether a given year is a leap year.
    *
    * @param   year
    *          The four-digit year to query if it is a leap year.
    *
    * @return  Returns \c true if the given year is a leap year; otherwise
    *          returns \c false.
    */
   bool isLeapYear(int year);
}

#endif
