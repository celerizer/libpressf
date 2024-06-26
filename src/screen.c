#include <string.h>

#include "screen.h"

static u8 screen_dirty_any;
static u8 screen_dirty[64];

/* The palette index is inversed to save a calculation in GET_PALETTE */
static const u16 PIXEL_COLOR_RGB565[4][4] =
{
  /* Green background */
  {0x066B, 0xF98A, 0x49FE, 0x97F4},
  /* Blue background */
  {0x066B, 0xF98A, 0x49FE, 0xE71C},
  /* Grey background */
  {0x066B, 0xF98A, 0x49FE, 0xCE9F},
  /* Black and white */
  {0xFFFF, 0xFFFF, 0xFFFF, 0x0000}
};

static const u16 PIXEL_COLOR_RGB5551[4][4] =
{
  {0x0657, 0xF995, 0x49FD, 0x97E9},
  {0x0657, 0xF995, 0x49FD, 0xE739},
  {0x0657, 0xF995, 0x49FD, 0xCEBF},
  {0xFFFF, 0xFFFF, 0xFFFF, 0x0001}
};

/* Returns the two-bit pixel data for a given pixel index */
#define GET_PIXEL(a, b) ((a[(b) / 4] >> (3 - ((b) & 3)) * 2) & 3)

/* Returns the 2-bit palette index for a scanline (defined by bit 1 of columns 125 and 126) */
#define GET_PALETTE(a, b) ((GET_PIXEL(a, ((b) & 0xFF80) + 125) & 2) >> 1) + (GET_PIXEL(a, ((b) & 0xFF80) + 126) & 2)

static u8 draw_frame_full(u8 *vram, u16 *buffer, const u16 (*lut)[4])
{
  if (!screen_dirty_any)
    return FALSE;
  else
  {
    u8 palette, pixel_data, x_pos, y_pos;
    u16 buffer_pos = 0;

    for (y_pos = 0; y_pos < VRAM_HEIGHT; y_pos++)
    {
      /* Don't waste time painting lines that haven't changed this frame */
      if (!screen_dirty[y_pos])
      {
        buffer_pos += VRAM_WIDTH;
        continue;
      }

      palette = GET_PALETTE(vram, buffer_pos);

      for (x_pos = 0; x_pos < VRAM_WIDTH; x_pos++, buffer_pos++)
      {
        pixel_data = GET_PIXEL(vram, buffer_pos);
        buffer[buffer_pos] = lut[palette][pixel_data];
      }

      screen_dirty[y_pos] = FALSE;
    }

    screen_dirty_any = FALSE;
  }

  return TRUE;
}

static u8 draw_frame(u8 *vram, u16 *buffer, const u16 (*lut)[4])
{
  if (!screen_dirty_any)
    return FALSE;
  else
  {
    u8 palette, pixel_data, x_pos, y_pos;
    u16 buffer_pos = 0;

    for (y_pos = 4; y_pos < 4 + SCREEN_HEIGHT; y_pos++)
    {
      /* Don't waste time painting lines that haven't changed this frame */
      if (!screen_dirty[y_pos])
      {
        buffer_pos += SCREEN_WIDTH;
        continue;
      }

      palette = GET_PALETTE(vram, y_pos * VRAM_WIDTH);

      for (x_pos = 4; x_pos < 4 + SCREEN_WIDTH; x_pos++, buffer_pos++)
      {
        pixel_data = GET_PIXEL(vram, y_pos * VRAM_WIDTH + x_pos);
        buffer[buffer_pos] = lut[palette][pixel_data];
      }

      screen_dirty[y_pos] = FALSE;
    }

    screen_dirty_any = FALSE;
  }

  return TRUE;
}

u8 draw_frame_rgb565(u8 *vram, u16 *buffer)
{
  return draw_frame(vram, buffer, PIXEL_COLOR_RGB565);
}

u8 draw_frame_rgb565_full(u8 *vram, u16 *buffer)
{
  return draw_frame_full(vram, buffer, PIXEL_COLOR_RGB565);
}

u8 draw_frame_rgb5551(u8 *vram, u16 *buffer)
{
  return draw_frame(vram, buffer, PIXEL_COLOR_RGB5551);
}

u8 draw_frame_rgb5551_full(u8 *vram, u16 *buffer)
{
  return draw_frame_full(vram, buffer, PIXEL_COLOR_RGB5551);
}

/* Force the emulator to redraw every pixel next frame, 
   helpful for state loads */
void force_draw_frame()
{
  memset(screen_dirty, TRUE, sizeof(screen_dirty));
  screen_dirty_any = TRUE;
}

/* Writes a 2-bit pixel into the correct position in VRAM 
   We assume "value" is also 2-bit. */
void vram_write(u8 *vram, u8 x, u8 y, u8 value)
{
  u16 byte = (x + y * 128)/4;
  u8 final;
  u8 mask;

  screen_dirty_any = TRUE;
  screen_dirty[y]  = TRUE;
  switch (x & 3)
  {
  case 0:
    mask = 0x3F;
    break;
  case 1:
    mask = 0xCF;
    break;
  case 2:
    mask = 0xF3;
    break;
  case 3:
    mask = 0xFC;
    break;
  /* Cannot happen, but here to silence a warning. */
  default:
    return;
  }
  final = (u8)(value << ((3 - (x & 3)) * 2));

  vram[byte] &= mask;
  vram[byte] |= final;
}
