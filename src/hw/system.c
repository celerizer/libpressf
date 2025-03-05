#include <string.h>

#include "system.h"

#include "selector_control.h"
#include "hand_controller.h"
#include "2102.h"
#include "2114.h"
#include "vram.h"
#include "3851.h"
#include "beeper.h"
#include "schach_led.h"

f3850_t* f8_main_cpu(f8_system_t *system)
{
  return system->f8devices[0].device;
}

f8_device_t* f8_main_cpu_device(f8_system_t *system)
{
  return &system->f8devices[0];
}

u8 f8_device_init(f8_device_t *device, const f8_device_id_t type)
{
  if (device && type > F8_DEVICE_INVALID && type < F8_DEVICE_SIZE)
  {
    switch (type)
    {
    case F8_DEVICE_3850:
      device->init = f3850_init;
      break;
    case F8_DEVICE_3851:
      device->init = f3851_init;
      break;
    case F8_DEVICE_MK4027:
      device->init = vram_init;
      break;
    case F8_DEVICE_SELECTOR_CONTROL:
      device->init = selector_control_init;
      break;
    case F8_DEVICE_HAND_CONTROLLER:
      device->init = hand_controller_init;
      break;
    case F8_DEVICE_BEEPER:
      device->init = beeper_init;
      break;
    case F8_DEVICE_SCHACH_LED:
      device->init = schach_led_init;
      break;
    case F8_DEVICE_2114:
      device->init = f2114_init;
      break;
    case F8_DEVICE_2102:
      device->init = f2102_init;
      break;
    default:
      return FALSE;
    }
    device->init(device);

    return TRUE;
  }

  return FALSE;
}

u8 f8_device_set_start(f8_device_t *device, unsigned start)
{
  if (start + device->length < 0x10000)
  {
    device->start = start;
    device->end = start + device->length - 1;

    return TRUE;
  }

  return FALSE;
}

u8 f8_system_init(f8_system_t *system, const system_preset_t *preset)
{
  unsigned i, j;

  /* Every F8 system has a central 3850 CPU */
  f8_device_init(&system->f8devices[0], F8_DEVICE_3850);
  system->main_cpu = system->f8devices[0].device;

  for (i = 0, j = 1; i < SYSTEM_HOOKUP_MAX; i++)
  {
    const software_hookup_t *hookup = &preset->hookups[i];
    f8_device_t *device = &system->f8devices[hookup->id];

    if (preset->hookups[i].type == F8_DEVICE_INVALID)
      break;
    else
    {
      /* Create the device if it does not already exist */
      if (!device->device)
      {
        f8_device_init(device, hookup->type);
        j++;
      }

      /* Setup the IO transfer */
      if (hookup->func_in)
      {
        unsigned k;

        for (k = 0; k < F8_MAX_IO_LINK; k++)
        {
          if (!system->io_ports[hookup->port].func_in[k])
          {
            system->io_ports[hookup->port].device_in[k] = device;
            system->io_ports[hookup->port].func_in[k] = hookup->func_in;
            break;
          }
        }
      }
      if (hookup->func_out)
      {
        unsigned k;

        for (k = 0; k < F8_MAX_IO_LINK; k++)
        {
          if (!system->io_ports[hookup->port].func_out[k])
          {
            system->io_ports[hookup->port].device_out[k] = device;
            system->io_ports[hookup->port].func_out[k] = hookup->func_out;
            break;
          }
        }
      }

      /* Setup ROMC-enabled devices */
      device->start = hookup->start;
      if (device->length)
        device->end = hookup->start + device->length - 1;
      else
        device->end = 0;

      /**
       * If using simple ROMC mode, map to contiguous memory.
       * @todo Constructor is still called, leading to double allocate.
       */
#if !PF_ROMC
      if (!(device->flags & F8_NO_ROMC))
        device->data = &system->memory[hookup->start];
#endif
    }
  }
  system->f8device_count = j;

  return TRUE;
}

f8_byte f8_fetch(f8_system_t *system, unsigned address)
{
  f8_byte value;

  f8_read(system, &value, address, 1);

  return value;
}

unsigned f8_read(f8_system_t *system, void *dest, unsigned address,
                 unsigned size)
{
  unsigned i;

  for (i = 0; i < system->f8device_count; i++)
  {
    f8_device_t *device = &system->f8devices[i];

    if (device->flags & F8_NO_ROMC)
      continue;
    if (device->length && address >= device->start && address <= device->end)
    {
      memcpy(dest, &device->data[address - device->start], size);
      return size;
    }
  }

  return 0;
}

/**
 * @todo Support writes that extend across multiple devices
 */
unsigned f8_write(f8_system_t *system, unsigned address, const void *src,
                  unsigned size)
{
  unsigned i;

  for (i = 0; i < system->f8device_count; i++)
  {
    f8_device_t *device = &system->f8devices[i];

    if (device->flags & F8_NO_ROMC)
      continue;
    if (device->length && address >= device->start && address <= device->end)
    {
      memcpy(&device->data[address - device->start], src, size);
      return TRUE;
    }
  }

  return FALSE;
}

u8 f8_hack_skip_verification(f8_system_t *system)
{
  /* Apply NOPs to the instructions for comparing ROM data to $55, so
   * verification always returns true */
  const u8 hack[] = { 0x2B, 0x2B };

  return f8_write(system, 0x0015, hack, sizeof(hack)) == sizeof(hack);
}

u8 f8_hack_tv_powww(struct f8_system_t *system)
{
  /* Overwrite code for loading timer minutes so timer always sets to 00:15 */
  const u8 hack[] = { 0x67, 0x69, 0x70, 0x5D, 0x20, 0x15,
                      0x5C, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B };

  return f8_write(system, 0x0251, hack, sizeof(hack)) == sizeof(hack);
}

u8 f8_settings_apply(struct f8_system_t *system, f8_settings_t settings)
{
  u8 success = TRUE;

  system->settings = settings;
  if (system->settings.cf_skip_cartridge_verification)
    success &= f8_hack_skip_verification(system);
  if (system->settings.cf_tv_powww)
    success &= f8_hack_tv_powww(system);

  return success;
}

u8 f8_settings_apply_default(struct f8_system_t *system)
{
  const f8_settings_t f8_settings_default =
  {
    F8_SYSTEM_CHANNEL_F,
    F8_CLOCK_CHANNEL_F_NTSC,
    FALSE,
    FALSE,
    FALSE,
    FALSE
  };

  return f8_settings_apply(system, f8_settings_default);
}

unsigned f8_system_set_device_out_cb(f8_system_t *system,
  const f8_device_id_t type, F8D_OP_OUT_T func)
{
  unsigned set = 0;
  unsigned i;

  for (i = 0; i < F8_MAX_IO_PORTS; i++)
  {
    io_t *io = &system->io_ports[i];

    if (!io)
      break;
    else
    {
      unsigned j;

      for (j = 0; j < F8_MAX_IO_LINK; j++)
      {
        if (io->device_out[j] && io->device_out[j]->type == type)
        {
          io->func_out[j] = func;
          set++;
        }
      }
    }
  }

  return set;
}

unsigned f8_system_set_device_in_cb(f8_system_t *system,
  const f8_device_id_t type, F8D_OP_IN_T func)
{
  unsigned set = 0;
  unsigned i;

  for (i = 0; i < F8_MAX_IO_PORTS; i++)
  {
    io_t *io = &system->io_ports[i];

    if (!io)
      break;
    else
    {
      unsigned j;

      for (j = 0; j < F8_MAX_IO_LINK; j++)
      {
        if (io->device_in[j] && io->device_in[j]->type == type)
        {
          io->func_in[j] = func;
          set++;
        }
      }
    }
  }

  return set;
}
