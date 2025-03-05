#ifndef PRESS_F_SYSTEM_H
#define PRESS_F_SYSTEM_H

#include "3850.h"
#include "f8_device.h"
#include "../config.h"
#include "../software.h"

#define F8_MAX_IO_PORTS 128

/**
 * Arbitrary limit for max number of devices hooked up to a system.
 */
#define F8_MAX_DEVICES 32

/**
 * Arbitrary limit for max number of devices on one IO port.
 */
#define F8_MAX_IO_LINK 2

/**
 * A holder that serves as a "connection" between the emulated system and an
 * IO device.
 **/
typedef struct io_t
{
  struct f8_device_t *device_out[F8_MAX_IO_LINK];
  struct f8_device_t *device_in[F8_MAX_IO_LINK];
  F8D_OP_OUT_T *func_out[F8_MAX_IO_LINK];
  F8D_OP_IN_T *func_in[F8_MAX_IO_LINK];
  f8_byte data;
} io_t;

enum f8_f3850_clock
{
  /* 1.7897725MHz */
  F8_CLOCK_CHANNEL_F_NTSC = 298295417,

  /* 2.000000MHz */
  F8_CLOCK_CHANNEL_F_PAL_GEN_1 = 333333333,

  /* 1.970491MHz */
  F8_CLOCK_CHANNEL_F_PAL_GEN_2 = 328415167
};

#define F8_MHZ_TO_CLOCK(a) (a * 1000000 / 60 * 10000)

/* Runtime options */
typedef struct f8_settings_t
{
  /**
   * Determines which system preset to load if software identification fails.
   * See "software.h"
   */
  u8 default_system;

  /**
   * The clock speed of the main 3850 CPU. Represented as the unsigned integer
   * result of: MHz * 1,000,000 Hz/MHz / 60 frames/second * 10,000 (constant
   * used for some level of decimal precision as an integer). In other words,
   * CPU MHz * 166666666.667.
   * Should be able to be safely changed at runtime.
   * @see f8_f3850_clock
   */
  int f3850_clock_speed;

  /**
   * Bool: Always hookup a 2114 chip at $2800 under Channel F series presets.
   * This is very common for Channel F homebrew.
   */
  u8 cf_always_scach;

  /**
   * Bool: Rasterizes extra VRAM data to the framebuffer.
   */
  u8 cf_full_vram;

  /**
   * Bool: Allows any software to be played under Channel F series presets.
   * Done by NOPing $0015 and $0016 in the 3851 BIOS.
   */
  u8 cf_skip_cartridge_verification;

  /**
   * Bool: Enables "TV Powww!" mode, which applies a hack to the BIOS to force
   * a 15-second timer to any games that support it.
   * This flag also acts as a hint to the frontend to use microphone data to
   * control the game by inputting INPUT_PUSH.
   */
  u8 cf_tv_powww;
} f8_settings_t;

typedef struct f8_system_t
{
#if !PF_ROMC
  f8_word dc0;
  f8_word dc1;
  f8_word pc0;
  f8_word pc1;
  f8_byte memory[0x10000];
#endif

  f8_byte dbus;

  int cycles;

  f8_device_t f8devices[F8_MAX_DEVICES];
  unsigned f8device_count;
  f3850_t *main_cpu;

  io_t io_ports[F8_MAX_IO_PORTS];

  f8_settings_t settings;
} f8_system_t;

/**
 * Gets a pointer to the main CPU of an F8 system.
 * This returns the first 3850 unit in the devices array.
 */
f3850_t* f8_main_cpu(f8_system_t *system);

/**
 * Gets a pointer to the device wrapper of the main CPU of an F8 system.
 * This returns the first 3850 unit in the devices array.
 */
f8_device_t* f8_main_cpu_device(f8_system_t *system);

f8_byte f8_fetch(f8_system_t *system, unsigned address);

/**
 * Reads data from the F8 system into a buffer.
 * @param system A pointer to an F8 system.
 * @param dest A pointer to the destination buffer to read into.
 * @param address The virtual address in the F8 system to read from.
 * @param size The number of bytes to read.
 * @return The number of bytes read, equal to size if there is no error.
 */
unsigned f8_read(f8_system_t *system, void *dest, unsigned address,
                 unsigned size);

/**
 * Writes data from a buffer into the F8 system.
 * @param system A pointer to an F8 system.
 * @param address The virtual address in the F8 system to write to.
 * @param src A pointer to the data buffer to write from.
 * @param size The number of bytes to write.
 * @return The number of bytes written, equal to size if there is no error.
 */
unsigned f8_write(f8_system_t *system, unsigned address, const void *src,
                  unsigned size);

u8 f8_device_add(f8_system_t *system, f8_device_t *device);

u8 f8_device_init(f8_device_t *device, const f8_device_id_t type);

u8 f8_device_remove(f8_system_t *system, f8_device_t *device);

u8 f8_device_remove_index(f8_system_t *system, unsigned index);

u8 f8_device_set_start(f8_device_t *device, unsigned start);

u8 f8_settings_apply(f8_system_t *system, f8_settings_t settings);

u8 f8_settings_apply_default(f8_system_t *system);

u8 f8_system_init(f8_system_t *system, const system_preset_t *preset);

/**
 * Finds every F8 device in the system with a given type, and sets the function
 * pointer used for its IN/INS instruction callback.
 * Returns the number of devices found and set.
 */
unsigned f8_system_set_device_in_cb(f8_system_t *system,
  const f8_device_id_t type, F8D_OP_IN_T func);

/**
 * Finds every F8 device in the system with a given type, and sets the function
 * pointer used for its OUT/OUTS instruction callback.
 * Returns the number of devices found and set.
 */
unsigned f8_system_set_device_out_cb(f8_system_t *system,
  const f8_device_id_t type, F8D_OP_OUT_T func);

#endif
