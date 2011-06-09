/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "SimpleApiErrors.h"

namespace
{
   static int sLastError = SIMPLE_NO_ERROR; 
}

extern "C"
{

int getLastError()
{
   int rval = sLastError;
   sLastError = SIMPLE_NO_ERROR;
   return rval;
}

const char* getErrorString(int code)
{
   switch(code)
   {
   case SIMPLE_NO_ERROR:
      return "";
   case SIMPLE_WRONG_TYPE:
      return "Wrong or invalid type";
   case SIMPLE_NOT_FOUND:
      return "Requested item not found";
   case SIMPLE_BAD_PARAMS:
      return "Bad parameters specified";
   case SIMPLE_BUFFER_SIZE:
      return "Buffer is too small";
   case SIMPLE_NO_MEM:
      return "Not enough memory to process the request";
   case SIMPLE_EXISTS:
      return "The element already exists";
   case SIMPLE_WRONG_VIEW_TYPE:
      return "The requested action requires a SpatialDataView.";
   case SIMPLE_OTHER_FAILURE:
      return "Unknown or unclassified error";
   default:
      return "Unknown error";
   }
}

void setLastError(int code)
{
   sLastError = code;
}

};