/*  $Id: ossimConfig.h.in 13607 2008-09-25 19:39:45Z gpotts $ */
#ifndef ossimConfig_HEADER
#define ossimConfig_HEADER

/* Define to "1" if you have libz installed, "0" if not. */
#define OSSIM_HAS_LIBZ 1

/* Define to "1" if you have MPI(lam/mpi), "0" if not. */
#define OSSIM_HAS_MPI 0

/* Define to "1" if you have FREETYPE, "0" if not. */
#define OSSIM_HAS_FREETYPE 1

/* Define to "1" if you have GEOTIFF, "0" if not. */
#define OSSIM_HAS_GEOTIFF 1

/* Define to "1" if you have HDF5 installed, "0" if not. */
#define OSSIM_HAS_HDF5 0

/* Enable cvs id strings for use with "ident" application. */
#define OSSIM_ID_ENABLED 1

/* The sizes of various standard integer types. */
#define SIZE_OF_SHORT_INT 2
#define SIZE_OF_INT 4
#define SIZE_OF_LONG 4
#define SIZE_OF_LONG_LONG 8
#define SIZE_OF_UNSIGNED_SHORT_INT 2
#define SIZE_OF_UNSIGNED_INT 4
#define SIZE_OF_UNSIGNED_LONG 4
#define SIZE_OF_UNSIGNED_LONG_LONG 8
#define SIZE_OF_DOUBLE 8
#define SIZE_OF_FLOAT 4
#define SIZE_OF_LONG_DOUBLE 8



typedef char                   ossim_int8;
typedef unsigned char          ossim_uint8;
typedef signed char            ossim_sint8;

#if SIZE_OF_SHORT_INT == 2
typedef short ossim_int16;
typedef signed short ossim_sint16;
typedef unsigned short ossim_uint16;
#define OSSIM_HAVE_INT16_T
#define OSSIM_HAVE_UINT16_T
#elif SIZE_OF_INT == 2
typedef int ossim_int16;
typedef signed int ossim_sint16;
typedef unsigned int ossim_uint16;
#define OSSIM_HAVE_INT16_T
#endif

#if SIZE_OF_INT == 4
typedef int ossim_int32;
typedef signed int ossim_sint32;
typedef unsigned int ossim_uint32;
#define OSSIM_HAVE_INT32_T
#define OSSIM_HAVE_UINT32_T
#elif SIZE_OF_LONG == 4
typedef long ossim_int32;
typedef signed long ossim_sint32;
typedef unsigned long ossim_uint32;
#define OSSIM_HAVE_INT32_T
#define OSSIM_HAVE_UINT32_T

#endif

#if !defined(OSSIM_HAVE_INT32_T)
#error No 32-bit integer type was found.
#endif

#if SIZE_OF_LONG_LONG == 8
typedef long long ossim_int64;
typedef unsigned long long ossim_uint64;
typedef signed long long ossim_sint64;
#define OSSIM_HAVE_INT64_T
#define OSSIM_HAVE_UINT64_T
#elif SIZE_OF_INT == 8
typedef int ossim_int64;
typedef signed int ossim_sint64;
typedef unsigned int ossim_uint64;
#define OSSIM_HAVE_INT64_T
#define OSSIM_HAVE_UINT64_T
#elif SIZE_OF_LONG == 8
typedef long ossim_int64;
typedef unsigned long ossim_uint64;
typedef signed long ossim_sint64;
#define OSSIM_HAVE_INT64_T
#define OSSIM_HAVE_UINT64_T
#endif

#if !defined(OSSIM_HAVE_INT64_T)
#error No 64-bit integer type was found.
#endif

#if SIZE_OF_DOUBLE == 8
typedef double ossim_float64;
#define OSSIM_HAVE_FLOAT64_T
#endif

#if !defined(OSSIM_HAVE_FLOAT64_T)
#error No 64-bit integer type was found.
#endif

#if SIZE_OF_FLOAT == 4
typedef float ossim_float32;
#define OSSIM_HAVE_FLOAT32_T
#endif

#if !defined(OSSIM_HAVE_FLOAT32_T)
#error No 64-bit integer type was found.
#endif

#if SIZE_OF_LONG_DOUBLE == 16
typedef float ossim_float128;
#define OSSIM_HAVE_FLOAT128_T
#endif

#endif /* End of "#ifndef ossimConfig_HEADER" */
