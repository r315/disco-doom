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
//   Setup a game, startup stuff.
//
//-----------------------------------------------------------------------------


#ifndef __P_SETUP__
#define __P_SETUP__

#include "doomdata.h"

// NOT called by W_Ticker. Fixme.
void
P_SetupLevel
( int		episode,
  int		map,
  int		playermask,
  skill_t	skill);

// Called by startup code.
void P_Init (void);

// Player spawn spots for deathmatch.
extern  mapthing_t*	    deathmatch_p;
extern  mapthing_t      deathmatchstarts[MAX_DM_STARTS];

// Player spawn spots.
extern  mapthing_t      playerstarts[MAXPLAYERS];

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
