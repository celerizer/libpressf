#include <stdlib.h>

#include "../input.h"
#include "hand_controller.h"

static const char *name = "Hand-Controller";
static const int type = F8_DEVICE_HAND_CONTROLLER;

#define HAND_CONTROLLER_ENABLE (1 << 6)

F8D_OP_OUT(hand_controller_output)
{
  f8_hand_controller_t *m_hc = device->device;

  /* This bit is set ON during vram writes */
  if (m_hc)
    m_hc->enabled = value.u & HAND_CONTROLLER_ENABLE ? FALSE : TRUE;

  *io_data = value;
}

F8D_OP_IN(hand_controller_input_1)
{
  f8_hand_controller_t *m_hc = device->device;

  if (m_hc && m_hc->enabled)
    io_data->u = get_input(1);
}

F8D_OP_IN(hand_controller_input_4)
{
  f8_hand_controller_t *m_hc = device->device;

  if (m_hc && m_hc->enabled)
    io_data->u = get_input(4);
}

void hand_controller_init(f8_device_t *device)
{
  if (device)
  {
    device->device = (f8_hand_controller_t*)malloc(sizeof(f8_hand_controller_t));
    device->name = name;
    device->type = type;
    device->flags = F8_NO_ROMC;
  }
}
