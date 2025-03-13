#include "../dma.h"

#include "3850.h"

static const char *name = "Fairchild 3850 (Central Processing Unit)";
static const int type = F8_DEVICE_3850;

void f3850_init(f8_device_t *device)
{
  if (device)
  {
    if (!device->device)
      device->device = (f3850_t*)pf_dma_alloc(sizeof(f3850_t), TRUE);
    device->name = name;
    device->type = type;

    device->init = f3850_init;
  }
}
