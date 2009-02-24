/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef SWITCHONENCODING_H
#define SWITCHONENCODING_H

#include "ComplexData.h"

#define switchOnEncoding(encoding,function,...) \
   switch (encoding) \
   { \
   case INT1UBYTE: \
      function((unsigned char*)__VA_ARGS__); \
      break; \
   case INT1SBYTE: \
      function((signed char*)__VA_ARGS__); \
      break; \
   case INT2UBYTES: \
      function((unsigned short*)__VA_ARGS__); \
      break; \
   case INT2SBYTES: \
      function((signed short*)__VA_ARGS__); \
      break; \
   case INT4UBYTES: \
      function((unsigned int*)__VA_ARGS__); \
      break; \
   case INT4SBYTES: \
      function((signed int*)__VA_ARGS__); \
      break; \
   case FLT4BYTES: \
      function((float*)__VA_ARGS__); \
      break; \
   case FLT8BYTES: \
      function((double*)__VA_ARGS__); \
      break; \
   default: \
      break; \
   }

#define switchOnComplexEncoding(encoding,function,...) \
   switch (encoding) \
   { \
   case INT1UBYTE: \
      function((unsigned char*)__VA_ARGS__); \
      break; \
   case INT1SBYTE: \
      function((signed char*)__VA_ARGS__); \
      break; \
   case INT2UBYTES: \
      function((unsigned short*)__VA_ARGS__); \
      break; \
   case INT2SBYTES: \
      function((signed short*)__VA_ARGS__); \
      break; \
   case INT4SCOMPLEX: \
      function((IntegerComplex*)__VA_ARGS__); \
      break; \
   case INT4UBYTES: \
      function((unsigned int*)__VA_ARGS__); \
      break; \
   case INT4SBYTES: \
      function((signed int*)__VA_ARGS__); \
      break; \
   case FLT4BYTES: \
      function((float*)__VA_ARGS__); \
      break; \
   case FLT8COMPLEX: \
      function((FloatComplex*)__VA_ARGS__); \
      break; \
   case FLT8BYTES: \
      function((double*)__VA_ARGS__); \
      break; \
   default: \
      break; \
   }

#endif
