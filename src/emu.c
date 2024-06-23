#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "emu.h"
#include "hle.h"
#include "romc.h"

#define A    ((f3850_t*)system->f8devices[0].device)->accumulator
#define ISAR ((f3850_t*)system->f8devices[0].device)->isar
#define W    ((f3850_t*)system->f8devices[0].device)->status_register

#define J    ((f3850_t*)system->f8devices[0].device)->scratchpad[0x09]
#define HU   ((f3850_t*)system->f8devices[0].device)->scratchpad[0x0A]
#define HL   ((f3850_t*)system->f8devices[0].device)->scratchpad[0x0B]
#define KU   ((f3850_t*)system->f8devices[0].device)->scratchpad[0x0C]
#define KL   ((f3850_t*)system->f8devices[0].device)->scratchpad[0x0D]
#define QU   ((f3850_t*)system->f8devices[0].device)->scratchpad[0x0E]
#define QL   ((f3850_t*)system->f8devices[0].device)->scratchpad[0x0F]

#if PF_ROMC
#define DC0 system->f8devices[0].dc0
#define DC1 system->f8devices[0].dc1
#define PC0 system->f8devices[0].pc0
#define PC1 system->f8devices[0].pc1
#else
#define DC0 system->dc0
#define DC1 system->dc1
#define PC0 system->pc0
#define PC1 system->pc1
#endif

opcode_t opcodes[256] =
{
  { 1, "LR A, Ku",   "Loads r12 (upper byte of K) into the accumulator." },
  { 1, "LR A, Kl",   "Loads r13 (lower byte of K) into the accumulator." },
  { 1, "LR A, Qu",   "Loads r14 (upper byte of Q) into the accumulator." },
  { 1, "LR A, Ql",   "Loads r15 (lower byte of Q) into the accumulator." },
  { 1, "LR Ku, A",   "Loads the accumulator into r12 (upper byte of K)." },
  { 1, "LR Kl, A",   "Loads the accumulator into r13 (lower byte of K)." },
  { 1, "LR Qu, A",   "Loads the accumulator into r14 (upper byte of Q)." },
  { 1, "LR Ql, A",   "Loads the accumulator into r15 (lower byte of Q)." },
  { 1, "LR K, PC1",  "Loads PC1 into K (r12 and r13)." },
  { 1, "LR PC1, K",  "Loads K (r12 and r13) into PC1." },
  { 1, "LR A, ISAR", "Loads the register referenced by the ISAR into the accumulator."},
  { 1, "LR ISAR, A", "Loads the accumulator into the register referenced by the ISAR."},
  { 1, "PK",         "Loads PC0 into PC1, then loads K (r12 and r13) into PC0."},
  { 1, "LR PC0, Q",  "Loads Q (r14 and r15) into PC0."},
  { 1, "LR Q, DC0",  "Loads DC0 into Q (r14 and r15)."},
  { 1, "LR DC0, Q",  "Loads Q (r14 and r15) into DC0."},
  { 1, "LR DC0, H",  "Loads H (r10 and r11) into DC0."},
  { 1, "LR H, DC0",  "Loads DC0 into H (r10 and r11)."},
  { 1, "SR 1",       "Shifts the accumulator right by one bit."},
  { 1, "SL 1",       "Shifts the accumulator left by one bit."},
  { 1, "SR 4",       "Shifts the accumulator right by four bits."},
  { 1, "SL 4",       "Shifts the accumulator left by four bits."},
  { 1, "LM",         "Loads the value addressed by DC0 into the accumulator."},
  { 1, "ST",         "Loads the accumulator into DC0."},
  { 1, "COM",        "Complements the accumulator."},
  { 1, "LNK",        "Adds the carry status flag to the accumulator."},
  { 1, "DI" ,        "Disables interrupt requests."},
  { 1, "EI" ,        "Enables interrupt requests."},
  { 1, "POP",        "Loads PC1 into PC0."},

};

/* 1789772.5Hz / 60 * 10000 */
#define PF_CHANNEL_F_CLOCK_NTSC 298295417

/* 2000000Hz / 60 * 10000 */
#define PF_CHANNEL_F_CLOCK_PAL_GEN_1 333333333

/* 1970491Hz / 60 * 10000 */
#define PF_CHANNEL_F_CLOCK_PAL_GEN_2 328415167

#define F8_OP(a) void a(f8_system_t *system)

unsigned get_status(f8_system_t *system, const unsigned flag)
{
  return (W & flag) != 0;
}

void set_status(f8_system_t *system, const unsigned flag, unsigned enable)
{
  W = (unsigned char)(enable ? W | flag : W & ~flag);
}

void add(f8_system_t *system, f8_byte *dest, unsigned src)
{
  unsigned result = dest->u + src;

  /* Sign bit is set if the result is positive or zero */
  set_status(system, STATUS_SIGN, !(result & B10000000));

  /* We carried if we went above max 8-bit */
  set_status(system, STATUS_CARRY, result & 0x100);

  /* We zeroed if the result was zero */
  set_status(system, STATUS_ZERO, (result & 0xFF) == 0);

  /* We overflowed up or down if result goes beyond 8 bit (changed sign) */
  set_status(system, STATUS_OVERFLOW, ((dest->u ^ result) & (src ^ result) & 0x80));

  dest->u = (unsigned char)result;
}

/**
 * See Section 6.4
 */
void add_bcd(f8_system_t *system, f8_byte *augend, unsigned addend)
{
  unsigned tmp = augend->u + addend;
  int c = 0;
  int ic = 0;

  if (((augend->u + addend) & 0xff0) > 0xf0)
    c = 1;
  if ((augend->u & 0x0f) + (addend & 0x0f) > 0x0F)
    ic = 1;

  add(system, augend, addend);

  if (!c && !ic)
    tmp = ((tmp + 0xa0) & 0xf0) + ((tmp + 0x0a) & 0x0f);
  else if (!c && ic)
    tmp = ((tmp + 0xa0) & 0xf0) + (tmp & 0x0f);
  else if (c && !ic)
    tmp = (tmp & 0xf0) + ((tmp + 0x0a) & 0x0f);

  augend->u = (unsigned char)tmp;
}

/**
 * Gets a pointer to the byte the ISAR (Indirect Scratchpad Address Register)
 * is addressing. The lower 4 bits of the current opcode determine how the
 * ISAR is addressing memory.
 * 00 - 0B : Returns the specified byte directly; ignores ISAR.
 * 0C      : Returns the byte addressed by ISAR.
 * 0D      : Returns the byte addressed by ISAR, increments ISAR.
 * 0E      : Returns the byte addressed by ISAR, decrements ISAR.
 * 0F      : Returns NULL.
 * See Table 6-2.
 **/

#define ISAR_OP(a) \
  &f8_main_cpu(system)->scratchpad[a];

#define ISAR_OP_0 ISAR_OP(0)
#define ISAR_OP_1 ISAR_OP(1)
#define ISAR_OP_2 ISAR_OP(2)
#define ISAR_OP_3 ISAR_OP(3)
#define ISAR_OP_4 ISAR_OP(4)
#define ISAR_OP_5 ISAR_OP(5)
#define ISAR_OP_6 ISAR_OP(6)
#define ISAR_OP_7 ISAR_OP(7)
#define ISAR_OP_8 ISAR_OP(8)
#define ISAR_OP_9 ISAR_OP(9)
#define ISAR_OP_10 ISAR_OP(10)
#define ISAR_OP_11 ISAR_OP(11)

#define ISAR_OP_12 \
  &f8_main_cpu(system)->scratchpad[ISAR & 0x3F];

#define ISAR_OP_13 \
  &f8_main_cpu(system)->scratchpad[ISAR & 0x3F]; \
  ISAR = (ISAR & B00111000) | ((ISAR + 1) & B00000111);

#define ISAR_OP_14 \
  &f8_main_cpu(system)->scratchpad[ISAR & 0x3F]; \
  ISAR = (ISAR & B00111000) | ((ISAR - 1) & B00000111);

static void update_status(f8_system_t *system)
{
  add(system, &A, 0);
}

#if !PF_ROMC
static f8_byte next(f8_system_t *system)
{
  system->dbus = f8_fetch(system, PC0);
  system->cycles += CYCLE_SHORT;
  PC0++;
  return system->dbus;
}
#endif

/**
 * 00
 * LR A, KU
 */
F8_OP(lr_a_ku)
{
  A = KU;
}

/**
 * 01
 * LR A, KL
 */
F8_OP(lr_a_kl)
{
  A = KL;
}

/**
 * 02
 * LR A, QU
 */
F8_OP(lr_a_qu)
{
  A = QU;
}

/**
 * 03
 * LR A, QL
 */
F8_OP(lr_a_ql)
{
  A = QL;
}

/**
 * 04
 * LR KU, A
 */
F8_OP(lr_ku_a)
{
  KU = A;
}

/**
 * 05
 * LR KL, A
 */
F8_OP(lr_kl_a)
{
  KL = A;
}

/**
 * 06
 * LR QU, A
 */
F8_OP(lr_qu_a)
{
  QU = A;
}

/**
 * 07
 * LR QL, A
 */
F8_OP(lr_ql_a)
{
  QL = A;
}

/*
   08
   LR K, PC1
   Load a word into K from the backup process counter.
*/
F8_OP(lr_k_pc1)
{
#if PF_ROMC
  romc07(system);
  KU = system->dbus;
  romc0b(system);
  KL = system->dbus;
#else
  KU.u = (u8)((PC1 & 0xFF00) >> 8);
  KL.u = PC1 & 0xFF;
  system->cycles += CYCLE_LONG * 2;
#endif
}

/*
   09
   LR PC1, K
   Load a word into the backup process counter from K.
*/
F8_OP(lr_pc1_k)
{
#if PF_ROMC
  system->dbus = KU;
  romc15(system);
  system->dbus = KL;
  romc18(system);
#else
  PC1 = (u16)(KU.u << 8 | KL.u);
  system->cycles += CYCLE_LONG * 2;
#endif
}

/* 0A */
F8_OP(lr_a_isar)
{
  A.u = ISAR & B00111111;
}

/* 0B */
F8_OP(lr_isar_a)
{
  ISAR = A.u & B00111111;
}

/* 0C */
/* PK: Loads process counter into backup, K into process counter */
F8_OP(pk)
{
#if PF_ROMC
  system->dbus = KL;
  romc12(system);
  system->dbus = KU;
  romc14(system);
#else
  PC1 = PC0;
  PC0 = (u16)(KU.u << 8 | KL.u);
  system->cycles += CYCLE_LONG * 2;
#endif
}

/* 0D */
F8_OP(lr_pc0_q)
{
#if PF_ROMC
  system->dbus = QL;
  romc17(system);
  system->dbus = QU;
  romc14(system);
#else
  PC0 = (u16)(QU.u << 8 | QL.u);
  system->cycles += CYCLE_LONG * 2;
#endif
}

/* 0E */
F8_OP(lr_q_dc0)
{
#if PF_ROMC
  romc06(system);
  QU = system->dbus;
  romc09(system);
  QL = system->dbus;
#else
  QU.u = DC0 & 0xFF00 >> 8;
  QL.u = DC0 & 0x00FF;
  system->cycles += CYCLE_LONG * 2;
#endif
}

/* 0F */
F8_OP(lr_dc0_q)
{
#if PF_ROMC
  system->dbus = QU;
  romc16(system);
  system->dbus = QL;
  romc19(system);
#else
  DC0 = (u16)(QU.u << 8 | QL.u);
  system->cycles += CYCLE_LONG * 2;
#endif
}

/* 10 */
F8_OP(lr_dc0_h)
{
#if PF_ROMC
  system->dbus = HU;
  romc16(system);
  system->dbus = HL;
  romc19(system);
#else
  DC0 = (u16)(HU.u << 8 | HL.u);
  system->cycles += CYCLE_LONG * 2;
#endif
}

/* 11 */
F8_OP(lr_h_dc0)
{
#if PF_ROMC
  romc06(system);
  HU = system->dbus;
  romc09(system);
  HL = system->dbus;
#else
  HU.u = DC0 & 0xFF00 >> 8;
  HL.u = DC0 & 0x00FF;
  system->cycles += CYCLE_LONG * 2;
#endif
}

static void shift(f8_system_t *system, unsigned right, unsigned amount)
{
  A.u = (u8)(right ? A.u >> amount : A.u << amount);
  set_status(system, STATUS_OVERFLOW, FALSE);
  set_status(system, STATUS_ZERO,     A.u == 0);
  set_status(system, STATUS_CARRY,    FALSE);
  set_status(system, STATUS_SIGN,     A.s >= 0);
}

/**
 * 12
 * SR 1 - Shift Right 1
 * The contents of the accumulator are shifted right either one or four bit
 * positions, depending on the value of the SR instruction operand.
 */
F8_OP(sr_a)
{
  shift(system, TRUE, 1);
}

/**
 * 13
 * SL 1 - Shift Left 1
 * The contents of the accumulator are shifted left either one or four bit
 * positions, depending on the value of the SL instruction operand.
 */
F8_OP(sl_a)
{
  shift(system, FALSE, 1);
}

/* 14 */
F8_OP(sr_a_4)
{
  shift(system, TRUE, 4);
}

/* 15 */
F8_OP(sl_a_4)
{
  shift(system, FALSE, 4);
}

/**
 * 16
 * LM - Load Accumulator from Memory
 * The contents of the memory byte addressed by the DC0 register are loaded
 * into the accumulator. The contents of the DC0 registers are incremented
 * as a result of the LM instruction execution.
 */
F8_OP(lm)
{
#if PF_ROMC
  romc02(system);
  A = system->dbus;
#else
  f8_read(system, &A, DC0, sizeof(A));
  DC0++;
  system->cycles += CYCLE_LONG;
#endif
}

/**
 * 17
 * ST - Store to Memory
 * The contents of the accumulator are stored in the memory location addressed
 * by the Data Counter (DC0) registers.
 * The DC registers' contents are incremented as a result of the instruction
 * execution.
 */
F8_OP(st)
{
#if PF_ROMC
  system->dbus = A;
  romc05(system);
#else
  f8_write(system, DC0, &A, sizeof(A));
  DC0++;
  system->cycles += CYCLE_LONG;
#endif
}

/**
 * 18
 * COM - Complement
 * The accumulator is loaded with its one's complement.
 */
F8_OP(com)
{
  A.u ^= 0xFF;
  update_status(system);
}

/**
 * 19
 * LNK - Link Carry to the Accumulator
 * The carry bit is binary added to the least significant bit of the
 * accumulator. The result is stored in the accumulator.
 */
F8_OP(lnk)
{
  add(system, &A, get_status(system, STATUS_CARRY));
}

/**
 * 1A
 * DI - Disable Interrupt
 * The interrupt control bit, ICB, is reset; no interrupt requests will be
 * acknowledged by the 3850 CPU.
 */
F8_OP(di)
{
#if PF_ROMC
  romc1cs(system);
#else
  system->cycles += CYCLE_SHORT;
#endif
  set_status(system, STATUS_INTERRUPTS, FALSE);
}

/**
 * 1B
 * EI - Enable Interrupt
 * The interrupt control bit is set. Interrupt requests will now be
 * acknowledged by the CPU.
 */
F8_OP(ei)
{
#if PF_ROMC
   romc1cs(system);
#else
   system->cycles += CYCLE_SHORT;
#endif
   set_status(system, STATUS_INTERRUPTS, TRUE);
}

/**
 * 1C
 * POP - Return from Subroutine
 * The contents of the Stack Registers (PC1) are transferred to the
 * Program Counter Registers (PC0).
 */
F8_OP(pop)
{
#if PF_ROMC
  romc04(system);
#else
  PC0 = PC1;
  system->cycles += CYCLE_SHORT;
#endif
}

/* 1D */
F8_OP(lr_w_j)
{
#if PF_ROMC
  romc1cs(system);
#else
  system->cycles += CYCLE_SHORT;
#endif
  W = J.u & B00011111;
}

/* 1E */
F8_OP(lr_j_w)
{
  J.u = W & B00011111;
}

/**
 * 1F
 * INC - Increment Accumulator
 * The content of the accumulator is increased by one binary count.
 */
F8_OP(inc)
{
  add(system, &A, 1);
}

/**
 * 20
 * LI - Load Immediate
 * The value provided by the operand of the LI instruction is loaded into the
 * accumulator.
 */
F8_OP(li)
{
#if PF_ROMC
  romc03l(system);
  A = system->dbus;
#else
  A = next(system);
  system->cycles += CYCLE_LONG;
#endif
}

/**
 * 21
 * NI - AND Immediate
 * An 8-bit value provided by the operand of the NI instruction is ANDed with
 * the contents of the accumulator. The results are stored in the accumulator.
 */
F8_OP(ni)
{
#if PF_ROMC
  romc03l(system);
  A.u &= system->dbus.u;
#else
  A.u &= next(system).u;
  system->cycles += CYCLE_SHORT;
#endif
  add(system, &A, 0);
}

/**
 * 22
 * OI - OR Immediate
 * An 8-bit value provided by the operand of the I/O instruction is ORed with
 * the contents of the accumulator. The results are stored in the accumulator.
 */
F8_OP(oi)
{
#if PF_ROMC
  romc03l(system);
  A.u |= system->dbus.u;
#else
  A.u |= next(system).u;
  system->cycles += CYCLE_SHORT;
#endif
  add(system, &A, 0);
}

/**
 * 23
 * XI - Exclusive-OR Immediate
 * The contents of the 8-bit value provided by the operand of the XI
 * instruction are EXCLUSIVE-ORed with the contents of the accumulator.
 * The results are stored in the accumulator.
 */
F8_OP(xi)
{
#if PF_ROMC
  romc03l(system);
  A.u ^= system->dbus.u;
#else
  A.u ^= next(system).u;
  system->cycles += CYCLE_SHORT;
#endif
  add(system, &A, 0);
}

/**
 * 24
 * AI - Add Immediate to Accumulator
 * The 8-bit (two hexadecimal digit) value provided by the instruction operand
 * is added to the current contents of the accumulator. Binary addition is
 * performed.
 */
F8_OP(ai)
{
#if PF_ROMC
  romc03l(system);
  add(system, &A, system->dbus.u);
#else
  add(system, &A, next(system).u);
  system->cycles += CYCLE_SHORT;
#endif
}

/**
 * 25
 * CI - Compare Immediate
 * The contents of the accumulator are subtracted from the operand of the CI
 * instruction. The result is not saved but the status bits are set or reset to
 * reflect the results of the operation.
 */
F8_OP(ci)
{
  f8_byte immediate;

#if PF_ROMC
  romc03l(system);
  immediate = system->dbus;
#else
  immediate = next(system);
  system->cycles += CYCLE_SHORT;
#endif
  add(system, &immediate, (~A.u & 0xFF) + 1);
}

/**
 * 26
 * IN - Input Long Address
 * The data input to the I/O port specified by the operand of the IN
 * instruction is stored in the accumulator.
 * The I/O port addresses 4 through 255 may be addressed by the IN
 * instruction.
 */
F8_OP(in)
{
  io_t *io;
  u8 i;

#if PF_ROMC
  /* Apparently only 128 devices can be hooked up, don't know why */
  romc03l(system);
  W = 0; /* todo: why? */
  io = &system->io_ports[system->dbus.u & B01111111];
#else
  io = &system->io_ports[next(system).u & B01111111];
#endif

  for (i = 0; i < F8_MAX_IO_LINK; i++)
    if (io->func_in[i])
      io->func_in[i](io->device_in[i], &io->data);

#if PF_ROMC
  romc1b(system);
#else
  system->cycles += CYCLE_LONG * 2;
#endif

  A = io->data;
  add(system, &A, 0);
}

/**
 * 27
 * OUT - Output Long Address
 * The I/O port addressed by the operand of the OUT instruction is loaded
 * with the contents of the accumulator.
 * I/O ports with addresses from 4 through 255 may be accessed with the
 * OUT instruction.
 * @todo What happens when 0-3 are addressed?
 * @todo This should be using ROMC 1A
 */
F8_OP(out)
{
  io_t *io;
  u8 address, i;
  u8 found = FALSE;

#if PF_ROMC
  romc03l(system);
  address = system->dbus.u;
#else
  address = next(system).u;
#endif

#if PF_SAFETY
  if (address < 4)
    return;
#endif

  io = &system->io_ports[address];

  for (i = 0; i < F8_MAX_IO_LINK; i++)
  {
    if (io->func_out[i])
    {
      io->func_out[i](io->device_out[i], &io->data, A);
      found = TRUE;
    }
  }
  if (!found)
    io->data = A;

#if PF_ROMC
  system->cycles += CYCLE_LONG;
#else
  system->cycles += CYCLE_LONG * 2;
#endif
}

/**
 * 28
 * PI - Call to Subroutine Immediate
 * The contents of the Program Counters are stored in the Stack Registers, PC1,
 * then the then the 16-bit address contained in the operand of the PI
 * instruction is loaded into the Program Counters. The accumulator is used as
 * a temporary storage register during transfer of the most significant byte of
 * the address. Previous accumulator results will be altered.
 */
F8_OP(pi)
{
#if PF_ROMC
  romc03l(system);
  A = system->dbus;
  romc0d(system);
  romc0c(system);
  system->dbus = A;
  romc14(system);
#else
  A = next(system);
  PC1 = PC0 + 1;
  PC0 = (u16)(A.u << 8 | f8_fetch(system, PC0).u);
  system->cycles += CYCLE_LONG * 3 + CYCLE_SHORT;
#endif
}

/**
 * 29
 * JMP - Branch Immediate
 * As the result of a JMP instruction execution, a branch to the memory
 * location addressed by the second and third bytes of the instruction
 * occurs. The second byte contains the high order eight bits of the
 * memory address; the third byte contains the low order eight bits of
 * the memory address.
 * The accumulator is used to temporarily store the most significant byte
 * of the memory address; therefore, after the JMP instruction is executed,
 * the initial contents of the accumulator are lost.
 */
F8_OP(jmp)
{
#if PF_ROMC
  romc03l(system);
  A = system->dbus;
  romc0c(system);
  system->dbus = A;
  romc14(system);
#else
  A = next(system);
  PC0 = (u16)(A.u << 8 | next(system).u);
  system->cycles += CYCLE_LONG * 3;
#endif
}

/**
 * 2A
 * DCI - Load DC Immediate
 * The DCI instruction is a three-byte instruction. The contents of the second
 * byte replace the high order byte of the DC0 registers; the contents of the
 * third byte replace the low order byte of the DC0 registers.
 */
F8_OP(dci)
{
#if PF_ROMC
  romc11(system);
  romc03s(system);
  romc0e(system);
  romc03s(system);
#else
  DC0 = (u16)(next(system).u << 8);
  DC0 |= next(system).u;
  system->cycles += CYCLE_LONG * 2 + CYCLE_SHORT * 2;
#endif
}

/**
 * 2B
 * NOP - No Operation
 * No function is performed.
 */
F8_OP(nop)
{
  /* This does nothing to intentionally silence a warning */
  (void)system;
}

/**
 * 2C
 * XDC - Exchange Data Counters
 * Execution of the instruction XDC causes the contents of the auxiliary data
 * counter registers (DC1) to be exchanged with the contents of the data
 * counter registers (DC0).
 * This instruction is only significant when a 3852 or 3853 Memory Interface
 * device is part of the system configuration.
 * @todo "The PSUs will have DC0 unaltered." ...?
 */
F8_OP(xdc)
{
#if PF_ROMC
  romc1d(system);
#else
  u16 temp = DC0;

  DC0 = DC1;
  DC1 = temp;
  system->cycles += CYCLE_SHORT;
#endif
}

/**
 * 30 - 3F
 * DS - Decrease Scratchpad Content
 * The content of the scratchpad register addressed by the operand (Sreg) is
 * decremented by one binary count. The decrement is performed by adding H'FF'
 * to the scratchpad register.
 */

#define F8_OP_DS(a) \
  F8_OP(ds_##a) \
  { \
    f8_byte *address = ISAR_OP_##a; \
    add(system, address, 0xFF); \
    /* This operation retrieves the next with a long-cycle ROMC00. */ \
    system->cycles += CYCLE_LONG - CYCLE_SHORT; \
  }

F8_OP_DS(0)
F8_OP_DS(1)
F8_OP_DS(2)
F8_OP_DS(3)
F8_OP_DS(4)
F8_OP_DS(5)
F8_OP_DS(6)
F8_OP_DS(7)
F8_OP_DS(8)
F8_OP_DS(9)
F8_OP_DS(10)
F8_OP_DS(11)
F8_OP_DS(12)
F8_OP_DS(13)
F8_OP_DS(14)

/**
 * 40 - 4F
 * LR A, r - Load Register
 */

#define F8_OP_LR_A_R(a) \
  F8_OP(lr_a_r_##a) \
  { \
    f8_byte *address = ISAR_OP_##a; \
    A = *address; \
  }

F8_OP_LR_A_R(0)
F8_OP_LR_A_R(1)
F8_OP_LR_A_R(2)
F8_OP_LR_A_R(3)
F8_OP_LR_A_R(4)
F8_OP_LR_A_R(5)
F8_OP_LR_A_R(6)
F8_OP_LR_A_R(7)
F8_OP_LR_A_R(8)
F8_OP_LR_A_R(9)
F8_OP_LR_A_R(10)
F8_OP_LR_A_R(11)
F8_OP_LR_A_R(12)
F8_OP_LR_A_R(13)
F8_OP_LR_A_R(14)

/**
 * 50 - 5F
 * LR r, A - Load Register
 */

#define F8_OP_LR_R_A(a) \
  F8_OP(lr_r_a_##a) \
  { \
    f8_byte *address = ISAR_OP_##a; \
    *address = A; \
  }

F8_OP_LR_R_A(0)
F8_OP_LR_R_A(1)
F8_OP_LR_R_A(2)
F8_OP_LR_R_A(3)
F8_OP_LR_R_A(4)
F8_OP_LR_R_A(5)
F8_OP_LR_R_A(6)
F8_OP_LR_R_A(7)
F8_OP_LR_R_A(8)
F8_OP_LR_R_A(9)
F8_OP_LR_R_A(10)
F8_OP_LR_R_A(11)
F8_OP_LR_R_A(12)
F8_OP_LR_R_A(13)
F8_OP_LR_R_A(14)

/**
 * 60 - 67
 * LISU - Load Upper Octal Digit of ISAR
 * A 3-bit value provided by the LISU instruction operand is loaded into the
 * three most significant bits of the ISAR. The three least significant bits
 * of the ISAR are not altered.
 */

#define F8_OP_LISU(a) \
  F8_OP(lisu_##a) \
  { \
    ISAR &= B00000111; \
    ISAR |= a << 3; \
  }

F8_OP_LISU(0)
F8_OP_LISU(1)
F8_OP_LISU(2)
F8_OP_LISU(3)
F8_OP_LISU(4)
F8_OP_LISU(5)
F8_OP_LISU(6)
F8_OP_LISU(7)

/**
 * 68 - 6F
 * LISL - Load Lower Octal Digit of ISAR
 * A 3-bit value provided by the LISL instruction operand is loaded into the
 * three least significant bits of the ISAR. The three most significant bits
 * of the ISAR are not altered.
 */

#define F8_OP_LISL(a) \
  F8_OP(lisl_##a) \
  { \
    ISAR &= B00111000; \
    ISAR |= a; \
  }

F8_OP_LISL(0)
F8_OP_LISL(1)
F8_OP_LISL(2)
F8_OP_LISL(3)
F8_OP_LISL(4)
F8_OP_LISL(5)
F8_OP_LISL(6)
F8_OP_LISL(7)

/**
 * 70
 * CLR - Clear Accumulator
 * The contents of the accumulator are set to zero.
 */

/**
 * 71 - 7F
 * LIS - Load Immediate Short
 * A 4-bit value provided by the LIS instruction operand is loaded into the
 * four least significant bits of the accumulator. The most significant four
 * bits of the accumulator are set to ''0''.
 */

#define F8_OP_LIS(a) F8_OP(lis_##a) { A.u = a; }
F8_OP_LIS(0)
F8_OP_LIS(1)
F8_OP_LIS(2)
F8_OP_LIS(3)
F8_OP_LIS(4)
F8_OP_LIS(5)
F8_OP_LIS(6)
F8_OP_LIS(7)
F8_OP_LIS(8)
F8_OP_LIS(9)
F8_OP_LIS(10)
F8_OP_LIS(11)
F8_OP_LIS(12)
F8_OP_LIS(13)
F8_OP_LIS(14)
F8_OP_LIS(15)

#if PF_ROMC
#define F8_OP_BT(a) \
  F8_OP(bt_##a) \
  { \
    romc1cs(system); \
    if (a & W) \
      romc01(system); \
    else \
      romc03s(system); \
  }
#else
#define F8_OP_BT(a) \
  F8_OP(bt_##a) \
  { \
    if (a & W) \
    { \
      PC0 += next(system).s - 1; \
      system->cycles += CYCLE_LONG + CYCLE_SHORT; \
    } \
    else \
    { \
      PC0++; \
      system->cycles += CYCLE_SHORT * 2; \
    } \
  }
#endif

/**
 * 80
 * BT0 - Do Not Branch
 * An effective 3-cycle NO-OP
 */
F8_OP_BT(0)

/**
 * 81
 * BT1 / BP - Branch if Positive
 */
F8_OP_BT(1)

/**
 * 82
 * BT2 / BC - Branch on Carry
 */
F8_OP_BT(2)

/**
 * 83
 * BT3 - Branch if Positive or on Carry
 */
F8_OP_BT(3)

/**
 * 84
 * BT4 / BZ - Branch if Zero
 */
F8_OP_BT(4)

/**
 * 85
 * BT5 - Branch if Positive
 * Same function as BP
 */
F8_OP_BT(5)

/**
 * 86
 * BT6 - Branch if Zero or on Carry
 */
F8_OP_BT(6)

/**
 * BT7 - Branch if Positive or on Carry
 * Same function as BT3
 */
F8_OP_BT(7)

/**
 * 88
 * AM - Add (Binary) Memory to Accumulator
 * The content of the memory location addressed by the DC0 registers is added
 * to the accumulator. The sum is returned in the accumulator. Memory is not
 * altered. Binary addition is performed. The contents of the DC0 registers
 * are incremented by 1.
 */
F8_OP(am)
{
#if PF_ROMC
  romc02(system);
  add(system, &A, system->dbus.u);
#else
  add(system, &A, f8_fetch(system, DC0).u);
  DC0++;
  system->cycles += CYCLE_LONG;
#endif
}

/**
 * 89
 * AMD - Add Decimal Accumulator with Memory
 * The accumulator and the memory location addressed by the DC0 registers are
 * assumed to contain two BCD digits. The content of the address memory byte
 * is added to the contents of the accumulator to give a BCD result in the
 * accumulator.
 */
F8_OP(amd)
{
#if PF_ROMC
  romc02(system);
  add_bcd(system, &A, system->dbus.u);
#else
  add_bcd(system, &A, f8_fetch(system, DC0).u);
  system->cycles += CYCLE_LONG;
#endif
}

/**
 * 8A
 * NM - Logical AND from Memory
 * The content of memory addressed by the data counter registers is ANDed with
 * the content of the accumulator. The results are stored in the accumulator.
 * The contents of the data counter registers are incremented.
 */
F8_OP(nm)
{
#if PF_ROMC
  romc02(system);
  A.u &= system->dbus.u;
#else
  A.u &= f8_fetch(system, DC0).u;
  system->cycles += CYCLE_LONG;
#endif
  update_status(system);
}

/**
 * 8B
 * OM - Logical OR from Memory
 * The content of memory byte addressed by...
 * @todo write
 */
F8_OP(om)
{
#if PF_ROMC
  romc02(system);
  A.u |= system->dbus.u;
#else
  A.u |= f8_fetch(system, DC0).u;
  system->cycles += CYCLE_LONG;
#endif
  update_status(system);
}

/**
 * 8C
 * XM - Exclusive OR from Memory
 * @todo write
 */
F8_OP(xm)
{
#if PF_ROMC
  romc02(system);
  A.u ^= system->dbus.u;
#else
  A.u ^= f8_fetch(system, DC0).u;
  system->cycles += CYCLE_LONG;
#endif
  update_status(system);
}

/**
 * 8D
 * CM - Compare Memory to Accumulator
 * The CM instruction is the same is the same as the CI instruction except the
 * memory contents addressed by the DC0 registers, instead of an immediate
 * value, are compared to the contents of the accumulator.
 * Memory contents are not altered. Contents of the DC0 registers are
 * incremented.
 */
F8_OP(cm)
{
  f8_byte temp;

#if PF_ROMC
  romc02(system);
  temp = system->dbus;
#else
  temp = f8_fetch(system, DC0);
  system->cycles += CYCLE_LONG;
#endif
  add(system, &temp, (~A.u & 0xFF) + 1);
}

/**
 * 8E
 * ADC - Add Accumulator to Data Counter
 * The contents of the accumulator are treated as a signed binary number, and
 * are added to the contents of every DC0 register. The result is stored in the
 * DC0 registers. The accumulator contents do not change.
 * No status bits are modified.
 */
F8_OP(adc)
{
#if PF_ROMC
  system->dbus = A;
  romc0a(system);
#else
  DC0 += A.s;
  system->cycles += CYCLE_LONG;
#endif
}

/**
 * 8F
 * BR7 - Branch on ISAR
 * Branch will occur if any of the low 3 bits of ISAR are reset.
 */
F8_OP(br7)
{
#if PF_ROMC
  if ((ISAR & B00000111) != B00000111)
    romc01(system);
  else
    romc03s(system);
#else
  if ((ISAR & B00000111) != B00000111)
  {
    PC0 += next(system).s - 1;
    system->cycles += CYCLE_LONG + CYCLE_SHORT;
  }
  else
  {
    PC0++;
    system->cycles += CYCLE_SHORT * 2;
  }
#endif
}

/**
 * 90 - 9F
 * BF - Branch on False
 */
F8_OP(bf)
{
#if PF_ROMC
  /* Wait? */
  romc1cs(system);
  if (!((system->dbus.u & B00001111) & W))
    romc01(system);
  else
    romc03s(system);
#else
  if (!((system->dbus.u & B00001111) & W))
  {
    PC0 += next(system).s - 1;
    system->cycles += CYCLE_LONG + CYCLE_SHORT;
  }
  else
  {
    PC0++;
    system->cycles += CYCLE_SHORT * 2;
  }
#endif
}

/**
 * A0 - AF
 * INS - Input Short Address
 * Data input to the I/O port specified by the operand of the INS instruction
 * is loaded into the accumulator. An I/O port with an address within the
 * range 0 through 15 may be accessed by this instruction.
 * If an I/O port or pin is being used for both input and output, the port or
 * pin previously used for output must be cleared before it can be used to
 * input data.
 */
F8_OP(ins)
{
  unsigned address = system->dbus.u & B00001111;
  io_t *io = &system->io_ports[address];
  u8 i;

  if (address < 2)
#if PF_ROMC
    romc1cs(system);
#else
    system->cycles += CYCLE_SHORT;
#endif
  else
#if PF_ROMC
  {
    romc1cl(system);
    romc1b(system);
  }
#else
  system->cycles += CYCLE_LONG * 2;
#endif

  for (i = 0; i < F8_MAX_IO_LINK; i++)
    if (io->func_in[i])
      io->func_in[i](io->device_in[i], &io->data);
  A = io->data;

  add(system, &A, 0);
}

/**
 * B0 - BF
 * OUTS - Output Short Address
 * The I/O port addressed by the operand of the OUTS instruction object code
 * is loaded with the contents of the accumulator. I/O ports with addresses
 * from 0 to 15 may be accessed by this instruction. The I/O port addresses
 * are defined in Table 6-6. Outs 0 or 1 is CPU port only.
 * No status bits are modified.
 */
F8_OP(outs)
{
  unsigned address = system->dbus.u & B00001111;
  io_t *io = &system->io_ports[address];
  u8 found = FALSE;
  u8 i;

  if (address < 2)
#if PF_ROMC
    romc1cs(system);
#else
    system->cycles += CYCLE_SHORT;
#endif
  else
#if PF_ROMC
  {
    romc1cl(system);
    romc1a(system);
  }
#else
  system->cycles += CYCLE_LONG * 2;
#endif

  for (i = 0; i < F8_MAX_IO_LINK; i++)
  {
    if (io->func_out[i])
    {
      if (io->device_out[i]->set_timing)
        io->device_out[i]->set_timing(io->device_out[i],
                                      system->cycles,
                                      system->total_cycles);
      io->func_out[i](io->device_out[i], &io->data, A);
    }
  }
  if (!found)
    io->data = A;
}

/**
 * C0 - CF
 * AS - Binary Addition, Scratchpad Memory to Accumulator
 * The content of the scratchpad register referenced by the instruction
 * operand (Sreg) is added to the accumulator using binary addition. The
 * result of the binary addition is stored in the accumulator. The scratchpad
 * register contents remain unchanged. Depending on the value of Sreg, ISAR
 * may be unaltered, incremented, or decremented.
 */

#define F8_OP_AS(a) \
  F8_OP(as_##a) \
  { \
    f8_byte *reg = ISAR_OP_##a; \
    add(system, &A, reg->u); \
  }

F8_OP_AS(0)
F8_OP_AS(1)
F8_OP_AS(2)
F8_OP_AS(3)
F8_OP_AS(4)
F8_OP_AS(5)
F8_OP_AS(6)
F8_OP_AS(7)
F8_OP_AS(8)
F8_OP_AS(9)
F8_OP_AS(10)
F8_OP_AS(11)
F8_OP_AS(12)
F8_OP_AS(13)
F8_OP_AS(14)

/**
 * D0 - DF
 * ASD (Add Source Decimal)
 * Add a register to the accumulator as binary-coded decimal.
 */

#if PF_ROMC
#define F8_OP_ASD(a) \
  F8_OP(asd_##a) \
  { \
    f8_byte *reg = ISAR_OP_##a; \
    romc1cs(system); \
    add_bcd(system, &A, reg->u); \
  }
#else
#define F8_OP_ASD(a) \
  F8_OP(asd_##a) \
  { \
    f8_byte *reg = ISAR_OP_##a; \
    system->cycles += CYCLE_SHORT; \
    add_bcd(system, &A, reg->u); \
  }
#endif

F8_OP_ASD(0)
F8_OP_ASD(1)
F8_OP_ASD(2)
F8_OP_ASD(3)
F8_OP_ASD(4)
F8_OP_ASD(5)
F8_OP_ASD(6)
F8_OP_ASD(7)
F8_OP_ASD(8)
F8_OP_ASD(9)
F8_OP_ASD(10)
F8_OP_ASD(11)
F8_OP_ASD(12)
F8_OP_ASD(13)
F8_OP_ASD(14)

/**
 * E0 - EF
 * XS (eXclusive or Source)
 * Logical XOR a register into the accumulator.
 */

#define F8_OP_XS(a) \
  F8_OP(xs_##a) \
  { \
    f8_byte *reg = ISAR_OP_##a; \
    A.u ^= reg->u; \
    update_status(system); \
  }

F8_OP_XS(0)
F8_OP_XS(1)
F8_OP_XS(2)
F8_OP_XS(3)
F8_OP_XS(4)
F8_OP_XS(5)
F8_OP_XS(6)
F8_OP_XS(7)
F8_OP_XS(8)
F8_OP_XS(9)
F8_OP_XS(10)
F8_OP_XS(11)
F8_OP_XS(12)
F8_OP_XS(13)
F8_OP_XS(14)

/**
 * F0 - FF
 * NS (aNd Source)
 * Logical AND a register into the accumulator.
 */

#define F8_OP_NS(a) \
  F8_OP(ns_##a) \
  { \
    f8_byte *reg = ISAR_OP_##a; \
    A.u &= reg->u; \
    update_status(system); \
  }

F8_OP_NS(0)
F8_OP_NS(1)
F8_OP_NS(2)
F8_OP_NS(3)
F8_OP_NS(4)
F8_OP_NS(5)
F8_OP_NS(6)
F8_OP_NS(7)
F8_OP_NS(8)
F8_OP_NS(9)
F8_OP_NS(10)
F8_OP_NS(11)
F8_OP_NS(12)
F8_OP_NS(13)
F8_OP_NS(14)

/**
 * 2D - 2F
 * Invalid / Undefined opcode
 */
F8_OP(invalid)
{
  nop(system);
}

typedef void F8_OP_T(f8_system_t*);
static F8_OP_T *operations[256] =
{
  lr_a_ku,    lr_a_kl,    lr_a_qu,    lr_a_ql,
  lr_ku_a,    lr_kl_a,    lr_qu_a,    lr_ql_a,
  lr_k_pc1,   lr_pc1_k,   lr_a_isar,  lr_isar_a,
  pk,         lr_pc0_q,   lr_q_dc0,   lr_dc0_q,
  lr_dc0_h,   lr_h_dc0,   sr_a,       sl_a,
  sr_a_4,     sl_a_4,     lm,         st,
  com,        lnk,        di,         ei,
  pop,        lr_w_j,     lr_j_w,     inc,
  li,         ni,         oi,         xi,
  ai,         ci,         in,         out,
  pi,         jmp,        dci,        nop,
  xdc,        invalid,    invalid,    invalid,
  ds_0,       ds_1,       ds_2,       ds_3,
  ds_4,       ds_5,       ds_6,       ds_7,
  ds_8,       ds_9,       ds_10,      ds_11,
  ds_12,      ds_13,      ds_14,      invalid,
  lr_a_r_0,   lr_a_r_1,   lr_a_r_2,   lr_a_r_3,
  lr_a_r_4,   lr_a_r_5,   lr_a_r_6,   lr_a_r_7,
  lr_a_r_8,   lr_a_r_9,   lr_a_r_10,  lr_a_r_11,
  lr_a_r_12,  lr_a_r_13,  lr_a_r_14,  invalid,
  lr_r_a_0,   lr_r_a_1,   lr_r_a_2,   lr_r_a_3,
  lr_r_a_4,   lr_r_a_5,   lr_r_a_6,   lr_r_a_7,
  lr_r_a_8,   lr_r_a_9,   lr_r_a_10,  lr_r_a_11,
  lr_r_a_12,  lr_r_a_13,  lr_r_a_14,  invalid,
  lisu_0,     lisu_1,     lisu_2,     lisu_3,
  lisu_4,     lisu_5,     lisu_6,     lisu_7,
  lisl_0,     lisl_1,     lisl_2,     lisl_3,
  lisl_4,     lisl_5,     lisl_6,     lisl_7,
  lis_0,      lis_1,      lis_2,      lis_3,
  lis_4,      lis_5,      lis_6,      lis_7,
  lis_8,      lis_9,      lis_10,     lis_11,
  lis_12,     lis_13,     lis_14,     lis_15,
  bt_0,       bt_1,       bt_2,       bt_3,
  bt_4,       bt_5,       bt_6,       bt_7,
  am,         amd,        nm,         om,
  xm,         cm,         adc,        br7,
  bf, bf, bf, bf, bf, bf, bf, bf, bf, bf, bf, bf, bf, bf, bf, bf,
  ins, ins, ins, ins, ins, ins, ins, ins, ins, ins, ins, ins, ins, ins, ins, ins,
  outs, outs, outs, outs, outs, outs, outs, outs, outs, outs, outs, outs, outs, outs, outs, outs,
  as_0,       as_1,       as_2,       as_3,
  as_4,       as_5,       as_6,       as_7,
  as_8,       as_9,       as_10,      as_11,
  as_12,      as_13,      as_14,      invalid,
  asd_0,      asd_1,      asd_2,      asd_3,
  asd_4,      asd_5,      asd_6,      asd_7,
  asd_8,      asd_9,      asd_10,     asd_11,
  asd_12,     asd_13,     asd_14,     invalid,
  xs_0,       xs_1,       xs_2,       xs_3,
  xs_4,       xs_5,       xs_6,       xs_7,
  xs_8,       xs_9,       xs_10,      xs_11,
  xs_12,      xs_13,      xs_14,      invalid,
  ns_0,       ns_1,       ns_2,       ns_3,
  ns_4,       ns_5,       ns_6,       ns_7,
  ns_8,       ns_9,       ns_10,      ns_11,
  ns_12,      ns_13,      ns_14,      invalid,
};

u8 pressf_init(f8_system_t *system)
{
  if (!system)
    return FALSE;
  else
  {
    memset(system, 0, sizeof(f8_system_t));
    system->total_cycles = PF_CHANNEL_F_CLOCK_NTSC;
    f3850_init(&system->f8devices[0]);

    return TRUE;
  }
}

/**
 * Has the F8 system execute one instruction.
 */
void pressf_step(f8_system_t *system)
{
#if PF_HAVE_HLE_BIOS
  void (*hle_func)() = hle_get_func_from_addr(PC0.u);
#endif

#if PF_DEBUGGER
  if (debug_should_break(PC0))
    return;
#endif

#if PF_ROMC
  romc00s(system);
#else
  system->dbus = next(system);
#endif

#if PF_HAVE_HLE_BIOS
  if (hle_func)
  {
    hle_func(system);
    return;
  }
#endif
  operations[system->dbus.u](system);
}

u8 pressf_run(f8_system_t *system)
{
  if (!system)
    return FALSE;
  else
  {
    unsigned i;

    /* Step through a frame worth of cycles */
    do pressf_step(system);
    while (system->total_cycles > system->cycles);
    system->cycles -= system->total_cycles;

    /* Run the "on finish frame" callback for all devices */
    for (i = 0; i < system->f8device_count; i++)
    {
      if (system->f8devices[i].finish_frame)
        system->f8devices[i].finish_frame(&system->f8devices[i]);
    }
  }

  return TRUE;
}

void pressf_reset(f8_system_t *system)
{
  if (system)
  {
#if PF_ROMC
    romc08(system);
#else
    PC1 = PC0;
    PC0 = 0;
#endif
  }
}
