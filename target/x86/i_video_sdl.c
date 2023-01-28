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
//	DOOM graphics stuff for SDL library
//
//-----------------------------------------------------------------------------

static const char
	rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>

#include "target.h"

#include "m_swap.h"
#include "i_system.h"
#include "v_video.h"
#include "common.h"
#include "d_main.h"

#include "hu_lib.h"
#include "hu_stuff.h"

#include "doomdef.h"

static SDL_Surface *surface;
static byte *screen;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int multiply = 2;

static hu_textline_t	h_fps;

//
//  Translates the key
//

int xlatekey(SDL_keysym *key)
{

	int rc;

	switch (key->sym)
	{
	case SDLK_LEFT:
		rc = KEY_LEFTARROW;
		break;
	case SDLK_RIGHT:
		rc = KEY_RIGHTARROW;
		break;
	case SDLK_DOWN:
		rc = KEY_DOWNARROW;
		break;
	case SDLK_UP:
		rc = KEY_UPARROW;
		break;
	case SDLK_ESCAPE:
		rc = KEY_ESCAPE;
		break;
	case SDLK_RETURN:
		rc = KEY_ENTER;
		break;
	case SDLK_TAB:
		rc = KEY_TAB;
		break;
	case SDLK_F1:
		rc = KEY_F1;
		break;
	case SDLK_F2:
		rc = KEY_F2;
		break;
	case SDLK_F3:
		rc = KEY_F3;
		break;
	case SDLK_F4:
		rc = KEY_F4;
		break;
	case SDLK_F5:
		rc = KEY_F5;
		break;
	case SDLK_F6:
		rc = KEY_F6;
		break;
	case SDLK_F7:
		rc = KEY_F7;
		break;
	case SDLK_F8:
		rc = KEY_F8;
		break;
	case SDLK_F9:
		rc = KEY_F9;
		break;
	case SDLK_F10:
		rc = KEY_F10;
		break;
	case SDLK_F11:
		rc = KEY_F11;
		break;
	case SDLK_F12:
		rc = KEY_F12;
		break;

	case SDLK_BACKSPACE:
	case SDLK_DELETE:
		rc = KEY_BACKSPACE;
		break;

	case SDLK_PAUSE:
		rc = KEY_PAUSE;
		break;

	case SDLK_EQUALS:
		rc = KEY_EQUALS;
		break;

	case SDLK_KP_MINUS:
	case SDLK_MINUS:
		rc = KEY_MINUS;
		break;

	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
		rc = KEY_RSHIFT;
		break;

	case SDLK_LCTRL:
	case SDLK_RCTRL:
		rc = KEY_RCTRL;
		break;

	case SDLK_LALT:
	//case SDLK_LMETA:
	case SDLK_RALT:
		//case SDLK_RMETA:
		rc = KEY_RALT;
		break;

	default:
		rc = key->sym;
		break;
	}

	return rc;
}

//
// I_StartFrame
//
//void I_StartFrame (void)
//{
// er?

//}

/* This processes SDL events */
void I_GetEvent(SDL_Event *Event)
{
	uint8_t buttonstate;
	event_t event;	

	switch (Event->type)
	{
	case SDL_KEYDOWN:
		event.type = ev_keydown;
		event.data1 = xlatekey(&Event->key.keysym);
		D_PostEvent(&event);
		//printf("Key %d, Event: %s\n", event.data1, "key down");
		break;

	case SDL_KEYUP:
		event.type = ev_keyup;
		event.data1 = xlatekey(&Event->key.keysym);
		D_PostEvent(&event);
		//printf("Key %d, Event: %s\n", event.data1, "key up");
		break;

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		buttonstate = SDL_GetMouseState(NULL, NULL);
		event.type = ev_mouse;
		event.data1 = 0 | (buttonstate & SDL_BUTTON(1) ? 1 : 0) | (buttonstate & SDL_BUTTON(2) ? 2 : 0) | (buttonstate & SDL_BUTTON(3) ? 4 : 0);
		event.data2 = event.data3 = 0;
		D_PostEvent(&event);
		break;

#if (SDL_MAJOR_VERSION >= 0) && (SDL_MINOR_VERSION >= 9)
	case SDL_MOUSEMOTION:
		/* Ignore mouse warp events */
		if ((Event->motion.x != surface->w / 2) || (Event->motion.y != surface->h / 2))
		{
			/* Warp the mouse back to the center */
			if (grabMouse)
			{
				SDL_WarpMouse(surface->w / 2, surface->h / 2);
			}
			event.type = ev_mouse;
			event.data1 = 0 | (Event->motion.state & SDL_BUTTON(1) ? 1 : 0) | (Event->motion.state & SDL_BUTTON(2) ? 2 : 0) | (Event->motion.state & SDL_BUTTON(3) ? 4 : 0);
			event.data2 = Event->motion.xrel << 2;
			event.data3 = -Event->motion.yrel << 2;
			D_PostEvent(&event);
		}
		break;
#endif

	case SDL_QUIT:
		I_Quit();
	}	
}

//
// I_ShutdownGraphics
//
void I_ShutdownGraphics(void)
{
	SDL_Quit();
}

//
// I_StartTic
//
void I_StartTic(void)
{
	SDL_Event Event;
	//event_t event;

	while (SDL_PollEvent(&Event))
		I_GetEvent(&Event);
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit(void)
{
	// what is this?
}

static void printFps(char *txt) {
	HUlib_clearTextLine(&h_fps);
	while (*txt) {
		HUlib_addCharToTextLine(&h_fps, *txt++);
	}
	HUlib_drawTextLine(&h_fps, false);
}

static void updateFps() {
	static int fps_tick = 0;
	static char tmp[10];
	static int fps = 0;

	if (fps_tick < SDL_GetTicks()) {		
		sprintf(tmp, "%d", fps);				
		fps = 0;
		fps_tick = SDL_GetTicks() + 1000;
	}
	else {
		fps++;
	}

	printFps(tmp);
}

//
// I_FinishUpdate
//
void I_FinishUpdate(void)
{

	static int lasttic;
	int tics;
	int i;

	// draws little dots on the bottom of the surface
	if (d_devparm)
	{

		i = I_GetTime();
		tics = i - lasttic;
		lasttic = i;
		if (tics > 20)
			tics = 20;

		for (i = 0; i < tics * 2; i += 2)
			screen[(SCREENHEIGHT - 10) * SCREENWIDTH + i] = 0;
		for (; i < 20 * 2; i += 2)
			screen[(SCREENHEIGHT - 10) * SCREENWIDTH + i] = 0;
		
		updateFps();
	}
	

	// scales the surface size before blitting it
	if (SDL_MUSTLOCK(surface))
	{
		if (SDL_LockSurface(surface) < 0)
		{
			return;
		}
	}

	if ((multiply == 1) && SDL_MUSTLOCK(surface))
	{
		unsigned char *olineptr;
		unsigned char *ilineptr;
		int y;

		ilineptr = (unsigned char *)screen;
		olineptr = (unsigned char *)surface->pixels;

		y = SCREENHEIGHT;
		while (y--)
		{
			memcpy(olineptr, ilineptr, surface->w);
			ilineptr += SCREENWIDTH;
			olineptr += surface->pitch;
		}
	}
	else if (multiply == 2)
	{
		unsigned int *olineptrs[2];
		unsigned int *ilineptr;
		int x, y, i;
		unsigned int twoopixels;
		unsigned int twomoreopixels;
		unsigned int fouripixels;

		ilineptr = (unsigned int *)(screen);
		for (i = 0; i < 2; i++)
		{
			olineptrs[i] =
				(unsigned int *)&((uint8_t *)surface->pixels)[i * surface->pitch];
		}

		y = SCREENHEIGHT;
		while (y--)
		{
			x = SCREENWIDTH;
			do
			{
				fouripixels = *ilineptr++;
				twoopixels = (fouripixels & 0xff000000) | ((fouripixels >> 8) & 0xffff00) | ((fouripixels >> 16) & 0xff);
				twomoreopixels = ((fouripixels << 16) & 0xff000000) | ((fouripixels << 8) & 0xffff00) | (fouripixels & 0xff);
#ifdef __BIG_ENDIAN__
				*olineptrs[0]++ = twoopixels;
				*olineptrs[1]++ = twoopixels;
				*olineptrs[0]++ = twomoreopixels;
				*olineptrs[1]++ = twomoreopixels;
#else
				*olineptrs[0]++ = twomoreopixels;
				*olineptrs[1]++ = twomoreopixels;
				*olineptrs[0]++ = twoopixels;
				*olineptrs[1]++ = twoopixels;
#endif
			} while (x -= 4);
			olineptrs[0] += surface->pitch / 4;
			olineptrs[1] += surface->pitch / 4;
		}
	}
	else if (multiply == 3)
	{
		unsigned int *olineptrs[3];
		unsigned int *ilineptr;
		int x, y, i;
		unsigned int fouropixels[3];
		unsigned int fouripixels;

		ilineptr = (unsigned int *)(screen);
		for (i = 0; i < 3; i++)
		{
			olineptrs[i] =
				(unsigned int *)&((uint8_t *)surface->pixels)[i * surface->pitch];
		}

		y = SCREENHEIGHT;
		while (y--)
		{
			x = SCREENWIDTH;
			do
			{
				fouripixels = *ilineptr++;
				fouropixels[0] = (fouripixels & 0xff000000) | ((fouripixels >> 8) & 0xff0000) | ((fouripixels >> 16) & 0xffff);
				fouropixels[1] = ((fouripixels << 8) & 0xff000000) | (fouripixels & 0xffff00) | ((fouripixels >> 8) & 0xff);
				fouropixels[2] = ((fouripixels << 16) & 0xffff0000) | ((fouripixels << 8) & 0xff00) | (fouripixels & 0xff);
#ifdef __BIG_ENDIAN__
				*olineptrs[0]++ = fouropixels[0];
				*olineptrs[1]++ = fouropixels[0];
				*olineptrs[2]++ = fouropixels[0];
				*olineptrs[0]++ = fouropixels[1];
				*olineptrs[1]++ = fouropixels[1];
				*olineptrs[2]++ = fouropixels[1];
				*olineptrs[0]++ = fouropixels[2];
				*olineptrs[1]++ = fouropixels[2];
				*olineptrs[2]++ = fouropixels[2];
#else
				*olineptrs[0]++ = fouropixels[2];
				*olineptrs[1]++ = fouropixels[2];
				*olineptrs[2]++ = fouropixels[2];
				*olineptrs[0]++ = fouropixels[1];
				*olineptrs[1]++ = fouropixels[1];
				*olineptrs[2]++ = fouropixels[1];
				*olineptrs[0]++ = fouropixels[0];
				*olineptrs[1]++ = fouropixels[0];
				*olineptrs[2]++ = fouropixels[0];
#endif
			} while (x -= 4);
			olineptrs[0] += 2 * surface->pitch / 4;
			olineptrs[1] += 2 * surface->pitch / 4;
			olineptrs[2] += 2 * surface->pitch / 4;
		}
	}
	if (SDL_MUSTLOCK(surface))
	{
		SDL_UnlockSurface(surface);
	}
	SDL_UpdateRect(surface, 0, 0, 0, 0);
}

//
// I_ReadScreen
//
void I_ReadScreen(byte *scr)
{
	memcpy(scr, screen, SCREENWIDTH * SCREENHEIGHT);
}

byte* I_GetScreen(void) {return screen; }
//
// I_SetPalette
//
void I_SetPalette(byte *palette)
{
	int i;
	SDL_Color colors[256];

	for (i = 0; i < 256; ++i)
	{
		colors[i].r = gammatable[usegamma][*palette++];
		colors[i].g = gammatable[usegamma][*palette++];
		colors[i].b = gammatable[usegamma][*palette++];
		colors[i].unused = 0;
	}
	SDL_SetColors(surface, colors, 0, 256);
}

void I_InitGraphics(void)
{

	static int firsttime = 1;
	Uint16 video_w, video_h, w, h;
	uint8_t video_bpp;
	Uint32 video_flags;

	if (!firsttime)
		return;

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0)
		fprintf(stderr, "Could not initialize SDL: %s", SDL_GetError());

	firsttime = 0;

	video_flags = (SDL_SWSURFACE | SDL_HWPALETTE);
	if (!!COM_CheckParm("-fullscreen"))
		video_flags |= SDL_FULLSCREEN;

	if (COM_CheckParm("-2"))
		multiply = 2;

	if (COM_CheckParm("-3"))
		multiply = 3;

	video_w = w = SCREENWIDTH * multiply;
	video_h = h = SCREENHEIGHT * multiply;
	video_bpp = 8;

	/* We need to allocate a software surface because the DOOM! code expects
       the surface surface to be valid all of the time.  Properly done, the
       rendering code would allocate the video surface in video memory and
       then call SDL_LockSurface()/SDL_UnlockSurface() around frame rendering.
       Eventually SDL will support flipping, which would be really nice in
       a complete-frame rendering application like this.
    */
	switch (video_w / w)
	{
	case 3:
		multiply *= 3;
		break;
	case 2:
		multiply *= 2;
		break;
	case 1:
		multiply *= 1;
		break;
	default:;
	}
	
	if (multiply > 3)
	{
		I_Error("Smallest available mode (%dx%d) is too large!",
				video_w, video_h);
	}

	surface = SDL_SetVideoMode(video_w, video_h, 8, video_flags);
	if (surface == NULL)
	{
		I_Error("Could not set %dx%d video mode: %s", video_w, video_h,
				SDL_GetError());
	}

	SDL_ShowCursor(0);
	SDL_WM_SetCaption("SDL DOOM! v1.10", "doom");

	/* Set up the surface displays */
	w = SCREENWIDTH * multiply;
	h = SCREENHEIGHT * multiply;

	if (multiply == 1 && !SDL_MUSTLOCK(surface))
	{
		screen = (unsigned char *)surface->pixels;
	}
	else
	{
		screen = (unsigned char *)I_AllocLow (SCREENWIDTH * SCREENHEIGHT);
		if (screen == NULL)
			I_Error("Couldn't allocate screen memory");
	}

	HUlib_initTextLine(&h_fps, 10, 10, hu_font, HU_FONTSTART);
}
