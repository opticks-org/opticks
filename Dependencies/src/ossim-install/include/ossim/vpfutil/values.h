/* Old compatibility names for <limits.h> and <float.h> constants.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 51 Franklin Street,
   Fifth Floor, Boston, MA 02110-1301 USA  */

/* This interface is obsolete.  New programs should use
   <limits.h> and/or <float.h> instead of <values.h>.  */

#ifndef	_VALUES_H
#define	_VALUES_H	1

#include <limits.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifdef MAXSHORT
#undef MAXSHORT
#endif
#ifdef MAXFLOAT
#undef MAXFLOAT
#endif

#define _TYPEBITS(type)	(sizeof (type) * CHAR_BIT)

#define CHARBITS	_TYPEBITS (char)
#define SHORTBITS	_TYPEBITS (short int)
#define INTBITS		_TYPEBITS (int)
#define LONGBITS	_TYPEBITS (long int)
#define PTRBITS		_TYPEBITS (char *)
#define DOUBLEBITS	_TYPEBITS (double)
#define FLOATBITS	_TYPEBITS (float)

#ifndef MINSHORT
#  define   MINSHORT	SHRT_MIN
#endif
#ifndef MININT
#  define	MININT		INT_MIN
#endif
#ifndef MINLONG
#  define	MINLONG		LONG_MIN
#endif

#ifndef MAXSHORT
#  define	MAXSHORT	SHRT_MAX
#endif
#ifndef MAXINT
#  define	MAXINT		INT_MAX
#endif
#ifndef MAXLONG
#  define	MAXLONG		LONG_MAX
#endif

#define HIBITS		MINSHORT
#define HIBITL		MINLONG


#include <float.h>

#define	MAXDOUBLE	DBL_MAX
#define	MAXFLOAT	FLT_MAX
#define	MINDOUBLE	DBL_MIN
#define	MINFLOAT	FLT_MIN
#define	DMINEXP		DBL_MIN_EXP
#define	FMINEXP		FLT_MIN_EXP
#define	DMAXEXP		DBL_MAX_EXP
#define	FMAXEXP		FLT_MAX_EXP


#ifdef __USE_MISC
/* Some systems define this name instead of CHAR_BIT or CHARBITS.  */
# define BITSPERBYTE	CHAR_BIT
#endif
#ifdef __cplusplus
}
#endif

#endif	/* values.h */
