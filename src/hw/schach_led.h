#ifndef PRESS_F_SCHACH_LED_H
#define PRESS_F_SCHACH_LED_H

#include "f8_device.h"

typedef enum
{
  F8_SCACH_LED_INVALID = 0,

  F8_SCHACH_LED_OFF,
  F8_SCHACH_LED_ON
} f8_schach_led_state;

typedef struct
{
  f8_schach_led_state state;
} f8_schach_led_t;

void schach_led_init(f8_device_t *device);

#endif
