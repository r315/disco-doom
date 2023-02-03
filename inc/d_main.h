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
//	System specific interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef __D_MAIN__
#define __D_MAIN__

#include "d_event.h"
#include "doomdef.h"

void D_DoomMain (int argc, char **argv);
// Called by IO functions when input is detected.
void D_PageTicker (void);
void D_PageDrawer (void);
void D_StartTitle (void);
void D_AdvanceDemo (void);
void D_DoAdvanceDemo (void);
void D_PostEvent (event_t* ev);
void D_ProcessEvents (void);
void D_CheckEventsAbort(void);
int access(char *file, int mode);

// ------------------------
// Command line parameters.
//
extern  boolean	    nomonsters;	    // checkparm of -nomonsters
extern  boolean	    respawnparm;	// checkparm of -respawn
extern  boolean	    fastparm;	    // checkparm of -fast
extern  boolean	    d_devparm;	    // DEBUG: launched with -devparm

//?
// debug flag to cancel adaptiveness
extern  boolean     singletics;	

// File handling stuff.
extern  FILE*		debugfile;

// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern  gamestate_t wipegamestate;

extern char         *basedir;
#endif
