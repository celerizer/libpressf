#include <string.h>

#include "../dma.h"
#include "system.h"

#include "selector_control.h"
#include "hand_controller.h"
#include "2102.h"
#include "2114.h"
#include "vram.h"
#include "3851.h"
#include "beeper.h"
#include "schach_led.h"

static const system_preset_t f8_systems[] =
{
  {
    "Fairchild Channel F",
    F8_SYSTEM_CHANNEL_F,
    {
      /* Program ROM */
#if PF_ROMC
      { F8_DEVICE_3851, 1, 0, 0x0000, NULL, NULL },
      { F8_DEVICE_3851, 2, 0, 0x0400, NULL, NULL },
#endif

      /* VRAM */
      { F8_DEVICE_MK4027, 3, 0, 0, NULL, mk4027_write },
      { F8_DEVICE_MK4027, 3, 1, 0, NULL, mk4027_color },
      { F8_DEVICE_MK4027, 3, 4, 0, NULL, mk4027_set_x },
      { F8_DEVICE_MK4027, 3, 5, 0, NULL, mk4027_set_y },

      /* Selector Control buttons (5 buttons on the game console) */
      { F8_DEVICE_SELECTOR_CONTROL, 4, 0, 0, selector_control_input, NULL },

      /* Left Hand-Controller */
      { F8_DEVICE_HAND_CONTROLLER, 5, 4, 0, hand_controller_input_4, NULL },
      { F8_DEVICE_HAND_CONTROLLER, 5, 0, 0, NULL, hand_controller_output },

      /* Right Hand-Controller */
      { F8_DEVICE_HAND_CONTROLLER, 6, 1, 0, hand_controller_input_1, NULL },
      { F8_DEVICE_HAND_CONTROLLER, 6, 0, 0, NULL, hand_controller_output },

      /* Beeper */
      { F8_DEVICE_BEEPER, 7, 5, 0, NULL, beeper_out },

      /* Cartridge ROM */
#if PF_ROMC
      { F8_DEVICE_3851, 8, 0, 0x800, NULL, NULL },
      { F8_DEVICE_3851, 9, 0, 0xC00, NULL, NULL },
      { F8_DEVICE_3851, 10, 0, 0x1000, NULL, NULL },
      { F8_DEVICE_3851, 11, 0, 0x1400, NULL, NULL },
      { F8_DEVICE_3851, 12, 0, 0x1800, NULL, NULL },
      { F8_DEVICE_3851, 13, 0, 0x1C00, NULL, NULL },
      { F8_DEVICE_3851, 14, 0, 0x2000, NULL, NULL },
      { F8_DEVICE_3851, 15, 0, 0x2400, NULL, NULL },

      { F8_DEVICE_2114, 16, 0, 0x2800, NULL, NULL },
      { F8_DEVICE_2114, 17, 0, 0x2A00, NULL, NULL },
      { F8_DEVICE_2114, 18, 0, 0x2C00, NULL, NULL },
      { F8_DEVICE_2114, 19, 0, 0x2E00, NULL, NULL },
#endif
      { F8_DEVICE_SCHACH_LED, 20, 0, 0x3800, NULL, NULL },

      { F8_DEVICE_INVALID, 0, 0, 0, NULL, NULL }
    }
  },

  { NULL, F8_SYSTEM_UNKNOWN, { { F8_DEVICE_INVALID, 0, 0, 0, NULL, NULL } } }
};

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
#if PF_ROMC
  if (start + device->length < 0x10000)
  {
    device->start = start;
    device->end = start + device->length - 1;

    return TRUE;
  }
#endif

  return FALSE;
}

u8 f8_system_init_preset(f8_system_t *system, const system_preset_t *preset)
{
  unsigned i, j;

  system->memory = pf_dma_alloc(0x10000, TRUE, 4);

  /* Every F8 system has a central 3850 CPU */
  system->f8devices[0].device = &system->main_cpu;
  f8_device_init(&system->f8devices[0], F8_DEVICE_3850);

  for (i = 0, j = 1; i < SYSTEM_HOOKUP_MAX, j < F8_MAX_DEVICES; i++)
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
#if PF_ROMC
      device->start = hookup->start;
      device->end = hookup->start + device->length - 1;
#else
      if (!(device->flags & F8_NO_ROMC))
        device->data = &system->memory[hookup->start];
#endif
    }
  }
  system->f8device_count = j;

  return TRUE;
}

u8 f8_system_init(f8_system_t *system, unsigned id)
{
  unsigned i = 0;

  if (id == F8_SYSTEM_UNKNOWN || id >= F8_SYSTEM_SIZE)
    return FALSE;
  else while (f8_systems[i].system)
  {
    if (f8_systems[i].system == id)
      return f8_system_init_preset(system, &f8_systems[i]);
    i++;
  }

  return FALSE;
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
#if PF_ROMC
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
#else
  if (size && size + address <= 0x10000)
  {
    memcpy(dest, &system->memory[address], size);
    return size;
  }
#endif

  return 0;
}

/**
 * @todo Support writes that extend across multiple devices
 */
unsigned f8_write(f8_system_t *system, unsigned address, const void *src,
                  unsigned size)
{
#if PF_ROMC
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
#else
  if (size && size + address <= 0x10000)
  {
    memcpy(&system->memory[address], src, size);
    return TRUE;
  }
#endif

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
