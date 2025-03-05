#include "config.h"
#include "romc.h"
#include "types.h"

/* Macros to keep code less redundant */

#define INIT_DEVICES \
  f8_device_t *device; \
  unsigned i;

#define FOREACH_DEVICE \
  for (i = 0; i < system->f8device_count; i++) \
  { \
    device = &system->f8devices[i];

/* Helper functions */

#if PF_ROMC
/**
 * Returns whether a given virtual address is within the region a given
 * F8 device is mapped in.
 */
static u8 f8device_contains(const f8_device_t *device, u16 address)
{
  return device->length && (address >= device->start) && (address <= device->end);
}

static f8_byte *f8device_vptr(const f8_device_t *device, u16 address)
{
  if (device->length)
  {
    address -= device->start;
    return &device->data[address];
  }
  else
    return NULL;
}

static void f8device_write(f8_device_t *device, u16 address, f8_byte data)
{
  if (!(device->flags & F8_DATA_WRITABLE))
    return;
  address -= device->start;
  device->data[address] = data;
}
#endif

/**
 * ROMC 0 0 0 0 0 / 00 / S, L
 * ---
 * Instruction Fetch. The device whose address space includes the contents of
 * the PC0 register must place on the data bus the op code addressed by PC0;
 * then all devices increment the contents of PC0.
 * NOTES: If a device's PC0 goes out of sync with the rest, this could cause
 *   multiple devices to write to the data bus. What's the accurate
 *   behavior here?
 **/
static void romc00(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES
  int found = FALSE;

#if !PF_ROMC_REDUNDANCY
  FOREACH_DEVICE
    if (device->flags & F8_NO_PC0)
      continue;
    else if (!found && f8device_contains(device, device->pc0.u))
    {
      system->dbus = *f8device_vptr(device, device->pc0.u);
      found = TRUE;
    }
    device->pc0.u += 1;
  }
#else
  FOREACH_DEVICE
    if (f8device_contains(device, device->pc0.u))
      system->dbus = *f8device_vptr(device, device->pc0.u);
    device->pc0.u += 1;
  }
#endif
#else
  system->dbus = system->memory[system->pc0.u];
  system->pc0.u += 1;
#endif
}

void romc00s(f8_system_t *system)
{
  romc00(system);
  system->cycles += CYCLE_SHORT;
}

void romc00l(f8_system_t *system)
{
  romc00(system);
  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 0 0 0 0 1 / 01 / L
 * The device whose address space includes the contents of the PC0 register
 * must place on the data bus the contents of the memory location addressed
 * by PC0; then all devices add the 8-bit value on the data bus, as a
 * signed binary number, to PC0.
 */
void romc01(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC0)
      continue;
    else if (f8device_contains(device, device->pc0.u))
    {
      system->dbus = *f8device_vptr(device, device->pc0.u);
#if !PF_ROMC_REDUNDANCY
      break;
#endif
    }
  }

  FOREACH_DEVICE
    device->pc0.u += system->dbus.s;
  }
#else
  system->dbus = system->memory[system->pc0.u];
  system->pc0.u += system->dbus.s;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 0 0 0 1 0 / 02 / L
 * The device whose DC0 addresses a memory word within the address space of
 * that device must place on the data bus the contents of the memory location
 * addressed by DC0; then all devices increment DC0.
 */
void romc02(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES
  int found = FALSE;

  FOREACH_DEVICE
    if (device->flags & F8_NO_DC0)
      continue;
    if (!found && f8device_contains(device, device->dc0.u))
    {
      found = TRUE;
      system->dbus = *f8device_vptr(device, device->dc0.u);
    }
    device->dc0.u += 1;
  }
#else
  system->dbus = system->memory[system->dc0.u];
  system->dc0.u += 1;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 0 0 0 1 1 / 03 / L, S
 * Similar to 00, except that it is used for Immediate Operand fetches (using
 * PC0) instead of instruction fetches.
 * @todo The documentation calls it similar and LONG/SHORT is flipped, is
 * there any functional difference?
 */
void romc03s(f8_system_t *system)
{
  romc00s(system);
}

void romc03l(f8_system_t *system)
{
  romc00l(system);
}

/**
 * ROMC 0 0 1 0 0 / 04 / S
 * Copy the contents of PC1 into PC0.
 */
void romc04(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC1)
      continue;
    device->pc0 = device->pc1;
  }
#else
  system->pc0 = system->pc1;
#endif

  system->cycles += CYCLE_SHORT;
}

/**
 * ROMC 0 0 1 0 1 / 05 / L
 * Store the data bus contents into the memory location pointed to by DC0;
 * increment DC0.
 */
void romc05(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_DC0)
      continue;
    if (f8device_contains(device, device->dc0.u))
      f8device_write(device, device->dc0.u, system->dbus);
    device->dc0.u += 1;
  }
#else
  system->memory[system->dc0.u] = system->dbus;
  system->dc0.u += 1;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 0 0 1 1 0 / 06 / L
 * Place the high order byte of DC0 on the data bus.
 * NOTE: Because timing here won't be accurate anyway, and all devices should
 *   write the same value, we only worry about the first F8 device (probably
 *   the 3850).
*/
void romc06(f8_system_t *system)
{
#if PF_ROMC
  system->dbus = system->f8devices[0].dc0.bytes.h;
#else
  system->dbus = system->dc0.bytes.h;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 0 0 1 1 1 / 07 / L
 * Place the high order byte of PC1 on the data bus.
 * NOTE: See above.
 */
void romc07(f8_system_t *system)
{
#if PF_ROMC
  system->dbus = system->f8devices[0].pc1.bytes.h;
#else
  system->dbus = system->pc1.bytes.h;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 0 1 0 0 0 / 08 / L
 * All devices copy the contents of PC0 into PC1. The CPU outputs zero on the
 * data bus in this ROMC state. Load the data bus into both halves of PC0,
 * thus clearing the register.
 */
void romc08(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  system->dbus.u = 0;

  FOREACH_DEVICE
    device->pc1 = device->pc0;
    device->pc0.u = 0;
  }
#else
  system->dbus.u = 0;
  system->pc1 = system->pc0;
  system->pc0.u = 0;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 0 1 0 0 1 / 09 / L
 * The device whose address space includes the contents of the DC0 register
 * must place the low order byte of DC0 onto the data bus.
 * TODO: Bitmask might not be needed.
 */
void romc09(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_DC0)
      continue;
    if (f8device_contains(device, device->dc0.u))
      system->dbus = device->dc0.bytes.l;
  }
#else
  system->dbus = system->dc0.bytes.l;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 0 1 0 1 0 / 0A / L
 * All devices add the 8-bit value on the data bus, treated as a signed binary
 * number, to the data counter.
 */
void romc0a(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_DC0)
      continue;
    device->dc0.u += system->dbus.s;
  }
#else
  system->dc0.u += system->dbus.s;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 0 1 0 1 1 / 0B / L
 * The device whose address space includes the value in PC1 must place the low
 * order byte of PC1 on the data bus.
 * TODO: Bitmask may not be needed.
 */
void romc0b(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC1)
      continue;
    if (f8device_contains(device, device->pc1.u))
      system->dbus = device->pc1.bytes.l;
  }
#else
  system->dbus = system->pc1.bytes.l;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 0 1 1 0 0 / 0C / L
 * The device whose address space includes the contents of the PC0 register
 * must place the contents of the memory word addressed by PC0 onto the
 * data bus; then all devices move the value that has just been placed on
 * the data bus into the low order byte of PC0.
 */
void romc0c(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC0)
      continue;
    else if (f8device_contains(device, device->pc0.u))
    {
      system->dbus = *f8device_vptr(device, device->pc0.u);
#if !PF_ROMC_REDUNDANCY
      break;
#endif
    }
  }
  FOREACH_DEVICE
    device->pc0.bytes.l = system->dbus;
  }
#else
  system->dbus = system->memory[system->pc0.u];
  system->pc0.bytes.l = system->dbus;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 0 1 1 0 1 / 0D / S
 * All devices store in PC1 the current contents of PC0, incremented by 1;
 * PC0 is unaltered.
 */
void romc0d(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC1)
      continue;
    device->pc1.u = device->pc0.u + 1;
  }
#else
  system->pc1.u = system->pc0.u + 1;
#endif

  system->cycles += CYCLE_SHORT;
}

/**
 * ROMC 0 1 1 1 0 / 0E / L
 * The device whose memory space includes the contents of PC0 must place the
 * contents of the word addressed by PC0 onto the data bus. The value on the
 * data bus is then moved to the low order byte of DC0 by all devices.
 */
void romc0e(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC0)
      continue;
    else if (f8device_contains(device, device->pc0.u))
    {
      system->dbus = *f8device_vptr(device, device->pc0.u);
#if !PF_ROMC_REDUNDANCY
      break;
#endif
    }
  }
  FOREACH_DEVICE
    if (device->flags & F8_NO_DC0)
      continue;
    device->dc0.bytes.l = system->dbus;
  }
#else
  system->dbus = system->memory[system->pc0.u];
  system->dc0.bytes.l = system->dbus;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 0 1 1 1 1 / 0F / L
 * The interrupting device with highest priority must place the low order
 * byte of the interrupt vector on the data bus. All devices must copy the
 * contents of PC0 into PC1. All devices must move the contents of the data bus
 * into the low order byte of PC0.
 * @todo Interrupt stuff
 */
void romc0f(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  /*
    TODO: The interrupting device with highest priority must place the low
    order byte of the interrupt vector on the data bus.
  */
  FOREACH_DEVICE
    if (device->flags & F8_NO_PC0)
      continue;
    device->pc1 = device->pc0;
    device->pc0.bytes.l = system->dbus;
  }
#else
  system->pc1 = system->pc0;
  system->pc0.bytes.l = system->dbus;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 *  ROMC 1 0 0 0 0 / 10 / L
 *  Inhibit any modification to the interrupt priority logic.
 *  NOTE: TODO
 */
void romc10(f8_system_t *system)
{
  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 0 0 0 1 / 11 / L
 * The device whose memory space includes the contents of PC0 must place the
 * contents of the addressed memory word onto the data bus. All devices must
 * then move the contents of the data bus to the upper byte of DC0.
 */
void romc11(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC0)
      continue;
    else if (f8device_contains(device, device->pc0.u))
    {
      system->dbus = *f8device_vptr(device, device->pc0.u);
#if !PF_ROMC_REDUNDANCY
      break;
#endif
    }
  }
  FOREACH_DEVICE
    if (device->flags & F8_NO_DC0)
      continue;
    device->dc0.bytes.h = system->dbus;
  }
#else
  system->dbus = system->memory[system->pc0.u];
  system->dc0.bytes.h = system->dbus;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 0 0 1 0 / 12 / L
 * All devices copy the contents of PC0 into PC1. All devices then move the
 * contents of the data bus into the low order byte of PC0.
 */
void romc12(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC0)
      continue;
    device->pc1 = device->pc0;
    device->pc0.bytes.l = system->dbus;
  }
#else
  system->pc1 = system->pc0;
  system->pc0.bytes.l = system->dbus;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 0 0 1 1 / 13 / L
 * TODO: This
 */
void romc13(f8_system_t *system)
{
  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 0 1 0 0 / 14 / L
 * All devices move the contents of the dbus into the high order byte of PC0.
 */
void romc14(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC0)
      continue;
    device->pc0.bytes.h = system->dbus;
  }
#else
  system->pc0.bytes.h = system->dbus;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 0 1 0 1 / 15 / L
 * All devices move the contents of the dbus into the high order byte of PC1.
 */
void romc15(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC1)
      continue;
    device->pc1.bytes.h = system->dbus;
  }
#else
  system->pc1.bytes.h = system->dbus;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 0 1 1 0 / 16 / L
 * All devices move the contents of the dbus into the high order byte of DC0.
 */
void romc16(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_DC0)
      continue;
    device->dc0.bytes.h = system->dbus;
  }
#else
  system->dc0.bytes.h = system->dbus;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 0 1 1 1 / 17 / L
 * All devices move the contents of the dbus into the low order byte of PC0.
 */
void romc17(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC0)
      continue;
    device->pc0.bytes.l = system->dbus;
  }
#else
  system->pc0.bytes.l = system->dbus;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 1 0 0 0 / 18 / L
 * All devices move the contents of the dbus into the low order byte of PC1.
 */
void romc18(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC1)
      continue;
    device->pc1.bytes.l = system->dbus;
  }
#else
  system->pc1.bytes.l = system->dbus;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 1 0 0 1 / 19 / L
 * All devices move the contents of the dbus into the low order byte of DC0.
 */
void romc19(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_DC0)
      continue;
    device->dc0.bytes.l = system->dbus;
  }
#else
  system->dc0.bytes.l = system->dbus;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 1 0 1 0 / 1A / L
 * During the prior cycle, an I/O port timer or interrupt controller register
 * was addressed; the device containing the addressed port must move the
 * current contents of the data bus into the addressed port.
 * @todo
 */
void romc1a(f8_system_t *system)
{
  system->io_ports[system->dbus.u].data = system->dbus;

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 1 0 1 1 / 1B / L
 * During the prior cycle, the data bus specified the address of an I/O port.
 * The device containing the addressed I/O port must place the contents of
 * the I/O port on the data bus.
 * (Note that the contents of timer and interrupt control registers cannot be
 * read back onto the data bus.)
 */
void romc1b(f8_system_t *system)
{
  system->dbus = system->io_ports[system->dbus.u].data;

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 1 1 0 0 / 1C / L or S
 * None.
 */
void romc1cs(f8_system_t *system)
{
  system->cycles += CYCLE_SHORT;
}

void romc1cl(f8_system_t *system)
{
  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 1 1 0 1 / 1D / S
 * Devices with DC0 and DC1 registers must switch registers. Devices without a
 * DC1 register perform no operation.
 */
void romc1d(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES
  f8_word temp;

  FOREACH_DEVICE
    if (device->flags & F8_NO_DC1)
      continue;
    temp = device->dc0;
    device->dc0 = device->dc1;
    device->dc1 = temp;
  }
#else
  f8_word temp = system->dc0;
  system->dc0 = system->dc1;
  system->dc1 = temp;
#endif

  system->cycles += CYCLE_SHORT;
}

/**
 * ROMC 1 1 1 1 0 / 1E / L
 * The device whose address space includes the contents of PC0 must place the
 * low order byte of PC0 onto the data bus.
 */
void romc1e(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC0)
      continue;
    else if (f8device_contains(device, device->pc0.u))
    {
      system->dbus = device->pc0.bytes.l;
#if !PF_ROMC_REDUNDANCY
      break;
#endif
    }
  }
#else
  system->dbus = system->pc0.bytes.l;
#endif

  system->cycles += CYCLE_LONG;
}

/**
 * ROMC 1 1 1 1 0 / 1F / L
 * The device whose address space includes the contents of PC0 must place the
 * high order byte of PC0 onto the data bus.
 */
void romc1f(f8_system_t *system)
{
#if PF_ROMC
  INIT_DEVICES

  FOREACH_DEVICE
    if (device->flags & F8_NO_PC0)
      continue;
    else if (f8device_contains(device, device->pc0.u))
    {
      system->dbus = device->pc0.bytes.h;
#if !PF_ROMC_REDUNDANCY
      break;
#endif
    }
  }
#else
  system->dbus = system->pc0.bytes.h;
#endif

  system->cycles += CYCLE_LONG;
}
