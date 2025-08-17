#include "../dma.h"

#include "f8_device.h"

#include <string.h>

u8 f8_generic_serialize(const f8_device_t *device, u8 *buffer,
  unsigned *offset, unsigned size)
{
  if (!device || !device->data || !buffer)
    return FALSE;
  else if (*offset + device->length > size)
    return FALSE;
  else
  {
    memcpy(buffer + *offset, device->data, device->length);
    *offset += device->length;

    return TRUE;
  }
}

u8 f8_generic_unserialize(f8_device_t *device, const u8 *buffer,
  unsigned *offset, unsigned size)
{
  if (!device || !device->data || !buffer)
    return FALSE;
  else if (*offset + device->length > size)
    return FALSE;
  else
  {
    memcpy(device->data, buffer + *offset, device->length);
    *offset += device->length;

    return TRUE;
  }
}

void f8_generic_init(f8_device_t *device, unsigned size)
{
  if (device)
  {
    /* Only allocate if not using fixed memory map */
#if PF_ROMC
    device->data = pf_dma_alloc(size, FALSE);
#endif
    device->length = (u16)size;
    device->serialize = f8_generic_serialize;
    device->unserialize = f8_generic_unserialize;
  }
}
