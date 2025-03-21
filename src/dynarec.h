#ifndef PRESS_F_DYNAREC_H
#define PRESS_F_DYNAREC_H

#include "hw/system.h"

typedef struct f8_dynarec_block_t
{
  /* The value of the program counter in which this block should start */
  u16 start;

  /* The value of the program counter after this block ends */
  u16 end;

  /* The number of cycles this block processes in in the F8 system */
  unsigned cycles;

  /* The recompiled block of instructions */
  void (*function)(f8_system_t*);

  /* The size in bytes of the recompiled block of instructions */
  unsigned size;

  /*
   * The next block to process after either following a branch or performing
   * an unconditional IO action.
   * If NULL, the next block should be looked up in the hash table, or fall
   * back on interpretter.
   */
  struct f8_dynarec_block_t *take;

  /* The next block to process when not following a branch */
  struct f8_dynarec_block_t *skip;
} f8_dynarec_block_t;

typedef struct
{
  /**
   * Size, in words, of the recompiled instruction. Not necesarily the size
   * of the full function itself, as we may want to trim prologues and
   * epilogues that the compiler adds.
   */
  unsigned size_host;

  /**
   * Size, in words, of the function prologue added by the compiler. Used to
   * offset to the relevant code.
   */
  unsigned size_prologue;

  /* The size, in bytes, of the source instruction */
  unsigned size_guest;

  /* The number of cycles this instruction executes in on the guest */
  unsigned cycles;

  /* The pointer to start of recompiled bytecode */
  void (*function)(void);
} f8_dynarec_op_t;

f8_dynarec_block_t f8_dynarec_init(f8_system_t *system);

unsigned f8_dynarec_run(unsigned cycles);

#endif
