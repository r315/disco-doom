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
// $Log:$
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: m_bbox.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";


#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"
#include "d_net.h"
#include "g_game.h"
#include "target.h"
#include "i_system.h"

static ticcmd_t	emptycmd;

int I_strncasecmp(char *str1, char *str2, int len)
{
	char c1, c2;

	while ( *str1 && *str2 && len-- ) {
		c1 = *str1++;
		c2 = *str2++;
		if ( toupper(c1) != toupper(c2) )
			return(1);
	}
	return(0);
}

ticcmd_t* I_BaseTiccmd(void)
{
    return &emptycmd;
}

//
// I_GetTime
// returns time in 1/35 second tics
//
int  I_GetTime (void)
{
	return (T_GetTick()*TICRATE) / 1000;
}

//
// I_Init
//
void I_Init (void)
{  
}

//
// I_Quit
//
void I_Quit (void)
{
    D_QuitNetGame ();
    I_ShutdownSound();
    I_ShutdownMusic();
    M_SaveDefaults ();
    I_ShutdownGraphics();
    exit(0);
}

void I_WaitVBL(int count)
{
    T_Delay((count*1000)/70);
}

void I_BeginRead(void)
{
}

void I_EndRead(void)
{
}

byte* I_AllocLow(int length)
{
    byte*	mem;
    mem = (byte *)calloc (length, 1);

    if(!mem){
        I_Error("I_AllocLow: malloc fail");
    }

    return mem;
}


//
// I_Error
//
void I_Error (char *error, ...)
{
    va_list	argptr;

    // Message first.
    va_start (argptr,error);
    fprintf (stderr, "Error: ");
    vfprintf (stderr,error,argptr);
    fprintf (stderr, "\n");
    va_end (argptr);

    fflush( stderr );

    // Shutdown. Here might be other errors.
    if (demorecording)
	G_CheckDemoStatus();

    D_QuitNetGame ();
    I_ShutdownGraphics();
	__debugbreak();
    exit(-1);
}
