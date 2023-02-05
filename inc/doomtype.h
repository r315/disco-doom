// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// DESCRIPTION:
//	Simple basic typedefs, isolated here to make it easier
//	 separating modules.
//    
//-----------------------------------------------------------------------------


#ifndef __DOOMTYPE__
#define __DOOMTYPE__


typedef unsigned char  byte;
typedef signed char    sbyte;
typedef unsigned short u16;
typedef signed short   s16;
typedef unsigned int   u32;
typedef signed int     s32;

/* Fixed to use builtin boolean type with C++. */
#ifdef __cplusplus
typedef bool boolean;
#else
#define false 0
#define true  1
typedef unsigned int boolean;
#endif

// Predefined with some OS.
#ifdef LINUX
	#include <values.h>
#else
	#ifndef MAXCHAR
	#define MAXCHAR		((char)0x7f)
#endif

#ifndef MAXSHORT
	#define MAXSHORT	((short)0x7fff)
#endif

// Max pos 32-bit int.
#ifndef MAXINT
	#define MAXINT		((int)0x7fffffff)	
#endif

#ifndef MAXLONG
	#define MAXLONG		((long)0x7fffffff)
#endif

#ifndef MINCHAR
	#define MINCHAR		((char)0x80)
#endif

#ifndef MINSHORT
	#define MINSHORT	((short)0x8000)
#endif

// Max negative 32-bit integer.
#ifndef MININT
	#define MININT		((int)0x80000000)	
#endif

#ifndef MINLONG
	#define MINLONG		((long)0x80000000)
#endif

#endif




#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
