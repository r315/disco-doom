
#ifndef _audio_h_
#define _audio_h_

#include "board.h"

#define AUDIO_MAX_BUFF_SIZE     4096
#define DEFAULT_VOLUME          20


typedef struct audiospec{
    uint32_t freq;
    uint32_t channels;
    uint32_t size;
    uint32_t volume;
    uint16_t *buf;
    uint16_t playing;
    void (*callback)(void *stream, uint32_t len);
}audiospec_t;


void AUD_Init(audiospec_t *spec);
void AUD_Start(audiospec_t *spec);
void AUD_HW_Init(audiospec_t *spec);
void AUD_Start(audiospec_t *spec);
#endif