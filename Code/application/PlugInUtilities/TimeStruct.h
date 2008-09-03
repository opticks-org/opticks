/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */
 
#ifndef TIME_STRUCT
#define TIME_STRUCT

#include <time.h>

namespace TimeStruct
{

const long TimeScaleOffset = 946771200; // seconds between 1/1/1940 and 1/1/1970.


unsigned int ToSam(struct tm *pTimeStruct);
struct tm *FromSam(unsigned int sam);
bool IsLeapYear (int year);

}

#endif

 
