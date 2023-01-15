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
//
//
//-----------------------------------------------------------------------------


#ifndef __P_INTER__
#define __P_INTER__

#include "doomdef.h"

boolean	P_GivePower(player_t*, int);

// LUT of ammunition limits for each kind.
// This doubles with BackPack powerup item.
extern int      maxammo[NUMAMMO];
extern int		clipammo[NUMAMMO];

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
