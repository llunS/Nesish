#include "prerendersl.h"

#include "ppu/placcessor.h"
#include "ppu/pipeline/plctx.h"

#include <string.h>

void
prerendersl_Init(prerendersl_s *self, placcessor_s *accessor)
{
    self->accessor_ = accessor;
    bgfetchsl_Init(&self->bg_, accessor);
    spevalfetchsl_Init(&self->sp_, accessor);
}

void
prerendersl_Tick(prerendersl_s *self, cycle_t col)
{
    switch (col) {
    case 1: {
        /* clear flags */
        // VSO (VBLANK, Sprite 0 hit, Sprite overflow)
        *placcessor_RegOf(self->accessor_, PR_PPUSTATUS) &= 0x1F;

        // OAM corruption
        // https://www.nesdev.org/wiki/PPU_sprite_evaluation#Notes
        // Simply do this near the start of pre-render scanline.
        // This should be done with rendering enabled, according to
        // test cpu_dummy_writes/cpu_dummy_writes_oam.nes
        if (placcessor_RenderingEnabled(self->accessor_)) {
            u8 oamCpAddr =
                placcessor_GetReg(self->accessor_, PR_OAMADDR) & 0xF8;
            if (oamCpAddr) {
                // memcpy is ok, memmove is not needed.
                memcpy(placcessor_GetOamPtr(self->accessor_, 0),
                       placcessor_GetOamPtr(self->accessor_, oamCpAddr), 8);
            }
        }
    } break;

    case 280:
    case 281:
    case 282:
    case 283:
    case 284:
    case 285:
    case 286:
    case 287:
    case 288:
    case 289:
    case 290:
    case 291:
    case 292:
    case 293:
    case 294:
    case 295:
    case 296:
    case 297:
    case 298:
    case 299:
    case 300:
    case 301:
    case 302:
    case 303:
    case 304: {
        if (placcessor_RenderingEnabled(self->accessor_)) {
            /* reload vertical bits */
            u16 *v = placcessor_GetV(self->accessor_);
            const u16 t = *placcessor_GetT(self->accessor_);
            *v = (*v & 0x041F) | (t & ~0x041F);
        }
    } break;

    case 338: {
        // Think of it as between the end of 338 and the start of 339. We
        // don't support true parallelism yet. We determine whether to skip
        // here, to pass the test
        // ppu_vbl_nmi/rom_singles/10-even_odd_timing.nes
        if (placcessor_GetCtx(self->accessor_)->OddFrame &&
            placcessor_BgEnabled(self->accessor_)) {
            placcessor_GetCtx(self->accessor_)->SkipCycle = true;
        }
    } break;

    default:
        break;
    }

    if ((1 <= col && col <= 257) || (321 <= col && col <= 340)) {
        bgfetchsl_Tick(&self->bg_, col);
    }
    if (257 <= col && col <= 320) {
        spevalfetchsl_Tick(&self->sp_, col);
    }
}
