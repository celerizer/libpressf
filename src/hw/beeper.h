#ifndef PRESS_F_BEEPER_H
#define PRESS_F_BEEPER_H

#include "../config.h"
#include "f8_device.h"

typedef struct
{
  int current_cycles;
  int total_cycles;
  u8 frequency_last;
  unsigned last_tick;
  float amplitude;
  unsigned time;
  u8 frequencies[PF_SOUND_SAMPLES];
  short samples[PF_SOUND_SAMPLES * 2];
} f8_beeper_t;

F8D_OP_OUT(beeper_out);

void beeper_init(f8_device_t *device);

#endif
