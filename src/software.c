#include "hw/hand_controller.h"
#include "hw/selector_control.h"
#include "hw/2102.h"
#include "hw/vram.h"
#include "hw/beeper.h"

#include "software.h"

const software_t pf_software[] =
{
  {
    "Videocart-10 (Maze, Jailbreak, Blind-man's-bluff, Trailblazer)",
    F8_SYSTEM_CHANNEL_F,
    {
      0x4d42b296,
      0x0a948b61
    },
    {
      { F8_DEVICE_2102, 0, 0x25, 0, NULL, f2102_out_write },
      { F8_DEVICE_2102, 0, 0x24, 0, NULL, f2102_out_address }
    }
  },
  {
    "Videocart-18 (Hangman)",
    F8_SYSTEM_CHANNEL_F,
    {
      0x9238d6ce
    },
    {
      { F8_DEVICE_2102, 0, 0x21, 0, NULL, f2102_out_write },
      { F8_DEVICE_2102, 0, 0x20, 0, NULL, f2102_out_address },
    }
  },
  {
    "SABA Videoplay 20 (Schach)",
    F8_SYSTEM_CHANNEL_F,
    {
      0x04fb6dce
    },
    {
      { F8_DEVICE_2114, 0, 0, 0x2800, NULL, NULL },
      { F8_DEVICE_2114, 1, 0, 0x2A00, NULL, NULL },
      { F8_DEVICE_2114, 2, 0, 0x2C00, NULL, NULL },
      { F8_DEVICE_2114, 3, 0, 0x2E00, NULL, NULL },
      { F8_DEVICE_SCHACH_LED, 4, 0, 0x3800, NULL, NULL }
    }
  },
  { NULL, 0, { 0 }, { { F8_DEVICE_INVALID, 0, 0, 0, NULL, NULL } } }
};

u32 crc32_for_byte(u32 r)
{
  int j;

  for (j = 0; j < 8; ++j)
    r = (r & 1? 0: (u32)0xEDB88320L) ^ r >> 1;

  return r ^ (u32)0xFF000000L;
}

void crc32(const void *data, u32 n_bytes, u32* crc)
{
  static u32 table[0x100];
  u32 i;

  if (!*table)
    for (i = 0; i < 0x100; ++i)
      table[i] = crc32_for_byte(i);
  for (i = 0; i < n_bytes; ++i)
    *crc = table[(u8)*crc ^ ((const u8*)data)[i]] ^ *crc >> 8;
}

const software_t* software_identify(const void *data, u32 size)
{
  const software_t* software = &pf_software[0];
  u32 crc = 0;

  crc32(data, size, &crc);
  while (software->title)
  {
    int i;

    for (i = 0; i < SOFTWARE_CRC32_MAX; i++)
      if (software->crc32[i] == crc)
        return software;
    software++;
  }

  return NULL;
}
