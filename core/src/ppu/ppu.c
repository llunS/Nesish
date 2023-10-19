#include "ppu.h"

#include "memory/vmem.h"
#include "nhassert.h"
#include "ppu/palettedef.h"

#include <string.h>

static bool
regRO(ppureg_e reg);
static bool
regWO(ppureg_e reg);

static void
incVramAddr(ppu_s *self);

static void
resetTrivial(ppu_s *self);

bool
ppu_Init(ppu_s *self, vmem_s *vmem, const NHDFlag *flags, NHLogger *logger)
{
    memset(self->regs_, 0, sizeof(self->regs_));

    memset(self->oam_, 0, sizeof(self->oam_));

    self->vmem_ = vmem;

    placcessor_init_(&self->plAccessor_, self);

    pl_Init(&self->pl_, &self->plAccessor_);

    if (!frmbuf_Init(&self->backbuf_))
    {
        goto outerr;
    }
    if (!frmbuf_Init(&self->frontbuf_))
    {
        goto outerr;
    }

    self->pal_.ToRgb = palettedef_ToRgb;

    self->iodb_ = 0;

    self->debugflags_ = flags;
    dbgpal_Init(&self->palsnap_);
    dbgoam_Init(&self->oamsnap_);
    dbgpattble_Init(&self->ptntblsnapL_);
    dbgpattble_Init(&self->ptntblsnapR_);
    self->ptntblPalIdx_ = 0;

    self->logger_ = logger;

    return true;
outerr:
    ppu_Deinit(self);
    return false;
}

void
ppu_Deinit(ppu_s *self)
{
    frmbuf_Deinit(&self->frontbuf_);
    frmbuf_Deinit(&self->backbuf_);
}

void
ppu_Powerup(ppu_s *self)
{
    // https://www.nesdev.org/wiki/PPU_power_up_state
    // Excluding side effects via ppu_regOf(), not sure if it's right.
    *ppu_regOf(self, PR_PPUCTRL) = 0x00;
    *ppu_regOf(self, PR_PPUMASK) = 0x00;
    *ppu_regOf(self, PR_PPUSTATUS) = 0xA0; // +0+x xxxx
    *ppu_regOf(self, PR_OAMADDR) = 0x00;
    self->w_ = 0; // Latch is cleared as well
    *ppu_regOf(self, PR_PPUSCROLL) = 0x00;
    *ppu_regOf(self, PR_PPUADDR) = 0x00;
    self->ppudatabuf_ = 0x00;
    self->plctx_.OddFrame = false;

    // ---- Palette
    static const u8 colorIndices[NH_PALETTE_SIZE] = {
        /* clang-format off */
            0x09, 0x01, 0x00, 0x01, 0x00, 0x02, 0x02, 0x0D,
            0x08, 0x10, 0x08, 0x24, 0x00, 0x00, 0x04, 0x2C,
            0x09, 0x01, 0x34, 0x03, 0x00, 0x04, 0x00, 0x14,
            0x08, 0x3A, 0x00, 0x02, 0x00, 0x20, 0x2C, 0x08
        /* clang-format on */
    };
    // static_assert(colorIndices[NH_PALETTE_SIZE - 1], "Elements missing");
    for (int i = 0; i < NH_PALETTE_SIZE; ++i)
    {
        (void)membase_SetB(&self->vmem_->Base,
                           (addr_t)(NH_PALETTE_ADDR_HEAD + i), colorIndices[i]);
    }

    /* Unspecified:
     * OAM
     * Nametable RAM
     * CHR RAM
     */

    resetTrivial(self);
}

void
ppu_Reset(ppu_s *self)
{
    *ppu_regOf(self, PR_PPUCTRL) = 0x00;
    *ppu_regOf(self, PR_PPUMASK) = 0x00;
    self->w_ = 0; // Latch is cleared as well
    *ppu_regOf(self, PR_PPUSCROLL) = 0x00;
    self->ppudatabuf_ = 0x00;
    self->plctx_.OddFrame = false;

    /* Unchanged:
     * OAM
     */
    /* Unchanged:
     * Palette
     * Nametable RAM
     * CHR RAM
     */

    resetTrivial(self);
}

void
resetTrivial(ppu_s *self)
{
    self->v_ = self->t_ = self->x_ = 0;

    pl_Reset(&self->pl_);

    self->iodb_ = 0;
}

void
ppu_Tick(ppu_s *self, bool noNmi)
{
    // @TODO: Warm up stage?

    self->noNmi_ = noNmi;

    pl_Tick(&self->pl_);

    self->noNmi_ = false;
}

bool
ppu_Nmi(const ppu_s *self)
{
    return (ppu_getReg(self, PR_PPUCTRL) & 0x80) &&
           (ppu_getReg(self, PR_PPUSTATUS) & 0x80);
}

u8
ppu_ReadReg(ppu_s *self, ppureg_e reg)
{
    if (regWO(reg))
    {
        // Open bus
        return self->iodb_;
    }

    u8 val = self->regs_[reg];

    switch (reg)
    {
        case PR_PPUSTATUS:
        {
            // Other 5 bits open bus behavior.
            val = (val & 0xE0) | (self->iodb_ & ~0xE0);

            self->regs_[reg] &= 0x7F; // clear VBLANK flag
            self->w_ = 0;
        }
        break;

        case PR_OAMDATA:
        {
            val = self->oam_[self->regs_[PR_OAMADDR]];
        }
        break;

        case PR_PPUDATA:
        {
            addr_t vaddr = self->v_ & NH_PPU_ADDR_MASK;
            u8 tmpval;
            NHErr err = vmem_GetB(self->vmem_, vaddr, &tmpval);
            if (NH_FAILED(err))
            {
                ASSERT_FATAL(self->logger_, "Failed to read PPUDATA: " ADDRFMT,
                             vaddr);
                tmpval = 0xFF;
            }
            if (0 <= vaddr && vaddr <= NH_NT_MIRROR_ADDR_TAIL)
            {
                val = self->ppudatabuf_;
                self->ppudatabuf_ = tmpval;
            }
            else
            {
                u8 tmpNtVal;
                addr_t ntaddr = vaddr & NH_NT_MIRROR_ADDR_MASK;
                err = vmem_GetB(self->vmem_, ntaddr, &tmpNtVal);
                if (NH_FAILED(err))
                {
                    ASSERT_FATAL(
                        self->logger_,
                        "Failed to read PPUDATA nametable data: " ADDRFMT,
                        ntaddr);
                    tmpNtVal = 0xFF;
                }
                val = tmpval;
                self->ppudatabuf_ = tmpNtVal;
            }

            incVramAddr(self);
        }
        break;

        default:
            break;
    }

    // fill the latch
    self->iodb_ = val;
    return val;
}

void
ppu_WriteReg(ppu_s *self, ppureg_e reg, u8 val)
{
    // fill the latch
    self->iodb_ = val;

    if (regRO(reg))
    {
        return;
    }

    self->regs_[reg] = val;

    switch (reg)
    {
        case PR_PPUCTRL:
        {
            self->t_ = (self->t_ & ~0x0C00) | ((u16)(val & 0x03) << 10);
        }
        break;

        case PR_OAMDATA:
        {
            // @QUIRK: "The three unimplemented bits of each sprite's byte 2 do
            // not exist in the PPU and always read back as 0 on PPU revisions
            // that allow reading PPU OAM through OAMDATA ($2004). This can be
            // emulated by ANDing byte 2 with $E3 either when writing to or when
            // reading from OAM"
            // https://www.nesdev.org/wiki/PPU_OAM#Byte_2
            u8 oamaddr = self->regs_[PR_OAMADDR];
            if ((oamaddr & 0x03) == 0x02)
            {
                val &= 0xE3;
            }
            self->oam_[oamaddr] = val;
            ++self->regs_[PR_OAMADDR];

            // @TODO: Writes during rendering
            // https://www.nesdev.org/wiki/PPU_registers#OAM_data_($2004)_%3C%3E_read/write
            // Writes to OAMDATA during rendering (on the pre-render line and
            // the visible lines 0-239, provided either sprite or background
            // rendering is enabled) do not modify values in OAM, but do perform
            // a glitchy increment of OAMADDR, bumping only the high 6 bits
            // (i.e., it bumps the [n] value in PPU sprite evaluation - it's
            // plausible that it could bump the low bits instead depending on
            // the current status of sprite evaluation). This extends to DMA
            // transfers via OAMDMA, since that uses writes to $2004. For
            // emulation purposes, it is probably best to completely ignore
            // writes during rendering.
        }
        break;

        case PR_PPUSCROLL:
        {
            if (!self->w_)
            {
                self->t_ = (self->t_ & ~0x001F) | (val >> 3);
                self->x_ = val & 0x07;
            }
            else
            {
                self->t_ = (self->t_ & ~0x73E0) | ((u16)(val & 0x07) << 12) |
                           ((u16)(val & 0xF8) << 2);
            }
            self->w_ = !self->w_;
        }
        break;

        case PR_PPUADDR:
        {
            if (!self->w_)
            {
                self->t_ = (self->t_ & ~0xFF00) | ((u16)(val & 0x3F) << 8);
            }
            else
            {
                self->t_ = (self->t_ & ~0x00FF) | (u16)(val);
                self->v_ = self->t_;
            }
            self->w_ = !self->w_;
        }
        break;

        case PR_PPUMASK:
        {
            // @TODO: Color emphasis
            // https://www.nesdev.org/wiki/Colour_emphasis
        }
        break;

        case PR_PPUDATA:
        {
            addr_t vaddr = self->v_ & NH_PPU_ADDR_MASK;
            NHErr err = membase_SetB(&self->vmem_->Base, vaddr, val);
            if (NH_FAILED(err))
            {
                ASSERT_FATAL(self->logger_,
                             "Failed to write PPUDATA: " ADDRFMT ", " U8FMTX,
                             vaddr, val);
            }

            incVramAddr(self);
        }
        break;

        default:
            break;
    }
}

bool
regRO(ppureg_e reg)
{
    switch (reg)
    {
        case PR_PPUSTATUS:
            return true;
            break;

        default:
            break;
    }
    return false;
}

bool
regWO(ppureg_e reg)
{
    switch (reg)
    {
        case PR_PPUCTRL:
        case PR_PPUMASK:
        case PR_OAMADDR:
        case PR_PPUSCROLL:
        case PR_PPUADDR:
            return true;
            break;

        default:
            break;
    }
    return false;
}

const frmbuf_s *
ppu_getFrame(const ppu_s *self)
{
    return &self->frontbuf_;
}

const dbgpal_s *
ppu_dbgGetPal(const ppu_s *self)
{
    return &self->palsnap_;
}

const dbgoam_s *
ppu_dbgGetOam(const ppu_s *self)
{
    return &self->oamsnap_;
}

const dbgpattbl_s *
ppu_dbgGetPtnTbl(const ppu_s *self, bool right)
{
    return right ? &self->ptntblsnapR_ : &self->ptntblsnapL_;
}

void
ppu_dbgSetPtnTblPal(ppu_s *self, unsigned char idx)
{
    self->ptntblPalIdx_ = idx;
}

u8 *
ppu_regOf(ppu_s *self, ppureg_e reg)
{
    return &self->regs_[reg];
}

u8
ppu_getReg(const ppu_s *self, ppureg_e reg)
{
    return self->regs_[reg];
}

void
incVramAddr(ppu_s *self)
{
    u8 offset = (ppu_getReg(self, PR_PPUCTRL) & 0x04) ? 32 : 1;
    self->v_ += offset;

    // @TODO: Quirk of 2007 read/write
    // Outside of rendering, reads from or writes to $2007 will add either 1 or
    // 32 to v depending on the VRAM increment bit set via $2000. During
    // rendering (on the pre-render line and the visible lines 0-239, provided
    // either background or sprite rendering is enabled), it will update v in an
    // odd way, triggering a coarse X increment and a Y increment simultaneously
    // (with normal wrapping behavior). Internally, this is caused by the carry
    // inputs to various sections of v being set up for rendering, and the $2007
    // access triggering a "load next value" signal for all of v (when not
    // rendering, the carry inputs are set up to linearly increment v by either
    // 1 or 32). This behavior is not affected by the status of the increment
    // bit. The Young Indiana Jones Chronicles uses this for some effects to
    // adjust the Y scroll during rendering, and also Burai Fighter (U) to draw
    // the scorebar. If the $2007 access happens to coincide with a standard
    // VRAM address increment (either horizontal or vertical), it will
    // presumably not double-increment the relevant counter.
}
