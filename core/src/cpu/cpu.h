#pragma once

#include "types.h"

typedef struct NHLogger NHLogger;

typedef struct mmem_s mmem_s;
typedef struct instrdesc_s instrdesc_s;

// Same underlying type as "this->P", so the enumerators can be directly
// used in bitwise operations.
typedef enum cpustatus_e {
    // http://www.oxyron.de/html/opcodes02.html
    CS_C = 1 << 0, // carry flag (1 on unsigned overflow)
    CS_Z = 1 << 1, // zero flag (1 when all bits of a result are 0)
    CS_I = 1 << 2, // IRQ flag (when 1, no interupts will occur (exceptions are
                   // IRQs forced by BRK and NMIs))
    CS_D = 1 << 3, // decimal flag (1 when CPU in BCD mode)
    CS_B = 1 << 4, // break flag (1 when interupt was caused by a BRK)
    CS_U = 1 << 5, // unused (always 1)
    CS_V = 1 << 6, // overflow flag (1 on signed overflow)
    CS_N = 1 << 7, // negative flag (1 when result is negative)
} cpustatus_e;

typedef struct instrctx_s {
    u8 Opcode;
    int CyclePlus1;
    instrdesc_s *Instr;
} instrctx_s;

typedef struct cpu_s {
    // ---- Registers
    // https://wiki.nesdev.org/w/index.php?title=CPU_registers
    u8 a_;   // Accumulator
    u8 x_;   // Index X
    u8 y_;   // Index Y
    u16 pc_; // Program Counter
    // https://wiki.nesdev.org/w/index.php?title=Stack
    u8 s_; // Stack Pointer, 6502 uses a descending, empty stack.
    u8 p_; // Status

    // ---- External components references
    mmem_s *mmem_;

    // This may wrap around back to 0, which is fine, since current
    // implementation doesn't assume infinite range.
    cycle_t cycle_;
    bool instrhalt_; // halt caused by instructions
    bool dmahalt_;

    // ---- temporaries for one tick
    bool ppustatReadTmp_;
    bool writeTickTmp_; // used in pretickChkToFlagHalt()
    bool maskReadTmp_;  // used in get_byte()
                        // ---- preserve across ticks
    cycle_t prevPpudataRead_;
    cycle_t prevJoy1Read_;
    cycle_t prevJoy2Read_;

    // ---- function context
    bool *read2002tmp_;
    bool rdytmp_;

    // ---- interrupt lines poll cache
    bool nmiAsserted_;
    // ---- interrupt signals
    bool nmiSig_; // pending NMI
    bool irqSig_; // pending IRQ
    bool resetSig_;
    // ---- interrupts implementation used in execution of instruction
    bool irqPcNoInc_;
    bool irqNoMemWrite_;
    bool isNmi_;
    bool irqPcNoIncTmp_;
    bool irqNoMemWriteTmp_;
    bool isNmiTmp_;

    /* Set and used in execution of instruction, not to be confused with
     * the real-time status of the hardware bus (not considered) */
    addr_t addrbus_;
    u8 databus_;
    /* Set and used in one instruction */
    u8 pageoffset_;
    /* Set in one instruction tick and used in core */
    addr_t iEffAddr_;

    instrctx_s instrctx_;

    NHLogger *logger_;
} cpu_s;

void
cpu_Init(cpu_s *self, mmem_s *mmem, NHLogger *logger);

void
cpu_Powerup(cpu_s *self);
void
cpu_Reset(cpu_s *self);

/// @param rdy RDY line input enabled
/// @param dmaOpCycle If this cycle is a DMA operation cycle
/// @param read2002 Whether $2002 was read at this tick
/// @return Whether an instruction has completed.
bool
cpu_PreTick(cpu_s *self, bool rdy, bool dmaOpCycle, bool *read2002);
void
cpu_PostTick(cpu_s *self, bool nmi, bool apuirq);

bool
cpu_DmaHalt(const cpu_s *self);

/* Test */

void
cpu_TestSetEntry(cpu_s *self, addr_t entry);
void
cpu_TestSetP(cpu_s *self, u8 val);

cycle_t
cpu_TestGetCycle(const cpu_s *self);

u8
cpu_TestGetA(const cpu_s *self);
u8
cpu_TestGetX(const cpu_s *self);
u8
cpu_TestGetY(const cpu_s *self);
addr_t
cpu_TestGetPc(const cpu_s *self);
u8
cpu_TestGetS(const cpu_s *self);
u8
cpu_TestGetP(const cpu_s *self);

void
cpu_TestGetInstrBytes(const cpu_s *self, addr_t addr, u8 bytes[3], int *len);

// ----- memory operations
u8
cpu_getb_(const cpu_s *self, addr_t addr);
void
cpu_setb_(cpu_s *self, addr_t addr, u8 b);

// ----- stack operations
void
cpu_pushByte_(cpu_s *self, u8 b);
void
cpu_prePopByte_(cpu_s *self);
u8
cpu_postPopByte_(cpu_s *self);

bool
cpu_checkFlag_(const cpu_s *self, cpustatus_e flag);
void
cpu_setFlag_(cpu_s *self, cpustatus_e flag);
void
cpu_unsetFlag_(cpu_s *self, cpustatus_e flag);
void
cpu_testFlag_(cpu_s *self, cpustatus_e flag, bool cond);

void
cpu_halt_(cpu_s *self);

bool
cpu_inHwIrq_(const cpu_s *self);

void
cpu_pollInterrupt_(cpu_s *self);
