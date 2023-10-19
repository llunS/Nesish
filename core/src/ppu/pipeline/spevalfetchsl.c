#include "spevalfetchsl.h"

#include "spec.h"
#include "ppu/placcessor.h"
#include "nhassert.h"
#include "byteutils.h"
#include "ppu/pipeline/plctx.h"
#include "memory/vmem.h"

static void
secOamClear(cycle_t step, placcessor_s *accessor);
static void
spEval(cycle_t step, placcessor_s *accessor, spevalfetchctx_s *slctx);
static void
spFetchReload(cycle_t step, placcessor_s *accessor, spevalfetchctx_s *slctx);

void
spevalfetchsl_Init(spevalfetchsl_s *self, placcessor_s *accessor)
{
    self->accessor_ = accessor;
}

void
spevalfetchsl_Tick(spevalfetchsl_s *self, cycle_t col)
{
    // col
    // [1, 320] for visible line
    // [257, 320] for pre-render line

    if (1 <= col && col <= 64)
    {
        secOamClear(col - 1, self->accessor_);
    }
    else if (65 <= col && col <= 256)
    {
        if (65 == col)
        {
            u8 oamaddr = placcessor_GetReg(self->accessor_, PR_OAMADDR);
            self->ctx_.SecOamWriteIdx = 0;
            self->ctx_.CopyCtr = 0;
            // Perhaps we shouldn't cache this either
            self->ctx_.InitOamAddr = oamaddr;
            self->ctx_.N = self->ctx_.M = 0;
            self->ctx_.NOverflow = false;
            self->ctx_.SpGot = 0;
            self->ctx_.SpOverflow = false;
            self->ctx_.SecOamWritten = false;
            self->ctx_.Sp0InRange = false;
        }

        spEval(col - 65, self->accessor_, &self->ctx_);
    }
    else if (257 <= col && col <= 320)
    {
        if (257 == col)
        {
            // Sprite evaluation is done already at tick 257

            // Pass out the flag at fetch stage (not eval stage), in case the
            // flag is used by current scanline instead of the next one.
            // In other words, "WithSp0" should not be overwritten in rendering
            // stage (while it's in use).
            // This relys on the fact that Render is ticked before SpEvalFetch.
            placcessor_GetCtx(self->accessor_)->WithSp0 = self->ctx_.Sp0InRange;
            // The same applies to "SpCount"
            placcessor_GetCtx(self->accessor_)->SpCount = self->ctx_.SpGot;

            self->ctx_.SecOamReadIdx = 0;
        }

        if (placcessor_RenderingEnabled(self->accessor_))
        {
            spFetchReload((col - 257) % 8, self->accessor_, &self->ctx_);
        }

#ifndef NDEBUG
        /* do some checks */
        if (320 == col)
        {
            if (placcessor_RenderingEnabled(self->accessor_) &&
                self->ctx_.SecOamReadIdx != NH_SEC_OAM_SIZE)
            {
                ASSERT_ERROR(placcessor_GetLogger(self->accessor_),
                             "Wrong secondary OAM read index: " U8FMT,
                             self->ctx_.SecOamReadIdx);
            }
        }
#endif
    }

#if 1
// Cycles 321-340+0: Read the first byte in secondary OAM (while the PPU fetches
// the first two background tiles for the next scanline)
// We don't emulate it.
#endif
}

void
secOamClear(cycle_t step, placcessor_s *accessor)
{
    // step: [0, 64)

    if (step % 2 == 0)
    {
        // read from OAMDATA, we don't emulate it.
    }
    else
    {
        placcessor_GetCtx(accessor)->SecOam[step / 2] = 0xFF;
    }
}

static bool
got8sp(spevalfetchctx_s *slctx)
{
    return slctx->SpGot >= 8;
}

static bool
writeDisabled(spevalfetchctx_s *slctx)
{
    return slctx->NOverflow || got8sp(slctx);
}

static void
addRead(placcessor_s *accessor, spevalfetchctx_s *slctx, bool incN, bool incM,
        bool carry)
{
    bool carryN = false;
    if (incM)
    {
        ++slctx->M;
        if (slctx->M >= NH_OAM_SP_SIZE)
        {
            slctx->M = 0;

            if (carry)
            {
                carryN = true;
            }
        }
    }
    slctx->N += (int)(incN) + (int)(carryN);
    if (slctx->N >= NH_MAX_SP_NUM)
    {
        slctx->N %= NH_MAX_SP_NUM;

        // all 64 are checked
        slctx->NOverflow = true;
    }

    int newReadAddr =
        slctx->InitOamAddr + (slctx->N * NH_OAM_SP_SIZE + slctx->M);
    *placcessor_RegOf(accessor, PR_OAMADDR) = (u8)(newReadAddr);
}

static void
writeSecOam(placcessor_s *accessor, spevalfetchctx_s *slctx, u8 val, bool ince)
{
    if (writeDisabled(slctx))
    {
        return;
    }

    placcessor_GetCtx(accessor)->SecOam[slctx->SecOamWriteIdx] = val;
    slctx->SecOamWritten = true;
    if (ince)
    {
        ++slctx->SecOamWriteIdx;

        // got 1 sprite
        if (slctx->SecOamWriteIdx % NH_OAM_SP_SIZE == 0)
        {
            ++slctx->SpGot;
        }
    }
}

static void
incWrite(spevalfetchctx_s *slctx)
{
    if (writeDisabled(slctx))
    {
        return;
    }

    ++slctx->SecOamWriteIdx;
}

void
spEval(cycle_t step, placcessor_s *accessor, spevalfetchctx_s *slctx)
{
    // step: [0, 192)

    // @TODO: hide sprites before OAMADDR
    // No more sprites will be found once the end of OAM is reached, effectively
    // hiding any sprites before the starting OAMADDR
    // https://www.nesdev.org/wiki/PPU_registers#Values_during_rendering

    // The process must be halted immediately, i.e. in real time.
    // "If [PPUMASK]] ($2001) with both BG and sprites disabled, rendering will
    // be halted immediately."
    // https://www.nesdev.org/wiki/PPU_sprite_evaluation#Rendering_disable_or_enable_during_active_scanline
    // test: sprite_overflow_tests/5.Emulator.nes failure #4
    if (placcessor_RenderingEnabled(accessor))
    {
        // read cycle
        if (step % 2 == 0)
        {
            u8 oamaddr = placcessor_GetReg(accessor, PR_OAMADDR);
            slctx->SpEvalBus = placcessor_GetOamByte(accessor, oamaddr);
        }
        // write cycle
        else
        {
            // Y
            if (!slctx->CopyCtr)
            {
                u8 y = slctx->SpEvalBus;

                // Test this before "writeSecOam()"
                bool sp0 = !slctx->SecOamWritten;
                // copy Y to secondary OAM
                writeSecOam(accessor, slctx, y, false);

                if (slctx->NOverflow || slctx->SpOverflow)
                {
                    // try next Y
                    addRead(accessor, slctx, true, false, true);
                }
                else
                {
                    // static_assert(NH_PATTERN_TILE_HEIGHT == 8,
                    //               "Invalid NH_PATTERN_TILE_HEIGHT");
                    u8 spH = placcessor_Is8x16(accessor)
                                 ? NH_PATTERN_TILE_HEIGHT * 2
                                 : NH_PATTERN_TILE_HEIGHT;
                    bool inRange =
                        (y <= placcessor_GetCtx(accessor)->ScanlineNo &&
                         placcessor_GetCtx(accessor)->ScanlineNo < y + spH);
                    if (inRange)
                    {
                        // copy the rest
                        incWrite(slctx);
                        slctx->CopyCtr = NH_OAM_SP_SIZE - 1;

                        addRead(accessor, slctx, false, true, true);

                        if (sp0)
                        {
                            slctx->Sp0InRange = true;
                        }
                        if (got8sp(slctx))
                        {
                            // set sprite overflow flag
                            slctx->SpOverflow = true;
                            *placcessor_RegOf(accessor, PR_PPUSTATUS) |= 0x20;
                        }
                    }
                    else
                    {
                        // try next Y
                        if (!got8sp(slctx))
                        {
                            addRead(accessor, slctx, true, false, true);
                        }
                        else
                        {
                            // @QUIRK: Sprite overflow bug
                            // https://www.nesdev.org/wiki/PPU_sprite_evaluation#Sprite_overflow_bug
                            addRead(accessor, slctx, true, true, false);
                        }
                    }
                }
            }
            // rest 3
            else
            {
                bool prevGot8 = got8sp(slctx);
                writeSecOam(accessor, slctx, slctx->SpEvalBus, true);
                if (prevGot8 != got8sp(slctx))
                {
                    if (!got8sp(slctx) || slctx->CopyCtr != 1)
                    {
                        ASSERT_FATAL(placcessor_GetLogger(accessor),
                                     "Wrong sprite evaluation timing in "
                                     "copying rest: %d",
                                     slctx->CopyCtr);
                    }
                }

                addRead(accessor, slctx, false, true, true);

                --slctx->CopyCtr;
            }
        }
    }
}

static u8
readSecOam(placcessor_s *accessor, spevalfetchctx_s *slctx)
{
    return placcessor_GetCtx(accessor)->SecOam[slctx->SecOamReadIdx++];
}

/* fetch pattern table tile byte */
static u8
getPatSliver(placcessor_s *accessor, spevalfetchctx_s *slctx, u8 spidx,
             bool upper)
{
    // https://www.nesdev.org/wiki/PPU_rendering#Cycles_257-320
    // We don't emulate dummy fetch to 0xFF
    if (spidx >= slctx->SpGot)
    {
        // Transparent values for left-over sprites
        return 0x00;
    }

    bool tblR;
    placcessor_ResolveSpPtnTbl(slctx->SpTileByte, placcessor_Is8x16(accessor),
                               placcessor_GetReg(accessor, PR_PPUCTRL) & 0x08,
                               &tblR);

    bool flipY = slctx->SpAttrByte & 0x80;
    u8 yInSp = (u8)(placcessor_GetCtx(accessor)->ScanlineNo - slctx->SpPosY);
    u8 tileidx;
    placcessor_ResolveSpTile(slctx->SpTileByte, placcessor_Is8x16(accessor),
                             flipY, yInSp, &tileidx);

    // static_assert(NH_PATTERN_TILE_HEIGHT == 8,
    //               "Invalid NH_PATTERN_TILE_HEIGHT");
    u8 fineY = yInSp % NH_PATTERN_TILE_HEIGHT;
    if (flipY)
    {
        fineY = (NH_PATTERN_TILE_HEIGHT - 1) - fineY;
    }

    addr_t sliverAddr = placcessor_GetSliverAddr(tblR, tileidx, upper, fineY);
    u8 ptnByte;
    {
        NHErr err =
            vmem_GetB(placcessor_GetVmem(accessor), sliverAddr, &ptnByte);
        if (NH_FAILED(err))
        {
            ASSERT_FATAL(placcessor_GetLogger(accessor),
                         "Failed to fetch pattern byte for sp: " ADDRFMT ", %d",
                         sliverAddr, upper);
            ptnByte = 0xFF; // set to apparent value.
        }
    }

    // Reverse the bits to implement horizontal flipping.
    bool flipX = slctx->SpAttrByte & 0x40;
    if (flipX)
    {
        ReverseByte(&ptnByte);
    }

    return ptnByte;
}

void
spFetchReload(cycle_t step, placcessor_s *accessor, spevalfetchctx_s *slctx)
{
    // step: [0, 8)

    switch (step)
    {
        // The first 2 ticks seem to be unspecified in the nesdev
        // forum, we are just guessing here.
        case 0:
        {
            // Mark down the current processing sprite index at first
            // tick. Do this before "readSecOam()" since it modifies
            // "SecOamReadIdx".
            slctx->SpIdxReload = slctx->SecOamReadIdx / NH_OAM_SP_SIZE;

            // Read Y.
            slctx->SpPosY = readSecOam(accessor, slctx);
        }
        break;

        case 1:
        {
            // Read tile.
            slctx->SpTileByte = readSecOam(accessor, slctx);
        }
        break;

        case 2:
        {
            // Read attribute.
            u8 attr = readSecOam(accessor, slctx);
            slctx->SpAttrByte = attr;
            placcessor_GetCtx(accessor)->SpAttr[slctx->SpIdxReload] = attr;
        }
        break;

        case 3:
        {
            // Read X.
            u8 x = readSecOam(accessor, slctx);
            placcessor_GetCtx(accessor)->SpPosX[slctx->SpIdxReload] = x;
        }
        break;

#if 1
// https://www.nesdev.org/wiki/PPU_sprite_evaluation#Details
// 4-7: Read the X-coordinate of the selected sprite from secondary OAM 4 times
// We don't emulate this.
#endif

        case 5:
        {
            placcessor_GetCtx(accessor)->SfSpPatLower[slctx->SpIdxReload] =
                getPatSliver(accessor, slctx, slctx->SpIdxReload, false);
        }
        break;

        case 7:
        {
            placcessor_GetCtx(accessor)->SfSpPatUpper[slctx->SpIdxReload] =
                getPatSliver(accessor, slctx, slctx->SpIdxReload, true);
        }
        break;

        default:
            break;
    }

    // OAMADDR is set to 0 during each of ticks 257-320 (the sprite tile loading
    // interval)
    // https://www.nesdev.org/wiki/PPU_registers#OAM_address_($2003)_%3E_write
    *placcessor_RegOf(accessor, PR_OAMADDR) = 0;
}
