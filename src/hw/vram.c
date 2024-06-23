#include "vram.h"
#include "../screen.h"

#include <stdlib.h>

static const char *name = "VRAM";
static const int type = F8_DEVICE_MK4027;

#define RAM_WRT (1 << 5)

F8D_OP_OUT(mk4027_write)
{
  vram_t *m_vram = (vram_t*)device->device;

  /* Write to VRAM if RAM_WRT is pulsed */
  if (!(io_data->u & RAM_WRT) && (value.u & RAM_WRT))
    vram_write(m_vram->data, m_vram->x, m_vram->y, m_vram->color);

  *io_data = value;
}

F8D_OP_OUT(mk4027_color)
{
  vram_t *m_vram = (vram_t*)device->device;

  if (m_vram)
    m_vram->color = (value.u & B11000000) >> 6;

  *io_data = value;
}

F8D_OP_OUT(mk4027_set_x)
{
  vram_t *m_vram = (vram_t*)device->device;

  if (m_vram)
    m_vram->x = (value.u ^ 0xFF) & B01111111;

  *io_data = value;
}

F8D_OP_OUT(mk4027_set_y)
{
  vram_t *m_vram = (vram_t*)device->device;

  if (m_vram)
    m_vram->y = (value.u ^ 0xFF) & B00111111;

  *io_data = value;
}

void vram_init(f8_device_t *device)
{
  if (device)
  {
    device->device = (vram_t*)malloc(sizeof(vram_t));
    device->name = name;
    device->type = type;
    device->flags = F8_NO_ROMC;
  }
}
