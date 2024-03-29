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
//
// $Log:$
//
// DESCRIPTION:
//	Main loop menu stuff.
//	Default Config File.
//	PCX Screenshots.
//
//-----------------------------------------------------------------------------

static const char
    rcsid[] = "$Id: m_misc.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <ctype.h>

#include "doomdef.h"
#include "g_game.h"
#include "z_zone.h"
#include "m_swap.h"
#include "common.h"
#include "w_wad.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "dstrings.h"
#include "m_misc.h"
#include "s_sound.h"
#include "d_main.h"
#include "r_main.h"
#include "m_menu.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

typedef struct default_s
{
    char *name;
    int *location;
    int defaultvalue;
    int scantranslate; // PC scan code hack
    int untranslated;  // lousy hack
} default_t;

typedef struct pcx_s
{
    char manufacturer;
    char version;
    char encoding;
    char bits_per_pixel;

    unsigned short xmin;
    unsigned short ymin;
    unsigned short xmax;
    unsigned short ymax;

    unsigned short hres;
    unsigned short vres;

    unsigned char palette[48];

    char reserved;
    char color_planes;
    unsigned short bytes_per_line;
    unsigned short palette_type;

    char filler[58];
    unsigned char data; // unbounded
} pcx_t;

static int usemouse;
static int usejoystick;
static int numdefaults;
static char defaultfile[64];

//
// DEFAULTS
//
static default_t defaults[] =
    {
        {"mouse_sensitivity", &mouseSensitivity, 5},
        {"sfx_volume", &snd_SfxVolume, 8},
        {"music_volume", &snd_MusicVolume, 8},
        {"show_messages", &showMessages, 1},

        {"key_right", &key_right, KEY_RIGHTARROW},
        {"key_left", &key_left, KEY_LEFTARROW},
        {"key_up", &key_up, KEY_UPARROW},
        {"key_down", &key_down, KEY_DOWNARROW},
        {"key_forward", &key_forward, 'w'},
        {"key_backward", &key_backward, 's'},
        {"key_strafeleft", &key_strafeleft, 'a'},
        {"key_straferight", &key_straferight, 'd'},

        {"key_fire", &key_fire, ' '},
        {"key_use", &key_use, 'e'},
        {"key_strafe", &key_strafe, KEY_RALT},
        {"key_speed", &key_speed, KEY_RSHIFT},

        {"use_mouse", &usemouse, 1},
        {"mouseb_fire", &mousebfire, 0},
        {"mouseb_strafe", &mousebstrafe, 1},
        {"mouseb_forward", &mousebforward, 2},

        {"use_joystick", &usejoystick, 0},
        {"joyb_fire", &joybfire, 0},
        {"joyb_strafe", &joybstrafe, 1},
        {"joyb_use", &joybuse, 3},
        {"joyb_speed", &joybspeed, 2},

        {"screenblocks", &screenblocks, 10}, // set screen size

        {"snd_channels", &numChannels, 3},

        {"usegamma", &usegamma, 0},

#ifndef __BEOS__
        {"chatmacro0", (int *)&chat_macros[0], (int)HUSTR_CHATMACRO0},
        {"chatmacro1", (int *)&chat_macros[1], (int)HUSTR_CHATMACRO1},
        {"chatmacro2", (int *)&chat_macros[2], (int)HUSTR_CHATMACRO2},
        {"chatmacro3", (int *)&chat_macros[3], (int)HUSTR_CHATMACRO3},
        {"chatmacro4", (int *)&chat_macros[4], (int)HUSTR_CHATMACRO4},
        {"chatmacro5", (int *)&chat_macros[5], (int)HUSTR_CHATMACRO5},
        {"chatmacro6", (int *)&chat_macros[6], (int)HUSTR_CHATMACRO6},
        {"chatmacro7", (int *)&chat_macros[7], (int)HUSTR_CHATMACRO7},
        {"chatmacro8", (int *)&chat_macros[8], (int)HUSTR_CHATMACRO8},
        {"chatmacro9", (int *)&chat_macros[9], (int)HUSTR_CHATMACRO9}
#endif

};

//
// M_SaveDefaults
//
void M_SaveDefaults(void)
{
    int i;
    int v;
    FILE *f;

    f = fopen(defaultfile, "w");
    if (!f)
        return; // can't write the file, but don't complain

    for (i = 0; i < numdefaults; i++)
    {
        if (defaults[i].defaultvalue > -0xfff && defaults[i].defaultvalue < 0xfff)
        {
            v = *defaults[i].location;
            fprintf(f, "%s\t\t%i\n", defaults[i].name, v);
        }
        else
        {
            fprintf(f, "%s\t\t\"%s\"\n", defaults[i].name,
                    *(char **)(defaults[i].location));
        }
    }

    fclose(f);
}

//
// M_LoadDefaults
//
void M_LoadDefaults(void)
{
    int i;
    int len;
    FILE *f;
    char def[80];
    char strparm[100];
    char *newstring = NULL;
    int parm;
    boolean isstring;

    // set everything to base values
    numdefaults = sizeof(defaults) / sizeof(defaults[0]);

    for (i = 0; i < numdefaults; i++)
        *defaults[i].location = defaults[i].defaultvalue;

    // check for a custom default file
    char *config_param = COM_GetParm("-config");

    if (config_param){
        sprintf(defaultfile, "%s", config_param);        
    }else{
        sprintf(defaultfile, "%s/%s", basedir, DEFAULT_FILE);
    }

    // read the file in, overriding any set defaults
    f = fopen(defaultfile, "r");
    if (f)
    {
        while (!feof(f))
        {
            isstring = false;
            if (fscanf(f, "%79s %[^\n]\n", def, strparm) == 2)
            {
                if (strparm[0] == '"')
                {
                    // get a string default
                    isstring = true;
                    len = strlen(strparm);
                    newstring = (char *)I_AllocLow(len);
                    strparm[len - 1] = 0;
                    strcpy(newstring, strparm + 1);
                }
                else if (strparm[0] == '0' && strparm[1] == 'x')
                    sscanf(strparm + 2, "%x", &parm);
                else
                    sscanf(strparm, "%i", &parm);

                for (i = 0; i < numdefaults; i++)
                    if (!strcmp(def, defaults[i].name))
                    {
                        if (!isstring)
                            *defaults[i].location = parm;
                        else
                            *defaults[i].location = (int)newstring;
                        break;
                    }
            }
        }

        fclose(f);
    }

    HU_DisplayMessages(!!showMessages);
}

int M_DrawText(int x, int y, boolean direct, char *string)
{
    int c;
    int w;

    while (*string)
    {
        c = toupper(*string) - HU_FONTSTART;
        string++;
        if (c < 0 || c > HU_FONTSIZE)
        {
            x += 4;
            continue;
        }

        w = SHORT(hu_font[c]->width);
        if (x + w > SCREENWIDTH)
            break;
        if (direct)
            V_DrawPatchDirect(x, y, 0, hu_font[c]);
        else
            V_DrawPatch(x, y, 0, hu_font[c]);
        x += w;
    }

    return x;
}

//
// M_WriteFile
//
boolean M_WriteFile(char const *name, void *source, int length)
{
    FILE *handle;
    int count;

    handle = fopen(name, "wb");

    if (handle == NULL)
        return false;

    count = fwrite(source, 1, length, handle);
    fclose(handle);

    if (count < length)
        return false;

    return true;
}

//
// M_ReadFile
//
int M_ReadFile(char const *name, byte **buffer)
{
    FILE *handle;
    int count, length;
    byte *buf;

    handle = fopen(name, "rb");
    if (handle == NULL)
        I_Error("Couldn't open file '%s'", name);
    fseek(handle, 0, SEEK_END);
    length = ftell(handle);
    rewind(handle);
    buf = Z_Malloc(length, PU_STATIC, NULL);
    count = fread(buf, 1, length, handle);
    fclose(handle);

    if (count < length)
        I_Error("Fail to read file '%s'", name);

    *buffer = buf;
    return length;
}

//
// WritePCXfile
//
static void WritePCXfile(char *filename, int width, int height, byte *data, byte *palette)
{
    int i;
    int length;
    pcx_t *pcx;
    byte *pack;

    pcx = Z_Malloc(width * height * 2 + 1000, PU_STATIC, NULL);

    pcx->manufacturer = 0x0a; // PCX id
    pcx->version = 5;         // 256 color
    pcx->encoding = 1;        // uncompressed
    pcx->bits_per_pixel = 8;  // 256 color
    pcx->xmin = 0;
    pcx->ymin = 0;
    pcx->xmax = SHORT(width - 1);
    pcx->ymax = SHORT(height - 1);
    pcx->hres = SHORT(width);
    pcx->vres = SHORT(height);
    memset(pcx->palette, 0, sizeof(pcx->palette));
    pcx->color_planes = 1; // chunky image
    pcx->bytes_per_line = SHORT(width);
    pcx->palette_type = SHORT(2); // not a grey scale
    memset(pcx->filler, 0, sizeof(pcx->filler));

    // pack the image
    pack = &pcx->data;

    for (i = 0; i < width * height; i++)
    {
        if ((*data & 0xc0) != 0xc0)
            *pack++ = *data++;
        else
        {
            *pack++ = 0xc1;
            *pack++ = *data++;
        }
    }

    // write the palette
    *pack++ = 0x0c; // palette ID byte
    for (i = 0; i < 768; i++)
        *pack++ = *palette++;

    // write output file
    length = pack - (byte *)pcx;
    M_WriteFile(filename, pcx, length);

    Z_Free(pcx);
}

//
// M_ScreenShot
//
void M_ScreenShot(void)
{
    int i;
    byte *linear;
    char lbmname[12];

    // munge planar buffer to linear
    linear = screens[2];
    I_ReadScreen(linear);

    // find a file name to save it to
    strcpy(lbmname, "DOOM00.pcx");

    for (i = 0; i <= 99; i++)
    {
        lbmname[4] = i / 10 + '0';
        lbmname[5] = i % 10 + '0';
        if (access(lbmname, 0) == -1)
            break; // file doesn't exist
    }
    if (i == 100)
        I_Error("M_ScreenShot: Couldn't create a PCX");

    // save the pcx file
    WritePCXfile(lbmname,
                 SCREENWIDTH, SCREENHEIGHT,
                 linear,
                 W_CacheLumpName("PLAYPAL", PU_CACHE));

    players[consoleplayer].message = "screen shot";
}
