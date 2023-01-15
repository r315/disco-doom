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
//   Duh.
// 
//-----------------------------------------------------------------------------


#ifndef __G_GAME__
#define __G_GAME__

#include "doomdef.h"
#include "d_event.h"
#include "d_ticcmd.h"
#include "m_fixed.h"
#include "d_player.h"

//
// GAME
//
void G_DeathMatchSpawnPlayer (int playernum);

void G_InitNew (skill_t skill, int episode, int map);

// Can be called by the startup code or M_Responder.
// A normal game starts at map 1,
// but a warp test can start elsewhere
void G_DeferedInitNew (skill_t skill, int episode, int map);

void G_DeferedPlayDemo (char* demo);

// Can be called by the startup code or M_Responder,
// calls P_SetupLevel or W_EnterWorld.
void G_LoadGame (char* name);

void G_DoLoadGame (void);

// Called by M_Responder.
void G_SaveGame (int slot, char* description);

// Only called by startup code.
void G_RecordDemo (char* name);

void G_BeginRecording (void);

void G_PlayDemo (char* name);
void G_TimeDemo (char* name);
boolean G_CheckDemoStatus (void);

void G_ExitLevel (void);
void G_SecretExitLevel (void);

void G_WorldDone (void);

void G_Ticker (void);
boolean G_Responder (event_t*	ev);

void G_ScreenShot (void);

void G_BuildTiccmd (ticcmd_t* cmd);

extern gameaction_t     gameaction;
extern fixed_t          forwardmove[2];
extern fixed_t          sidemove[2];
extern skill_t          gameskill;
extern int              gameepisode;
extern int		        gamemap;
// Nightmare mode flag, single player.
extern  boolean         respawnmonsters;
// Netgame? Only true if >1 player.
extern  boolean	        netgame;
// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
extern  boolean	        deathmatch;	
extern  boolean	        paused;		// Game Pause?
extern  boolean		    viewactive;
extern  boolean		    nodrawers;

// Player taking events, and displaying.
extern int              consoleplayer;	
extern int              displayplayer;


// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern int              totalkills;
extern int              totalitems;
extern int              totalsecret;

// --------------------------------------
// DEMO playback/recording related stuff.
// No demo, there is a human player in charge?
// Disable save/end game?
extern boolean          usergame;

extern boolean          demoplayback;
extern boolean          demorecording;


// Quit after playing a demo from cmdline.
extern boolean          singledemo;	

extern gamestate_t      gamestate;

extern int              gametic;

// Bookkeeping on players - state.
extern player_t         players[MAXPLAYERS];
// Alive? Disconnected?
extern boolean          playeringame[MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern wbstartstruct_t		wminfo;	
// if true, load all graphics at level load
extern boolean          precache;
extern int              bodyqueslot;
#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
