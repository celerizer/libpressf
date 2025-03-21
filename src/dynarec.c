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
extern unsigned f8_asm_op_sizes[256];

f8_dynarec_block_t f8_dynarec_init(f8_system_t *system)
{
  f8_dynarec_block_t block;
  f8_dynarec_op_t *test;
  unsigned test_words = 0;

  block.function = pf_dma_alloc(32 * 4, TRUE, 4);

  memcpy(block.function + test_words, f8_asm_op12, f8_asm_op_sizes[0x12]);
  test_words += f8_asm_op_sizes[0x12];

  memcpy(block.function + test_words, f8_asm_op13, f8_asm_op_sizes[0x13]);
  test_words += f8_asm_op_sizes[0x13];

  return block;
}
