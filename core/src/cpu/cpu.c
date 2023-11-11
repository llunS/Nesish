#include "cpu.h"

#include "byteutils.h"
#include "nhassert.h"
#include "spec.h"
#include "memory/mmem.h"
#include "cpu/instrtable.h"

#define NH_BRK_OPCODE 0

static int
testGetOperandBytes(u8 opcode);

static u16
getByte2(const cpu_s *self, addr_t addr);

static bool
inNmi(const cpu_s *self);
static bool
inReset(const cpu_s *self);

void
cpu_Init(cpu_s *self, mmem_s *mmem, NHLogger *logger)
{
    self->mmem_ = mmem;
    self->logger_ = logger;
}

void
cpu_Powerup(cpu_s *self)
{
    // https://wiki.nesdev.org/w/index.php?title=CPU_power_up_state
    self->p_ = 0x34; // (IRQ disabled)
    self->a_ = self->x_ = self->y_ = 0;
    self->s_ = 0xFD;

    {
        self->cycle_ = 0;
        self->instrhalt_ = false;
        self->dmahalt_ = false;

        self->maskReadTmp_ = false;
        self->prevPpudataRead_ = 0;
        self->prevJoy1Read_ = 0;
        self->prevJoy2Read_ = 0;

        self->nmiAsserted_ = false;
        self->nmiSig_ = false;
        self->irqSig_ = false;
        self->resetSig_ = false;
        self->irqPcNoInc_ = false;
        self->irqNoMemWrite_ = false;
        self->isNmi_ = false;
        self->irqPcNoIncTmp_ = false;
        self->irqNoMemWriteTmp_ = false;
        self->isNmiTmp_ = false;

        self->instrctx_ = (instrctx_s){0x00, 0, NULL};
    }

    // program entry point
    self->pc_ = getByte2(self, NH_RESET_VECTOR_ADDR);
}

void
cpu_Reset(cpu_s *self)
{
    self->resetSig_ = true;

    self->instrhalt_ = false;
}

void
cpu_TestSetEntry(cpu_s *self, addr_t entry)
{
    self->pc_ = entry;
}

void
cpu_TestSetP(cpu_s *self, u8 val)
{
    self->p_ = val;
}

static void
pretickDeferRet(cpu_s *self)
{
    // clear some flags
    self->maskReadTmp_ = false;
    // pass out outputs
    *self->read2002tmp_ = self->ppustatReadTmp_;
}

static void
pretickChkToFlagHalt(cpu_s *self)
{
    if (!self->dmahalt_)
    {
        // Can only halt on read cycle
        if (self->rdytmp_ && !self->writeTickTmp_)
        {
            self->dmahalt_ = true;
        }
    }
}

bool
cpu_PreTick(cpu_s *self, bool rdy, bool dmaOpCycle, bool *read2002)
{
    // prep pretickDeferRet()
    self->ppustatReadTmp_ = false;
    self->read2002tmp_ = read2002;

    if (self->instrhalt_)
    {
        // Don't inc "self->cycle_"?
        pretickDeferRet(self);
        return false;
    }

    // check to unset halt flag
    if (self->dmahalt_)
    {
        if (!rdy)
        {
            self->dmahalt_ = false;
        }
    }
    // do nothing when the CPU is halted and the DMA is doing its work
    // -- inc "self->cycle_"?
    // -- still poll interrupts? not sure, take it as yes for now
    // then implement it as masking out read operatoin, so we can still poll
    // interrupts
    // -- CPU must be halted in read cycle since that's when it can be halted by
    // DMA
    self->maskReadTmp_ = self->dmahalt_ && dmaOpCycle;
    // prep pretickChkToFlagHalt()
    // common function to check to set halt flag
    // won't conflict with the above unset logic due to "rdy"
    self->rdytmp_ = rdy;

    bool instrdone = false;
    // Fetch opcode
    if (!self->instrctx_.Instr)
    {
        // default to read tick so we only need to mark write operations.
        // i.e. each cycle is read cycle unless marked otherwise.
        self->writeTickTmp_ = false;
        u8 opcode = cpu_getb_(self, self->pc_);
        pretickChkToFlagHalt(self);

        if (!self->dmahalt_)
        {
            // IRQ/RESET/NMI/BRK all use the same behavior.
            if (cpu_inHwIrq_(self))
            {
                // Force the instruction register to $00 and discard the fetch
                // opcode
                opcode = NH_BRK_OPCODE;
            }
            if (!self->irqPcNoInc_)
            {
                ++self->pc_;
            }

            self->instrctx_.Opcode = opcode;
            self->instrctx_.CyclePlus1 = 0;
            self->instrctx_.Instr = &cpu_instrtable_[opcode];
        }
    }
    // Rest cycles of the instruction
    else
    {
        /* Backup states that may be altered after one instruction cycle and may
         * be used as input of the cycle themselves */
        // Registers
        u8 prevA = self->a_;
        u8 prevX = self->x_;
        u8 prevY = self->y_;
        u16 prevPC = self->pc_;
        u8 prevS = self->s_;
        u8 prevP = self->p_;
        // Intermediate states
        addr_t prevAddrBus = self->addrbus_;
        u8 prevDataBus = self->databus_;

        self->writeTickTmp_ = false;
        self->instrctx_.Instr->Frm(self->instrctx_.CyclePlus1, self,
                                   self->instrctx_.Instr->Core, &instrdone);
        pretickChkToFlagHalt(self);

        if (!self->dmahalt_)
        {
            ++self->instrctx_.CyclePlus1;
        }
        // Restore changes, so that repeat cycles get the same result, excluding
        // possible multiple side effects.
        else
        {
            self->a_ = prevA;
            self->x_ = prevX;
            self->y_ = prevY;
            self->pc_ = prevPC;
            self->s_ = prevS;
            self->p_ = prevP;

            self->addrbus_ = prevAddrBus;
            self->databus_ = prevDataBus;
            // no need for "self->pageoffset_" and "self->iEffAddr_"
        }
        if (instrdone)
        {
            // ------ Current
            if (!self->dmahalt_)
            {
                // Use inReset() before updating relevant flags
                if (inReset(self))
                {
                    // Indicating RESET is handled.
                    self->resetSig_ = false;
                }
                // Use inNmi() before updating relevant flags
                if (inNmi(self))
                {
                    // Indicating NMI is handled.
                    // @TEST: Right to do this at last cycle?
                    self->nmiSig_ = false;
                }
            }

            // ------ Next
            /* Poll interrupts */
            // Poll interrupts even it's halted by DMAs (self->dmahalt_)
            // Most instructions poll interrupts at the last cycle.
            // Special cases are listed and handled on thier own.
            if (self->instrctx_.Opcode == NH_BRK_OPCODE ||
                self->instrctx_.Instr->Addrmode == AM_REL)
            {
                // The interrupt sequences themselves do not perform
                // interrupt polling, meaning at least one instruction from
                // the interrupt handler will execute before another
                // interrupt is serviced
                // https://www.nesdev.org/wiki/CPU_interrupts#Detailed_interrupt_behavior
                // All types of interrupts reuse the same logic as BRK for
                // the most part.

                // Branch instructions are also special, we handle it
                // in its implementation. They are identified by address
                // mode.
            }
            else
            {
                // Poll interrupts at the last cycle
                cpu_pollInterrupt_(self);
            }

            if (!self->dmahalt_)
            {
                // Swap at the end of instruction, setup states for next
                // instruction
                {
                    self->irqPcNoInc_ = self->irqPcNoIncTmp_;
                    self->irqNoMemWrite_ = self->irqNoMemWriteTmp_;
                    self->isNmi_ = self->isNmiTmp_;

                    self->irqPcNoIncTmp_ = false;
                    self->irqNoMemWriteTmp_ = false;
                    self->isNmiTmp_ = false;
                }

                // So that next instruction can continue afterwards.
                self->instrctx_.Instr = NULL;
            }
        }
    }
    ++self->cycle_;

    pretickDeferRet(self);
    // When it's halted by DMCs at read cycle, flagging it as not done seems
    // reasonable. Even if the read cycle may be the last cycle of an
    // instruction.
    return self->dmahalt_ ? false : instrdone;
}

void
cpu_PostTick(cpu_s *self, bool nmi, bool apuirq)
{
    if (self->instrhalt_)
    {
        return;
    }

    // Assuming the detectors don't consider if it's halted by DMAs
    // (self->dmahalt_)

    // Edge/Level detector polls lines during φ2 of each CPU cycle.
    // So this sets up signals for NEXT cycle.
    // --- NMI
    if (!self->nmiAsserted_ && nmi)
    {
        self->nmiSig_ = true; // raise an internal signal
    }
    self->nmiAsserted_ = nmi;
    // --- IRQ
    self->irqSig_ = false; // inactive unless keep asserting.
    if (apuirq)
    {
        // The cause of delayed IRQ response for some instructions.
        if (!cpu_checkFlag_(self, CS_I))
        {
            self->irqSig_ = true; // raise an internal signal
        }
    }
}

bool
cpu_DmaHalt(const cpu_s *self)
{
    return self->dmahalt_;
}

cycle_t
cpu_TestGetCycle(const cpu_s *self)
{
    return self->cycle_;
}

u8
cpu_TestGetA(const cpu_s *self)
{
    return self->a_;
}

u8
cpu_TestGetX(const cpu_s *self)
{
    return self->x_;
}

u8
cpu_TestGetY(const cpu_s *self)
{
    return self->y_;
}

addr_t
cpu_TestGetPc(const cpu_s *self)
{
    return self->pc_;
}

u8
cpu_TestGetS(const cpu_s *self)
{
    return self->s_;
}

u8
cpu_TestGetP(const cpu_s *self)
{
    return self->p_;
}

void
cpu_TestGetInstrBytes(const cpu_s *self, addr_t addr, u8 bytes[3], int *len)
{
    u8 opcode = cpu_getb_(self, addr);
    bytes[0] = opcode;

    int operandByteCnt = testGetOperandBytes(opcode);
    for (int i = 0; i < operandByteCnt; ++i)
    {
        bytes[i + 1] = cpu_getb_(self, (addr_t)(addr + i + 1));
    }

    *len = operandByteCnt + 1;
}

int
testGetOperandBytes(u8 opcode)
{
    addrmode_e addrmode = cpu_instrtable_[opcode].Addrmode;
    switch (addrmode)
    {
        case AM_IMP:
        case AM_ACC:
            return 0;
            break;

        case AM_IMM:
        case AM_ZP0:
        case AM_ZPX:
        case AM_ZPY:
        case AM_IZX:
        case AM_IZY:
        case AM_REL:
            return 1;
            break;

        case AM_ABS:
        case AM_ABX:
        case AM_ABY:
        case AM_IND:
            return 2;
            break;

        default:
            return 0; // absurd value
            break;
    }
}

u8
cpu_getb_(const cpu_s *self, addr_t addr)
{
    // Mask out read
    if (self->maskReadTmp_)
    {
        // Whatever can be returned, since after CPU resumes it will still do it
        // again (of course without "self->maskReadTmp_" set)
        return 0x00;
    }

    addr_t realaddr = addr;
    if (NH_PPU_REG_ADDR_HEAD <= addr && addr <= NH_PPU_REG_ADDR_TAIL)
    {
        realaddr = (addr & NH_PPU_REG_ADDR_MASK) | NH_PPU_REG_ADDR_HEAD;
    }

    u8 byte;
    // Ignore subsequent joypad reads in contiguous set of CPU cycles.
    // Verified by dmc_dma_during_read4/dma_4016_read.nes
    // https://www.nesdev.org/wiki/DMA#Register_conflicts
    /* Joypads are clocked via direct lines from the CPU, called joypad 1 /OE
     * and joypad 2 /OE, rather than going over the address bus. These output
     * enables remain asserted the entire CPU cycle and even across adjacent
     * cycles if they're both reading the same joypad register. Therefore,
     * controllers only see a single read for each contiguous set of reads of a
     * joypad register. */
    if (NH_CTRL1_REG_ADDR == realaddr &&
        self->prevJoy1Read_ + 1 == self->cycle_)
    {
        byte = mmem_GetLatch(self->mmem_);
    }
    else if (NH_CTRL2_REG_ADDR == realaddr &&
             self->prevJoy2Read_ + 1 == self->cycle_)
    {
        byte = mmem_GetLatch(self->mmem_);
    }
    else
    {
        bool retainLatch = false;
        u8 prevLatch = 0;
        bool returnLatch = false;
        /* Ignore subsequent PPUDATA reads in contiguous set of CPU cycles */
        // Use previous open bus value to pass both
        // 1) dmc_dma_during_read4/dma_2007_read.nes
        // 2) dmc_dma_during_read4/double_2007_read.nes
        if (NH_PPUDATA_ADDR == realaddr &&
            self->prevPpudataRead_ + 1 == self->cycle_)
        {
            prevLatch = mmem_GetLatch(self->mmem_);
            retainLatch = true;
            returnLatch = true;
        }

        NHErr err = mmem_GetB(self->mmem_, addr, &byte);
        if (NH_FAILED(err))
        {
            ASSERT_ERROR(self->logger_,
                         "Invalid memory read: " ADDRFMT ", " NHERRFMT, addr,
                         err);
            byte = 0xFF; // Apparent value
        }

        if (retainLatch)
        {
            mmem_OverrideLatch(self->mmem_, prevLatch);
        }
        if (returnLatch)
        {
            byte = mmem_GetLatch(self->mmem_);
        }
    }

    switch (realaddr)
    {
        case NH_PPUSTATUS_ADDR:
            ((cpu_s *)self)->ppustatReadTmp_ = true;
            break;
        case NH_PPUDATA_ADDR:
            // Assuming cpu_getb_(self,) is called at most once per CPU cycle
            ((cpu_s *)self)->prevPpudataRead_ = self->cycle_;
            break;
        case NH_CTRL1_REG_ADDR:
            // Assuming cpu_getb_(self,) is called at most once per CPU cycle
            ((cpu_s *)self)->prevJoy1Read_ = self->cycle_;
            break;
        case NH_CTRL2_REG_ADDR:
            // Assuming cpu_getb_(self,) is called at most once per CPU cycle
            ((cpu_s *)self)->prevJoy2Read_ = self->cycle_;
            break;
        default:
            break;
    }

    return byte;
}

void
cpu_setb_(cpu_s *self, addr_t addr, u8 b)
{
    // The mark of this flag relys on the instruction implementation
    // calling read or wrtie interface each cycle.
    self->writeTickTmp_ = true;

    // At hardware, this is done by setting to line to read instead of write
    if (!self->irqNoMemWrite_)
    {
        NHErr err = membase_SetB(&self->mmem_->Base, addr, b);
        if (NH_FAILED(err) && err == NH_ERR_READ_ONLY &&
            err == NH_ERR_UNAVAILABLE)
        {
            ASSERT_ERROR(self->logger_,
                         "Invalid memory write: " ADDRFMT ", " NHERRFMT, addr,
                         err);
        }
    }
}

u16
getByte2(const cpu_s *self, addr_t addr)
{
    u8 low = cpu_getb_(self, addr);
    u8 high = cpu_getb_(self, addr + 1);
    return ToByte2(high, low);
}

void
cpu_pushByte_(cpu_s *self, u8 b)
{
    // It may not actually write.
    LOG_TRACE(self->logger_, "Push byte: " U8FMTX, b);

    cpu_setb_(self, NH_STACK_PAGE_MASK | self->s_, b);
    // The pointer decrement happens regardless of write or not
    --self->s_;
}

void
cpu_prePopByte_(cpu_s *self)
{
    ++self->s_;
}

u8
cpu_postPopByte_(cpu_s *self)
{
    u8 byte = cpu_getb_(self, NH_STACK_PAGE_MASK | self->s_);
    LOG_TRACE(self->logger_, "Pop byte: " U8FMTX, byte);
    return byte;
}

bool
cpu_checkFlag_(const cpu_s *self, cpustatus_e flag)
{
    return (self->p_ & flag) == flag;
}

void
cpu_setFlag_(cpu_s *self, cpustatus_e flag)
{
    self->p_ |= flag;
}

void
cpu_unsetFlag_(cpu_s *self, cpustatus_e flag)
{
    self->p_ &= ~flag;
}

void
cpu_testFlag_(cpu_s *self, cpustatus_e flag, bool cond)
{
    cond ? cpu_setFlag_(self, flag) : cpu_unsetFlag_(self, flag);
}

void
cpu_halt_(cpu_s *self)
{
    LOG_INFO(self->logger_, "Halting...");

    self->instrhalt_ = true;
}

void
cpu_pollInterrupt_(cpu_s *self)
{
    // Assuming CPU still polls when it is halted by DMAs (self->dmahalt_)

    /* It may be polled multiple times during an instruction.
     * e.g. Branch instructions may poll 2 times if branch is taken and page is
     * crossed.
     */
    // Already pending interrupt sequence for execution next.
    bool pendingAlready = self->irqPcNoIncTmp_;
    if (pendingAlready)
    {
        return;
    }

    // @TEST: a) RESET has highest priority? b)RESET is like others, polled only
    // at certain points?
    if (self->resetSig_)
    {
        self->irqPcNoIncTmp_ = true;
        self->irqNoMemWriteTmp_ = true;
    }
    // NMI has higher precedence than IRQ
    // https://www.nesdev.org/wiki/NMI
    else if (self->nmiSig_)
    {
        self->irqPcNoIncTmp_ = true;
        self->isNmiTmp_ = true;
    }
    else if (self->irqSig_)
    {
        self->irqPcNoIncTmp_ = true;
    }
}

/// @brief Is running hardware interrupts (IRQ/RESET/NMI)
bool
cpu_inHwIrq_(const cpu_s *self)
{
    return self->irqPcNoInc_;
}

/// @brief Is running NMI interrupt sequence
bool
inNmi(const cpu_s *self)
{
    return self->isNmi_;
}

/// @brief Is running RESET interrupt sequence
bool
inReset(const cpu_s *self)
{
    return /*cpu_inHwIrq_(self) &&*/ self->irqNoMemWrite_;
}
