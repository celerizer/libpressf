#ifndef PRESS_F_F8_DEVICE_H
#define PRESS_F_F8_DEVICE_H

#include "../config.h"
#include "../types.h"

typedef enum
{
  F8_DEVICE_INVALID = 0,

  F8_DEVICE_2102,
  F8_DEVICE_2102L,
  F8_DEVICE_2111,

  /* F8 system main CPU */
  F8_DEVICE_3850,

  /* 1024 byte program storage unit */
  F8_DEVICE_3851,

  /* 2048 byte program storage unit */
  F8_DEVICE_3856,

  F8_DEVICE_KDBUG,
  f8_DEVICE_FAIRBUG_SERIAL,
  F8_DEVICE_FAIRBUG_PARALLEL,

  /* VRAM device used in the Channel F */
  F8_DEVICE_MK4027,

  /* 4-tone beeper for audio in the Channel F */
  F8_DEVICE_BEEPER,

  /* 4-button control on the console for the Channel F */
  F8_DEVICE_SELECTOR_CONTROL,

  /* A controller for the Channel F */
  F8_DEVICE_HAND_CONTROLLER,

  /* An LED used in Schach to indicate when the computer is thinking */
  F8_DEVICE_SCHACH_LED,

  /* Extra RAM chip used in the Schach videocart */
  F8_DEVICE_2114,

  F8_DEVICE_SIZE
} f8_device_id_t;

/*
  Some devices may not actually have registers that are included in the
  spec. These flags will prevent accessing them if needed.
*/
#define F8_NO_PC0 (1 << 0)
#define F8_NO_PC1 (1 << 1)
#define F8_NO_DC0 (1 << 2)
#define F8_NO_DC1 (1 << 3)
#define F8_NO_ROMC (F8_NO_PC0 | F8_NO_PC1 | F8_NO_DC0 | F8_NO_DC1)

/*
  The memory region includes cartridge, BIOS, or otherwise copyrighted data.
  Its contents should not be included during serialization.
*/
#define F8_HAS_COPYRIGHTED_DATA (1 << 4)

/*
  The memory region is writable.
  For example, this would be TRUE on a RAM chip but FALSE on a cartridge.
*/
#define F8_DATA_WRITABLE (1 << 5)

/*
  The memory region is mapped to a battery-backed SRAM chip.
  Memory regions with this flag should have their data flushed to the disk.
*/
#define F8_HAS_BATTERY (1 << 6)

/* Arbitrary number of maximum mappings that can be applied to a device */
#define F8_MAPPING_MAX 2

typedef void F8D_MMAP_IN_T(struct f8_device_t*, u16);

typedef void F8D_MMAP_OUT_T(struct f8_device_t*, u16, f8_byte);

typedef struct
{
  /* Pointer to the mapped data */
  f8_byte *data;

  /* Beginning address of mapped memory */
  u16 start;

  /* End address of mapped memory */
  u16 end;

  /* Length of mapped memory */
  u16 length;

  /* Mask that can be applied to address for wrapping around */
  u16 mask;

  /* A function to be called when a byte is read from this region */
  F8D_MMAP_IN_T *func_in;

  /* A function to be called when a byte is written to this region */
  F8D_MMAP_OUT_T *func_out;
} f8_device_mapping_t;

typedef struct f8_device_t
{
  const char *name;

/*
   Hardware
*/
#if PF_ROMC
  f8_word pc0;
  f8_word pc1;
  f8_word dc0;
  f8_word dc1;
#endif

  f8_device_mapping_t mappings[F8_MAPPING_MAX];

  f8_device_id_t type;

/*
  Bitfield specifying some properties of the device and how to access it.
  See the F8 flags defined above.
*/
  u32 flags;

/*
  Device-specific behavior
*/
  void *device;
  void (*init)(struct f8_device_t *device);
  void (*free)(struct f8_device_t *device);
  void (*reset)(struct f8_device_t *device);

  /**
   * A pointer to a function that outputs serialized data for the machine
   * state. For example, for savestates, rewind, and netplay.
   * @param device The device to serialize.
   * @param buffer The buffer to write the serialized data to.
   * @param offset The current offset in the buffer to write to.
   * @param size The size of the buffer.
   * @return TRUE if serialization was successful, FALSE otherwise.
   */
  u8 (*serialize)(const struct f8_device_t *device, u8 *buffer,
    unsigned *offset, unsigned size);

  /**
   * A pointer to a function that reads in serialized data. For example, for
   * savestates, rewind, and netplay.
   * @param device The device to unserialize.
   * @param buffer The buffer to read the serialized data from.
   * @param offset The current offset in the buffer to read from.
   * @param size The size of the buffer.
   * @return TRUE if unserialization was successful, FALSE otherwise.
   */
  u8 (*unserialize)(struct f8_device_t *device, const u8 *buffer,
    unsigned *offset, unsigned size);

  /**
   * A pointer to a function that informs the device of the point of execution
   * in the current frame. Called before inputting or outputting data for this
   * port. For example, outputting sound in the correct spot on a waveform.
   * @param device The device to set the timing for.
   * @param current The current point in the frame, in CPU cycles.
   * @param total The total number of CPU cycles in the frame.
   */
  void (*set_timing)(struct f8_device_t *device, int current, int total);

  /**
   * A pointer to a function that is called at the end of a frame, to finalize
   * any data sent to the device over the course of a frame. For example,
   * rasterizing a framebuffer after a series of VRAM calls.
   */
  void (*finish_frame)(struct f8_device_t *device);
} f8_device_t;

/**
 * A function called on an F8 device when the main CPU executes an IN or
 * INS instruction.
 * The accumulator is then set to the value in "io_data" automatically.
 */
#define F8D_OP_IN(a) void a(f8_device_t *device, f8_byte *io_data)

/**
 * A function called on an F8 device when the main CPU executes an OUT or
 * OUTS instruction.
 * Implementations of this function must set "io_data" to "value".
 */
#define F8D_OP_OUT(a) void a(f8_device_t *device, f8_byte *io_data, f8_byte value)

/** @see F8D_OP_IN */
typedef void F8D_OP_IN_T(f8_device_t*, f8_byte*);

/** @see F8D_OP_OUT */
typedef void F8D_OP_OUT_T(f8_device_t*, f8_byte*, f8_byte);

void f8_generic_init(f8_device_t *device, unsigned size);

u8 f8_generic_serialize(const f8_device_t *device, u8 *buffer,
  unsigned *offset, unsigned size);

u8 f8_generic_unserialize(f8_device_t *device, const u8 *buffer,
  unsigned *offset, unsigned size);

#endif
