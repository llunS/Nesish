#include "bgfetchsl.h"

#include "types.h"
#include "ppu/placcessor.h"
#include "nhassert.h"
#include "memory/vmem.h"
#include "ppu/pipeline/plctx.h"

static void
ntByteFetch(placcessor_s *accessor);
static void
tileFetch(cycle_t step, placcessor_s *accessor);

static void
shiftRegsShift(placcessor_s *accessor);
static void
shiftRegsReload(placcessor_s *accessor);

void
bgfetchsl_Init(bgfetchsl_s *self, placcessor_s *accessor)
{
    self->accessor_ = accessor;
}

void
bgfetchsl_Tick(bgfetchsl_s *self, cycle_t col)
{
    // col: [1, 257] | [321, 340]

    // @NOTE: shift registers shift should happen before shift registers reload
    if ((2 <= col && col <= 257) || (322 <= col && col <= 337))
    {
        /* shift */
        shiftRegsShift(self->accessor_);
    }

    switch (col)
    {
        case 256:
        {
            if (placcessor_RenderingEnabled(self->accessor_))
            {
                /* increment Y component of v */
                u16 *v = placcessor_GetV(self->accessor_);
                if ((*v & 0x7000) != 0x7000) // if fine Y < 7
                {
                    *v += 0x1000; // increment fine Y
                }
                else
                {
                    *v &= ~0x7000;                   // fine Y = 0
                    u8 y = (u8)((*v & 0x03E0) >> 5); // let y = coarse Y
                    if (y == 29)
                    {
                        y = 0;        // coarse Y = 0
                        *v ^= 0x0800; // switch vertical nametable
                    }
                    else if (y == 31)
                    {
                        y = 0; // coarse Y = 0, nametable not switched
                    }
                    else
                    {
                        y += 1; // increment coarse Y
                    }
                    *v = (*v & ~0x03E0) |
                         ((u16)(y) << 5); // put coarse Y back into v
                }
            }
        }
        // fall through
        case 8:
        case 16:
        case 24:
        case 32:
        case 40:
        case 48:
        case 56:
        case 64:
        case 72:
        case 80:
        case 88:
        case 96:
        case 104:
        case 112:
        case 120:
        case 128:
        case 136:
        case 144:
        case 152:
        case 160:
        case 168:
        case 176:
        case 184:
        case 192:
        case 200:
        case 208:
        case 216:
        case 224:
        case 232:
        case 240:
        case 248:
        /* first two tiles on next scanline */
        case 328:
        case 336:
        {
            if (placcessor_RenderingEnabled(self->accessor_))
            {
                /* increment X component of v */
                u16 *v = placcessor_GetV(self->accessor_);
                if ((*v & 0x001F) == 31) // if coarse X == 31
                {
                    *v &= ~0x001F; // coarse X = 0
                    *v ^= 0x0400;  // switch horizontal nametable
                }
                else
                {
                    *v += 1; // increment coarse X
                }
            }
        }
        break;

        case 257:
        {
            if (placcessor_RenderingEnabled(self->accessor_))
            {
                /* copies all bits related to horizontal position from t to v */
                u16 *v = placcessor_GetV(self->accessor_);
                const u16 t = *placcessor_GetT(self->accessor_);
                *v = (*v & ~0x041F) | (t & 0x041F);
            }

            shiftRegsReload(self->accessor_);
        }
        break;

        /* mysterious 2 nametable byte fetches */
        case 338:
        case 340:
        {
            ntByteFetch(self->accessor_);
        }
        break;

        case 9:
        case 17:
        case 25:
        case 33:
        case 41:
        case 49:
        case 57:
        case 65:
        case 73:
        case 81:
        case 89:
        case 97:
        case 105:
        case 113:
        case 121:
        case 129:
        case 137:
        case 145:
        case 153:
        case 161:
        case 169:
        case 177:
        case 185:
        case 193:
        case 201:
        case 209:
        case 217:
        case 225:
        case 233:
        case 241:
        case 249:
        /* first two tiles on next scanline */
        case 329:
        case 337:
        {
            shiftRegsReload(self->accessor_);
        }
        break;

        default:
            break;
    }

    // @NOTE: shift registers reload should happen before tile fetch
    if (1 <= col && col <= 256)
    {
        tileFetch((col - 1) % 8, self->accessor_);
    }
    /* first two tiles on next scanline */
    else if (321 <= col && col <= 336)
    {
        tileFetch((col - 321) % 8, self->accessor_);
    }
}

void
ntByteFetch(placcessor_s *accessor)
{
    const u16 v = *placcessor_GetV(accessor);
    addr_t tileaddr = 0x2000 | (v & 0x0FFF);

    u8 byte;
    NHErr err = vmem_GetB(placcessor_GetVmem(accessor), tileaddr, &byte);
    if (NH_FAILED(err))
    {
        ASSERT_FATAL(placcessor_GetLogger(accessor),
                     "Failed to fetch nametable byte for bg: " U16FMTX
                     ", " ADDRFMT,
                     v, tileaddr);
        byte = 0xFF; // set to apparent value.
    }

    placcessor_GetCtx(accessor)->BgNtByte = byte;
}

/* fetch pattern table tile byte */
static u8
getPatSliver(placcessor_s *accessor, bool upper)
{
    bool tblR = placcessor_GetReg(accessor, PR_PPUCTRL) & 0x10;
    u8 tileidx = placcessor_GetCtx(accessor)->BgNtByte;
    u8 finey = (u8)((*placcessor_GetV(accessor) >> 12) & 0x07);

    addr_t sliveraddr = placcessor_GetSliverAddr(tblR, tileidx, upper, finey);
    u8 byte;
    NHErr err = vmem_GetB(placcessor_GetVmem(accessor), sliveraddr, &byte);
    if (NH_FAILED(err))
    {
        ASSERT_FATAL(placcessor_GetLogger(accessor),
                     "Failed to fetch pattern byte for bg: " ADDRFMT ", %d",
                     sliveraddr, upper);
        byte = 0xFF; // set to apparent value.
    }

    return byte;
}

void
tileFetch(cycle_t step, placcessor_s *accessor)
{
    // step: [0, 8)

    switch (step)
    {
        case 1:
        {
            /* fetch nametable byte */
            ntByteFetch(accessor);
        }
        break;

        case 3:
        {
            const u16 v = *placcessor_GetV(accessor);

            int coarseX = v & 0x001F;
            int coarseY = (v >> 5) & 0x001F;

            /* fetch attribute table byte */
            addr_t attrAddr = 0x23C0 | (v & 0x0C00) |
                              (addr_t)((coarseY << 1) & 0x38) |
                              (addr_t)(coarseX >> 2);
            u8 attrByte;
            NHErr err =
                vmem_GetB(placcessor_GetVmem(accessor), attrAddr, &attrByte);
            if (NH_FAILED(err))
            {
                ASSERT_FATAL(placcessor_GetLogger(accessor),
                             "Failed to fetch attribute byte for bg: " U16FMTX
                             ", " ADDRFMT,
                             v, attrAddr);
                attrByte = 0xFF; // set to apparent value.
            }

            // index of 2x2-tile block in 4x4-tile block.
            int col = coarseX % 4 / 2;
            int row = coarseY % 4 / 2;
            int shifts = (row * 2 + col) * 2;
            u8 attrPalIdx = (attrByte >> shifts) & 0x03;

            /* fetch attribute table index */
            placcessor_GetCtx(accessor)->BgAttrPalIdx = attrPalIdx;
        }
        break;

        case 5:
        {
            /* fetch pattern table tile low */
            placcessor_GetCtx(accessor)->BgLowerSliver =
                getPatSliver(accessor, false);
        }
        break;

        case 7:
        {
            /* fetch pattern table tile high */
            placcessor_GetCtx(accessor)->BgUpperSliver =
                getPatSliver(accessor, true);
        }
        break;

        default:
            break;
    }
}

void
shiftRegsShift(placcessor_s *accessor)
{
    plctx_s *ctx = placcessor_GetCtx(accessor);
    ctx->SfBgPatLower <<= 1;
    ctx->SfBgPatUpper <<= 1;
    ctx->SfBgPalIdxLower <<= 1;
    ctx->SfBgPalIdxUpper <<= 1;
}

void
shiftRegsReload(placcessor_s *accessor)
{
    plctx_s *ctx = placcessor_GetCtx(accessor);
    ctx->SfBgPatLower = (ctx->SfBgPatLower & 0xFF00) | ctx->BgLowerSliver;
    ctx->SfBgPatUpper = (ctx->SfBgPatUpper & 0xFF00) | ctx->BgUpperSliver;
    ctx->SfBgPalIdxLower = (ctx->SfBgPalIdxLower & 0xFF00) |
                           (ctx->BgAttrPalIdx & 0x01 ? 0xFF : 0x00);
    ctx->SfBgPalIdxUpper = (ctx->SfBgPalIdxUpper & 0xFF00) |
                           (ctx->BgAttrPalIdx & 0x02 ? 0xFF : 0x00);
}
