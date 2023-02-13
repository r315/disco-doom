#ifndef __STCLASSIC_H__
#define __STCLASSIC_H__

#include "doomtype.h"
#include "d_event.h"
#include "d_player.h"

// All positions are relative to screen

// Size of statusbar.
#define ST_HEIGHT	(32)
#define ST_WIDTH	(BASE_WIDTH)
#define ST_BG_COUNT 3

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

// Location of status bar
#define ST_X			((SCREENWIDTH / 2) - (BASE_WIDTH / 2))
#define ST_Y			(SCREENHEIGHT - ST_HEIGHT)

// AMMO number position 
#define ST_AMMOWIDTH    3			// Number of digits
#define ST_AMMOX        (ST_X + 2)
#define ST_AMMOY        (ST_Y + 3)

// HEALTH number pos.
#define ST_HEALTHWIDTH  3
#define ST_HEALTHX      (ST_X + 48)
#define ST_HEALTHY      (ST_Y + 3)

// Weapon pos.
#define ST_ARMS_COUNT	6
#define ST_ARMSX        (ST_X + 111)
#define ST_ARMSY        (ST_Y + 4)
#define ST_ARMSBGX      (ST_X + 104)
#define ST_ARMSBGY      (ST_Y + 0)
#define ST_ARMSXSPACE   12
#define ST_ARMSYSPACE   10

// Frags pos.
#define ST_FRAGSWIDTH   2
#define ST_FRAGSX       (ST_X + 100)
#define ST_FRAGSY       (ST_Y + 3)

// ARMOR number pos.
#define ST_ARMORWIDTH   3
#define ST_ARMORX       (ST_X + 179)
#define ST_ARMORY       (ST_Y + 3)

// Key icon positions.
#define ST_KEY_COUNT	3
#define ST_KEY0X        (ST_X + 239)
#define ST_KEY0Y        (ST_Y + 3)
#define ST_KEYYSPACE	10
#define ST_KEYFLAGSOFF  (0 << 0)
#define ST_KEYFLAGON	(1 << 0)
#define ST_KEYFLAGDRAW  (1 << 1)

// Ammunition counter.
#define ST_AMMO_SLOTS	4
#define ST_AMMO0WIDTH   3
#define ST_AMMO0X       (ST_X + 275)
#define ST_AMMO0Y       (ST_Y + 5)
#define ST_AMMOXSPACE	10
#define ST_AMMOYSPACE	6

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH    3
#define ST_MAXAMMO0X        (ST_X + 293)
#define ST_MAXAMMO0Y        (ST_Y + 5)

//
// Number of status faces.
//
#define ST_NUMPAINFACES     5
#define ST_NUMSTRAIGHTFACES 3
#define ST_NUMTURNFACES     2
#define ST_NUMSPECIALFACES  3

#define ST_FX				(ST_X + 143) // Backgound origin
#define ST_FY				(ST_Y + 1)

#define ST_FACESX           (ST_X + 143) // Face origin
#define ST_FACESY           (ST_Y + 0)

#define ST_NUMPAINFACES     5
#define ST_NUMSTRAIGHTFACES 3

#define ST_TURNOFFSET       (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET       (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET   (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET    (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE          (ST_NUMPAINFACES * ST_FACESTRIDE)
#define ST_DEADFACE         (ST_GODFACE + 1)

#define ST_EVILGRINCOUNT    (2 * TICRATE)
#define ST_STRAIGHTFACECOUNT (TICRATE / 2)
#define ST_TURNCOUNT        (1 * TICRATE)
#define ST_OUCHCOUNT        (1 * TICRATE)
#define ST_RAMPAGEDELAY     (2 * TICRATE)

#define ST_MUCHPAIN         20
#define ST_FACESTRIDE \
    (ST_NUMSTRAIGHTFACES + ST_NUMTURNFACES + ST_NUMSPECIALFACES)


//
// STATUS BAR
//
void ST_Ticker(void);
void ST_Drawer(void);
void ST_Init(void);
void ST_Start(player_t *pl);
boolean ST_Responder(event_t *ev);
void ST_Visible(boolean en);

#endif 