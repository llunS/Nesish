#include "console.h"

#include "cartridge/cartloader.h"
#include "spec.h"
#include "log.h"
#include "nhassert.h"

#include <string.h>
#include <stdlib.h>

static void
hardwire(console_s *self);

static u8
readCtrlReg(console_s *self, ctrlreg_e reg);
static void
writeCtrlReg(console_s *self, ctrlreg_e reg, u8 val);

static void
resetTrivial(console_s *self);

static void
releaseCart(console_s *self);

bool
console_Init(console_s *self, NHLogger *logger)
{
    /* before goto */
    self->cart_.Impl = NULL;

    cpu_Init(&self->cpu_, &self->mmem_, logger);
    if (!mmem_Init(&self->mmem_, logger))
    {
        goto outerr;
    }
    if (!ppu_Init(&self->ppu_, &self->vmem_, &self->debugflags_, logger))
    {
        goto outerr;
    }
    oamdma_Init(&self->oamdma_, &self->apuclock_, &self->mmem_, &self->ppu_);
    if (!vmem_Init(&self->vmem_, logger))
    {
        goto outerr;
    }
    apuclock_Init(&self->apuclock_);
    apu_Init(&self->apu_, &self->apuclock_, &self->dmcdma_, logger);
    dmcdma_Init(&self->dmcdma_, &self->apuclock_, &self->mmem_, &self->apu_);

    memset(self->ctrlregs_, 0, sizeof(self->ctrlregs_));
    memset(self->ctrls_, 0, sizeof(self->ctrls_));

    self->logger_ = logger;

    self->debugflags_ = NHD_DBG_OFF;

    self->time_ = 0;

    hardwire(self);

    return true;
outerr:
    console_Deinit(self);
    return false;
}

void
console_Deinit(console_s *self)
{
    releaseCart(self);

    vmem_Deinit(&self->vmem_);
    ppu_Deinit(&self->ppu_);
    mmem_Deinit(&self->mmem_);
}

static NHErr
ppuRegGet(const mementry_s *entry, addr_t addr, u8 *val)
{
    ppu_s *ppu = (ppu_s *)entry->Opaque;

    addr_t addrval = (addr & NH_PPU_REG_ADDR_MASK) | NH_PPU_REG_ADDR_HEAD;
    ppureg_e reg = (ppureg_e)(addrval - entry->Begin + PR_PPUCTRL);
    *val = ppu_ReadReg(ppu, reg);
    return NH_ERR_OK;
}

static NHErr
ppuRegSet(const mementry_s *entry, addr_t addr, u8 val)
{
    ppu_s *ppu = (ppu_s *)entry->Opaque;

    addr_t addrval = (addr & NH_PPU_REG_ADDR_MASK) | NH_PPU_REG_ADDR_HEAD;
    ppureg_e reg = (ppureg_e)(addrval - entry->Begin + PR_PPUCTRL);
    ppu_WriteReg(ppu, reg, val);
    return NH_ERR_OK;
}

static NHErr
aocRegGet(const mementry_s *entry, addr_t addr, u8 *val)
{
    console_s *self = (console_s *)entry->Opaque;

    // OAM DMA
    if (NH_OAMDMA_ADDR == addr)
    {
        return NH_ERR_WRITE_ONLY;
    }
    // Controller registers.
    else if (NH_CTRL1_REG_ADDR == addr || NH_CTRL2_REG_ADDR == addr)
    {
        u8 b = readCtrlReg(self, NH_CTRL1_REG_ADDR == addr ? CR_4016 : CR_4017);
        // Partial open bus
        // https://www.nesdev.org/wiki/Open_bus_behavior#CPU_open_bus
        *val = (b & 0x1F) | (mmem_GetLatch(&self->mmem_) & ~0x1F);
        return NH_ERR_OK;
    }
    // Left with APU registers
    else
    {
        apureg_e reg = apu_Addr2Reg(addr);
        if (AR_SIZE == reg)
        {
            *val = 0xFF;
            return NH_ERR_PROGRAMMING;
        }
        // APU Status doesn't require partial open bus as it
        // seems, according to
        // cpu_exec_space/test_cpu_exec_space_apu.nes
        u8 b;
        NHErr err = apu_ReadReg(&self->apu_, reg, &b);
        if (NH_FAILED(err))
        {
            return err;
        }
        *val = b;
        return NH_ERR_OK;
    }
}

static NHErr
aocRegSet(const mementry_s *entry, addr_t addr, u8 val)
{
    console_s *self = (console_s *)entry->Opaque;

    if (NH_OAMDMA_ADDR == addr)
    {
        // OAM DMA high address (this port is located on the CPU)
        oamdma_Initiate(&self->oamdma_, val);
        return NH_ERR_OK;
    }
    else if (NH_CTRL1_REG_ADDR == addr)
    {
        writeCtrlReg(self, CR_4016, val);
        return NH_ERR_OK;
    }
    else
    {
        apureg_e reg = apu_Addr2Reg(addr);
        if (AR_SIZE == reg)
        {
            return NH_ERR_PROGRAMMING;
        }
        apu_WriteReg(&self->apu_, reg, val);
        return NH_ERR_OK;
    }
}

void
hardwire(console_s *self)
{
    /* PPU registers */
    {
        mementry_s entry;
        mementry_InitExt(&entry, NH_PPU_REG_ADDR_HEAD, NH_PPU_REG_ADDR_TAIL,
                         false, ppuRegGet, ppuRegSet, &self->ppu_);
        membase_SetMapping(&self->mmem_.Base, MMP_PPU, entry);
    }
    /* APU registers, OAM DMA register, Controller register */
    {
        mementry_s entry;
        mementry_InitExt(&entry, NH_APU_REG_ADDR_HEAD, NH_APU_REG_ADDR_TAIL,
                         false, aocRegGet, aocRegSet, self);
        membase_SetMapping(&self->mmem_.Base, MMP_AOC, entry);
    }
}

u8
readCtrlReg(console_s *self, ctrlreg_e reg)
{
    u8 val = self->ctrlregs_[reg];

    switch (reg)
    {
        case CR_4016:
        case CR_4017:
        {
            int index = reg - CR_4016;
            _Static_assert(CONSOLE_CTRLSIZE == CR_SIZE,
                           "Controller index mismatch");
            if (index < 0 || index >= CONSOLE_CTRLSIZE)
            {
                ASSERT_FATAL(self->logger_,
                             "Implementation error for controller register: %d",
                             index);
            }
            else
            {
                NHController *ctrl = self->ctrls_[index];
                // @TODO: Other control bits
                bool primaryBit = 0;
                if (!ctrl)
                {
                    // Report 0 for unconnected controller.
                    // https://www.nesdev.org/wiki/Standard_controller#Output_($4016/$4017_read)
                    primaryBit = 0;
                }
                else
                {
                    primaryBit = (bool)ctrl->report(ctrl->user);
                }
                // Other bits are 0 as initialized.
                val = (val & 0xFE) | (u8)primaryBit;
            }
        }
        break;

        default:
            break;
    }

    return val;
}

void
writeCtrlReg(console_s *self, ctrlreg_e reg, u8 val)
{
    self->ctrlregs_[reg] = val;

    switch (reg)
    {
        case CR_4016:
        {
            bool strobeOn = val & 0x01;
            for (NHCtrlPort i = 0; i < CONSOLE_CTRLSIZE; ++i)
            {
                NHController *ctrl = self->ctrls_[i];
                if (!ctrl)
                {
                    continue;
                }

                ctrl->strobe(strobeOn, ctrl->user);
            }
        }
        break;

        default:
            break;
    }
}

void
console_PlugCtrl(console_s *self, NHCtrlPort slot, NHController *ctrl)
{
    self->ctrls_[slot] = ctrl;
}

void
console_UnplugCtrl(console_s *self, NHCtrlPort slot)
{
    self->ctrls_[slot] = NULL;
}

NHErr
console_InsertCart(console_s *self, const char *rompath)
{
    NHErr err = NH_ERR_OK;

    // 1. load cartridge
    LOG_INFO(self->logger_, "Loading cartridge...");
    cart_s cart;
    cart.Impl = NULL;
    err = cartld_LoadCart(rompath, CK_INES, &cart, self->logger_);
    if (NH_FAILED(err))
    {
        goto outend;
    }
    err = cart.Validate(cart.Impl);
    if (NH_FAILED(err))
    {
        goto outend;
    }

    // 2. map to address space
    LOG_INFO(self->logger_, "Mapping cartridge...");
    cart.MapMemory(cart.Impl, &self->mmem_, &self->vmem_);

outend:
    if (NH_FAILED(err))
    {
        if (cart.Impl)
        {
            cart.UnmapMemory(cart.Impl, &self->mmem_, &self->vmem_);

            cart.Deinit(cart.Impl);
            free(cart.Impl);
            cart.Impl = NULL;
        }
    }
    else
    {
        releaseCart(self);
        self->cart_ = cart;
    }

    return err;
}

void
console_RemoveCart(console_s *self)
{
    releaseCart(self);
}

void
releaseCart(console_s *self)
{
    if (self->cart_.Impl)
    {
        self->cart_.UnmapMemory(self->cart_.Impl, &self->mmem_, &self->vmem_);

        self->cart_.Deinit(self->cart_.Impl);
        free(self->cart_.Impl);
        self->cart_.Impl = NULL;
    }
}

void
console_Powerup(console_s *self)
{
    if (!self->cart_.Impl)
    {
        LOG_ERROR(self->logger_, "Power up without cartridge inserted");
        return;
    }

    // @NOTE: Setup memory first, since other components depend on their
    // states.
    // Setup internal RAM first in case mapper changes its content
    // afterwards (if there is any). Set to a consistent RAM startup state.
    mmem_SetBulk(&self->mmem_, NH_INTERNAL_RAM_ADDR_HEAD,
                 NH_INTERNAL_RAM_ADDR_TAIL, 0xFF);
    self->cart_.Powerup(self->cart_.Impl);

    cpu_Powerup(&self->cpu_);
    ppu_Powerup(&self->ppu_);
    apu_Powerup(&self->apu_);
    oamdma_Powerup(&self->oamdma_);
    dmcdma_Powerup(&self->dmcdma_);
    apuclock_Powerup(&self->apuclock_);

    self->time_ = 0;

    resetTrivial(self);
}

void
console_Reset(console_s *self)
{
    if (!self->cart_.Impl)
    {
        LOG_ERROR(self->logger_, "Reset without cartridge inserted");
        return;
    }

    // The internal memory was unchanged,
    // i.e. "self->mmem_" is not changed
    self->cart_.Reset(self->cart_.Impl);

    cpu_Reset(&self->cpu_);
    ppu_Reset(&self->ppu_);
    apu_Reset(&self->apu_);
    oamdma_Reset(&self->oamdma_);
    dmcdma_Reset(&self->dmcdma_);
    apuclock_Reset(&self->apuclock_);

    resetTrivial(self);
}

void
resetTrivial(console_s *self)
{
    for (ctrlreg_e i = 0; i < CR_SIZE; ++i)
    {
        self->ctrlregs_[i] = 0;
    }
    for (NHCtrlPort i = 0; i < CONSOLE_CTRLSIZE; ++i)
    {
        if (self->ctrls_[i])
        {
            self->ctrls_[i]->reset(self->ctrls_[i]->user);
        }
    }
}

cycle_t
console_Advance(console_s *self, double delta)
{
    double next = self->time_ + delta;
    cycle_t cpuTicks =
        (cycle_t)(next * NH_CPU_HZ) - (cycle_t)(self->time_ * NH_CPU_HZ);
    self->time_ = next;
    return cpuTicks;
}

bool
console_Tick(console_s *self, bool *cpuInstr)
{
    // @NOTE: Tick DMA before CPU, since CPU may be halted by them
    // @NOTE: the RDY disable implementation depends on this order.
    bool dmaHalt = cpu_DmaHalt(&self->cpu_);
    bool dmcDmaGet = dmcdma_Tick(&self->dmcdma_, dmaHalt);
    bool oamDmaOp = oamdma_Tick(&self->oamdma_, dmaHalt, dmcDmaGet);

    // NTSC version ticks PPU 3 times per CPU tick

    // @NOTE: The tick order between CPU and PPU has to do with VBL timing.
    // Order: P->C(pre)->P->C(post)->P, plus one special case of Reading $2002
    // one PPU clock before VBL is set.
    // Test rom: vbl_nmi_timing/2.vbl_timing.nes, etc.
    // https://www.nesdev.org/wiki/PPU_frame_timing#VBL_Flag_Timing
    /* Reading $2002 within a few PPU clocks of when VBL is set results in
     * special-case behavior. Reading one PPU clock before reads it as clear and
     * never sets the flag or generates NMI for that frame. Reading on the same
     * PPU clock or one later reads it as set, clears it, and suppresses the NMI
     * for that frame. Reading two or more PPU clocks before/after it's set
     * behaves normally (reads flag's value, clears it, and doesn't affect NMI
     * operation). This suppression behavior is due to the $2002 read pulling
     * the NMI line back up too quickly after it drops (NMI is active low) for
     * the CPU to see it. (CPU inputs like NMI are sampled each clock.) */
    ppu_Tick(&self->ppu_, false);

    bool read2002 = false;
    bool instrDone = cpu_PreTick(
        &self->cpu_, dmcdma_Rdy(&self->dmcdma_) || oamdma_Rdy(&self->oamdma_),
        dmcDmaGet || oamDmaOp, &read2002);
    if (cpuInstr)
    {
        *cpuInstr = instrDone;
    }

    ppu_Tick(&self->ppu_, read2002);
    cpu_PostTick(&self->cpu_, ppu_Nmi(&self->ppu_), apu_Irq(&self->apu_));
    ppu_Tick(&self->ppu_, false);

    // @NOTE: Tick after CPU pre_tick(), frame counter reset relys on this.
    // blargg_apu_2005.07.30/04.clock_jitter.nes
    // @NOTE: Tick after CPU pre_tick(), length counter halt delay relys on
    // this. blargg_apu_2005.07.30/10.len_halt_timing.nes
    // @NOTE: Tick after CPU pre_tick(), length counter reload during ticking
    // relys on this. blargg_apu_2005.07.30/11.len_reload_timing.nes
    // @NOTE: Tick after CPU post_tick(), according to
    // blargg_apu_2005.07.30/08.irq_timing.nes
    apu_Tick(&self->apu_);

    // Tick the clock last
    apuclock_Tick(&self->apuclock_);

    // APU generates a sample every CPU cycle.
    return true;
}

const frmbuf_s *
console_GetFrm(const console_s *self)
{
    return ppu_getFrame(&self->ppu_);
}

int
console_GetSampleRate(const console_s *self)
{
    (void)(self);
    // APU generates a sample every CPU cycle.
    return NH_CPU_HZ;
}

double
console_GetSample(const console_s *self)
{
    return apu_Amp(&self->apu_);
}

void
console_SetDebugOn(console_s *self, NHDFlag flag)
{
    DebugOn(&self->debugflags_, flag);
}

void
console_SetDebugOff(console_s *self, NHDFlag flag)
{
    DebugOff(&self->debugflags_, flag);
}

const dbgpal_s *
console_DbgGetPal(const console_s *self)
{
    return ppu_dbgGetPal(&self->ppu_);
}

const dbgoam_s *
console_DbgGetOam(const console_s *self)
{
    return ppu_dbgGetOam(&self->ppu_);
}

const dbgpattbl_s *
console_DbgGetPtnTbl(const console_s *self, bool right)
{
    return ppu_dbgGetPtnTbl(&self->ppu_, right);
}

void
console_DbgSetPtnTblPal(console_s *self, NHDPaletteSet palset)
{
    ppu_dbgSetPtnTblPal(&self->ppu_, (unsigned char)palset);
}

cpu_s *
console_TestGetCpu(console_s *self)
{
    return &self->cpu_;
}
