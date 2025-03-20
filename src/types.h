#ifndef PRESS_F_TYPES_H
#define PRESS_F_TYPES_H

#include "config.h"
#include "constants.h"

#define i8 signed char
#define u8 unsigned char
#define i16 signed short
#define u16 unsigned short
#define i32 signed long
#define u32 unsigned long

#define F8_UNUSED(a) (void)a

typedef union
{
  u8 u;
  i8 s;
} f8_byte;

/**
 * A 16-bit value in native endianness.
 */
typedef union
{
  struct
  {
#if PF_BIG_ENDIAN
    f8_byte h;
    f8_byte l;
#else
    f8_byte l;
    f8_byte h;
#endif
  } bytes;
  unsigned short u;
  signed short i;
} f8_word;

/**
 * A 16-bit value in explicit big endianness.
 */
typedef struct
{
  f8_byte h;
  f8_byte l;
} h8_word_be;

enum
{
  PF_FONT_0 = 0,

  PF_FONT_1,
  PF_FONT_2,
  PF_FONT_3,
  PF_FONT_4,
  PF_FONT_5,
  PF_FONT_6,
  PF_FONT_7,
  PF_FONT_8,
  PF_FONT_9,
  PF_FONT_G,
  PF_FONT_QUESTION_MARK,
  PF_FONT_T,
  PF_FONT_SPACE,
  PF_FONT_M,
  PF_FONT_X,
  PF_FONT_BLOCK,
  PF_FONT_COLON,
  PF_FONT_HYPHEN,
  PF_FONT_GOALIE_A,
  PF_FONT_GOALIE_B,
  PF_FONT_BALL,
  PF_FONT_LINE_000,
  PF_FONT_LINE_180,
  PF_FONT_LINE_030,
  PF_FONT_LINE_045,
  PF_FONT_LINE_060,
  PF_FONT_LINE_240,
  PF_FONT_LINE_225,
  PF_FONT_LINE_210,

  PF_FONT_SIZE,
};

#endif
