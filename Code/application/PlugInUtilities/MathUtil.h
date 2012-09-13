/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef MATH_UTIL_H
#define MATH_UTIL_H

/**
 * This will convert the double into an int by rounding up if the decimal is 0.5 or greater
 * and round down if the decimal is less than 0.5. Round up and down will work properly
 * regardless of whether the value is positive or negative.
 */
inline int roundDouble(double value)
{
   if (value < 0)
   {
      return (int)(value - 0.5);
   }
   else
   {
      return (int)(value + 0.5);
   }
}

#endif
