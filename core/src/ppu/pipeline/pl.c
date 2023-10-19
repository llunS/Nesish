#include "pl.h"

#include "ppu/placcessor.h"
#include "spec.h"
#include "ppu/pipeline/plctx.h"

#define POSTRENDER_SL_IDX 261
#define POSTRENDER_SL -1

static void
advCounter(pl_s *self);

static const int SCANLINE_COUNT = 262;

void
pl_Init(pl_s *self, placcessor_s *accessor)
{
    self->accessor_ = accessor;

    prerendersl_Init(&self->prerendersl_, accessor);
    visiblesl_Init(&self->visiblesl_, accessor);

    pl_Reset(self);
}

void
pl_Reset(pl_s *self)
{
    // Start from pre-render scanline
    // Or else ppu_vbl_nmi/ppu_vbl_nmi.nes fails,
    // even if each individual test passes. Don't know why.
    self->currslIdx_ = POSTRENDER_SL_IDX;
    self->currslCol_ = 0;

    plctx_s *ctx = placcessor_GetCtx(self->accessor_);
    ctx->OddFrame = false;
    ctx->SkipCycle = false;
    ctx->ScanlineNo = POSTRENDER_SL;
    ctx->PixelRow = ctx->PixelCol = 0;
}

void
pl_Tick(pl_s *self)
{
    if (0 <= self->currslIdx_ && self->currslIdx_ <= 239)
    {
        /* Skip 1 cycle on first scanline, if current frame is odd and rendering
         * is enabled */
        if (0 == self->currslIdx_ && 0 == self->currslCol_)
        {
            if (placcessor_GetCtx(self->accessor_)->SkipCycle)
            {
                advCounter(self);
            }
        }

        visiblesl_Tick(&self->visiblesl_, self->currslCol_);
    }
    else if (241 == self->currslIdx_)
    {
        if (1 == self->currslCol_)
        {
            if (!placcessor_NoNmi(self->accessor_))
            {
                /* Set NMI_occurred in PPU to true */
                *placcessor_RegOf(self->accessor_, PR_PPUSTATUS) |= 0x80;
            }
        }
    }
    else if (261 == self->currslIdx_)
    {
        prerendersl_Tick(&self->prerendersl_, self->currslCol_);
    }

    advCounter(self);
}

void
advCounter(pl_s *self)
{
    // Update scaline column
    if (self->currslCol_ + 1 >= NH_SCANLINE_CYCLES)
    {
        self->currslCol_ = 0;

        // Update scanline row
        if (self->currslIdx_ + 1 >= SCANLINE_COUNT)
        {
            self->currslIdx_ = 0;
        }
        else
        {
            ++self->currslIdx_;
        }

        plctx_s *ctx = placcessor_GetCtx(self->accessor_);
        // Set scanline number based on index
        ctx->ScanlineNo =
            self->currslIdx_ + 1 >= SCANLINE_COUNT ? -1 : self->currslIdx_;
        // New frame
        if (POSTRENDER_SL == ctx->ScanlineNo)
        {
            ctx->OddFrame = !ctx->OddFrame;
            ctx->SkipCycle = false;
        }
    }
    else
    {
        ++self->currslCol_;
    }
}
