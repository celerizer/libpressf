#ifndef F8_ASM_MIPS1_H
#define F8_ASM_MIPS1_H

#include "offsets.h"

#define A $s0
#define PC0 $s1
#define PC1 $s2
#define ISAR $s3
#define W $s4
#define DC0 $s5
#define DC1 $s6
#define SCRATCHPAD $s7

#define A_mem F8_OFFSET_A($a0)
#define W_mem F8_OFFSET_W($a0)

#define r0 (F8_OFFSET_SCRATCHPAD + 0)($a0)
#define r1 (F8_OFFSET_SCRATCHPAD + 1)($a0)
#define r2 (F8_OFFSET_SCRATCHPAD + 2)($a0)
#define r3 (F8_OFFSET_SCRATCHPAD + 3)($a0)
#define r4 (F8_OFFSET_SCRATCHPAD + 4)($a0)
#define r5 (F8_OFFSET_SCRATCHPAD + 5)($a0)
#define r6 (F8_OFFSET_SCRATCHPAD + 6)($a0)
#define r7 (F8_OFFSET_SCRATCHPAD + 7)($a0)
#define r8 (F8_OFFSET_SCRATCHPAD + 8)($a0)
#define r9 (F8_OFFSET_SCRATCHPAD + 9)($a0)
#define ra (F8_OFFSET_SCRATCHPAD + 10)($a0)
#define rb (F8_OFFSET_SCRATCHPAD + 11)($a0)

#define KU F8_OFFSET_KU($a0)
#define KL F8_OFFSET_KL($a0)
#define QU F8_OFFSET_QU($a0)
#define QL F8_OFFSET_QL($a0)

#define MAGIC 0x4B42

#endif
