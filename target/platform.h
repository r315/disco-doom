#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#if defined(linux) || defined (__WIN32__)
#include <SDL.h>
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "SDL_mutex.h"
//#include "SDL_byteorder.h"
#include "SDL_version.h"
#define PLATFORM_GetTicks() SDL_GetTicks()
#define PLATFORM_Delay(d) SDL_Delay(d)
#elif defined(__arm__)
#include "stm32f7xx_hal.h"
#define PLATFORM_GetTicks() HAL_GetTick()
#define PLATFORM_Delay(d) HAL_Delay(d)
#endif

#if defined (__WIN32__)
#include <stdint.h>
typedef uint8_t Uint8;
#endif
#endif
