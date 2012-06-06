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
   #define EXPORT_SYMBOL __declspec(dllexport)

   #define OPTICKS_BYTE_ORDER LITTLE_ENDIAN_BYTE_ORDER
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
   #define JPEG2000_SUPPORT
   #define CG_SUPPORTED

   //platform defines
   #define SLASH std::string("\\")
   #define EXE_EXTENSION std::string(".exe")
   #define FILENO _fileno
   #define LINKAGE __declspec(dllexport)
   #define HANDLE_TYPE HANDLE
   #define MICRON std::string("µ")
   #define DEG_CHAR std::string("°")
   #define OPENCOLLADA_SUPPORT

   #define snprintf _snprintf

   #include <stddef.h>
   #define DISAMBIGUATE_TEMPLATE
   #define HIDE_UNUSED_VARIABLE_WARNING

   #define GL_CALLBACK CALLBACK
#elif defined (__SUNPRO_CC)
   #include <sys/isa_defs.h>

   #define UNIX_API
   #define SOLARIS
   #define EXPORT_SYMBOL 

   #ifdef _BIG_ENDIAN
      #define OPTICKS_BYTE_ORDER BIG_ENDIAN_BYTE_ORDER
   #else
      #define OPTICKS_BYTE_ORDER LITTLE_ENDIAN_BYTE_ORDER
   #endif

   #define PTR_SIZE _POINTER_ALIGNMENT
   #define LONG_SIZE _LONG_ALIGNMENT


   //platform defines
   #define O_TEXT 0x0000 
   #define O_BINARY 0x0000
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
   #define DISAMBIGUATE_TEMPLATE
   #define HIDE_UNUSED_VARIABLE_WARNING

   #define GL_CALLBACK
#elif defined(__linux__)
#if !defined(__amd64__)
#error "Linux support requires a 64-bit x86 (AMD64) processor"
#endif

#if !defined(__GNUC__) || __GNUC__ != 4
#error "Linux support requires version 4 of the GNU g++ compiler"
#endif

   #include <stddef.h>
   #include <stdlib.h>
   #include <sys/types.h>
   #include <stdint.h>
   #include <stdexcept>

   #include <endian.h>
   #define UNIX_API
   #define LINUX
   #define CG_SUPPORTED
   #define EXPORT_SYMBOL 

   #if __BYTE_ORDER == __LITTLE_ENDIAN
      #define OPTICKS_BYTE_ORDER LITTLE_ENDIAN_BYTE_ORDER
   #else
      #define OPTICKS_BYTE_ORDER BIG_ENDIAN_BYTE_ORDER
   #endif

   #define PTR_SIZE __SIZEOF_POINTER__
   #define LONG_SIZE __SIZEOF_LONG__

   //platform defines
   #define O_TEXT 0x0000 
   #define O_BINARY 0x0000
   #define SLASH std::string("/")
   #define EXE_EXTENSION std::string("")
   #define FILENO fileno
   #define LINKAGE
   #define HANDLE_TYPE int
   #define MICRON std::string("u")
   #define DEG_CHAR std::string("*")

   //allow user to use __stdcall on unix and do the correct thing
   #define __stdcall

   #define DISAMBIGUATE_TEMPLATE template
   #define HIDE_UNUSED_VARIABLE_WARNING __attribute__((unused))

   #define GL_CALLBACK
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
