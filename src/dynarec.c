#include "dynarec.h"

#include "dma.h"
#include "hw/system.h"

#include <stddef.h>
#include <string.h>

#define AB (0)$a0

/**
 * This function sets up our jump into hot code by loading common guest
 * registers into host registers. It must be ensured these registers are not
 * touched by any other code during execution until they can be flushed to
 * RAM at the end of a block.
 */
static void f8_dynarec_block_begin(f8_system_t *system)
{
}

extern void f8_asm_op00(void);
extern void f8_asm_op01(void);
extern void f8_asm_op02(void);
extern void f8_asm_op03(void);
extern void f8_asm_op04(void);
extern void f8_asm_op05(void);
extern void f8_asm_op06(void);
extern void f8_asm_op07(void);
extern void f8_asm_op08(void);
extern void f8_asm_op12(void);
extern void f8_asm_op13(void);
extern void f8_asm_op18(void);
extern void f8_asm_op24(void);
extern void (*f8_asm_funcs[256])(void);
extern unsigned f8_asm_op_sizes[256];

f8_dynarec_block_t f8_dynarec_init(f8_system_t *system, unsigned size, unsigned offset)
{
  f8_dynarec_block_t block;
  f8_dynarec_op_t *test;
  unsigned test_words = 0;

  block.function = pf_dma_alloc(32 * 4, TRUE, 4);

  memcpy(block.function + test_words, f8_asm_op24, f8_asm_op_sizes[0x24]);
  test_words += f8_asm_op_sizes[0x24];

  //memcpy(block.function + test_words, f8_asm_op13, f8_asm_op_sizes[0x13]);
  //test_words += f8_asm_op_sizes[0x13];

#if 0
  while (offset < size - offset)
  {
    unsigned op = system->memory[offset].u;

    memcpy(block.function + test_words, f8_asm_funcs[op], f8_asm_op_sizes[op]);

    switch (op)
    {
    case 0x24: /* AI */
      *(unsigned*)(block.function + test_words) &= 0xFFFFFF00;
      *(unsigned*)(block.function + test_words) |= system->memory[offset + 1].u;
      break;
    case 0x25: /* CI */
      *(unsigned*)(block.function + test_words) &= 0xFFFFFF00;
      *(unsigned*)(block.function + test_words) |= (~(system->memory[offset + 1].u) & 0xFF);
      break;
    case 0x29: /* JMP x16 -> J x26 */
    {
      unsigned address;

      address = (B00000010 << 26) | (address & 0x03FFFFFF);

      break;   /* What we need to do here is count back around to find the recompiled address */
    }
    default:
      //memcpy(block.function + test_words, f8_asm_funcs[op], f8_asm_op_sizes[op]);
    }
  }
#endif

  return block;
}
