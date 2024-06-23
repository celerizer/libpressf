#ifndef PRESS_F_2102_H
#define PRESS_F_2102_H

#include "f8_device.h"

#define F2102_SIZE (1024 / 8)

typedef union
{
  struct
  {
    u8 a0 : 1;
    u8 a1 : 1;
    u8 a2 : 1;
    u8 a3 : 1;
    u8 a4 : 1;
    u8 a5 : 1;
    u8 a6 : 1;
    u8 a7 : 1;
    u8 a8 : 1;
    u8 a9 : 1;
    u8 unused : 6;
  } bits;
  f8_word raw;
} f2102_address_t;

typedef struct f2102_t
{
  f8_byte data[F2102_SIZE];
  f2102_address_t address;
} f2102_t;

F8D_OP_OUT(f2102_out_address);
F8D_OP_OUT(f2102_out_write);

void f2102_init(f8_device_t *device);

#endif
