#ifndef PRESS_F_SCREEN_H
#define PRESS_F_SCREEN_H

#include "types.h"

#define SCREEN_HEIGHT 58
#define SCREEN_WIDTH  102
#define VRAM_HEIGHT   64
#define VRAM_WIDTH    128

void draw_frame_argb8888(u8 *vram, u32 *buffer);
u8 draw_frame_rgb565(u8 *vram, u16 *buffer);
u8 draw_frame_rgb565_full(u8 *vram, u16 *buffer);
u8 draw_frame_rgb5551(u8 *vram, u16 *buffer);
u8 draw_frame_rgb5551_full(u8 *vram, u16 *buffer);

void force_draw_frame(void);
void vram_write(u8 *vram, u8 x, u8 y, u8 value);

#endif
