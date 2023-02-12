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
//	DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
//	plus functions to determine game mode (shareware, registered),
//	parse command line parameters, configure game parameters (turbo),
//	and call the startup functions.
//
//-----------------------------------------------------------------------------


static const char rcsid[] = "$Id: d_main.c,v 1.8 1997/02/03 22:45:09 b1 Exp $";

#include <stdio.h>
#include <stdlib.h>

#include "doomdef.h"
#include "dstrings.h"
#include "sounds.h"
#include "z_zone.h"
#include "w_wad.h"
#include "s_sound.h"
#include "v_video.h"
#include "f_finale.h"
#include "f_wipe.h"
#include "common.h"
#include "m_misc.h"
#include "m_menu.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_classic.h"
#include "am_map.h"
#include "p_setup.h"
#include "r_local.h"
#include "doomstat.h"
#include "d_main.h"
#include "d_net.h"

#define	BGCOLOR		7
#define	FGCOLOR		8
#define R_OK        4

#define MAX_PATH_SIZE   64

typedef struct wadnames_s
{
    const char *filename;
    GameMode_t mode;
}wadnames_t;

boolean     d_devparm;		// started game with -devparm
boolean     nomonsters;     // checkparm of -nomonsters
boolean     respawnparm;	// checkparm of -respawn
boolean     fastparm;		// checkparm of -fast
boolean     singletics;     // debug flag to cancel adaptiveness

FILE*	    debugfile;

static boolean        autostart;
static boolean        advancedemo;

//
// EVENT HANDLING
//
// Events are asynchronous inputs generally generated by the game user.
// Events can be discarded if no responder claims them
//
static event_t     events[MAXEVENTS];
static int         eventhead;
static int         eventtail;

//
//  DEMO LOOP
//
static int  d_demosequence;
static int  d_pagetic;
static char *d_pagename;

static char *wadfilename;
char        *basedir;       // game dir

static  boolean	    d_fullscreen;
static  boolean		d_viewactivestate;
static  boolean		d_menuactivestate;
static  boolean		d_inhelpscreensstate;
static  gamestate_t d_oldgamestate;
static  int			d_borderdrawcount;

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t wipegamestate = GS_DEMOSCREEN;

static const wadnames_t d_wadnames[] = {
        {"doom.wad", registered},
        {"doom1.wad", shareware},
        {"doomu.wad", retail},
        {"doom2.wad", commercial}
};

//==============================================================================
// D_PostEvent
// Called by the I/O functions when input is detected
//
void D_PostEvent (event_t* ev)
{
    events[eventhead++] = *ev;
    eventhead = eventhead & (MAXEVENTS-1);
}

//
// D_ProcessEvents
// Send all the events of the given timestamp down the responder chain
//
void D_ProcessEvents (void)
{
    event_t*	ev;
	
    // IF STORE DEMO, DO NOT ACCEPT INPUT
    if ( ( gamemode == commercial ) && (W_CheckNumForName("map01") < 0) )
      return;
	
    for ( ; eventtail != eventhead; eventtail = eventtail & (MAXEVENTS-1))
    {
        ev = &events[eventtail++];

        if (M_Responder (ev))
            continue;               // menu ate the event
        G_Responder (ev);
    }
}

void D_CheckEventsAbort(void)
{
    event_t *ev;

    for ( ; eventtail != eventhead; eventtail = eventtail & (MAXEVENTS-1)) 
    { 
		ev = &events[eventtail++]; 
		if (ev->type == ev_keydown && ev->data1 == KEY_ESCAPE){
	    	I_Error ("Network game synchronization aborted.");
		}		
    } 
}

static void D_ClearEvents(void)
{
    memset(events, 0, sizeof(events));
    eventtail = eventhead = 0;
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//

static void D_Display (void)
{
    int				nowtime;
    int				tics;
    int				wipestart;
    int				y;
    boolean			done;
    boolean			wipe;
    
    // change the view size if needed
    if (setsizeneeded)
    {
    	R_ExecuteSetViewSize ();
        d_oldgamestate = -1;                      // force background redraw
    	d_borderdrawcount = 3;
        d_fullscreen = viewheight == SCREENHEIGHT;
		if (d_fullscreen)
			ST_Visible(false);
		else
			ST_Visible(true);
    }

    // save the current screen if about to wipe
    if (gamestate != wipegamestate)
    {
    	wipe = true;
    	wipe_StartScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);
    }
    else
	   wipe = false;    	
    
    // do buffered drawing
    switch (gamestate)
    {
      case GS_LEVEL:
            if (!gametic)
	           break;

            HU_Erase();        	    
			// just put away the help screen
            if (d_inhelpscreensstate && !inhelpscreens)
				ST_Visible(true);

            //ST_Drawer (fullscreen, redrawsbar);
            ST_Drawer();
        	break;

      case GS_INTERMISSION:
        	WI_Drawer ();
        	break;

      case GS_FINALE:
        	F_Drawer ();
        	break;

      case GS_DEMOSCREEN:
        	D_PageDrawer ();
        	break;
    }
  
    // draw the view directly
    if (gamestate == GS_LEVEL && gametic){
        if (automapactive)
            AM_Drawer ();
        else
	        R_RenderPlayerView (&players[displayplayer]);
    	HU_Drawer ();
    }
    
    // clean up border stuff
    if (gamestate != GS_LEVEL && gamestate != d_oldgamestate)
	   I_SetPalette (W_CacheLumpName ("PLAYPAL",PU_CACHE));

    // see if the border needs to be initially drawn
    if (gamestate == GS_LEVEL && d_oldgamestate != GS_LEVEL)
    {
    	d_viewactivestate = false; // view was not active
        R_FillBackScreen ();     // draw the pattern into the back screen
    }

    // see if the border needs to be updated to the screen
    if (gamestate == GS_LEVEL && !automapactive && scaledviewwidth != SCREENWIDTH)
    {
    	if (menuactive || d_menuactivestate || !d_viewactivestate)
	       d_borderdrawcount = 3;

    	if (d_borderdrawcount)
	    {
	       R_DrawViewBorder ();    // erase old menu stuff
	       d_borderdrawcount--;
	    }
    }

    d_menuactivestate = menuactive;
    d_viewactivestate = viewactive;
    d_inhelpscreensstate = inhelpscreens;
    d_oldgamestate = wipegamestate = gamestate;
    
    // draw pause pic
    if (G_Paused())
    {
	   y = (automapactive) ? 4 : viewwindowy + 4;	  
	       
	   V_DrawPatch(viewwindowx + (scaledviewwidth - 68)/2, y, 
                    0,
                    W_CacheLumpName ("M_PAUSE", PU_CACHE));
    }

    // menus go directly to the screen
    M_Drawer ();          // menu is drawn even on top of everything
    NetUpdate ();         // send out any new accumulation

    // normal update
    if (!wipe)
    {
		I_FinishUpdate ();              // page flip or blit buffer
		return;
    }
    
    // wipe update
    wipe_EndScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);

    wipestart = I_GetTime () - 1;

    do
    {
	   do
	   {
	       nowtime = I_GetTime ();
	       tics = nowtime - wipestart;
	   } while (!tics);
	   wipestart = nowtime;
	   done = wipe_ScreenWipe(wipe_Melt
			       , 0, 0, SCREENWIDTH, SCREENHEIGHT, tics);
	   I_UpdateNoBlit ();
	   M_Drawer ();                            // menu is drawn even on top of wipes
	   I_FinishUpdate ();                      // page flip or blit buffer
    } while (!done);
}



//-------------------------------------------------------------
//
// D-DoomLoop()
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime, I_StartFrame, and I_StartTic
//
//-------------------------------------------------------------
static void D_DoomLoop (void)
{
#if 0
	if (COM_CheckParm ("-debugfile"))
    {
        char    filename[20];
        sprintf (filename,"debug%i.txt",consoleplayer);
        printf ("debug output to: %s\n",filename);
    	debugfile = fopen (filename,"w");
    }
#endif
   
    eventhead = eventtail = 0;

    while (1)
    {    	
    	// process one or more tics
        if (singletics)
        {
			//printf("singleticks");
            I_StartTic ();
            D_ProcessEvents ();
            G_BuildTiccmd (&netcmds[consoleplayer][maketic%BACKUPTICS]);
            if (advancedemo)
                D_DoAdvanceDemo ();
            M_Ticker ();
            G_Ticker ();
            gametic++;
            maketic++;
            //break;
        }
		else 
		{
			TryRunTics(); // will run at least one tic		
		}

		S_UpdateSounds (players[consoleplayer].mo);// move positional sounds

    	// Update display, next frame, with current state.
	    D_Display ();
    }
}

//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker (void)
{
    if (--d_pagetic < 0)
	   D_AdvanceDemo ();
}



//
// D_PageDrawer
//
void D_PageDrawer (void)
{
    V_DrawPatch (0,0, 0, W_CacheLumpName(d_pagename, PU_CACHE));
}


//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void)
{
    advancedemo = true;
}


//
// This cycles through the demo sequences.
// FIXME - version dependend demo numbers?
void D_DoAdvanceDemo (void)
{
    if(!advancedemo)
        return;

    players[consoleplayer].playerstate = PST_LIVE;  // not reborn
    advancedemo = false;
    usergame = false;               // no save / end game here
    //paused = false;
    G_SetGameAction(ga_nothing);

    if ( gamemode == retail )
      d_demosequence = (d_demosequence+1)%7;
    else
      d_demosequence = (d_demosequence+1)%6;
    
    switch (d_demosequence)
    {
		case 0:
            if ( gamemode == commercial )
                d_pagetic = 35 * 11;
        	else
                d_pagetic = 170;
                
            gamestate = GS_DEMOSCREEN;
			d_pagename = "TITLEPIC";
            if ( gamemode == commercial )
                S_StartMusic(mus_dm2ttl);
        	else
                S_StartMusic (mus_intro);
            break;
            
        case 1:	G_DeferedPlayDemo ("demo1");	break;
      
        case 2:
            d_pagetic = 200;
        	gamestate = GS_DEMOSCREEN;
        	d_pagename = "CREDIT";
        	break;
        	
        case 3:	G_DeferedPlayDemo ("demo2");	break;
        
        case 4:	gamestate = GS_DEMOSCREEN;
            if ( gamemode == commercial)
        	{
	            d_pagetic = 35 * 11;
                d_pagename = "TITLEPIC";
	            S_StartMusic(mus_dm2ttl);
            }
            else
            {
                d_pagetic = 200;
                if ( gamemode == retail )
                    d_pagename = "CREDIT";
        	    else if(gamemode == registered)
	                d_pagename = "HELP1";
				else if (gamemode == shareware)
					d_pagename = "HELP2";
	       }
           break;
           
        case 5:	G_DeferedPlayDemo ("demo3");	break;
      
        // THE DEFINITIVE DOOM Special Edition demo
        case 6:   G_DeferedPlayDemo ("demo4");    break;
    }
}



//
// D_StartTitle
//
void D_StartTitle (void)
{
    G_SetGameAction(ga_nothing);
    d_demosequence = -1;
    D_AdvanceDemo ();
}
/*
static char *D_GetFilename(char *path)
{
	char *ptr = path + strlen(path); // start from end

	while (ptr != path) {
#ifdef _WIN32
		if (*ptr == '/' || *ptr == '\\') {
#else
		if (*ptr == '/') {
#endif
			return ptr + 1;
		}
		ptr--;
	}
	return ptr;
}
*/

static int D_CheckWadFile(char *wadname) {
    
    if(wadname == NULL || *wadname == '\0'){
        return 0;
    }
    
	// Test access to file    
	return !access(wadname, R_OK);
}

//
// IdentifyVersion
// Checks availability of IWAD files by name,
// to determine whether registered/commercial features
// should be executed (notably loading PWAD's).
//
static GameMode_t D_IDVersion (void)
{
    char *wadfile_param;
    
    wadfilename = (char*)calloc(1, MAX_PATH_SIZE);

    if(!wadfilename){
        // Fail t allocate
        return indetermined;
    }

    // Check if forced shareware
    if (COM_CheckParm ("-shdev"))
    {
	    d_devparm = true;
	    //D_AddFile (DEVDATA"doom1.wad");
	    //D_AddFile (DEVMAPS"data_se/texture1.lmp");
	    //D_AddFile (DEVMAPS"data_se/pnames.lmp");
	    //strcpy (basedefault,DEVDATA"default.cfg");
        wadfile_param = "doom1.wad";
    }else{
        // check for given wad file
        wadfile_param = COM_GetParm("-wadfile");
    }

    for(int i = 0; i < sizeof(d_wadnames) / sizeof(wadnames_t); i++){
        if(wadfile_param){ 
            if(strcmp(d_wadnames[i].filename, wadfile_param)){
                continue;
            }
        }
        sprintf(wadfilename, "%s/%s", basedir, d_wadnames[i].filename);
        if (D_CheckWadFile(wadfilename)) {
            return d_wadnames[i].mode;
        }    
    }

    return indetermined;
}

//
// D_DoomMain
//
void D_DoomMain (int argc, char **argv)
{
    COM_Init(argc, argv);

    basedir = COM_GetParm("-basedir");
    
    if(!basedir){
        basedir = "/";
    }
    
    gamemode = D_IDVersion ();

    // Version select
    if(gamemode == indetermined){
        COM_Print("D_main: no wad file found");
        exit(-1);
    }

    modifiedgame = false;
    G_SetGameAction(ga_nothing);
	
	// hacks ???
    nomonsters  = COM_CheckParm("-nomonsters");
    respawnparm = COM_CheckParm("-respawn");
    fastparm    = COM_CheckParm("-fast");
    d_devparm   = COM_CheckParm("-devparm");  
	autostart   = COM_CheckParm("-autostart");
    singletics  = COM_CheckParm("-singletics");

    if(gamemode == shareware){
        COM_Print ("DOOM Shareware Startup v%u.%u\n",
		            VERSION_NUM/100, VERSION_NUM%100);
    }
    
	/* turbo option */
	char *turbo = COM_GetParm("-turbo"); // -turbo <10-400>
    if (turbo)
    {
	   int     scale = atoi(turbo);
        if (scale < 10)
    	    scale = 10;

        if (scale > 400)
            scale = 400;

        printf ("turbo scale: %i%%\n",scale);
        forwardmove[0] = forwardmove[0]*scale/100;
        forwardmove[1] = forwardmove[1]*scale/100;
        sidemove[0] = sidemove[0]*scale/100;
    	sidemove[1] = sidemove[1]*scale/100;
    }    
   
    // init subsystems
	COM_Print("I_Init: Setting up system state.\n");
	I_Init();

    COM_Print("I_InitGraphics: Setting graphics driver\n");
	I_InitGraphics();

    COM_Print("I_InitSound: Setting audio driver\n");
	I_InitSound();

    COM_Print ("V_Init: allocate screens.\n");
    V_Init ();

    COM_Print ("M_LoadDefaults: Loading system defaults.\n");
    M_LoadDefaults ();

    COM_Print ("Z_Init: Init zone memory allocation daemon. \n");
    Z_Init ();

    COM_Print ("W_Init: Init wadfiles.\n");
    W_AddFile(wadfilename);

    COM_Print ("M_Init: Init miscellaneous info.\n");
    M_Init ();

    COM_Print ("R_Init: Init DOOM refresh daemon");
    R_Init ();

    COM_Print ("\nP_Init: Init Playloop state.\n");
    P_Init ();
    
	COM_Print ("D_CheckNetGame: Checking network game status.\n");
    D_CheckNetGame ();

    COM_Print ("S_Init: Setting up sound.\n");
    S_Init (snd_SfxVolume, snd_MusicVolume);

    COM_Print ("HU_Init: Setting up heads up display.\n");
    HU_Init ();

    COM_Print ("ST_Init: Init status bar.\n");
    ST_Init ();

    D_ClearEvents();
        
    if (autostart)
    {  
        G_InitNew (sk_baby, 1, 1); // start directely on level one    
	}
	else 
    {
		D_StartTitle();                // start up intro loop
	}

    D_DoomLoop ();
}
