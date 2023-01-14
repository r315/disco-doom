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
rcsid[] = "$Id: m_argv.c,v 1.1 1997/02/03 22:45:10 b1 Exp $";

#include <stdint.h>
#include <string.h>
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

static int		m_argc;
static char**	m_argv;
static char     textout[1024];

void COM_Init (int argc, char **argv)
{
    m_argc = argc;
    m_argv = argv;
}

//
// COM_CheckParm
// Checks for the given parameter
// in the program's command line arguments.
// Returns the argument number (1 to argc-1)
// or 0 if not present
int COM_CheckParm (char *check)
{
    int		i;

    for (i = 1; i < m_argc; i++)
    {
	    if ( !I_strncasecmp(check, m_argv[i], strlen(check)))
	        return i;
    }

    return 0;
}

//
// COM_GetParm
// Get value of a parameter if exists
// return pointer to parameter value string or NULL
// if parameter or value don't exists
char *COM_GetParm (char *name)
{
    int idx = COM_CheckParm(name);

    if(idx && idx < m_argc - 1){
        return m_argv[idx + 1];
    }

    return NULL;
}

int COM_Print (const char* fmt, ...)
{
    va_list	argptr;
    va_start (argptr, fmt);
    vsprintf (textout, fmt, argptr);
    printf("%s", textout);
    va_end (argptr); 
}
