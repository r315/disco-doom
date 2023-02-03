#ifndef _TARGET_H_
#define _TARGET_H_

#if defined(linux) || defined (__WIN32__)
#include <SDL.h>
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "SDL_mutex.h"
//#include "SDL_byteorder.h"
#include "SDL_version.h"
#endif

int T_GetTick(void);
void T_Delay(int ms);

#if __ARM_EABI__
#include "board.h"
#endif
#endif
