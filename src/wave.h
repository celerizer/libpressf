#ifndef PRESS_F_WAVE_H
#define PRESS_F_WAVE_H

#include "config.h"

#if PF_FLOATING_POINT

#include "types.h"

#define PF_PI 3.141592653589793115997963468544185161590576171875f

/* Largest Taylor series factorial a float can hold */
#define PF_TERMS 32

float pf_wave(float x, u8 cosine);

void pf_generate_wavetables(u8 table[3][PF_SOUND_SAMPLES]);

#endif

#endif
