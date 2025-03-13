#include "hw/system.h"

typedef struct
{
  /* The value of the program counter in which this block should start */
  u16 start;

  /* The value of the program counter after this block ends */
  u16 end;

  /* The number of cycles this block processes in in the F8 system */
  unsigned cycles;

  /* The recompiled block of instructions */
  void (*function)(struct f8_system_t*);

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
  void (*function)(struct f8_system_t*);
} f8_dynarec_op_t;

/**
 * This function sets up our jump into hot code by loading common guest
 * registers into host registers. It must be ensured these registers are not
 * touched by any other code during execution until they can be flushed to
 * RAM at the end of a block.
 */
static void f8_dynarec_block_begin(f8_system_t *system)
{
  /**
   * For MIPS, we can use 10 temporary registers and 8 stored registers to
   * process our block. Here, we will keep our hottest F8 registers: the first
   * 12 general purpose registers and the 3 logical operators. This leaves us
   * with 3 temporary registers for emulating operations.
   * We need to be sure to restore registers s0-s7 when exiting hot code.
   */
#if 1 // PF_DYNAREC_TARGET == PF_TARGET_MIPS32
  __asm__ volatile
  (
    /** @todo store callee registers to memory */
    "lbu $t3, %[r0]($a0)\n"
    "lbu $t4, %[r1]($a0)\n"
    "lbu $t5, %[r2]($a0)\n"
    "lbu $t6, %[r3]($a0)\n"
    "lbu $t7, %[r4]($a0)\n"
    "lbu $t8, %[r5]($a0)\n"
    "lbu $t9, %[r6]($a0)\n"
    "lbu $s0, %[r7]($a0)\n"
    "lbu $s1, %[r8]($a0)\n"
    "lbu $s2, %[r9]($a0)\n"
    "lbu $s3, %[r10]($a0)\n"
    "lbu $s4, %[r11]($a0)\n"
    "lbu $s5, %[a]($a0)\n"
    "lbu $s6, %[is]($a0)\n"
    "lbu $s7, %[w]($a0)\n"
    "nop"
    :
    : [r11] "i" (offsetof(f8_system_t, main_cpu.scratchpad[11])),
      [a] "i" (offsetof(f8_system_t, main_cpu.accumulator)),
      [is] "i" (offsetof(f8_system_t, main_cpu.isar)),
      [w] "i" (offsetof(f8_system_t, main_cpu.status_register))
    : "s0", "s1", "s2", "memory"
  );
#endif
}

#if PF_DYNAREC_TARGET == PF_DYNAREC_MIPS32
#define A    "s5"
#define ISAR "s6"
#define W    "s7"
#define PC   "s4" /* PC0 in the lower half, PC1 in the upper half */
#define DC   "s3" /* DC0 in the lower half, DC1 in the upper half */
#define F8_DYNAREC_ASM_INPUTS \
  [a] "r" (A), \
  [a_mem] "i" (offsetof(system, system->main_cpu.accumulator)), \
  [isar] "r" (ISAR), \
  [isar_mem] "i" (offsetof(system, system->main_cpu.isar)), \
  [w] "r" (W), \
  [w_mem] "i" (offsetof(system, system->main_cpu.status_register)), \
  [ku] "i" (offsetof(system, system->main_cpu.scratchpad[0x0c])), \
  [pc] "r" (PC), \
  [pc_mem] "i" (offsetof(system, system->pc0)), \
  [dc] "r" (DC), \
  [dc_mem] "i" (offsetof(system, system->dc0))
#endif

static void f8_op00(f8_system_t *system)
{
#if PF_DYNAREC_TARGET == PF_DYNAREC_MIPS32
  __asm__ volatile
  (
    "lbu %[a], %[ku]($a0)\n"
    "nop" /* extra nop for load delay */

    "nop" : : F8_DYNAREC_ASM_INPUTS : A
  );
#endif
}
static f8_dynarec_op_t op00 = { 2, 0, 1, 40000, f8_op00 };

static void f8_op01(f8_system_t *system)
{
#if PF_DYNAREC_TARGET == PF_DYNAREC_MIPS32
  __asm__ volatile
  (
    "sb %[a], %[ku]($a0)\n"

    "nop" : : F8_DYNAREC_ASM_INPUTS : "memory"
  );
#endif
}
static f8_dynarec_op_t op01 = { 1, 0, 1, 40000, f8_op01 };

static void f8_op08(f8_system_t *system)
{
#if PF_DYNAREC_TARGET == PF_DYNAREC_MIPS32
  /**
   * Our trick here is to keep PC1 in the other half of the register that
   * represents PC0. This has a small cost but PC1 is accessed less often.
   */
  __asm__ volatile
  (
    "srl $t0, %[pc], 16\n"
    "sh $t0, %[ku]($a0)\n"

    "nop" : : F8_DYNAREC_ASM_INPUTS : "memory"
  );
#endif
}
static f8_dynarec_op_t op08 = { 2, 0, 1, 40000, f8_op08 };

static void f8_op09(f8_system_t *system)
{
#if PF_DYNAREC_TARGET == PF_DYNAREC_MIPS32
  __asm__ volatile
  (
    "lhu $t0, %[ku]($a0)\n"
    "sll $t0, $t0, 16\n"
    "lui %[pc], 0\n"
    "or %[pc], %[pc], $t0\n"

    "nop" : : F8_DYNAREC_ASM_INPUTS : "t0"
  );
#endif
}
static f8_dynarec_op_t op09 = { 4, 0, 1, 40000, f8_op09 };

static void f8_op12(f8_system_t *system)
{
#if PF_DYNAREC_TARGET == PF_DYNAREC_MIPS32
  /* sltu might be faster but needs two registers */
  __asm__ volatile
  (
    "srl %[a], %[a], 1\n" /* shift left */
    "andi %[w], %[w], %[mask_i]\n" /* only preserve i flag */
    "andi $t0, $r0, 1\n" /* sign bit is always on for shift right */
    "beqz %[a], not_zero\n"
    "ori $t0, $t0, %[mask_z]\n" /* turn z status on */
    "not_zero:\n"
    "or %[w], %[w], $t0\n" /* unset o,c; modify z,s */

    "nop"
    :
    : F8_DYNAREC_ASM_INPUTS,
      [mask_z] "i" (B00000100),
      [mask_i] "i" (B00010000)
    : "t0"
  );
#endif
}
static f8_dynarec_op_t op12 = { 6, 0, 1, 40000, f8_op12 };

static void f8_op13(f8_system_t *system)
{
#if PF_DYNAREC_TARGET == PF_DYNAREC_MIPS32
  /* sltu might be faster but needs two registers */
  __asm__ volatile
  (
    "sll %[a], %[a], 1\n" /* shift left */
    "andi %[w], %[w], %[mask_i]\n" /* only preserve i flag */
    "srl $t0, %[a], 7\n" /* get sign bit of new value */
    "xori $t0, $t0, 1\n" /* reverse it; sign bit is on for zero/positive */
    "beqz %[a], not_zero\n"
    "ori $t0, %[mask_z]\n" /* turn z status on */
    "not_zero:\n"
    "or %[w], %[w], $t0\n" /* unset o,c; modify z,s */

    "nop"
    :
    : F8_DYNAREC_ASM_INPUTS,
      [mask_z] "i" (B00000100),
      [mask_i] "i" (B00010000)
    : "t0"
  );
#endif
}
static f8_dynarec_op_t op13 = { 7, 0, 1, 40000, f8_op13 };

static void f8_op15(f8_system_t *system)
{
#if PF_DYNAREC_TARGET == PF_DYNAREC_MIPS32
  /* sltu might be faster but needs two registers */
  __asm__ volatile
  (
    "sll %[a], %[a], 4\n" /* shift left */
    "andi %[w], %[w], %[mask_i]\n" /* only preserve i flag */
    "srl $t0, %[a], 7\n" /* get sign bit of new value */
    "xori $t0, $t0, 1\n" /* reverse it; sign bit is on for zero/positive */
    "beqz %[a], not_zero\n"
    "ori $t0, %[mask_z]\n" /* turn z status on */
    "not_zero:\n"
    "or %[w], %[w], $t0\n" /* unset o,c; modify z,s */

    "nop"
    :
    : F8_DYNAREC_ASM_INPUTS,
      [mask_z] "i" (B00000100),
      [mask_i] "i" (B00010000)
    : "t0"
  );
#endif
}
static f8_dynarec_op_t op15 = { 7, 0, 1, 40000, f8_op15 };

int f8_dynarec_op00(const f8_system_t *system, f8_dynarec_block_t *block, unsigned address, unsigned offset)
{
  unsigned size;

  memcpy(&block[offset], f8_op00, 8);
}
