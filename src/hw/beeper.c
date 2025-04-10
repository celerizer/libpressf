#include "../dma.h"

#include "beeper.h"

static const char *name = "Beeper";
static const int type = F8_DEVICE_BEEPER;

#if !PF_FLOATING_POINT
#include "../wavetables/sin.h"
#else
#include "../wave.h"
static u8 sound_wavetable[3][PF_SOUND_SAMPLES];
static u8 sound_wavetables_initted = FALSE;
#endif

#if PF_AUDIO_ENABLE
static void sound_push_back(f8_beeper_t *beeper, unsigned frequency)
{
  unsigned current_tick;

  if (beeper->current_cycles >= beeper->total_cycles)
    current_tick = PF_SOUND_SAMPLES - 1;
  else if (beeper->total_cycles >= PF_SOUND_SAMPLES)
    current_tick = (beeper->current_cycles / (beeper->total_cycles / PF_SOUND_SAMPLES));
  else
    current_tick = (beeper->current_cycles * PF_SOUND_SAMPLES) / beeper->total_cycles;

  if (current_tick != beeper->last_tick)
  {
    unsigned i;

    for (i = beeper->last_tick; i < current_tick; i++)
      beeper->frequencies[i] = beeper->frequency_last;
  }
  beeper->last_tick = current_tick;
  beeper->frequency_last = frequency;
}
#endif

F8D_OP_OUT(beeper_out)
{
#if PF_AUDIO_ENABLE
  sound_push_back(device->device, value.u >> 6);
#else
  F8_UNUSED(device);
#endif
  *io_data = value;
}

void beeper_set_timing(f8_device_t *device, int current, int total)
{
  ((f8_beeper_t*)device->device)->current_cycles = current;
  ((f8_beeper_t*)device->device)->total_cycles = total;
}

void beeper_finish_frame(f8_device_t *device)
{
#if PF_AUDIO_ENABLE
  f8_beeper_t* m_beeper = ((f8_beeper_t*)device->device);
  unsigned i;

  /* Fill any unwritten samples with the last known tone */
  if (m_beeper->last_tick != PF_SOUND_SAMPLES - 1)
  {
    for (i = m_beeper->last_tick; i < PF_SOUND_SAMPLES; i++)
      m_beeper->frequencies[i] = m_beeper->frequency_last;
  }

  for (i = 0; i < PF_SOUND_SAMPLES; i++, m_beeper->time++)
  {
    if (m_beeper->frequencies[i] == 0)
    {
      /* Sound was turned off, reset the amplitude so our next sound pops */
      m_beeper->time = 0;
#if PF_FLOATING_POINT
      m_beeper->amplitude = PF_MAX_AMPLITUDE;
#endif
      m_beeper->samples[2 * i] = 0;
      m_beeper->samples[2 * i + 1] = 0;
    }
    else
    {
      /* Use sine wave to tell if our square wave is on or off */
      u8 on = sound_wavetable[m_beeper->frequencies[i] - 1]
                             [m_beeper->time % PF_SOUND_SAMPLES];
#if PF_FLOATING_POINT
      short final_sample = on ? (short)m_beeper->amplitude : 0;
#else
      short final_sample = on ? PF_MAX_AMPLITUDE : 0;
#endif
      m_beeper->samples[2 * i] = final_sample;
      m_beeper->samples[2 * i + 1] = final_sample;
#if PF_FLOATING_POINT
      m_beeper->amplitude *= PF_SOUND_DECAY;
#endif
    }
  }
  m_beeper->last_tick = 0;
#else
  F8_UNUSED(device);
#endif
}

void beeper_init(f8_device_t *device)
{
#if PF_FLOATING_POINT
  if (!sound_wavetables_initted)
  {
    pf_generate_wavetables(sound_wavetable);
    sound_wavetables_initted = TRUE;
  }
#endif
  if (device)
  {
    device->device = (f8_beeper_t*)pf_dma_alloc(sizeof(f8_beeper_t), TRUE);
    device->name = name;
    device->type = type;
    device->flags = F8_NO_ROMC;
    device->set_timing = beeper_set_timing;
    device->finish_frame = beeper_finish_frame;
  }
}
