/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SIMPLEAPIERRORS_H__
#define SIMPLEAPIERRORS_H__

#include "AppConfig.h"

#ifdef __cplusplus
extern "C"
{
#endif
   /** \defgroup simple_api Simple datacentric API */
   /*@{*/

   /**
    * @file SimpleApiErrors.h
    * Error reporting for the simple API.
    */

   /**
    * Get the error code for the most recent error.
    *
    * @return The error code for the most recent error.
    *
    * @see getErrorString()
    */
   EXPORT_SYMBOL int getLastError();

   /**
    * Translate an error code into a string which can be displayed to the user.
    *
    * @param code
    *        The error code to translate. Usually obtained with getLastError().
    *
    * @return A string which can be displayed to the user. This is a static string and should not be freed.
    *
    * @see getLastError()
    */
   EXPORT_SYMBOL const char* getErrorString(int code);

   /**
    * Set the error code for the most recent error.
    *
    * If an API function wishes to define a new code, an appropriate \#define
    * should be added to SimpleApiErrors and a message should be added to
    * SimpleApiError.cpp.
    *
    * @param code
    *        The error code to set.
    */
   EXPORT_SYMBOL void setLastError(int code);

   /** No error occurred */
   #define SIMPLE_NO_ERROR 0
   /** Wrong or invalid type */
   #define SIMPLE_WRONG_TYPE 1
   /** Requested item not found */
   #define SIMPLE_NOT_FOUND 2
   /** Bad parameters specified */
   #define SIMPLE_BAD_PARAMS 3
   /** Buffer is too small */
   #define SIMPLE_BUFFER_SIZE 4
   /** Not enough memory to process the request */
   #define SIMPLE_NO_MEM 5
   /** The element already exists */
   #define SIMPLE_EXISTS 6
   /** A SpatialDataView is required.
       This is a common View/Layer error so it is distinct from SIMPLE_WRONG_TYPE */
   #define SIMPLE_WRONG_VIEW_TYPE 7
   /** Unknown or unclassified error */
   #define SIMPLE_OTHER_FAILURE -1

   /*@}*/
#ifdef __cplusplus
}
#endif

#endif