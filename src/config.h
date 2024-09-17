#ifndef PRESS_F_CONFIG_H
#define PRESS_F_CONFIG_H

#include "types.h"

/**
 * Constantly checks the validity of pointers in main emulation loop.
 * Could be a performance hit and the program should be stable to the point
 * where this isn't needed.
 */
#ifndef PRESS_F_SAFETY
#define PRESS_F_SAFETY FALSE
#endif

/**
 * Enable audio processing.
 */
#ifndef PF_AUDIO_ENABLE
#define PF_AUDIO_ENABLE TRUE
#endif

/**
 * Whether or not build target is a big-endian machine.
 */
#ifndef PF_BIG_ENDIAN
#define PF_BIG_ENDIAN FALSE
#endif

/**
 * Enable the debugger.
 */
#ifndef PF_DEBUGGER
#define PF_DEBUGGER FALSE
#endif

/**
 * Defines whether or not to use static asserts at compile time.
 * Only necessary for developers.
 */
#ifndef PF_STATIC_ASSERTS
#define PF_STATIC_ASSERTS FALSE
#endif
#if PF_STATIC_ASSERTS
#define PF_STATIC_ASSERT(a, b) static_assert(a, b)
#else
#define PF_STATIC_ASSERT(a, b)
#endif

/**
 * Sampling frequency for sound.
 * Turn this down if audio causes slowdowns.
 */
#ifndef PF_SOUND_FREQUENCY
#define PF_SOUND_FREQUENCY 44100
#endif
PF_STATIC_ASSERT(!(PF_SOUND_FREQUENCY % 60),
                 "Audio frequency not evenly divisible by 60");

#ifndef PF_SOUND_DECAY
#define PF_SOUND_DECAY (1.0f - 77.0f / PF_SOUND_FREQUENCY)
#endif

#ifndef PF_SOUND_PERIOD
#define PF_SOUND_PERIOD (1.0f / PF_SOUND_FREQUENCY)
#endif

#ifndef PF_SOUND_SAMPLES
#define PF_SOUND_SAMPLES (PF_SOUND_FREQUENCY / 60)
#endif

/**
 * Determine whether HLE BIOS implementations should be used.
 * @todo This should be a runtime option as well.
 */
#ifndef PF_HAVE_HLE_BIOS
#define PF_HAVE_HLE_BIOS TRUE
#endif

/**
 * Minimum / maximum sound volume. Needs to be a signed short.
 */
#ifndef PF_MIN_AMPLITUDE
#define PF_MIN_AMPLITUDE 0x0000
#endif
#ifndef PF_MAX_AMPLITUDE
#define PF_MAX_AMPLITUDE 0x7FFF
#endif

/**
 * When set, the program is compiled without using dynamic memory allocation
 * via malloc/free. The program in this state will instead use a static
 * fixed-size heap, shared between all emulator instances. Use only if
 * absolutely necessary, such as on very limited embedded platforms.
 *
 * Requires PF_ROMC to be FALSE and a heap size (in bytes) must be
 * defined as PRESS_F_NO_DMA_SIZE.
 */
#ifndef PF_NO_DMA
#define PF_NO_DMA FALSE
#else
#if !defined(PF_NO_DMA_SIZE) || PF_NO_DMA_SIZE <= 0
#error "Static heap size not specified (define PF_NO_DMA_SIZE)."
#elif PF_ROMC
#error "PF_ROMC must be defined as FALSE if using no DMA."
#endif
#endif

/**
 * Controls whether or not to emulate ROMC functions.
 * Not emulating ROMC allows for using simpler instruction sets, but removes
 * some functionality.
 */
#ifndef PF_ROMC
#define PF_ROMC TRUE
#endif

/**
 * Controls whether or not to break redundant loops in ROMC functions.
 * While allowing this should be more accurate, the only way it should
 * change behavior is on miswired or otherwise damaged hardware.
 */
#ifndef PF_ROMC_REDUNDANCY
#define PF_ROMC_REDUNDANCY FALSE
#endif

/**
 * Controls whether or not memory regions that contain copyrighted material
 * (with flag F8_HAS_COPYRIGHTED_DATA) are included during serialization.
 */
#ifndef PF_SERIALIZE_COPYRIGHTED_CONTENT
#define PF_SERIALIZE_COPYRIGHTED_CONTENT FALSE
#endif

#endif
