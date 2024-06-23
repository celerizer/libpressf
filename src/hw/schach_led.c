#include "schach_led.h"

static const char *name = "Schach LED";
static const int type = F8_DEVICE_SCHACH_LED;

/**
 * Appears to write 0x00 to first byte on boot,
 * 0xC0 on second byte when finished thinking to turn the light on
 */

void schach_led_init(f8_device_t *device)
{
  if (device)
  {
    /* Not sure if this is the correct size */
    f8_generic_init(device, 256);
    device->name = name;
    device->type = type;
    device->flags = F8_DATA_WRITABLE;

    device->init = schach_led_init;
  }
}
