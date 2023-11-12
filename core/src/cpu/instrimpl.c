#include "instrimpl.h"

#include "cpu/cpu.h"
#include "byteutils.h"
#include "spec.h"

static void
frm_phr_impl(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 val);
static void
frm_plr_impl(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 *val);
static void
frm_abs_pre(int idx, cpu_s *cpu);
static void
frm_abi_pre(int idx, cpu_s *cpu, u8 val);
static void
frm_abi_r(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 val);
static void
frm_abi_w(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 val);
static void
frm_abi_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 val);
static void
frm_zpi_pre(int idx, cpu_s *cpu, u8 val);
static void
frm_zpi_r(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 val);
static void
frm_zpi_w(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 val);
static void
frm_izx_pre(int idx, cpu_s *cpu);
static void
frm_izy_pre(int idx, cpu_s *cpu);

static u8
core_cmp_impl(cpu_s *cpu, u8 in, u8 reg);
static void
core_and_op(cpu_s *cpu, u8 in);
static void
core_ror_op(cpu_s *cpu, u8 in, u8 *out);
static void
core_XhX_impl(cpu_s *cpu, u8 in, u8 *out);

static void
throwaway(cpu_s *cpu, u8 data);
static void
ignoreub(u8 src, u8 *dst);
static void
setlow(addr_t *dst, u8 val);
static void
sethigh(addr_t *dst, u8 val);
static u8
getlow(addr_t val);
static u8
gethigh(addr_t val);

static void
testN(cpu_s *cpu, u8 val);
static void
testZ(cpu_s *cpu, u8 val);

void
cpu_frm_brk(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    (void)(core);

    switch (idx) {
    default:
    case 0: {
        throwaway(cpu, cpu_getb_(cpu, cpu->pc_));
        if (!cpu->irqPcNoInc_) {
            ++cpu->pc_;
        }
    } break;

    case 1: {
        cpu_pushByte_(cpu, gethigh(cpu->pc_));
    } break;

    case 2: {
        cpu_pushByte_(cpu, getlow(cpu->pc_));
    } break;

    case 3: {
        // https://wiki.nesdev.org/w/index.php?title=Status_flags#The_B_flag
        // Push B flag differently
        u8 pushed = cpu->p_;
        if (cpu_inHwIrq_(cpu)) {
            pushed &= (u8)(~CS_B);
        } else {
            pushed |= (u8)(CS_B);
        }
        cpu_pushByte_(cpu, pushed);

        // Interrupt hijacking
        // Determine vector address at this cycle
        // Priority: RESET(?)>NMI>IRQ>BRK
        // Check using the current signal status, instead of what
        // initiates the interrupt sequence.
        // The hijacking doesn't lose the B flag
        // RESET
        if (cpu->resetSig_) {
            cpu->addrbus_ = NH_RESET_VECTOR_ADDR;
        }
        // NMI
        else if (cpu->nmiSig_) {
            cpu->addrbus_ = NH_NMI_VECTOR_ADDR;
        }
        // Since IRQ and BRK use the same vector, don't bother to check
        else {
            cpu->addrbus_ = NH_IRQ_VECTOR_ADDR;
        }
    } break;

    case 4: {
        setlow(&cpu->pc_, cpu_getb_(cpu, cpu->addrbus_));
        // Set I flag at this cycle according to doc:
        // https://www.nesdev.org/wiki/CPU_interrupts#Interrupt_hijacking
        cpu_setFlag_(cpu, CS_I);
    } break;

    case 5: {
        ++cpu->addrbus_;
        sethigh(&cpu->pc_, cpu_getb_(cpu, cpu->addrbus_));

        *done = true;
    } break;
    }
}

void
cpu_frm_rti(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    (void)(core);

    switch (idx) {
    default:
    case 0: {
        throwaway(cpu, cpu_getb_(cpu, cpu->pc_));
    } break;

    case 1: {
        cpu_prePopByte_(cpu);
    } break;

    case 2: {
        // Disregards bits 5 and 4 when reading flags from the stack
        // https://www.nesdev.org/wiki/Status_flags#The_B_flag
        ignoreub(cpu_postPopByte_(cpu), &cpu->p_);
        cpu_prePopByte_(cpu);
    } break;

    case 3: {
        setlow(&cpu->pc_, cpu_postPopByte_(cpu));
        cpu_prePopByte_(cpu);
    } break;

    case 4: {
        sethigh(&cpu->pc_, cpu_postPopByte_(cpu));

        *done = true;
    } break;
    }
}

void
cpu_frm_rts(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    (void)(core);

    switch (idx) {
    default:
    case 0: {
        throwaway(cpu, cpu_getb_(cpu, cpu->pc_));
    } break;

    case 1: {
        cpu_prePopByte_(cpu);
    } break;

    case 2: {
        setlow(&cpu->pc_, cpu_postPopByte_(cpu));
        cpu_prePopByte_(cpu);
    } break;

    case 3: {
        sethigh(&cpu->pc_, cpu_postPopByte_(cpu));
    } break;

    case 4: {
        ++cpu->pc_;

        *done = true;
    } break;
    }
}

void
frm_phr_impl(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 val)
{
    (void)(core);

    switch (idx) {
    default:
    case 0: {
        throwaway(cpu, cpu_getb_(cpu, cpu->pc_));
    } break;

    case 1: {
        cpu_pushByte_(cpu, val);

        *done = true;
    } break;
    }
}

void
cpu_frm_pha(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    frm_phr_impl(idx, cpu, core, done, cpu->a_);
}

void
cpu_frm_php(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    // https://wiki.nesdev.org/w/index.php?title=Status_flags#The_B_flag
    // Push the status register with the B flag set
    frm_phr_impl(idx, cpu, core, done, cpu->p_ | CS_B);
}

void
frm_plr_impl(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 *val)
{
    (void)(core);

    switch (idx) {
    default:
    case 0: {
        throwaway(cpu, cpu_getb_(cpu, cpu->pc_));
    } break;

    case 1: {
        cpu_prePopByte_(cpu);
    } break;

    case 2: {
        *val = cpu_postPopByte_(cpu);

        *done = true;
    } break;
    }
}

void
cpu_frm_pla(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    u8 val;
    frm_plr_impl(idx, cpu, core, done, &val);
    if (*done) {
        cpu->a_ = val;

        testN(cpu, cpu->a_);
        testZ(cpu, cpu->a_);
    }
}

void
cpu_frm_plp(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    u8 val;
    frm_plr_impl(idx, cpu, core, done, &val);
    if (*done) {
        // Disregards bits 5 and 4 when reading flags from the stack
        // https://www.nesdev.org/wiki/Status_flags#The_B_flag
        ignoreub(val, &cpu->p_);
    }
}

void
cpu_frm_jsr(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    (void)(core);

    switch (idx) {
    default:
    case 0: {
        cpu->databus_ = cpu_getb_(cpu, cpu->pc_++);
    } break;

    case 1: {
        // Unclear as to what internal operation is done at this cycle.
    } break;

    case 2: {
        cpu_pushByte_(cpu, gethigh(cpu->pc_));
    } break;

    case 3: {
        cpu_pushByte_(cpu, getlow(cpu->pc_));
    } break;

    case 4: {
        // Fetch high address byte before mutating PC
        u8 high = cpu_getb_(cpu, cpu->pc_);
        setlow(&cpu->pc_, cpu->databus_);
        sethigh(&cpu->pc_, high);

        *done = true;
    } break;
    }
}

void
cpu_frm_imp(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    // @TODO: Pipelining

    // Since the in and out won't be used in core for this kind of
    // instructions, reuse acc implemenation for simplicity.
    cpu_frm_acc(idx, cpu, core, done);
}

void
cpu_frm_acc(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0: {
        throwaway(cpu, cpu_getb_(cpu, cpu->pc_));

        // @TODO: Pipelining
        core(cpu, cpu->a_, &cpu->a_);

        *done = true;
    } break;
    }
}

void
cpu_frm_imm(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0: {
        // @TODO: Pipelining
        u8 out;
        core(cpu, cpu_getb_(cpu, cpu->pc_++), &out);
        (void)(out);

        *done = true;
    } break;
    }
}

void
cpu_frm_abs_jmp(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    (void)(core);

    switch (idx) {
    default:
    case 0: {
        cpu->databus_ = cpu_getb_(cpu, cpu->pc_++);
    } break;

    case 1: {
        // Fetch high address byte before mutating PC
        u8 high = cpu_getb_(cpu, cpu->pc_);
        setlow(&cpu->pc_, cpu->databus_);
        sethigh(&cpu->pc_, high);

        *done = true;
    } break;
    }
}

void
frm_abs_pre(int idx, cpu_s *cpu)
{
    switch (idx) {
    default:
    case 0: {
        setlow(&cpu->addrbus_, cpu_getb_(cpu, cpu->pc_++));
    } break;

    case 1: {
        sethigh(&cpu->addrbus_, cpu_getb_(cpu, cpu->pc_++));
    } break;
    }
}

void
cpu_frm_abs_r(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0:
    case 1: {
        frm_abs_pre(idx, cpu);
    } break;

    case 2: {
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, cpu_getb_(cpu, cpu->addrbus_), &out);
        (void)(out);

        *done = true;
    } break;
    }
}

void
cpu_frm_abs_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0:
    case 1: {
        frm_abs_pre(idx, cpu);
    } break;

    case 2: {
        cpu->databus_ = cpu_getb_(cpu, cpu->addrbus_);
    } break;

    case 3: {
        cpu_setb_(cpu, cpu->addrbus_, cpu->databus_);
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, cpu->databus_, &out);
        cpu->databus_ = out;
    } break;

    case 4: {
        cpu_setb_(cpu, cpu->addrbus_, cpu->databus_);

        *done = true;
    } break;
    }
}

void
cpu_frm_abs_w(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0:
    case 1: {
        frm_abs_pre(idx, cpu);
    } break;

    case 2: {
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, 0, &out);
        cpu_setb_(cpu, cpu->addrbus_, out);

        *done = true;
    } break;
    }
}

void
frm_abi_pre(int idx, cpu_s *cpu, u8 val)
{
    switch (idx) {
    default:
    case 0: {
        setlow(&cpu->addrbus_, cpu_getb_(cpu, cpu->pc_++));
    } break;

    case 1: {
        sethigh(&cpu->addrbus_, cpu_getb_(cpu, cpu->pc_++));
        addr_t newaddr = getlow(cpu->addrbus_) + val;
        cpu->pageoffset_ = !!(newaddr & 0xFF00);
        setlow(&cpu->addrbus_, getlow(newaddr));
    } break;
    }
}

void
frm_abi_r(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 val)
{
    switch (idx) {
    default:
    case 0:
    case 1: {
        frm_abi_pre(idx, cpu, val);
    } break;

    case 2: {
        // The read must be done regardlessly.
        u8 in = cpu_getb_(cpu, cpu->addrbus_);
        if (cpu->pageoffset_) {
            cpu->addrbus_ += ((addr_t)(cpu->pageoffset_) << 8);
        } else {
            cpu->iEffAddr_ = cpu->addrbus_;
            u8 out;
            core(cpu, in, &out);
            (void)(out);

            *done = true;
        }
    } break;

    case 3: {
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, cpu_getb_(cpu, cpu->addrbus_), &out);
        (void)(out);

        *done = true;
    } break;
    }
}

void
frm_abi_w(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 val)
{
    switch (idx) {
    default:
    case 0:
    case 1: {
        frm_abi_pre(idx, cpu, val);
    } break;

    case 2: {
        // Read from possibly smaller address first.
        (void)(cpu_getb_(cpu, cpu->addrbus_));
        // if (cpu->pageoffset_)
        {
            cpu->addrbus_ += ((addr_t)(cpu->pageoffset_) << 8);
        }
    } break;

    case 3: {
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, 0, &out);
        cpu_setb_(cpu, cpu->addrbus_, out);

        *done = true;
    } break;
    }
}

void
frm_abi_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 val)
{
    switch (idx) {
    default:
    case 0:
    case 1: {
        frm_abi_pre(idx, cpu, val);
    } break;

    case 2: {
        // Read from possibly smaller address first.
        (void)(cpu_getb_(cpu, cpu->addrbus_));
        // if (cpu->pageoffset_)
        {
            cpu->addrbus_ += ((addr_t)(cpu->pageoffset_) << 8);
        }
    } break;

    case 3: {
        cpu->databus_ = cpu_getb_(cpu, cpu->addrbus_);
    } break;

    case 4: {
        cpu_setb_(cpu, cpu->addrbus_, cpu->databus_);
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, cpu->databus_, &out);
        cpu->databus_ = out;
    } break;

    case 5: {
        cpu_setb_(cpu, cpu->addrbus_, cpu->databus_);

        *done = true;
    } break;
    }
}

void
cpu_frm_abx_r(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    frm_abi_r(idx, cpu, core, done, cpu->x_);
}

void
cpu_frm_abx_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    frm_abi_rmw(idx, cpu, core, done, cpu->x_);
}

void
cpu_frm_abx_w(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    frm_abi_w(idx, cpu, core, done, cpu->x_);
}

void
cpu_frm_aby_r(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    frm_abi_r(idx, cpu, core, done, cpu->y_);
}

void
cpu_frm_aby_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    frm_abi_rmw(idx, cpu, core, done, cpu->y_);
}

void
cpu_frm_aby_w(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    frm_abi_w(idx, cpu, core, done, cpu->y_);
}

void
cpu_frm_zp_r(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0: {
        cpu->addrbus_ = (addr_t)(cpu_getb_(cpu, cpu->pc_++));
    } break;

    case 1: {
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, cpu_getb_(cpu, cpu->addrbus_), &out);
        (void)(out);

        *done = true;
    } break;
    }
}

void
cpu_frm_zp_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0: {
        cpu->addrbus_ = (addr_t)(cpu_getb_(cpu, cpu->pc_++));
    } break;

    case 1: {
        cpu->databus_ = cpu_getb_(cpu, cpu->addrbus_);
    } break;

    case 2: {
        cpu_setb_(cpu, cpu->addrbus_, cpu->databus_);
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, cpu->databus_, &out);
        cpu->databus_ = out;
    } break;

    case 3: {
        cpu_setb_(cpu, cpu->addrbus_, cpu->databus_);

        *done = true;
    } break;
    }
}

void
cpu_frm_zp_w(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0: {
        cpu->addrbus_ = (addr_t)(cpu_getb_(cpu, cpu->pc_++));
    } break;

    case 1: {
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, 0, &out);
        cpu_setb_(cpu, cpu->addrbus_, out);

        *done = true;
    } break;
    }
}

void
frm_zpi_pre(int idx, cpu_s *cpu, u8 val)
{
    switch (idx) {
    default:
    case 0: {
        cpu->addrbus_ = (addr_t)(cpu_getb_(cpu, cpu->pc_++));
    } break;

    case 1: {
        (void)(cpu_getb_(cpu, cpu->addrbus_));
        cpu->addrbus_ = (cpu->addrbus_ + val) & 0x00FF;
    } break;
    }
}

void
frm_zpi_r(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 val)
{
    switch (idx) {
    default:
    case 0:
    case 1: {
        frm_zpi_pre(idx, cpu, val);
    } break;

    case 2: {
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, cpu_getb_(cpu, cpu->addrbus_), &out);
        (void)(out);

        *done = true;
    } break;
    }
}

void
frm_zpi_w(int idx, cpu_s *cpu, instrcore_f core, bool *done, u8 val)
{
    switch (idx) {
    default:
    case 0:
    case 1: {
        frm_zpi_pre(idx, cpu, val);
    } break;

    case 2: {
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, 0, &out);
        cpu_setb_(cpu, cpu->addrbus_, out);

        *done = true;
    } break;
    }
}

void
cpu_frm_zpx_r(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    frm_zpi_r(idx, cpu, core, done, cpu->x_);
}

void
cpu_frm_zpx_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0:
    case 1: {
        frm_zpi_pre(idx, cpu, cpu->x_);
    } break;

    case 2: {
        cpu->databus_ = cpu_getb_(cpu, cpu->addrbus_);
    } break;

    case 3: {
        cpu_setb_(cpu, cpu->addrbus_, cpu->databus_);
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, cpu->databus_, &out);
        cpu->databus_ = out;
    } break;

    case 4: {
        cpu_setb_(cpu, cpu->addrbus_, cpu->databus_);

        *done = true;
    } break;
    }
}

void
cpu_frm_zpx_w(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    frm_zpi_w(idx, cpu, core, done, cpu->x_);
}

void
cpu_frm_zpy_r(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    frm_zpi_r(idx, cpu, core, done, cpu->y_);
}

void
cpu_frm_zpy_w(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    frm_zpi_w(idx, cpu, core, done, cpu->y_);
}

void
cpu_frm_rel(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0: {
        cpu->databus_ = cpu_getb_(cpu, cpu->pc_++);
        u8 out;
        core(cpu, 0, &out);
        bool branchTaken = out;

        if (!branchTaken) {
            *done = true;
        }
        // This is polled regardless of branching or not
        cpu_pollInterrupt_(cpu);
        // Even if interrupt is detected, it is delayed until this
        // instruction is complete, or else the taken branch could be missed
        // after the handler returns.
        // So it's fine we poll in the middle of an instruction.
    } break;

    case 1: {
        // Branch is taken at this point

        u8 opcode = cpu_getb_(cpu, cpu->pc_);
        (void)(opcode);

        // Set PCL and calculate PCH offset, using larger type.
        // The conversion to i8 is needed, so that higher bits be
        // padded with sign bit when converted to larger unsigned type.
        i8 offset = (i8)(cpu->databus_);
        addr_t newpc = cpu->pc_ + offset;
        // Before PC is mutated
        // cpu->pageoffset_ is of u8, the same as gethigh()
        cpu->pageoffset_ = gethigh(newpc) - gethigh(cpu->pc_);
        setlow(&cpu->pc_, getlow(newpc));

        if (!cpu->pageoffset_) {
            *done = true;
        }
        // A taken non-page-crossing doesn't poll so it may delay the
        // interrupt by one instruction.
    } break;

    case 2: {
        // Branch occurs to different page at this point

        // The high byte of Program Counter (PCH) is invalid at this time,
        // i.e. it may be smaller or bigger by $0100.
        u8 opcode = cpu_getb_(cpu, cpu->pc_);
        (void)(opcode);

        // Fix PCH
        // if (cpu->pageoffset_)
        {
            cpu->pc_ += ((addr_t)(cpu->pageoffset_) << 8);
        }

        *done = true;
        cpu_pollInterrupt_(cpu);
    } break;
    }
}

void
frm_izx_pre(int idx, cpu_s *cpu)
{
    switch (idx) {
    default:
    case 0: {
        cpu->addrbus_ = (addr_t)(cpu_getb_(cpu, cpu->pc_++));
    } break;

    case 1: {
        (void)(cpu_getb_(cpu, cpu->addrbus_));
        cpu->addrbus_ = (cpu->addrbus_ + cpu->x_) & 0x00FF;
    } break;

    case 2: {
        cpu->databus_ = cpu_getb_(cpu, cpu->addrbus_);
    } break;

    case 3: {
        u8 high = cpu_getb_(cpu, (cpu->addrbus_ + 1) & 0x00FF);
        cpu->addrbus_ = ToByte2(high, cpu->databus_);
    } break;
    }
}

void
cpu_frm_izx_r(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0:
    case 1:
    case 2:
    case 3: {
        frm_izx_pre(idx, cpu);
    } break;

    case 4: {
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, cpu_getb_(cpu, cpu->addrbus_), &out);
        (void)(out);

        *done = true;
    } break;
    }
}

void
cpu_frm_izx_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0:
    case 1:
    case 2:
    case 3: {
        frm_izx_pre(idx, cpu);
    } break;

    case 4: {
        cpu->databus_ = cpu_getb_(cpu, cpu->addrbus_);
    } break;

    case 5: {
        cpu_setb_(cpu, cpu->addrbus_, cpu->databus_);
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, cpu->databus_, &out);
        cpu->databus_ = out;
    } break;

    case 6: {
        cpu_setb_(cpu, cpu->addrbus_, cpu->databus_);

        *done = true;
    } break;
    }
}

void
cpu_frm_izx_w(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0:
    case 1:
    case 2:
    case 3: {
        frm_izx_pre(idx, cpu);
    } break;

    case 4: {
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, 0, &out);
        cpu_setb_(cpu, cpu->addrbus_, out);

        *done = true;
    } break;
    }
}

void
frm_izy_pre(int idx, cpu_s *cpu)
{
    switch (idx) {
    default:
    case 0: {
        cpu->addrbus_ = (addr_t)(cpu_getb_(cpu, cpu->pc_++));
    } break;

    case 1: {
        cpu->databus_ = cpu_getb_(cpu, cpu->addrbus_);
    } break;

    case 2: {
        sethigh(&cpu->addrbus_, cpu_getb_(cpu, (cpu->addrbus_ + 1) & 0x00FF));
        addr_t sumaddr = (addr_t)(cpu->databus_) + cpu->y_;
        cpu->pageoffset_ = !!(sumaddr & 0xFF00);
        setlow(&cpu->addrbus_, getlow(sumaddr));
    } break;
    }
}

void
cpu_frm_izy_r(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0:
    case 1:
    case 2: {
        frm_izy_pre(idx, cpu);
    } break;

    case 3: {
        // The read must be done regardlessly.
        u8 in = cpu_getb_(cpu, cpu->addrbus_);
        if (cpu->pageoffset_) {
            cpu->addrbus_ += ((addr_t)(cpu->pageoffset_) << 8);
        } else {
            cpu->iEffAddr_ = cpu->addrbus_;
            u8 out;
            core(cpu, in, &out);
            (void)(out);

            *done = true;
        }
    } break;

    case 4: {
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, cpu_getb_(cpu, cpu->addrbus_), &out);
        (void)(out);

        *done = true;
    } break;
    }
}

void
cpu_frm_izy_rmw(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0:
    case 1:
    case 2: {
        frm_izy_pre(idx, cpu);
    } break;

    case 3: {
        // Read from possibly smaller address first.
        (void)(cpu_getb_(cpu, cpu->addrbus_));
        // if (cpu->pageoffset_)
        {
            cpu->addrbus_ += ((addr_t)(cpu->pageoffset_) << 8);
        }
    } break;

    case 4: {
        cpu->databus_ = cpu_getb_(cpu, cpu->addrbus_);
    } break;

    case 5: {
        cpu_setb_(cpu, cpu->addrbus_, cpu->databus_);
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, cpu->databus_, &out);
        cpu->databus_ = out;
    } break;

    case 6: {
        cpu_setb_(cpu, cpu->addrbus_, cpu->databus_);

        *done = true;
    } break;
    }
}

void
cpu_frm_izy_w(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    switch (idx) {
    default:
    case 0:
    case 1:
    case 2: {
        frm_izy_pre(idx, cpu);
    } break;

    case 3: {
        // Read from possibly smaller address first.
        (void)(cpu_getb_(cpu, cpu->addrbus_));
        // if (cpu->pageoffset_)
        {
            cpu->addrbus_ += ((addr_t)(cpu->pageoffset_) << 8);
        }
    } break;

    case 4: {
        cpu->iEffAddr_ = cpu->addrbus_;
        u8 out;
        core(cpu, 0, &out);
        cpu_setb_(cpu, cpu->addrbus_, out);

        *done = true;
    } break;
    }
}

void
cpu_frm_ind_jmp(int idx, cpu_s *cpu, instrcore_f core, bool *done)
{
    (void)(core);

    switch (idx) {
    default:
    case 0: {
        setlow(&cpu->addrbus_, cpu_getb_(cpu, cpu->pc_++));
    } break;

    case 1: {
        sethigh(&cpu->addrbus_, cpu_getb_(cpu, cpu->pc_++));
    } break;

    case 2: {
        cpu->databus_ = cpu_getb_(cpu, cpu->addrbus_);
    } break;

    case 3: {
        // Set PCL at this cycle, according to the doc
        setlow(&cpu->pc_, cpu->databus_);

        // addr_t bytes are fetched from the same page
        cpu->addrbus_ =
            ToByte2(gethigh(cpu->addrbus_), getlow(cpu->addrbus_ + 1));
        sethigh(&cpu->pc_, cpu_getb_(cpu, cpu->addrbus_));

        *done = true;
    } break;
    }
}

void
cpu_core_nop(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(cpu);
    (void)(in);
    (void)(out);
}

void
cpu_core_ora(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    cpu->a_ |= in;

    testN(cpu, cpu->a_);
    testZ(cpu, cpu->a_);
}

void
cpu_core_kil(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);

    cpu_halt_(cpu);
}

void
cpu_core_asl(cpu_s *cpu, u8 in, u8 *out)
{
    u8 newval = in << 1;
    *out = newval;

    testN(cpu, newval);
    testZ(cpu, newval);
    cpu_testFlag_(cpu, CS_C, CheckBit(7, in));
}

void
cpu_core_bpl(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    *out = !cpu_checkFlag_(cpu, CS_N);
}

void
cpu_core_clc(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);
    cpu_unsetFlag_(cpu, CS_C);
}

void
cpu_core_and(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    core_and_op(cpu, in);

    testN(cpu, cpu->a_);
    testZ(cpu, cpu->a_);
}

void
cpu_core_bit(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    testN(cpu, in);
    cpu_testFlag_(cpu, CS_V, CheckBit(6, in));
    testZ(cpu, cpu->a_ & in);
}

void
cpu_core_rol(cpu_s *cpu, u8 in, u8 *out)
{
    bool carry = cpu_checkFlag_(cpu, CS_C);
    *out = (in << 1) | (carry << 0);

    testN(cpu, *out);
    testZ(cpu, *out);
    cpu_testFlag_(cpu, CS_C, CheckBit(7, in));
}

void
cpu_core_bmi(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    *out = cpu_checkFlag_(cpu, CS_N);
}

void
cpu_core_sec(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);
    cpu_setFlag_(cpu, CS_C);
}

void
cpu_core_eor(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    cpu->a_ ^= in;

    testN(cpu, cpu->a_);
    testZ(cpu, cpu->a_);
}

void
cpu_core_lsr(cpu_s *cpu, u8 in, u8 *out)
{
    *out = in >> 1;

    testN(cpu, *out);
    testZ(cpu, *out);
    cpu_testFlag_(cpu, CS_C, CheckBit(0, in));
}

void
cpu_core_bvc(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    *out = !cpu_checkFlag_(cpu, CS_V);
}

void
cpu_core_cli(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);
    cpu_unsetFlag_(cpu, CS_I);
}

void
cpu_core_adc(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    bool carry = cpu_checkFlag_(cpu, CS_C);

    u8 prevval = cpu->a_;
    cpu->a_ += in + (u8)carry;

    // either new value < previous value, or the are the same even if carry
    // exists.
    bool unsignedOverflow =
        (cpu->a_ < prevval || (cpu->a_ == prevval && carry));
    bool signedOverflow = IsSignedOverflowAdc(prevval, in, carry);

    testN(cpu, cpu->a_);
    cpu_testFlag_(cpu, CS_V, signedOverflow);
    testZ(cpu, cpu->a_);
    cpu_testFlag_(cpu, CS_C, unsignedOverflow);
}

void
cpu_core_ror(cpu_s *cpu, u8 in, u8 *out)
{
    core_ror_op(cpu, in, out);

    testN(cpu, *out);
    testZ(cpu, *out);
    cpu_testFlag_(cpu, CS_C, CheckBit(0, in));
}

void
cpu_core_bvs(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    *out = cpu_checkFlag_(cpu, CS_V);
}

void
cpu_core_sei(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);
    cpu_setFlag_(cpu, CS_I);
}

void
cpu_core_sta(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    *out = cpu->a_;
}

void
cpu_core_sty(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    *out = cpu->y_;
}

void
cpu_core_stx(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    *out = cpu->x_;
}

void
cpu_core_dey(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);

    --cpu->y_;

    testN(cpu, cpu->y_);
    testZ(cpu, cpu->y_);
}

void
cpu_core_txa(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);

    cpu->a_ = cpu->x_;

    testN(cpu, cpu->a_);
    testZ(cpu, cpu->a_);
}

void
cpu_core_bcc(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    *out = !cpu_checkFlag_(cpu, CS_C);
}

void
cpu_core_tya(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);

    cpu->a_ = cpu->y_;

    testN(cpu, cpu->a_);
    testZ(cpu, cpu->a_);
}

void
cpu_core_txs(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);

    cpu->s_ = cpu->x_;
}

void
cpu_core_ldy(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    cpu->y_ = in;

    testN(cpu, cpu->y_);
    testZ(cpu, cpu->y_);
}

void
cpu_core_lda(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    cpu->a_ = in;

    testN(cpu, cpu->a_);
    testZ(cpu, cpu->a_);
}

void
cpu_core_ldx(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    cpu->x_ = in;

    testN(cpu, cpu->x_);
    testZ(cpu, cpu->x_);
}

void
cpu_core_tay(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);

    cpu->y_ = cpu->a_;

    testN(cpu, cpu->y_);
    testZ(cpu, cpu->y_);
}

void
cpu_core_tax(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);

    cpu->x_ = cpu->a_;

    testN(cpu, cpu->x_);
    testZ(cpu, cpu->x_);
}

void
cpu_core_bcs(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    *out = cpu_checkFlag_(cpu, CS_C);
}

void
cpu_core_clv(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);
    cpu_unsetFlag_(cpu, CS_V);
}

void
cpu_core_tsx(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);

    cpu->x_ = cpu->s_;

    testN(cpu, cpu->x_);
    testZ(cpu, cpu->x_);
}

void
cpu_core_cpy(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);
    (void)(core_cmp_impl(cpu, in, cpu->y_));
}

void
cpu_core_cmp(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);
    (void)(core_cmp_impl(cpu, in, cpu->a_));
}

void
cpu_core_dec(cpu_s *cpu, u8 in, u8 *out)
{
    *out = in - 1;

    testN(cpu, *out);
    testZ(cpu, *out);
}

void
cpu_core_iny(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);

    ++cpu->y_;

    testN(cpu, cpu->y_);
    testZ(cpu, cpu->y_);
}

void
cpu_core_dex(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);

    --cpu->x_;

    testN(cpu, cpu->x_);
    testZ(cpu, cpu->x_);
}

void
cpu_core_bne(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    *out = !cpu_checkFlag_(cpu, CS_Z);
}

void
cpu_core_cld(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);
    cpu_unsetFlag_(cpu, CS_D);
}

void
cpu_core_cpx(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);
    (void)(core_cmp_impl(cpu, in, cpu->x_));
}

void
cpu_core_sbc(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    bool borrow = !cpu_checkFlag_(cpu, CS_C);

    u8 prevval = cpu->a_;
    u16 minuend = cpu->a_;
    u16 subtrahend = in + (u16)borrow;
    cpu->a_ = (u8)(minuend - subtrahend);

    bool noborrow = minuend >= subtrahend;
    bool signedOverflow = IsSignedOverflowSbc(prevval, in, borrow);

    testN(cpu, cpu->a_);
    cpu_testFlag_(cpu, CS_V, signedOverflow);
    testZ(cpu, cpu->a_);
    cpu_testFlag_(cpu, CS_C, noborrow);
}

void
cpu_core_inc(cpu_s *cpu, u8 in, u8 *out)
{
    *out = in + 1;

    testN(cpu, *out);
    testZ(cpu, *out);
}

void
cpu_core_inx(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);

    ++cpu->x_;

    testN(cpu, cpu->x_);
    testZ(cpu, cpu->x_);
}

void
cpu_core_beq(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    *out = cpu_checkFlag_(cpu, CS_Z);
}

void
cpu_core_sed(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    (void)(out);
    cpu_setFlag_(cpu, CS_D);
}

void
cpu_core_slo(cpu_s *cpu, u8 in, u8 *out)
{
    cpu_core_asl(cpu, in, out);
    u8 tmp;
    cpu_core_ora(cpu, *out, &tmp);
    (void)(tmp);
}

void
cpu_core_anc(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    // http://www.oxyron.de/html/opcodes02.html
    // https://www.nesdev.com/extra_instructions.txt

    core_and_op(cpu, in);

    testN(cpu, cpu->a_);
    testZ(cpu, cpu->a_);
    cpu_testFlag_(cpu, CS_C, CheckBit(7, cpu->a_));
}

void
cpu_core_rla(cpu_s *cpu, u8 in, u8 *out)
{
    cpu_core_rol(cpu, in, out);
    u8 tmp;
    cpu_core_and(cpu, *out, &tmp);
    (void)(tmp);
}

void
cpu_core_alr(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    // http://www.oxyron.de/html/opcodes02.html

    // AND
    {
        u8 tmp;
        cpu_core_and(cpu, in, &tmp);
        (void)(tmp);
    }
    // LSR A
    cpu_core_lsr(cpu, cpu->a_, &cpu->a_);
}

void
cpu_core_sre(cpu_s *cpu, u8 in, u8 *out)
{
    cpu_core_lsr(cpu, in, out);
    u8 tmp;
    cpu_core_eor(cpu, *out, &tmp);
    (void)(tmp);
}

void
cpu_core_arr(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    // Similar to AND #i then ROR A, except sets the flags differently. N and Z
    // are normal, but C is bit 6 and V is bit 6 xor bit 5. A fast way to
    // perform signed division by 4 is: CMP #$80; ARR #$FF; ROR. This can be
    // extended to larger powers of two.

    // ROR depends on C flag, but AND doesn't affect C flag, so we can
    // reuse AND logic and do it first.
    core_and_op(cpu, in);
    u8 afterAnd = cpu->a_;
    // ROR A
    core_ror_op(cpu, afterAnd, &cpu->a_);

    testN(cpu, cpu->a_);
    cpu_testFlag_(cpu, CS_V, CheckBit(5, cpu->a_) ^ CheckBit(6, cpu->a_));
    testZ(cpu, cpu->a_);
    cpu_testFlag_(cpu, CS_C, CheckBit(7, afterAnd));
}

void
cpu_core_rra(cpu_s *cpu, u8 in, u8 *out)
{
    cpu_core_ror(cpu, in, out);
    u8 tmp;
    cpu_core_adc(cpu, *out, &tmp);
    (void)(tmp);
}

void
cpu_core_xaa(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);
    cpu->a_ = cpu->x_ & in;

    testN(cpu, cpu->a_);
    testZ(cpu, cpu->a_);
}

void
cpu_core_sax(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);
    *out = cpu->a_ & cpu->x_;
}

void
cpu_core_las(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    u8 newval = in & cpu->s_;
    cpu->a_ = cpu->x_ = cpu->s_ = newval;

    testN(cpu, cpu->a_);
    testZ(cpu, cpu->a_);
}

void
cpu_core_lax(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    cpu->a_ = in;
    cpu->x_ = in;

    testN(cpu, cpu->a_);
    testZ(cpu, cpu->a_);
}

void
cpu_core_axs(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(out);

    // http://www.oxyron.de/html/opcodes02.html
    // https://www.nesdev.org/wiki/Programming_with_unofficial_opcodes#Combined_operations

    u8 aAndx = cpu->a_ & cpu->x_;
    u8 delta = core_cmp_impl(cpu, in, aAndx);
    cpu->x_ = delta;
}

void
cpu_core_dcp(cpu_s *cpu, u8 in, u8 *out)
{
    cpu_core_dec(cpu, in, out);
    u8 tmp;
    cpu_core_cmp(cpu, *out, &tmp);
    (void)(tmp);
}

void
cpu_core_isc(cpu_s *cpu, u8 in, u8 *out)
{
    cpu_core_inc(cpu, in, out);
    u8 tmp;
    cpu_core_sbc(cpu, *out, &tmp);
    (void)(tmp);
}

void
cpu_core_tas(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);

    // http://www.oxyron.de/html/opcodes02.html
    // https://www.nesdev.com/extra_instructions.txt
    // @TEST: Test this instruction

    cpu->s_ = cpu->a_ & cpu->x_;
    core_XhX_impl(cpu, cpu->s_, out);
}

void
cpu_core_shy(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);

    core_XhX_impl(cpu, cpu->y_, out);
}

void
cpu_core_shx(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);

    core_XhX_impl(cpu, cpu->x_, out);
}

void
cpu_core_ahx(cpu_s *cpu, u8 in, u8 *out)
{
    (void)(in);

    core_XhX_impl(cpu, cpu->a_ & cpu->x_, out);
}

void
core_XhX_impl(cpu_s *cpu, u8 in, u8 *out)
{
    // http://www.oxyron.de/html/opcodes02.html
    // https://www.nesdev.com/extra_instructions.txt
    // https://github.com/ltriant/nes/blob/master/doc/undocumented_opcodes.txt
    // @TEST: Test this instruction

    // Assuming effective address is set already
    *out = in & (gethigh(cpu->iEffAddr_) + 1);
}

u8
core_cmp_impl(cpu_s *cpu, u8 in, u8 reg)
{
    u8 delta = reg - in;
    bool borrow = reg < in;

    testN(cpu, delta);
    testZ(cpu, delta);
    cpu_testFlag_(cpu, CS_C, !borrow);

    return delta;
}

void
core_and_op(cpu_s *cpu, u8 in)
{
    cpu->a_ &= in;
}

void
core_ror_op(cpu_s *cpu, u8 in, u8 *out)
{
    bool carry = cpu_checkFlag_(cpu, CS_C);
    *out = (in >> 1) | (carry << 7);
}

void
throwaway(cpu_s *cpu, u8 data)
{
    // What does it mean to throw it away?
    // Inferring from the wording in the doc, putting it in the bus does not
    // seem right.
    // cpu->databus_ = data;

    (void)(cpu);
    (void)(data);
}

void
ignoreub(u8 src, u8 *dst)
{
    u8 preserveMask = (CS_U | CS_B);
    *dst = (src & ~preserveMask) | (*dst & preserveMask);
}

void
setlow(addr_t *dst, u8 val)
{
    *dst = (*dst & 0xFF00) | (addr_t)(val);
}

void
sethigh(addr_t *dst, u8 val)
{
    *dst = (*dst & 0x00FF) | ((addr_t)(val) << 8);
}

u8
getlow(addr_t val)
{
    return (u8)(val);
}

u8
gethigh(addr_t val)
{
    return (u8)(val >> 8);
}

void
testN(cpu_s *cpu, u8 val)
{
    cpu_testFlag_(cpu, CS_N, CheckBit(7, val));
}

void
testZ(cpu_s *cpu, u8 val)
{
    cpu_testFlag_(cpu, CS_Z, val == 0);
}
