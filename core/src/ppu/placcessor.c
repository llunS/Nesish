#include "placcessor.h"

#include "ppu/ppu.h"
#include "debug/dbgspr.h"
#include "spec.h"
#include "byteutils.h"
#include "memory/vmem.h"

/* debug */

static bool
capturePalOn(placcessor_s *self);
static void
capturePalette(placcessor_s *self);

static bool
captureOamOn(placcessor_s *self);
static void
captureOam(placcessor_s *self);
static void
updateOamSpr(placcessor_s *self, dbgspr_s *spr, int idx);

static bool
capturePtnTblsOn(placcessor_s *self);
static void
capturePtnTbls(placcessor_s *self);

void
placcessor_init_(placcessor_s *self, ppu_s *ppu)
{
    self->ppu_ = ppu;
}

plctx_s *
placcessor_GetCtx(placcessor_s *self)
{
    return &self->ppu_->plctx_;
}

NHLogger *
placcessor_GetLogger(const placcessor_s *self)
{
    return self->ppu_->logger_;
}

u8 *
placcessor_RegOf(placcessor_s *self, ppureg_e reg)
{
    return ppu_regOf(self->ppu_, reg);
}

u8
placcessor_GetReg(const placcessor_s *self, ppureg_e reg)
{
    return ppu_getReg(self->ppu_, reg);
}

u16 *
placcessor_GetV(placcessor_s *self)
{
    return &self->ppu_->v_;
}

u16 *
placcessor_GetT(placcessor_s *self)
{
    return &self->ppu_->t_;
}

u8 *
placcessor_GetX(placcessor_s *self)
{
    return &self->ppu_->x_;
}

u8
placcessor_GetOamByte(placcessor_s *self, u8 addr)
{
    return *placcessor_GetOamPtr(self, addr);
}

u8 *
placcessor_GetOamPtr(placcessor_s *self, u8 addr)
{
    // static_assert(sizeof(self->ppu_->oam_) == 256, "Wrong primary OAM size");
    return &self->ppu_->oam_[addr];
}

vmem_s *
placcessor_GetVmem(placcessor_s *self)
{
    return self->ppu_->vmem_;
}

u8
placcessor_GetColorByte(placcessor_s *self, int idx)
{
    u8 byte = vmem_GetPalByte(placcessor_GetVmem(self), idx);
    // grayscale
    if (placcessor_GetReg(self, PR_PPUMASK) & 0x01)
    {
        byte &= 0x30;
    }
    return byte;
}

const palette_s *
placcessor_GetPal(placcessor_s *self)
{
    return &self->ppu_->pal_;
}

frmbuf_s *
placcessor_GetFrmbuf(placcessor_s *self)
{
    return &self->ppu_->backbuf_;
}

bool
placcessor_BgEnabled(const placcessor_s *self)
{
    return placcessor_GetReg(self, PR_PPUMASK) & 0x08;
}

bool
placcessor_SpEnabled(const placcessor_s *self)
{
    return placcessor_GetReg(self, PR_PPUMASK) & 0x10;
}

bool
placcessor_RenderingEnabled(const placcessor_s *self)
{
    return placcessor_BgEnabled(self) || placcessor_SpEnabled(self);
}

bool
placcessor_Is8x16(const placcessor_s *self)
{
    return placcessor_GetReg(self, PR_PPUCTRL) & 0x20;
}

bool
placcessor_NoNmi(const placcessor_s *self)
{
    return self->ppu_->noNmi_;
}

void
placcessor_FinishFrame(placcessor_s *self)
{
    // Do these the same time as we swap the buffer to synchronize with it.
    if (capturePalOn(self))
    {
        capturePalette(self);
    }
    if (captureOamOn(self))
    {
        captureOam(self);
    }
    if (capturePtnTblsOn(self))
    {
        capturePtnTbls(self);
    }

    frmbuf_Swap(&self->ppu_->frontbuf_, &self->ppu_->backbuf_);
}

addr_t
placcessor_GetSliverAddr(bool tblright, u8 tileidx, bool upper, u8 finey)
{
    addr_t sliveraddr = ((addr_t)tblright << 12) | ((addr_t)tileidx << 4) |
                        ((addr_t)upper << 3) | (addr_t)finey;
    return sliveraddr;
}

NHErr
placcessor_GetPtnSliver(bool tblright, u8 tileidx, bool upper, u8 finey,
                        const vmem_s *vmem, u8 *val)
{
    addr_t sliveraddr =
        placcessor_GetSliverAddr(tblright, tileidx, upper, finey);
    NHErr err = vmem_GetB(vmem, sliveraddr, val);
    return err;
}

void
placcessor_ResolveSpPtnTbl(u8 tilebyte, bool is8x16, bool ptnTblBit,
                           bool *highPtnTbl)
{
    *highPtnTbl = is8x16 ? (tilebyte & 0x01) : ptnTblBit;
}

void
placcessor_ResolveSpTile(u8 tilebyte, bool is8x16, bool flipy, u8 fineySp,
                         u8 *tileidx)
{
    if (is8x16)
    {
        u8 topTile = ((tilebyte >> 1) & 0x7F) << 1;
        bool topHalf = 0 <= fineySp && fineySp < 8;
        *tileidx = (topHalf ^ !flipy) ? topTile + 1 : topTile;
    }
    else
    {
        *tileidx = tilebyte;
    }
}

bool
capturePalOn(placcessor_s *self)
{
    return IsDebugOn(*self->ppu_->debugflags_, NHD_DBG_PALETTE);
}

void
capturePalette(placcessor_s *self)
{
    for (int i = 0; i < NHD_PALETTE_COLORS; ++i)
    {
        // static_assert(NHD_PALETTE_COLORS == 32, "Might overflow below");
        u8 clrbyte = placcessor_GetColorByte(self, i);
        color_s clr = placcessor_GetPal(self)->ToRgb((palettecolor_s){clrbyte});
        dbgpal_SetColor(&self->ppu_->palsnap_, i, clrbyte, clr.R, clr.G, clr.B);
    }
}

bool
captureOamOn(placcessor_s *self)
{
    return IsDebugOn(*self->ppu_->debugflags_, NHD_DBG_OAM);
}

void
captureOam(placcessor_s *self)
{
    for (int i = 0; i < NHD_OAM_SPRITES; ++i)
    {
        updateOamSpr(self, dbgoam_SprOf(&self->ppu_->oamsnap_, i), i);
    }
}

static color_s
getPalColor(placcessor_s *self, int idx)
{
    u8 clrbyte = placcessor_GetColorByte(self, idx);
    color_s clr = placcessor_GetPal(self)->ToRgb((palettecolor_s){clrbyte});
    return clr;
}

void
updateOamSpr(placcessor_s *self, dbgspr_s *spr, int idx)
{
    // Assuming OAMADDR starts at 0.
    u8 byteIdxStart = (u8)(idx * 4);
    u8 y = placcessor_GetOamByte(self, byteIdxStart);
    u8 tile = placcessor_GetOamByte(self, byteIdxStart + 1);
    u8 attr = placcessor_GetOamByte(self, byteIdxStart + 2);
    u8 x = placcessor_GetOamByte(self, byteIdxStart + 3);

    dbgspr_SetRaw(spr, y, tile, attr, x);

    bool mode8x16 = placcessor_Is8x16(self);
    dbgspr_SetMode(spr, mode8x16);

    bool flipy = attr & 0x80;
    bool flipx = attr & 0x40;

    bool tblR;
    placcessor_ResolveSpPtnTbl(
        tile, mode8x16, placcessor_GetReg(self, PR_PPUCTRL) & 0x08, &tblR);
    for (u8 yInSp = 0; yInSp < 16; ++yInSp)
    {
        if (!mode8x16 && yInSp >= 8)
        {
            break;
        }

        u8 tileidx;
        placcessor_ResolveSpTile(tile, mode8x16, flipy, yInSp, &tileidx);

        // static_assert(NH_PATTERN_TILE_HEIGHT == 8,
        //               "Invalid NH_PATTERN_TILE_HEIGHT");
        u8 finey = yInSp % NH_PATTERN_TILE_HEIGHT;
        if (flipy)
        {
            finey = (NH_PATTERN_TILE_HEIGHT - 1) - finey;
        }

        u8 ptnbit0B;
        {
            NHErr err = placcessor_GetPtnSliver(tblR, tileidx, false, finey,
                                                self->ppu_->vmem_, &ptnbit0B);
            if (NH_FAILED(err))
            {
                ptnbit0B = 0xFF; // set to apparent value.
            }
        }
        u8 ptnbit1B;
        {
            NHErr err = placcessor_GetPtnSliver(tblR, tileidx, true, finey,
                                                self->ppu_->vmem_, &ptnbit1B);
            if (NH_FAILED(err))
            {
                ptnbit1B = 0xFF; // set to apparent value.
            }
        }

        // Reverse the bits to implement horizontal flipping.
        if (flipx)
        {
            ReverseByte(&ptnbit0B);
            ReverseByte(&ptnbit1B);
        }

        /* Now that we have a row of data available */
        for (int fineX = 0; fineX < 8; ++fineX)
        {
            int addrPalSetOffset = attr & 0x03; // 4-color palette
            int ptn = ((bool)(ptnbit1B & (0x80 >> fineX)) << 1) |
                      ((bool)(ptnbit0B & (0x80 >> fineX)) << 0);
            // static_assert(NH_PALETTE_SIZE == 32,
            //               "Incorrect color byte position");
            color_s pixel = getPalColor(self, 16 + addrPalSetOffset * 4 + ptn);
            dbgspr_SetPixel(spr, yInSp, fineX, pixel);
        }
    }
}

bool
capturePtnTblsOn(placcessor_s *self)
{
    return IsDebugOn(*self->ppu_->debugflags_, NHD_DBG_PATTERN);
}

static void
captureTable(placcessor_s *self, bool right, dbgpattbl_s *tbl)
{
    // static_assert(DBGPATTBL_TILES == 16 * 16, "Incorrect loop count");
    // static_assert(std::numeric_limits<int>::max() >= 16 * 16,
    //               "Type of loop variable too small");
    for (int tileidx = 0; tileidx < DBGPATTBL_TILES; ++tileidx)
    {
        // static_assert(DBGPATTBL_TILE_H == 8, "Incorrect loop count");
        for (u8 fineY = 0; fineY < DBGPATTBL_TILE_H; ++fineY)
        {
            // static_assert(std::numeric_limits<u8>::max() >= 16 * 16 - 1,
            //               "Type of tile index incompatible for use of "
            //               "\"get_ptn_sliver\"");
            u8 ptnbit0B;
            {
                NHErr err =
                    placcessor_GetPtnSliver(right, (u8)tileidx, false, fineY,
                                            self->ppu_->vmem_, &ptnbit0B);
                if (NH_FAILED(err))
                {
                    ptnbit0B = 0xFF; // set to apparent value.
                }
            }
            u8 ptnbit1B;
            {
                NHErr err =
                    placcessor_GetPtnSliver(right, (u8)tileidx, true, fineY,
                                            self->ppu_->vmem_, &ptnbit1B);
                if (NH_FAILED(err))
                {
                    ptnbit1B = 0xFF; // set to apparent value.
                }
            }

            /* Now that we have a row of data available */
            // static_assert(DBGPATTBL_TILE_W == 8, "Incorrect loop count");
            for (int fineX = 0; fineX < DBGPATTBL_TILE_W; ++fineX)
            {
                int ptn = ((bool)(ptnbit1B & (0x80 >> fineX)) << 1) |
                          ((bool)(ptnbit0B & (0x80 >> fineX)) << 0);
                // static_assert(NH_PALETTE_SIZE == 32,
                //               "Incorrect color byte position");
                color_s pixel =
                    getPalColor(self, self->ppu_->ptntblPalIdx_ * 4 + ptn);
                dbgpattble_SetPixel(tbl, tileidx, fineY, fineX, pixel);
            }
        }
    }
}

void
capturePtnTbls(placcessor_s *self)
{
    captureTable(self, false, &self->ppu_->ptntblsnapL_);
    captureTable(self, true, &self->ppu_->ptntblsnapR_);
}
