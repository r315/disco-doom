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
//	System interface for sound.
//
//-----------------------------------------------------------------------------

static const char
    rcsid[] = "$Id: i_unix.c,v 1.5 1997/02/03 22:45:10 b1 Exp $";

#include <stdlib.h>
#include <math.h>

#include "doomdef.h"
#include "audio.h"
#include "z_zone.h"
#include "i_system.h"
#include "i_sound.h"
#include "w_wad.h"
#include "g_game.h"
#include "common.h"

#define SOUND_ENABLED   1

#define NUM_SFX_CHANNELS    8
#define SAMPLERATE          11025 // Hz
#define SAMPLECOUNT         512
#define NUM_AUDIO_CHANNELS  2 // Stereo

#if SOUND_ENABLED
// Needed for calling the actual sound output.
static audiospec_t  specs;
static int          lengths[NUM_SFX];
// The channel step amount...
unsigned int        channelstep[NUM_SFX_CHANNELS];
// ... and a 0.16 bit remainder of last step.
unsigned int        channelstepremainder[NUM_SFX_CHANNELS];
// The channel data pointers, start and end.
unsigned char       *channels[NUM_SFX_CHANNELS];
unsigned char       *channelsend[NUM_SFX_CHANNELS];
// Time/gametic that the channel started playing,
//  used to determine oldest, which automatically
//  has lowest priority.
// In case number of active sounds exceeds
//  available channels.
static int	        channelstart[NUM_SFX_CHANNELS];
// The sound in channel handles,
//  determined on registration,
//  might be used to unregister/stop/modify,
//  currently unused.
static int          channelhandles[NUM_SFX_CHANNELS];
// SFX id of the playing sound effect.
// Used to catch duplicates (like chainsaw).
static int	        channelids[NUM_SFX_CHANNELS];
// Pitch to stepping lookup, unused.
static int	        steptable[256];
// Hardware left and right channel volume lookup.
static int          *channelleftvol_lookup[NUM_SFX_CHANNELS];
static int          *channelrightvol_lookup[NUM_SFX_CHANNELS];
// Volume lookups.
static int          *vol_lookup;

static void         *next_buffer;
//
// This function loads the sound data from the WAD lump,
//  for single sound.
//
static void *getsfx(char *sfxname, int *len)
{
    unsigned char *sfx;
    unsigned char *paddedsfx;
    int i;
    int size;
    int paddedsize;
    char name[20];
    int sfxlump;

    // Get the sound data from the WAD, allocate lump
    //  in zone memory.
    sprintf(name, "ds%s", sfxname);

    // Now, there is a severe problem with the
    //  sound handling, in it is not (yet/anymore)
    //  gamemode aware. That means, sounds from
    //  DOOM II will be requested even with DOOM
    //  shareware.
    // The sound list is wired into sounds.c,
    //  which sets the external variable.
    // I do not do runtime patches to that
    //  variable. Instead, we will use a
    //  default sound for replacement.
    if (W_CheckNumForName(name) == -1)
        sfxlump = W_GetNumForName("dspistol");
    else
        sfxlump = W_GetNumForName(name);

    size = W_LumpLength(sfxlump);

    // Debug.
    // fprintf( stderr, "." );
    //fprintf( stderr, " -loading  %s (lump %d, %d bytes)\n",
    //	     sfxname, sfxlump, size );
    //fflush( stderr );

    sfx = (unsigned char *)W_CacheLumpNum(sfxlump, PU_STATIC);

    // Pads the sound effect out to the mixing buffer size.
    // The original realloc would interfere with zone memory.
    paddedsize = ((size - 8 + (SAMPLECOUNT - 1)) / SAMPLECOUNT) * SAMPLECOUNT;

    // Allocate from zone memory.
    paddedsfx = (unsigned char *)Z_Malloc(paddedsize + 8, PU_STATIC, 0);
    // ddt: (unsigned char *) realloc(sfx, paddedsize+8);
    // This should interfere with zone memory handling,
    //  which does not kick in in the soundserver.

    // Now copy and pad.
    memcpy(paddedsfx, sfx, size);
    for (i = size; i < paddedsize + 8; i++)
        paddedsfx[i] = 128;

    // Remove the cached lump.
    Z_Free(sfx);

    // Preserve padded length.
    *len = paddedsize;

    // Return allocated padded data.
    return (void *)(paddedsfx + 8);
}

//
// This function adds a sound to the
//  list of currently active sounds,
//  which is maintained as a given number
//  (eight, usually) of internal channels.
// Returns a handle.
//
static int addsfx(int sfxid, int volume, int step, int seperation)
{   
    static unsigned short	handlenums = 0;
 
    int		i;
    int		handle = -1;
    
    int		oldest = gametic;
    int		oldestnum = 0;
    int		slot;

    int		rightvol;
    int		leftvol;

    // Chainsaw troubles.
    // Play these sound effects only one at a time.
    if ( sfxid == sfx_sawup
	 || sfxid == sfx_sawidl
	 || sfxid == sfx_sawful
	 || sfxid == sfx_sawhit
	 || sfxid == sfx_stnmov
	 || sfxid == sfx_pistol	 )
    {
        // Loop all channels, check.
        for (i=0 ; i<NUM_SFX_CHANNELS ; i++)
        {
            // Active, and using the same SFX?
            if ( (channels[i]) && (channelids[i] == sfxid) )
            {
                // Reset.
                channels[i] = 0;
                // We are sure that iff,
                //  there will only be one.
                break;
            }
        }
    }

    // Loop all channels to find oldest SFX.
    for (i=0; (i<NUM_SFX_CHANNELS) && (channels[i]); i++)
    {
        if (channelstart[i] < oldest)
        {
            oldestnum = i;
            oldest = channelstart[i];
        }
    }

    // Tales from the cryptic.
    // If we found a channel, fine.
    // If not, we simply overwrite the first one, 0.
    // Probably only happens at startup.
    if (i == NUM_SFX_CHANNELS){
	    slot = oldestnum;
    }else{
	    slot = i;
    }

    // Okay, in the less recent channel,
    //  we will handle the new SFX.
    // Set pointer to raw data.
    channels[slot] = (unsigned char *) S_sfx[sfxid].data;
    // Set pointer to end of raw data.
    channelsend[slot] = channels[slot] + lengths[sfxid];

    // Reset current handle number, limited to 0..100.
    if (!handlenums){
	    handlenums = 100;
    }

    // Assign current handle number.
    // Preserved so sounds could be stopped (unused).
    channelhandles[slot] = handle = handlenums++;

    // Set stepping???
    // Kinda getting the impression this is never used.
    channelstep[slot] = step;
    // ???
    channelstepremainder[slot] = 0;
    // Should be gametic, I presume.
    channelstart[slot] = gametic;

    // Separation, that is, orientation/stereo.
    //  range is: 1 - 256
    seperation += 1;

    // Per left/right channel.
    //  x^2 seperation,
    //  adjust volume properly.
    volume *= 8;
    leftvol =
	volume - ((volume*seperation*seperation) >> 16); ///(256*256);
    seperation = seperation - 257;
    rightvol =
	volume - ((volume*seperation*seperation) >> 16);	

    // Sanity check, clamp volume.
    if (rightvol < 0 || rightvol > 127){
        I_Error("rightvol out of bounds");
    }
	
    
    if (leftvol < 0 || leftvol > 127){
        I_Error("leftvol out of bounds");
    }
    
    // Get the proper lookup table piece
    //  for this volume level???
    channelleftvol_lookup[slot] = &vol_lookup[leftvol*256];
    channelrightvol_lookup[slot] = &vol_lookup[rightvol*256];

    // Preserve sound SFX id,
    //  e.g. for avoiding duplicates of chainsaw.
    channelids[slot] = sfxid;

    // You tell me.
    return handle; 
}

static signed short clipSample(int sample)
{
    // Clamp to range. Left hardware channel.
    // Has been char instead of short.
    // if (dl > 127) *leftout = 127;
    // else if (dl < -128) *leftout = -128;
    // else *leftout = dl;
    if (sample > 0x7fff){
        return 0x7fff;
    }else if (sample < -0x8000){
        return -0x8000;
    }

    return sample; 
}

//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux
// version.
// See soundserver initdata().
//
static void I_SetChannels()
{
    // Init internal lookups (raw data, mixing buffer, channels).
    // This function sets up internal lookups used during
    // the mixing process. 
    int		i;
    int		j;
    
    int*	steptablemid = steptable + 128;
  
    // Okay, reset internal mixing channels to zero.
    /*for (i=0; i<NUM_SFX_CHANNELS; i++)
    {
        channels[i] = 0;
    }*/

    // This table provides step widths for pitch parameters.
    // I fail to see that this is currently used.
    for (i=-128 ; i<128 ; i++){
        steptablemid[i] = (int)(pow(2.0, (i/64.0)) * 65536.0);
        //COM_Print("steptable[%d] = %d\n", i, steptablemid[i]);
    }  
  
    // Generates volume lookup tables
    //  which also turn the unsigned samples
    //  into signed samples.
    for (i=0 ; i<128 ; i++){
        for (j=0 ; j<256 ; j++) {
            vol_lookup[i*256+j] = (i*(j-128)*256)/127;
            //fprintf(stderr, "vol_lookup[%d*256+%d] = %d\n", i, j, vol_lookup[i*256+j]);
        }
    }
}
#endif

void I_SetVolume(int volume)
{
    COM_Print("%s: %d\n",__FUNCTION__, volume);
}

// MUSIC API - dummy. Some code from DOS version.
void I_SetMusicVolume(int volume)
{
    COM_Print("%s: %d\n",__FUNCTION__, volume);
}

void I_StopSound(int handle)
{
    //COM_Print("%s handle: %d\n",__FUNCTION__, handle);
#if SOUND_ENABLED
    AUDIO_Stop(&specs);
#endif
}

int I_SoundIsPlaying(int handle)
{
    //COM_Print("%s, handle %d, gametic %d\n",__FUNCTION__, handle, gametic);
    // Ouch.
    return gametic < handle;
}

//
// This function loops all active (internal) sound
//  channels, retrieves a given number of samples
//  from the raw sound data, modifies it according
//  to the current (internal) channel parameters,
//  mixes the per channel samples into the given
//  mixing buffer, and clamping it to the allowed
//  range.
//
// This function currently supports only 16bit.
//
#if SOUND_ENABLED
static void I_MixSound(void *stream, int nsamples)
{
    // Mix current sound data.
    // Data, from raw sound, for right and left.
    unsigned int	sample;
    // Mixing channel index.
    int				chan;  
    // Pointers in audio stream, left, right, end.
    signed short*	leftout;
    signed short*	rightout;    
    // Left and right channel
    //  are in audio stream, alternating.
    leftout = (signed short *)stream;
    rightout = leftout + 1;

    // Mix sounds into the mixing buffer.
    while (nsamples--)
    {
	    // Reset left/right value. 
	    int dl = 0;
	    int dr = 0;

	    // Love thy L2 chache - made this a loop.
	    // Now more channels could be set at compile time
	    //  as well. Thus loop those  channels.
	    for ( chan = 0; chan < NUM_SFX_CHANNELS; chan++ )
	    {
	        // Check channel, if active.
	        if (channels[ chan ])
	        {
		        // Get the raw data from the channel. 
		        sample = *channels[ chan ];
		        // Add left and right part
		        //  for this channel (sound)
		        //  to the current data.
		        // Adjust volume accordingly.
		        dl += channelleftvol_lookup[ chan ][ sample ];
		        dr += channelrightvol_lookup[ chan ][ sample ];
		        // Increment index ???
		        channelstepremainder[ chan ] += channelstep[ chan ];
		        // MSB is next sample???
		        channels[ chan ] += channelstepremainder[ chan ] >> 16;
		        // Limit to LSB???
		        channelstepremainder[ chan ] &= 0xFFFF;
		        // Check whether we are done.
		        if (channels[ chan ] >= channelsend[ chan ]){
		            channels[ chan ] = 0;
                }
	        }
	    }

	    *leftout = clipSample(dl);
        *rightout = clipSample(dr);

        // Increment current pointers in stream
	    leftout += NUM_AUDIO_CHANNELS;
	    rightout += NUM_AUDIO_CHANNELS;
    }
}
#endif

void I_UpdateSound(void)
{
    if (next_buffer != NULL) {
		I_MixSound(next_buffer, specs.size);
		next_buffer = NULL;
	}
}

// 
// Callback function from SDL driver
//
static void I_UpdateCallback(void *stream, int len)
{
	next_buffer = stream;
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
    // I fail too see that this is used.
    // Would be using the handle to identify
    //  on which channel the sound might be active,
    //  and resetting the channel parameters.

    // UNUSED.
}

void I_ShutdownSound(void)
{

    COM_Print("%s\n",__FUNCTION__);
#if SOUND_ENABLED
    if(vol_lookup == NULL){
        free(vol_lookup);
    }
#endif
}

void I_PreCacheSounds(void)
{
#if SOUND_ENABLED
    for (uint8_t i = 1; i < NUM_SFX; i++)
    {
        // Alias? Example is the chaingun sound linked to pistol.
        if (!S_sfx[i].link)
        {
            // Load data from WAD file.
            S_sfx[i].data = getsfx(S_sfx[i].name, &lengths[i]);
        }
        else
        {
            // Previously loaded already?
            S_sfx[i].data = S_sfx[i].link->data;
            lengths[i] = lengths[(S_sfx[i].link - S_sfx) / sizeof(sfxinfo_t)];
        }
    }

    COM_Print("I_InitSound: pre-cached all sound data\n");
#endif
}

//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
int I_StartSound(int id, int vol, int sep, int pitch, int priority)
{
    int handle = 0;
#if SOUND_ENABLED
    handle = addsfx( id, vol, steptable[pitch], sep );
    //COM_Print("I_Sound: Starting sound %d with handle %d\n", id, handle );
#endif
    return handle;
}

//
// Initialises audio driver and precaches audio samples
//
// 
void I_InitSound()
{
#if SOUND_ENABLED
    specs.channels = NUM_AUDIO_CHANNELS;
    specs.freq = SAMPLERATE;
    specs.size = SAMPLECOUNT;
    specs.callback = (void(*)(void*, int))I_UpdateCallback;
    specs.volume = DEFAULT_VOLUME;

    AUDIO_Init(&specs);

    COM_Print("\tConfigured audio device with %d samples/slice\n", (int)specs.size);
    
    vol_lookup = (int*)malloc(128 * 256 * sizeof(int));

    if (vol_lookup == NULL)
        I_Error("\ntCouldn't allocate memory for volume lookup table");

    I_SetChannels();

    AUDIO_Start(&specs);

    COM_Print("\tsound module ready\n");
#endif
}

//
// MUSIC API.
// Still no music done.
// Remains. Dummies.
//
void I_InitMusic(void) {}
void I_ShutdownMusic(void) {}
void I_PlaySong(int handle, int looping){}
void I_PauseSong(int handle){}
void I_ResumeSong(int handle){}
void I_StopSong(int handle){}
void I_UnRegisterSong(int handle){}
int I_RegisterSong(void *data){return 1;}
int I_QrySongPlaying(int handle){ return 0; }
