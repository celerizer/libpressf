#ifndef PRESS_F_HLE_H
#define PRESS_F_HLE_H

#include "config.h"
#include "hw/system.h"

#if PF_HAVE_HLE_BIOS

typedef void (*F8_HLE_T)(f8_system_t *system);

/**
 * Return the HLE function pointer for a given BIOS address, or NULL if one
 * does not exist.
 */
F8_HLE_T hle_get_func_from_addr(u16 address);

#endif

#endif
