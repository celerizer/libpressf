#ifndef PRESS_F_2102_C
#define PRESS_F_2102_C

#include <stdlib.h>
#include <string.h>

#include "2102.h"

static const char *name = "Fairchild 2102/2102L (1024 x 1 Static RAM)";
static int type = F8_DEVICE_2102;

/*    ==================    */
/* A6 ||  1        16 || A7 */
/* A5 ||  2        15 || A6 */
/* RW ||  3        14 || A9 */
/* A1 ||  4        15 || CS */
/* A2 ||  5        12 || O  */
/* A3 ||  6        11 || I  */
/* A4 ||  7        10 || NA */
/* A0 ||  8         9 || NA */
/*    ==================    */

/* The pins are wired oddly in Videocarts that use this chip. */

/*              7   6   5   4   3   2   1   0  */
/* port a (p20) OUT  -   -   -  IN  A2  A3  RW */
/* port b (p21) A9  A8  A7  A1  A6  A5  A4  A0 */

typedef union
{
  struct
  {
    u8 rw : 1;
    u8 a3 : 1;
    u8 a2 : 1;
    u8 in : 1;
    u8 unused : 3;
    u8 out : 1;
  } bits;
  u8 raw;
} f2102_out_write_t;

typedef union
{
  struct
  {
    u8 a0 : 1;
    u8 a4 : 1;
    u8 a5 : 1;
    u8 a6 : 1;
    u8 a1 : 1;
    u8 a7 : 1;
    u8 a8 : 1;
    u8 a9 : 1;
  } bits;
  u8 raw;
} f2102_out_address_t;

F8D_OP_OUT(f2102_out_address)
{
  f2102_t *m_f2102 = (f2102_t*)device->device;
  f2102_out_address_t address;

  address.raw = value.u;

  /* Setup most of the addressing bits */
  m_f2102->address.bits.a0 = address.bits.a0;
  m_f2102->address.bits.a1 = address.bits.a1;
  m_f2102->address.bits.a4 = address.bits.a4;
  m_f2102->address.bits.a5 = address.bits.a5;
  m_f2102->address.bits.a6 = address.bits.a6;
  m_f2102->address.bits.a7 = address.bits.a7;
  m_f2102->address.bits.a8 = address.bits.a8;
  m_f2102->address.bits.a9 = address.bits.a9;

  *io_data = value;
}

F8D_OP_OUT(f2102_out_write)
{
  f2102_t *m_f2102 = (f2102_t*)device->device;
  f2102_out_write_t write;
  f8_byte *data;
  int bit;

  write.raw = value.u;

  /* Setup final two addressing bits */
  m_f2102->address.bits.a0 = write.bits.a2;
  m_f2102->address.bits.a1 = write.bits.a3;

  data = &device->data[m_f2102->address.raw.u / 8];
  bit = (1 << (m_f2102->address.raw.u % 8));

  /* Are we writing data? */
  if (write.bits.rw)
    data->u = (write.bits.in) ? (data->u & bit) : (data->u & ~bit);
  /* No, we're reading it. Turn output bit on/off for next IN instruction. */
  else
  {
    write.bits.out = data->u & bit;
    io_data->u = write.raw;
  }
}

void f2102_init(f8_device_t *device)
{
   if (!device)
     return;
   else
   {
     f2102_t *m_f2102 = (f2102_t*)calloc(sizeof(f2102_t), 1);
     device->device = m_f2102;
     device->data = m_f2102->data;
     device->name = name;
     device->type = type;
     device->flags = F8_NO_ROMC;
   }
}

void f2012_serialize(void *buffer, unsigned size, unsigned *offset, f8_device_t *device)
{
  /*
  f2102_t *m_f2102 = (f2102_t*)device->device;

  if (buffer)
    memcpy(buffer + *offset, m_f2102->data, sizeof(f2102_t));
  *offset += sizeof(f2102_t);
  */
}

void f2102_unserialize(void *buffer, unsigned size, unsigned *offset, f8_device_t *device)
{
  /*
  f2102_t *m_f2102 = (f2102_t*)device->device;

  if (buffer)
    memcpy(m_f2102->data, buffer + *offset, sizeof(f2102_t));
  *offset += sizeof(f2102_t);
  */
}

#endif
