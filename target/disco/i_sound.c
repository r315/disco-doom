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

#define NUM_CHANNELS    8
#define SAMPLERATE      11025 // Hz
#define SAMPLECOUNT     512

//#define NO_AUDIO

#ifndef NO_AUDIO
// Needed for calling the actual sound output.
static audiospec_t specs;
int lengths[NUMSFX];
// The channel step amount...
unsigned int channelstep[NUM_CHANNELS];
// ... and a 0.16 bit remainder of last step.
unsigned int channelstepremainder[NUM_CHANNELS];
// The channel data pointers, start and end.
unsigned char *channels[NUM_CHANNELS];
unsigned char *channelsend[NUM_CHANNELS];
// Time/gametic that the channel started playing,
//  used to determine oldest, which automatically
//  has lowest priority.
// In case number of active sounds exceeds
//  available channels.
int	channelstart[NUM_CHANNELS];
// The sound in channel handles,
//  determined on registration,
//  might be used to unregister/stop/modify,
//  currently unused.
int channelhandles[NUM_CHANNELS];
// SFX id of the playing sound effect.
// Used to catch duplicates (like chainsaw).
int	channelids[NUM_CHANNELS];
// Pitch to stepping lookup, unused.
int	steptable[256];
// Pitch to stepping lookup, unused.
int	steptable[256];
// Volume lookups.
int	*vol_lookup;
// Hardware left and right channel volume lookup.
int *channelleftvol_lookup[NUM_CHANNELS];
int *channelrightvol_lookup[NUM_CHANNELS];
#endif

#ifndef NO_AUDIO
//
// This function loads the sound data from the WAD lump,
//  for single sound.
//
void *getsfx(char *sfxname, int *len)
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
int addsfx(int sfxid, int volume, int step, int seperation)
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
        for (i=0 ; i<NUM_CHANNELS ; i++)
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
    for (i=0; (i<NUM_CHANNELS) && (channels[i]); i++)
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
    if (i == NUM_CHANNELS){
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
#endif
//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(sfxinfo_t *sfx)
{
    char namebuf[9];
    sprintf(namebuf, "ds%s", sfx->name);
    return W_GetNumForName(namebuf);
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
void I_SetChannels()
{
#ifndef NO_AUDIO
    // Init internal lookups (raw data, mixing buffer, channels).
    // This function sets up internal lookups used during
    //  the mixing process. 
    int		i;
    int		j;
    
    int*	steptablemid = steptable + 128;
  
    // Okay, reset internal mixing channels to zero.
    /*for (i=0; i<NUM_CHANNELS; i++)
    {
        channels[i] = 0;
    }*/

    // This table provides step widths for pitch parameters.
    // I fail to see that this is currently used.
    for (i=-128 ; i<128 ; i++){
        steptablemid[i] = (int)(pow(2.0, (i/64.0)) * 65536.0);
        //printf("steptable[%d] = %d\n", i, steptablemid[i]);
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
#endif
}

void I_SetSfxVolume(int volume)
{
    printf("I_SOUND: %s %d\n",__FUNCTION__, volume);
}

// MUSIC API - dummy. Some code from DOS version.
void I_SetMusicVolume(int volume)
{
    printf("%s\n",__FUNCTION__);
}

void I_StopSound(int handle)
{
    printf("%s\n",__FUNCTION__);
#ifndef NO_AUDIO
    AUDIO_Stop(&specs);
#endif
}

int I_SoundIsPlaying(int handle)
{
    //printf("%s, handle %d, gametic %d\n",__FUNCTION__, handle, gametic);
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
#ifndef NO_AUDIO
void I_UpdateSound(void *stream, uint32_t len)
{
    // Mix current sound data.
    // Data, from raw sound, for right and left.
    unsigned int	sample;
    int		dl;
    int		dr;
  
    // Pointers in audio stream, left, right, end.
    signed short*		leftout;
    signed short*		rightout;
    signed short*		leftend;
    // Step in stream, left and right, thus two.
    int				step;

    // Mixing channel index.
    int				chan;
    
    // Left and right channel
    //  are in audio stream, alternating.
    leftout = (signed short *)stream;
    rightout = ((signed short *)stream) + 1;
    step = 2;

    // Determine end, for left channel only
    //  (right channel is implicit).
    leftend = leftout + len * step;

    // Mix sounds into the mixing buffer.
    // Loop over step * SAMPLECOUNT,
    // that is 512 values for two channels.
    while (leftout != leftend)
    {
	    // Reset left/right value. 
	    dl = 0;
	    dr = 0;

	    // Love thy L2 chache - made this a loop.
	    // Now more channels could be set at compile time
	    //  as well. Thus loop those  channels.
	    for ( chan = 0; chan < NUM_CHANNELS; chan++ )
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
		        channelstepremainder[ chan ] &= 65536 - 1;
		        // Check whether we are done.
		        if (channels[ chan ] >= channelsend[ chan ]){
		            channels[ chan ] = 0;
                }
	        }
	    }

	    // Clamp to range. Left hardware channel.
	    // Has been char instead of short.
	    // if (dl > 127) *leftout = 127;
	    // else if (dl < -128) *leftout = -128;
	    // else *leftout = dl;
	    if (dl > 0x7fff){
	        *leftout = 0x7fff;
        }else if (dl < -0x8000){
	        *leftout = -0x8000;
        }else{
	        *leftout = dl;
        }

	    // Same for right hardware channel.
	    if (dr > 0x7fff){
	        *rightout = 0x7fff;
        }else if (dr < -0x8000){
	        *rightout = -0x8000;
        }else{
	        *rightout = dr;
        }	
        // Increment current pointers in stream
	    leftout += step;
	    rightout += step;
    }
}
#endif
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
    printf("%s\n",__FUNCTION__);
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
 #ifndef NO_AUDIO
    int handle;
  
//SDL_LockAudio();
    handle = addsfx( id, vol, steptable[pitch], sep );
//SDL_UnlockAudio();

    //printf("I_Sound: Starting sound %d with handle %d\n", id, handle );

    AUDIO_Play(&specs);
#endif
    return handle;
}

//
// Initialises audio driver and precaches audio samples
//
// 
void I_InitSound()
{
#ifndef NO_AUDIO
    specs.channels = 1;
    specs.freq = SAMPLERATE;
    specs.size = SAMPLECOUNT;
    specs.callback = I_UpdateSound;
    specs.volume = DEFAULT_VOLUME;

    printf("I_InitSound: ");

    AUDIO_Init(&specs);

    printf("configured audio device with %d samples/slice\n", (int)specs.size);
    
    vol_lookup = (int*)malloc(128 * 256 * sizeof(int));

    if (vol_lookup == NULL)
        I_Error("Couldn't allocate memory for volume lookup table");
    for (uint8_t i = 1; i < NUMSFX; i++)
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

    printf("I_InitSound: pre-cached all sound data\n");
    printf("I_InitSound: sound module ready\n");
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
