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
//	Main program, simply calls D_DoomMain high level loop.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: i_main.c,v 1.4 1997/02/03 22:45:10 b1 Exp $";

#include <signal.h>
#include "doomdef.h"
#include "common.h"
#include "d_main.h"
#include "target.h"

/* SDL 1.2.15 build flags
CFLAGS=-m32 CXXFLAGS=-m32 LDFLAGS=-m32 ./configure
CFLAGS=-m32 CXXFLAGS=-m32 LDFLAGS=-m32 make
*/

int T_GetTick(void){
    return SDL_GetTicks();
}

void T_Delay(int ms){
    SDL_Delay(ms);
}

int main( int argc, char** argv) 
{ 
    D_DoomMain (argc, argv); 
    return 0;
} 

#ifdef __linux__
void __debugbreak(void)
{
    raise(SIGABRT);
}
#endif