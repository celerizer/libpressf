#ifndef PRESS_F_HAND_CONTROLLER_H
#define PRESS_F_HAND_CONTROLLER_H

#include "f8_device.h"

typedef struct
{
  u8 enabled;
} f8_hand_controller_t;

F8D_OP_OUT(hand_controller_output);

F8D_OP_IN(hand_controller_input_1);

F8D_OP_IN(hand_controller_input_4);

void hand_controller_init(f8_device_t *device);

#endif
