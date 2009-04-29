/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef APPCONFIG_H
#define APPCONFIG_H

#define BIG_ENDIAN_BYTE_ORDER 4321
#define LITTLE_ENDIAN_BYTE_ORDER 1234

#if defined(_MSC_VER)
   #define WIN_API

   #define BYTE_ORDER LITTLE_ENDIAN_BYTE_ORDER
   #define LONG_SIZE 4
   typedef __int16 int16_t;
   typedef __int32 int32_t;
   typedef __int64 int64_t;
   typedef unsigned __int16 uint16_t;
   typedef unsigned __int32 uint32_t;
   typedef unsigned __int64 uint64_t;
   #if defined(_WIN64)
      //check for _WIN64 first, since on x64
      //both are defined.
      #define PTR_SIZE 8
      #define BROKEN_INLINE_HINT __declspec(noinline)
   #elif defined(_WIN32)
      #define PTR_SIZE 4
   #endif

   #define CG_SUPPORTED

   //platform defines
   #define SLASH std::string("\\")
   #define EXE_EXTENSION std::string(".exe")
   #define FILENO _fileno
   #define LINKAGE __declspec(dllexport)
   #define HANDLE_TYPE HANDLE
   #define MICRON std::string("µ")
   #define DEG_CHAR std::string("°")

   #define snprintf _snprintf

   #include <stddef.h>
#elif defined (__SUNPRO_CC)
   #include <sys/isa_defs.h>

   #define UNIX_API
   #define SOLARIS

   #ifdef _BIG_ENDIAN
      #define BYTE_ORDER BIG_ENDIAN_BYTE_ORDER
   #else
      #define BYTE_ORDER LITTLE_ENDIAN_BYTE_ORDER
   #endif

   #define PTR_SIZE _POINTER_ALIGNMENT
   #define LONG_SIZE _LONG_ALIGNMENT


   //platform defines
   #define O_TEXT O_BINARY
   #define SLASH std::string("/")
   #define EXE_EXTENSION std::string("")
   #define FILENO fileno
   #define LINKAGE
   #define HANDLE_TYPE int
   #define MICRON std::string("u")
   #define DEG_CHAR std::string("*")

   //allow user to use __stdcall on unix and do the correct thing
   #define __stdcall

   #include <sys/int_types.h>
#else
   #error Unrecognized build platform
#endif

#if !defined(BROKEN_INLINE_HINT)
#define BROKEN_INLINE_HINT
#endif

#define STRING2(x) #x
#define STRING(x) STRING2(x)

typedef intptr_t word_t;
typedef uintptr_t uword_t;

#define PI    3.14159265358979323e0  // PI

#endif
