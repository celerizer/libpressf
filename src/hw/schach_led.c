#include "schach_led.h"

#include "../dma.h"

static const char *name = "Schach LED";
static const int type = F8_DEVICE_SCHACH_LED;

void schach_led_3800(f8_device_t *device, u16 address, f8_byte value)
{
  f8_schach_led_t *m_led = device->device;

  F8_UNUSED(address);
  F8_UNUSED(value);
  m_led->state = F8_SCHACH_LED_OFF;
}

void schach_led_8000(f8_device_t *device, u16 address, f8_byte value)
{
  f8_schach_led_t *m_led = device->device;

  F8_UNUSED(address);
  F8_UNUSED(value);
  m_led->state = F8_SCHACH_LED_ON;
}

void schach_led_init(f8_device_t *device)
{
  if (device)
  {
    f8_schach_led_t *m_led = pf_dma_alloc(sizeof(f8_schach_led_t), TRUE);

    device->device = m_led;
    device->name = name;
    device->type = type;
    device->flags = F8_DATA_WRITABLE;

    device->mappings[0].length = 1;
    device->mappings[0].func_out = schach_led_3800;
    device->mappings[1].length = 1;
    device->mappings[1].func_out = schach_led_8000;

    device->init = schach_led_init;
  }
}
