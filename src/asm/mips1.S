/**
 * The following assembly is hot code for a "black box" dynamic recompiler.
 * All that needs to be known is that code blocks generated via this assembly
 * should contain a pointer to the f8_system_t in $a0, and will return in $v0
 * a boolean value representing whether the next branch is being taken.
 */

#include "../constants.h"
#include "mips1.h"

.global f8_asm_op00, f8_asm_op01, f8_asm_op02, f8_asm_op03
.global f8_asm_op04, f8_asm_op05, f8_asm_op06, f8_asm_op07
.global f8_asm_op08, f8_asm_op09, f8_asm_op0a, f8_asm_op0b
.global f8_asm_op0c, f8_asm_op0d, f8_asm_op0e, f8_asm_op0f
.global f8_asm_op10, f8_asm_op11, f8_asm_op12, f8_asm_op13
.global f8_asm_op14, f8_asm_op15, f8_asm_op16, f8_asm_op17
.global f8_asm_op18, f8_asm_op19, f8_asm_op1a, f8_asm_op1b
.global f8_asm_op1c, f8_asm_op1d, f8_asm_op1e, f8_asm_op1f
.global f8_asm_op20, f8_asm_op21, f8_asm_op22, f8_asm_op23
.global f8_asm_op24, f8_asm_op25, f8_asm_op26, f8_asm_op27
.global f8_asm_op28, f8_asm_op29, f8_asm_op2a, f8_asm_op2b
.global f8_asm_op2c, f8_asm_op2d, f8_asm_op2e, f8_asm_op2f
.global f8_asm_op30, f8_asm_op31, f8_asm_op32, f8_asm_op33
.global f8_asm_op34, f8_asm_op35, f8_asm_op36, f8_asm_op37
.global f8_asm_op38, f8_asm_op39, f8_asm_op3a, f8_asm_op3b
.global f8_asm_op3c, f8_asm_op3d, f8_asm_op3e, f8_asm_op3f
.global f8_asm_op40, f8_asm_op41, f8_asm_op42, f8_asm_op43
.global f8_asm_op44, f8_asm_op45, f8_asm_op46, f8_asm_op47
.global f8_asm_op48, f8_asm_op49, f8_asm_op4a, f8_asm_op4b
.global f8_asm_op4c, f8_asm_op4d, f8_asm_op4e, f8_asm_op4f
.global f8_asm_op50, f8_asm_op51, f8_asm_op52, f8_asm_op53
.global f8_asm_op54, f8_asm_op55, f8_asm_op56, f8_asm_op57
.global f8_asm_op58, f8_asm_op59, f8_asm_op5a, f8_asm_op5b
.global f8_asm_op5c, f8_asm_op5d, f8_asm_op5e, f8_asm_op5f
.global f8_asm_op60, f8_asm_op61, f8_asm_op62, f8_asm_op63
.global f8_asm_op64, f8_asm_op65, f8_asm_op66, f8_asm_op67
.global f8_asm_op68, f8_asm_op69, f8_asm_op6a, f8_asm_op6b
.global f8_asm_op6c, f8_asm_op6d, f8_asm_op6e, f8_asm_op6f
.global f8_asm_op70, f8_asm_op71, f8_asm_op72, f8_asm_op73
.global f8_asm_op74, f8_asm_op75, f8_asm_op76, f8_asm_op77
.global f8_asm_op78, f8_asm_op79, f8_asm_op7a, f8_asm_op7b
.global f8_asm_op7c, f8_asm_op7d, f8_asm_op7e, f8_asm_op7f
.global f8_asm_op80, f8_asm_op81, f8_asm_op82, f8_asm_op83
.global f8_asm_op84, f8_asm_op85, f8_asm_op86, f8_asm_op87
.global f8_asm_op88, f8_asm_op89, f8_asm_op8a, f8_asm_op8b
.global f8_asm_op8c, f8_asm_op8d, f8_asm_op8e, f8_asm_op8f
.global f8_asm_op90, f8_asm_op91, f8_asm_op92, f8_asm_op93
.global f8_asm_op94, f8_asm_op95, f8_asm_op96, f8_asm_op97
.global f8_asm_op98, f8_asm_op99, f8_asm_op9a, f8_asm_op9b
.global f8_asm_op9c, f8_asm_op9d, f8_asm_op9e, f8_asm_op9f
.global f8_asm_opa0, f8_asm_opa1, f8_asm_opa2, f8_asm_opa3
.global f8_asm_opa4, f8_asm_opa5, f8_asm_opa6, f8_asm_opa7
.global f8_asm_opa8, f8_asm_opa9, f8_asm_opaa, f8_asm_opab
.global f8_asm_opac, f8_asm_opad, f8_asm_opae, f8_asm_opaf
.global f8_asm_opb0, f8_asm_opb1, f8_asm_opb2, f8_asm_opb3
.global f8_asm_opb4, f8_asm_opb5, f8_asm_opb6, f8_asm_opb7
.global f8_asm_opb8, f8_asm_opb9, f8_asm_opba, f8_asm_opbb
.global f8_asm_opbc, f8_asm_opbd, f8_asm_opbe, f8_asm_opbf
.global f8_asm_opc0, f8_asm_opc1, f8_asm_opc2, f8_asm_opc3
.global f8_asm_opc4, f8_asm_opc5, f8_asm_opc6, f8_asm_opc7
.global f8_asm_opc8, f8_asm_opc9, f8_asm_opca, f8_asm_opcb
.global f8_asm_opcc, f8_asm_opcd, f8_asm_opce, f8_asm_opcf
.global f8_asm_opd0, f8_asm_opd1, f8_asm_opd2, f8_asm_opd3
.global f8_asm_opd4, f8_asm_opd5, f8_asm_opd6, f8_asm_opd7
.global f8_asm_opd8, f8_asm_opd9, f8_asm_opda, f8_asm_opdb
.global f8_asm_opdc, f8_asm_opdd, f8_asm_opde, f8_asm_opdf
.global f8_asm_ope0, f8_asm_ope1, f8_asm_ope2, f8_asm_ope3
.global f8_asm_ope4, f8_asm_ope5, f8_asm_ope6, f8_asm_ope7
.global f8_asm_ope8, f8_asm_ope9, f8_asm_opea, f8_asm_opeb
.global f8_asm_opec, f8_asm_oped, f8_asm_opee, f8_asm_opef
.global f8_asm_opf0, f8_asm_opf1, f8_asm_opf2, f8_asm_opf3
.global f8_asm_opf4, f8_asm_opf5, f8_asm_opf6, f8_asm_opf7
.global f8_asm_opf8, f8_asm_opf9, f8_asm_opfa, f8_asm_opfb
.global f8_asm_opfc, f8_asm_opfd, f8_asm_opfe, f8_asm_opff
.global f8_asm_op_sizes

/**
 * Unsets O and C, modifies Z and S based on a register value.
 * @todo Status register interrupt bit is mangled here. As of now, I don't
 * care as it saves an instruction.
 */
#define STATUS_0X0X(reg, name) .set noat; srl $at, reg, 7; xori $at, $at, 1; bgtz reg, name; ori $at, B00000100; name: or W, $zero, $at

/**
 * Unsets O and C, sets S, and modifies Z based on a register value.
 * Used only in right shift, as the shift will necessarily make it non-negative.
 */
#define STATUS_0X01(reg, name) ori W, $zero, 1; bgtz reg, name; ori W, W, B00000100; name:

#define STATUS_XXXX(regd, regs, name)

/**
 * Add two registers and update the emulated status register based on result.
 * Clobbers $t0 for temporary result and $t1 for temporary overflow check.
 * @todo I'm sure this can be sped up a little
 */
#define STATUS_ADD(regd, regs, name) \
  add $t0, regd, regs; \
  srl $at, $t0, 7; \
  xori $at, $at, 1; \
  andi $t1, $t0, 0xFF; \
  bgtz $t1, z_##name; \
  ori $at, B00000100; \
  z_##name: \
  or W, $zero, $at; \
  srl $at, $t0, 8; \
  sll $at, $at, 1; \
  or W, W, $at; \
  xor $at, regs, $t0; \
  xor $t1, regd, $t0; \
  and $at, $at, $t1; \
  andi $at, $at, B10000000; \
  srl $at, $at, 4; \
  or W, W, $at; \
  andi regd, $t0, 0xFF;

#define ISAR_DIR(i, r) lbu i, r; nop 
#define ISAR_NOC(i) add i, ISAR, SCRATCH; lbu i, 0(i); nop
#define ISAR_INC(i) .set noat; add i, ISAR, SCRATCH; lbu i, 0(i); addi $at, ISAR, 1; andi $at, $at, B00000111; andi ISAR, ISAR, B00111000; or ISAR, ISAR, $at
#define ISAR_DEC(i) .set noat; add i, ISAR, SCRATCH; lbu i, 0(i); addi $at, ISAR, -1; andi $at, $at, B00000111; andi ISAR, ISAR, B00111000; or ISAR, ISAR, $at

# LR A, KU
f8_asm_op00:
  lbu A, KU
  nop # extra nop for load delay

# LR A, KL
f8_asm_op01:
  lbu A, KL
  nop

# LR A, QU
f8_asm_op02:
  lbu A, QU
  nop

# LR A, QL
f8_asm_op03:
  lbu A, QL
  nop

# LR KU, A
f8_asm_op04:
  sb A, KU

# LR KL, A
f8_asm_op05:
  sb A, KL

# LR QU, A
f8_asm_op06:
  sb A, QU

# LR QL, A
f8_asm_op07:
  sb A, QL

# LR K, P
f8_asm_op08:
#if PF_BIG_ENDIAN
  sh PC0, KU
#else
  sb PC0, KL
  srl $t0, PC0, 8
  sb $t0, KU
#endif

# LR P, K
f8_asm_op09:
#if PF_BIG_ENDIAN
  lhu PC0, KU
  nop
#else
  lbu PC0, KU
  lbu $t0, KL
  sll PC0, PC0, 8
  or PC0, PC0, $t0  # change other 16 bit loads to be like this
#endif

# LR A, IS
f8_asm_op0a:
  andi A, ISAR, 0x3f

# LR IS, A
f8_asm_op0b:
  andi ISAR, A, 0x3f

# PK
f8_asm_op0c:
  andi PC1, PC0, 0xffff
  lbu PC0, KU
  lbu $t0, KL
  sll PC0, PC0, 8
  or PC0, PC0, $t0

f8_asm_op0d:

f8_asm_op0e:

# SR 1
f8_asm_op12:
  srl A, A, 1
  STATUS_0X01(A, sr1)

# SL 1
f8_asm_op13:
  sll A, A, 1
  STATUS_0X0X(A, sl1)

# SR 4
f8_asm_op14:
  srl A, A, 4
  STATUS_0X01(A, sr4)

# SL 4
f8_asm_op15:
  sll A, A, 4
  STATUS_0X0X(A, sl4)

# LM
f8_asm_op16:
  add $t0, SCRATCH, DC0
  lbu A, 0($t0)
  addi DC0, DC0, 1

# ST
f8_asm_op17:
  add $t0, SCRATCH, DC0
  sb A, 0($t0)
  addi DC0, DC0, 1

# COM
f8_asm_op18:
  xori A, A, 0xFF
  STATUS_0X0X(A, com)

# LNK @todo
f8_asm_op19:

# DI
f8_asm_op1a:
  andi W, W, B00001111

# EI
f8_asm_op1b:
  ori W, W, B00010000

# POP
f8_asm_op1c:
  andi PC0, PC1, 0xFFFF

# LR W, J
f8_asm_op1d:
  lbu W, r9
  nop
  andi W, W, B00011111

# LR J, W
f8_asm_op1e:
  sb W, r9

# INC
f8_asm_op1f:
  ori $t2, $zero, 1
  STATUS_ADD(A, $t2, inc)

# LI
# The dynarec will replace this magic with the correct value at runtime.
f8_asm_op20:
  ori A, $zero, MAGIC_8 

# NI
# The dynarec will replace this magic with the correct value at runtime.
f8_asm_op21:
  andi A, A, MAGIC_8 
  STATUS_0X0X(A, ni)

# OI
# The dynarec will replace this magic with the correct value at runtime.
f8_asm_op22:
  ori A, A, MAGIC_8
  STATUS_0X0X(A, oi)

# XI
# The dynarec will replace this magic with the correct value at runtime.
f8_asm_op23:
  xori A, A, MAGIC_8
  STATUS_0X0X(A, xi)

# AI
# The dynarec will replace this magic with the correct value at runtime.
f8_asm_op24:
  ori $t2, $zero, MAGIC_8
  STATUS_ADD(A, $t2, ai)

# CI
# The dynarec will replace this magic with the correct value at runtime.
f8_asm_op25:
  ori $t2, $zero, MAGIC_8
  STATUS_ADD($t2, A, ci) 

# IN
# There will be a context switch here and the host will handle it.
f8_asm_op26:
  # @todo

# OUT
# There will be a context switch here and the host will handle it.
f8_asm_op27:
  andi $v0, A, 0xFF
  # @todo

# PI
f8_asm_op28:
  nop # There will be a context switch here and the host will handle it.

# JMP
# The dynarec will replace this magic with the correct value at runtime.
f8_asm_op29:
  j MAGIC_16
  nop

# DCI
# The dynarec will replace this magic with the correct value at runtime.
f8_asm_op2a:
  ori DC0, $zero, MAGIC_16

# NOP 
f8_asm_op2b: # The dynarec should optimize this away. Included for completeness.

# XDC
f8_asm_op2c:
  xor DC0, DC1
  xor DC1, DC0
  xor DC0, DC1

# Invalid opcodes. Included for completeness.
f8_asm_op2d:
f8_asm_op2e:
f8_asm_op2f:

# DS
#define ASM_DS(i) f8_asm_op3##i: ISAR_DIR($t2, r##i); ori $t3, $zero, 0xff; STATUS_ADD($t2, $t3, ds##i); sb $t2, r##i
ASM_DS(0)
ASM_DS(1)
ASM_DS(2)
ASM_DS(3)
ASM_DS(4)
ASM_DS(5)
ASM_DS(6)
ASM_DS(7)
ASM_DS(8)
ASM_DS(9)
ASM_DS(a)
ASM_DS(b)
f8_asm_op3c:
  ISAR_NOC($t2)
  ori $t3, $zero, 0xff
  STATUS_ADD($t2, $t3, dsc)
  # sb
f8_asm_op3d:
  ISAR_INC($t2)
  ori $t3, $zero, 0xff
  STATUS_ADD($t2, $t3, dsd)
  # sb
f8_asm_op3e:
  ISAR_DEC($t2)
  ori $t3, $zero, 0xff
  STATUS_ADD($t2, $t3, dse)
  # sb
f8_asm_op3f:

# LR A, r0
f8_asm_op40:
  lbu A, r0
  nop

# LR A, r1
f8_asm_op41:
  lbu A, r1
  nop

# LISU
f8_asm_op60:
  andi ISAR, ISAR, B00000111
f8_asm_op61:
  andi ISAR, ISAR, B00000111
  ori ISAR, ISAR, B00001000
f8_asm_op62:
  andi ISAR, ISAR, B00000111
  ori ISAR, ISAR, B00010000
f8_asm_op63:
  andi ISAR, ISAR, B00000111
  ori ISAR, ISAR, B00011000
f8_asm_op64:
  andi ISAR, ISAR, B00000111
  ori ISAR, ISAR, B00100000
f8_asm_op65:
  andi ISAR, ISAR, B00000111
  ori ISAR, ISAR, B00101000
f8_asm_op66:
  andi ISAR, ISAR, B00000111
  ori ISAR, ISAR, B00110000
f8_asm_op67:
  ori ISAR, ISAR, B00111000

# LISL
f8_asm_op68:
  andi ISAR, ISAR, B00111000
f8_asm_op69:
  andi ISAR, ISAR, B00111000
  ori ISAR, ISAR, B00000001
f8_asm_op6a:
  andi ISAR, ISAR, B00111000
  ori ISAR, ISAR, B00000010
f8_asm_op6b:
  andi ISAR, ISAR, B00111000
  ori ISAR, ISAR, B00000011
f8_asm_op6c:
  andi ISAR, ISAR, B00111000
  ori ISAR, ISAR, B00000100
f8_asm_op6d:
  andi ISAR, ISAR, B00111000
  ori ISAR, ISAR, B00000101
f8_asm_op6e:
  andi ISAR, ISAR, B00111000
  ori ISAR, ISAR, B00000110
f8_asm_op6f:
  ori ISAR, ISAR, B00000111

# LIS
f8_asm_op70:
  ori A, $zero, 0
f8_asm_op71:
  ori A, $zero, 1
f8_asm_op72:
  ori A, $zero, 2
f8_asm_op73:
  ori A, $zero, 3
f8_asm_op74:
  ori A, $zero, 4
f8_asm_op75:
  ori A, $zero, 5
f8_asm_op76:
  ori A, $zero, 6
f8_asm_op77:
  ori A, $zero, 7
f8_asm_op78:
  ori A, $zero, 8
f8_asm_op79:
  ori A, $zero, 9
f8_asm_op7a:
  ori A, $zero, 10
f8_asm_op7b:
  ori A, $zero, 11
f8_asm_op7c:
  ori A, $zero, 12
f8_asm_op7d:
  ori A, $zero, 13
f8_asm_op7e:
  ori A, $zero, 14
f8_asm_op7f:
  ori A, $zero, 15

dummy:

# BT
#define ASM_BT(i) f8_asm_op8##i: andi $t0, W, i; bgtz $t0, dummy; nop
ASM_BT(0)
ASM_BT(1)
ASM_BT(2)
ASM_BT(3)
ASM_BT(4)
ASM_BT(5)
ASM_BT(6)
ASM_BT(7)

# AM
f8_asm_op88:
  add $t2, $a0, DC0
  lbu $t2, F8_OFFSET_VMEM($t2)
  nop
  STATUS_ADD(A, $t2, am)

# AMD @todo
f8_asm_op89:
  add $t2, $a0, DC0
  lbu $t2, F8_OFFSET_VMEM($t2)
  add DC0, DC0, 1
  STATUS_ADD(A, $t2, amd)

# NM
f8_asm_op8a:
  addu $t0, $a0, DC0
  lbu $t0, F8_OFFSET_VMEM($t0)
  add DC0, DC0, 1
  and A, A, $t0
  STATUS_0X0X(A, nm)

# OM
f8_asm_op8b:
  addu $t0, $a0, DC0
  lbu $t0, F8_OFFSET_VMEM($t0)
  add DC0, DC0, 1
  or A, A, $t0
  STATUS_0X0X(A, om)

# XM
f8_asm_op8c:
  addu $t0, $a0, DC0
  lbu $t0, F8_OFFSET_VMEM($t0)
  add DC0, DC0, 1
  xor A, A, $t0
  STATUS_0X0X(A, xm)

# BR 7
f8_asm_op8f:
  andi $t0, ISAR, 7
  xori $t0, $t0, 7
  beqz $t0, dummy
  nop

# BF
# define ASM_BF(i) f8_asm_op9##i: andi $t0, W, 0x##i; beqz $t0, dummy; nop
ASM_BF(0)
ASM_BF(1)
ASM_BF(2)
ASM_BF(3)
ASM_BF(4)
ASM_BF(5)
ASM_BF(6)
ASM_BF(7)
ASM_BF(8)
ASM_BF(9)
ASM_BF(a)
ASM_BF(b)
ASM_BF(c)
ASM_BF(d)
ASM_BF(e)
ASM_BF(f)

# INS

# OUTS

# AS
#define ASM_AS(i) f8_asm_opc##i: lbu $t2, r##i; nop; STATUS_ADD(A, $t2, as##i)
ASM_AS(0)
ASM_AS(1)
ASM_AS(2)
ASM_AS(3)
ASM_AS(4)
ASM_AS(5)
ASM_AS(6)
ASM_AS(7)
ASM_AS(8)
ASM_AS(9)
ASM_AS(a)
ASM_AS(b)
f8_asm_opcc:
  ISAR_NOC($t2)
  STATUS_ADD(A, $t2, asc)
f8_asm_opcd:
  ISAR_INC($t2)
  STATUS_ADD(A, $t2, asd)
f8_asm_opce:
  ISAR_DEC($t2)
  STATUS_ADD(A, $t2, ase)
f8_asm_opcf:

# XS
#define ASM_XS(i) f8_asm_ope##i: lbu $t0, r##i; nop; xor A, A, $t0; STATUS_0X0X(A, xs##i)
ASM_XS(0)
ASM_XS(1)
ASM_XS(2)
ASM_XS(3)
ASM_XS(4)
ASM_XS(5)
ASM_XS(6)
ASM_XS(7)
ASM_XS(8)
ASM_XS(9)
ASM_XS(a)
ASM_XS(b)
f8_asm_opec:
  ISAR_NOC($t0)
  xor A, A, $t0
  STATUS_0X0X(A, xsc)
f8_asm_oped:
  ISAR_INC($t0)
  xor A, A, $t0
  STATUS_0X0X(A, xsd)
f8_asm_opee:
  ISAR_DEC($t0)
  xor A, A, $t0
  STATUS_0X0X(A, xse)
f8_asm_opef:

# NS
#define ASM_NS(i) f8_asm_opf##i: lbu $t0, r##i; nop; and A, A, $t0; STATUS_0X0X(A, ns##i)
ASM_NS(0)
ASM_NS(1)
ASM_NS(2)
ASM_NS(3)
ASM_NS(4)
ASM_NS(5)
ASM_NS(6)
ASM_NS(7)
ASM_NS(8)
ASM_NS(9)
ASM_NS(a)
ASM_NS(b)
f8_asm_opfc:
  ISAR_NOC($t0)
  and A, A, $t0
  STATUS_0X0X(A, nsc)
f8_asm_opfd:
  ISAR_INC($t0)
  and A, A, $t0
  STATUS_0X0X(A, nsd)
f8_asm_opfe:
  ISAR_DEC($t0)
  and A, A, $t0
  STATUS_0X0X(A, nse)
f8_asm_opff:

.section .data
f8_asm_op_sizes:
  .word f8_asm_op01 - f8_asm_op00
  .word f8_asm_op02 - f8_asm_op01
  .word f8_asm_op03 - f8_asm_op02
  .word f8_asm_op04 - f8_asm_op03
  .word f8_asm_op05 - f8_asm_op04
  .word f8_asm_op06 - f8_asm_op05
  .word f8_asm_op07 - f8_asm_op06
  .word f8_asm_op08 - f8_asm_op07
  .word f8_asm_op09 - f8_asm_op08
  .word f8_asm_op0a - f8_asm_op09
  .word f8_asm_op0b - f8_asm_op0a
  .word f8_asm_op0c - f8_asm_op0b
  .word f8_asm_op0d - f8_asm_op0c
  .word f8_asm_op0e - f8_asm_op0d
  .word 0
  .word 0
  .word 0
  .word 0
  .word f8_asm_op13 - f8_asm_op12
  .word f8_asm_op14 - f8_asm_op13
  .word 0
  .word 0
  .word 0
  .word 0
  .word f8_asm_op19 - f8_asm_op18
  .word f8_asm_op1a - f8_asm_op19
  .word f8_asm_op1b - f8_asm_op1a
  .word f8_asm_op1c - f8_asm_op1b
  .word f8_asm_op1d - f8_asm_op1c
  .word f8_asm_op1e - f8_asm_op1d
  .word f8_asm_op1f - f8_asm_op1e
  .word f8_asm_op20 - f8_asm_op1f
  .word f8_asm_op21 - f8_asm_op20
  .word f8_asm_op22 - f8_asm_op21
  .word f8_asm_op23 - f8_asm_op22
  .word f8_asm_op24 - f8_asm_op23
  .word f8_asm_op25 - f8_asm_op24
  .word f8_asm_op26 - f8_asm_op25
  .word f8_asm_op27 - f8_asm_op26
  .word f8_asm_op28 - f8_asm_op27
  .word f8_asm_op29 - f8_asm_op28
  .word f8_asm_op2a - f8_asm_op29
  .word f8_asm_op2b - f8_asm_op2a
  .word f8_asm_op2c - f8_asm_op2b
  .word f8_asm_op2d - f8_asm_op2c
  .word f8_asm_op2e - f8_asm_op2d
  .word f8_asm_op2f - f8_asm_op2e

  .word f8_asm_op30 - f8_asm_op2f
  .word f8_asm_op31 - f8_asm_op30
  .word f8_asm_op32 - f8_asm_op31
  .word f8_asm_op33 - f8_asm_op32
  .word f8_asm_op34 - f8_asm_op33
  .word f8_asm_op35 - f8_asm_op34
  .word f8_asm_op36 - f8_asm_op35
  .word f8_asm_op37 - f8_asm_op36
  .word f8_asm_op38 - f8_asm_op37
  .word f8_asm_op39 - f8_asm_op38
  .word f8_asm_op3a - f8_asm_op39
  .word f8_asm_op3b - f8_asm_op3a
  .word f8_asm_op3c - f8_asm_op3b
  .word f8_asm_op3d - f8_asm_op3c
  .word f8_asm_op3e - f8_asm_op3d
  .word f8_asm_op3f - f8_asm_op3e
