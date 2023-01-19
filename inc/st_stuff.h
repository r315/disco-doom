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
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//
//-----------------------------------------------------------------------------

#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"

// Size of statusbar.
#define ST_HEIGHT	(32)
#define ST_WIDTH	(BASE_WIDTH)

//
// STATUS BAR DATA
//

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS    1
#define STARTBONUSPALS  9
#define NUMREDPALS      8
#define NUMBONUSPALS    4
// Radiation suit, green shift.
#define RADIATIONPAL    13
// N/256*100% probability
// that the normal face state will change
#define ST_FACEPROBABILITY 96
// For Responder
#define ST_TOGGLECHAT   KEY_ENTER

// Location of status bar
#define ST_X        ((SCREENWIDTH / 2) - (BASE_WIDTH / 2))
#define ST_Y		(SCREENHEIGHT - ST_HEIGHT)

#define ST_FX       (ST_X + 143)
#define ST_FY       (ST_Y + 1)

// Should be set to patch width
//  for tall numbers later on
#define ST_TALLNUMWIDTH (tallnum[0]->width)

// Number of status faces.
#define ST_NUMPAINFACES     5
#define ST_NUMSTRAIGHTFACES 3
#define ST_NUMTURNFACES     2
#define ST_NUMSPECIALFACES  3

#define ST_FACESTRIDE \
    (ST_NUMSTRAIGHTFACES + ST_NUMTURNFACES + ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES    2

#define ST_NUMFACES \
    (ST_FACESTRIDE * ST_NUMPAINFACES + ST_NUMEXTRAFACES)

#define ST_TURNOFFSET       (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET       (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET   (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET    (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE          (ST_NUMPAINFACES * ST_FACESTRIDE)
#define ST_DEADFACE         (ST_GODFACE + 1)

#define ST_FACESX           (ST_X + 143)
#define ST_FACESY           (ST_Y + 0)

#define ST_EVILGRINCOUNT    (2 * TICRATE)
#define ST_STRAIGHTFACECOUNT (TICRATE / 2)
#define ST_TURNCOUNT        (1 * TICRATE)
#define ST_OUCHCOUNT        (1 * TICRATE)
#define ST_RAMPAGEDELAY     (2 * TICRATE)

#define ST_MUCHPAIN         20

// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?

// AMMO number pos.
#define ST_AMMOWIDTH    3
#define ST_AMMOX        (ST_X + 44)
#define ST_AMMOY        (ST_Y + 3)

// HEALTH number pos.
#define ST_HEALTHWIDTH  3
#define ST_HEALTHX      (ST_X + 90)
#define ST_HEALTHY      (ST_Y + 3)

// Weapon pos.
#define ST_ARMSX        (ST_X + 111)
#define ST_ARMSY        (ST_Y + 4)
#define ST_ARMSBGX      (ST_X + 104)
#define ST_ARMSBGY      (ST_Y + 0)
#define ST_ARMSXSPACE   12
#define ST_ARMSYSPACE   10

// Frags pos.
#define ST_FRAGSWIDTH   2
#define ST_FRAGSX       (ST_X + 138)
#define ST_FRAGSY       (ST_Y + 3)

// ARMOR number pos.
#define ST_ARMORWIDTH   3
#define ST_ARMORX       (ST_X + 221)
#define ST_ARMORY       (ST_Y + 3)

// Key icon positions.
#define ST_KEY0WIDTH    8
#define ST_KEY0HEIGHT   5
#define ST_KEY0X        (ST_X + 239)
#define ST_KEY0Y        (ST_X + 3)
#define ST_KEY1WIDTH    ST_KEY0WIDTH
#define ST_KEY1X        (ST_X + 239)
#define ST_KEY1Y        (ST_Y + 13)
#define ST_KEY2WIDTH ST_KEY0WIDTH
#define ST_KEY2X        (ST_X + 239)
#define ST_KEY2Y        (ST_Y + 23)

// Ammunition counter.
#define ST_AMMO0WIDTH   3
#define ST_AMMO0HEIGHT  6
#define ST_AMMO0X       (ST_X + 288)
#define ST_AMMO0Y       (ST_Y + 5)
#define ST_AMMO1WIDTH   ST_AMMO0WIDTH
#define ST_AMMO1X       (ST_X + 288)
#define ST_AMMO1Y       (ST_Y + 11)
#define ST_AMMO2WIDTH ST_AMMO0WIDTH
#define ST_AMMO2X       (ST_X + 288)
#define ST_AMMO2Y       (ST_Y + 23)
#define ST_AMMO3WIDTH ST_AMMO0WIDTH
#define ST_AMMO3X       (ST_X + 288)
#define ST_AMMO3Y       (ST_Y + 17)

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH    3
#define ST_MAXAMMO0HEIGHT   5
#define ST_MAXAMMO0X        (ST_X + 314)
#define ST_MAXAMMO0Y        (ST_Y + 5)
#define ST_MAXAMMO1WIDTH ST_MAXAMMO0WIDTH
#define ST_MAXAMMO1X        (ST_X + 314)
#define ST_MAXAMMO1Y        (ST_Y + 11)
#define ST_MAXAMMO2WIDTH ST_MAXAMMO0WIDTH
#define ST_MAXAMMO2X        (ST_X + 314)
#define ST_MAXAMMO2Y        (ST_Y + 23)
#define ST_MAXAMMO3WIDTH ST_MAXAMMO0WIDTH
#define ST_MAXAMMO3X        (ST_X + 314)
#define ST_MAXAMMO3Y        (ST_Y + 17)

// pistol
#define ST_WEAPON0X         (ST_X + 110)
#define ST_WEAPON0Y         (ST_Y + 3)

// shotgun
#define ST_WEAPON1X         (ST_X + 122)
#define ST_WEAPON1Y         (ST_Y + 4)

// chain gun
#define ST_WEAPON2X         (ST_X + 134)
#define ST_WEAPON2Y         (ST_Y + 4)

// missile launcher
#define ST_WEAPON3X         (ST_X + 110)
#define ST_WEAPON3Y         (ST_Y + 13)

// plasma gun
#define ST_WEAPON4X         (ST_X + 122)
#define ST_WEAPON4Y         (ST_Y + 13)

// bfg
#define ST_WEAPON5X         (ST_X + 134)
#define ST_WEAPON5Y         (ST_Y + 13)

// WPNS title
#define ST_WPNSX            (ST_X + 109)
#define ST_WPNSY            (ST_Y + 23)

// DETH title
#define ST_DETHX            (ST_X + 109)
#define ST_DETHY            (ST_Y + 23)

// Incoming messages window location
// UNUSED
//  #define ST_MSGTEXTX	   (viewwindowx)
//  #define ST_MSGTEXTY	   (viewwindowy+viewheight-18)
#define ST_MSGTEXTX     0
#define ST_MSGTEXTY     0
// Dimensions given in characters.
#define ST_MSGWIDTH     52
// Or shall I say, in lines?
#define ST_MSGHEIGHT    1

#define ST_OUTTEXTX     0
#define ST_OUTTEXTY     6

// Width, in characters again.
#define ST_OUTWIDTH     52
// Height, in lines.
#define ST_OUTHEIGHT    1

#define ST_MAPWIDTH \
    (strlen(mapnames[(gameepisode - 1) * 9 + (gamemap - 1)]))

#define ST_MAPTITLEX \
    (SCREENWIDTH - ST_MAPWIDTH * ST_CHATFONTWIDTH)

#define ST_MAPTITLEY 0
#define ST_MAPHEIGHT 1

// States for status bar code.
typedef enum st_stateenum_e
{
    AutomapState,
    FirstPersonState
} st_stateenum_t;

// States for the chat code.
typedef enum st_chatstateenum_e
{
    StartChatState,
    WaitDestState,
    GetChatState
} st_chatstateenum_t;

//
// STATUS BAR
//
void ST_Ticker(void);
void ST_Drawer(boolean fullscreen, boolean refresh);
void ST_Start(void);
void ST_Init(void);
boolean ST_Responder(event_t *ev);

#endif
//-----------------------------------------------------------------------------
//
// $Log:$
//
//-----------------------------------------------------------------------------
