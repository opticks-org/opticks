/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DATETIME_H
#define DATETIME_H

#include <time.h>
#include <string>

#include "DataVariantValidator.h"

/**
 *  Specifies a date and a time.
 *
 *  Base class for date and time object, based upon the 
 *  capability provided by the standard C library's <time.h>.
 */
class DateTime
{
public:
   /**
    *  Gets the time in local coordinates, formatted as a string.
    *
    *  @warning This method uses the C library to perform UTC to
    *  local time conversions and on some versions of Windows this
    *  can incorrectly calculate daylight savings time for
    *  historical dates (ie. dates prior to the 2007 Daylight
    *  Savings Time switch)
    *
    *  @param   fmt
    *           A format specification, consistent with that documented
    *           for strftime in <time.h>
    *  @return  The contents of this date/time object, formatted as 
    *           requested by the client into a string.
    */
   virtual std::string getFormattedLocal(const std::string& fmt) const = 0;

   /**
    *  Gets the time in UTC coordinates, formatted as a string.
    *
    *  @param   fmt
    *           A format specification, consistent with that documented
    *           for strftime in <time.h>
    *  @return  The contents of this date/time object, formatted as 
    *           requested by the client into a string.
    */
   virtual std::string getFormattedUtc(const std::string& fmt) const = 0;

   /**
    *  Gets the time value as a value maniuplated by C.
    *
    *  @return  The time_t value that can be passed to traditional
    *           C runtime library functions.
    */
   virtual time_t getStructured() const = 0;

   /**
    *  Gets the time elapsed between this object and the argument object.
    *
    *  @return  The difference, in seconds, between the two date/time values.
    */
   virtual double getSecondsSince(const DateTime& other) const = 0;

   /**
    *  Resets this object to have the current operating system date and time.
    */
   virtual void setToCurrentTime() = 0;
   
   /**
    *  Sets the time value as a value manipulated by C.
    *
    *  @param val
    *           The time_t value that can be passed to traditional
    *           C runtime library functions.
    */
   virtual void setStructured(time_t val) = 0;

   /**
    *  Sets the date and time to explicit client values.
    *
    *  @param   year
    *           The four-digit year. Note that years greater than 2047
    *           or less than 1900 are problematical with C.
    *  @param   month
    *           Starting from 1 for January, up to 12 for December.
    *  @param   day
    *           The day within the month, starting from 1.
    *  @param   hour
    *           The hour within the day, from 0 up to 23.
    *  @param   minute
    *           The minute within the hour, from 0 to 59.
    *  @param   second
    *           The seconds within the minute, from 0 to 59.
    *
    *  @return  A boolean indicating if the full set of values was
    *           an acceptable date time, including checks for leap
    *           years, month-day limits, and so on.
    */
   virtual bool set(unsigned short year, unsigned short month,
      unsigned short day, unsigned short hour,
      unsigned short minute, unsigned short second) = 0;

   /**
    *  Sets the date and time to explicit client values.
    *
    *  @param   year
    *           The four-digit year. Note that years greater than 2047
    *           or less than 1900 are problematical with C.
    *  @param   month
    *           Starting from 1 for January, up to 12 for December.
    *  @param   day
    *           The day within the month, starting from 1.
    *
    *  @return  A boolean indicating if the full set of values was
    *           an acceptable date time, including checks for leap
    *           years, month-day limits, and so on.
    */
   virtual bool set(unsigned short year = 2000, unsigned short month = 01,
      unsigned short day = 01) = 0;

   /**
    *  Set the date and time from a specially formatted string.
    *
    *  @param   fromTime
    *           A string in a sub-set of ISO8601 format. No validation is performed
    *           on this string. This function will also accept strings that
    *           omit the time information, ie. from the 'T' onward.
    *           The lexical description is:
    *           yyyy '-' mm '-' dd 'T' hh ':' MM ':' ss 'Z'
    *           where:
    *              strings in '' are literal characters
    *              yyyy is a four-or-more digit numeral representing the year.
    *                   0000 is not valid and this function does not accept
    *                   negative years even though they are part of ISO8601
    *              mm is a two-digit numeral representing the month
    *              dd is a two-digit numeral representing the day
    *              hh is a two-digit numeral representing the hour. 24 is permitted if
    *                   hours and minutes are 00 and 00
    *              MM is a two-digit numeral representing the minute
    *              ss is a two-digit numeral representing the whole second
    *
    *  @return  true if successfull, false otherwise
    */
   virtual bool set(const std::string &fromTime) = 0;

   /**
    *  Queries whether the time component is valid.
    *
    *  The time component of the DateTime object will only be invalid if the
    *  date was most recently set with the set method that only takes the date.
    *
    *  @return  True if the time component is valid, otherwise false.
    *
    *  @see     set()
    */
   virtual bool isTimeValid() const = 0;

   /**
    *  Queries whether the DateTime is valid.
    *
    *  @return  True if the DateTime is valid, otherwise false.
    *
    *  @see     set()
    */
   virtual bool isValid() const = 0;

protected:
   /**
    * This should be destroyed by calling ObjectFactory::destroyObject.
    */
   virtual ~DateTime() {}
};

/**
 * \cond INTERNAL
 * These template specialization are required to allow these types to be put into a DataVariant.
 */
template <> class VariantTypeValidator<DateTime> {};
template <> class VariantTypeValidator<const DateTime> {};
/// \endcond

#endif
