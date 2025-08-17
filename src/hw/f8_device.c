#include "../dma.h"

#include "f8_device.h"

#include <string.h>

u8 f8_generic_serialize(const f8_device_t *device, u8 *buffer,
  unsigned *offset, unsigned size)
{
  if (!device || !device->mappings[0].data || !buffer)
    return FALSE;
  else if (*offset + device->mappings[0].length > size)
    return FALSE;
  else
  {
    memcpy(buffer + *offset, device->mappings[0].data, device->mappings[0].length);
    *offset += device->mappings[0].length;

    return TRUE;
  }
}

u8 f8_generic_unserialize(f8_device_t *device, const u8 *buffer,
  unsigned *offset, unsigned size)
{
  if (!device || !device->mappings[0].data || !buffer)
    return FALSE;
  else if (*offset + device->mappings[0].length > size)
    return FALSE;
  else
  {
    memcpy(device->mappings[0].data, buffer + *offset, device->mappings[0].length);
    *offset += device->mappings[0].length;

    return TRUE;
  }
}

void f8_generic_init(f8_device_t *device, unsigned size)
{
  if (device)
  {
    /* Only allocate if not using fixed memory map */
#if PF_ROMC
    device->mappings[0].data = pf_dma_alloc(size, FALSE);
#endif
    device->mappings[0].length = (u16)size;
    device->serialize = f8_generic_serialize;
    device->unserialize = f8_generic_unserialize;
  }
}
