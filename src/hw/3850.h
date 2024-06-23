#ifndef PRESS_F_3850_H
#define PRESS_F_3850_H

#include "f8_device.h"

#define F3850_SCRATCH_SIZE 64

typedef struct
{
  u8 l : 3;
  u8 h : 3;
  u8 unused : 2;
} f3850_isar_halves_t;

typedef struct
{
  u8 data : 6;
  u8 unused : 2;
} f3850_isar_raw_t;

typedef union
{
  f3850_isar_halves_t halves;
  f3850_isar_raw_t raw;
} f3850_isar_t;

typedef struct
{
  u8 sign : 1;
  u8 carry : 1;
  u8 zero : 1;
  u8 overflow : 1;
  u8 interrupts : 1;
  u8 unused : 3;
} f3850_status_register_flags_t;

typedef struct
{
  u8 data : 5;
  u8 unused : 3;
} f3850_status_register_raw_t;

typedef union
{
  f3850_status_register_flags_t flags;
  f3850_status_register_raw_t raw;
} f3850_status_register_t;

typedef struct f3850_t
{
  /* 8 bit */
  f8_byte accumulator;

  /* 6 bit */
  f3850_isar_t isar;

  /* 8 bit */
  f8_byte scratchpad[F3850_SCRATCH_SIZE];

  f3850_status_register_t status_register;
} f3850_t;

void f3850_init(f8_device_t *device);

#endif
