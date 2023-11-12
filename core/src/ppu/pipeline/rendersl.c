#include "rendersl.h"

#include "spec.h"
#include "ppu/placcessor.h"
#include "nhassert.h"
#include "byteutils.h"
#include "ppu/color.h"
#include "ppu/palettecolor.h"
#include "ppu/pipeline/plctx.h"
#include "ppu/palette.h"
#include "ppu/frmbuf.h"

typedef struct outcolor_s {
    color_s Color;
    u8 Pattern;    // 2-bit;
    bool Priority; // sprite only. true: behind background
    bool Sp0;      // sprite only. if this is sprite 0
} outcolor_s;

inline outcolor_s
outcolor_Bg(color_s clr, u8 pat)
{
    return (outcolor_s){clr, pat, false, false};
}
inline outcolor_s
outcolor_Sp(color_s clr, u8 pat, bool priority, bool sp0)
{
    return (outcolor_s){clr, pat, priority, sp0};
}

static const outcolor_s ColorEmpty = {
    .Color = {0x00, 0x00, 0x00},
    .Pattern = 0x00,
};

static outcolor_s
bgRender(placcessor_s *accessor);
static outcolor_s
spRender(placcessor_s *accessor, renderctx_s *slctx);
static void
muxer(placcessor_s *accessor, const outcolor_s bgclr, const outcolor_s spclr);

void
rendersl_Init(rendersl_s *self, placcessor_s *accessor)
{
    self->accessor_ = accessor;

    self->ctx_.ToDrawSpsFrontSz = 0;
    self->ctx_.ToDrawSpsFront = self->ctx_.ToDrawSps1;

    self->ctx_.ToDrawSpsBackSz = 0;
    self->ctx_.ToDrawSpsBack = self->ctx_.ToDrawSps2;

    self->ctx_.ActiveSpsSz = 0;
}

static color_s
getBackdropClr(placcessor_s *accessor)
{
    u8 backdropByte =
        placcessor_GetColorByte(accessor, NH_PALETTE_BACKDROP_IDX);
    color_s backdrop =
        placcessor_GetPal(accessor)->ToRgb((palettecolor_s){backdropByte});
    return backdrop;
}

void
rendersl_Tick(rendersl_s *self, cycle_t col)
{
    // col: [2, 257]

    // Since sprite evaluation doesn't occur on pre-render scanline
    // and thus no valid data is available for sprite rendering.
    bool hasValidSpData = 0 != placcessor_GetCtx(self->accessor_)->ScanlineNo;

    /* reset some states */
    if (2 == col) {
        // reset pixel coordinate
        if (0 == placcessor_GetCtx(self->accessor_)->ScanlineNo) {
            placcessor_GetCtx(self->accessor_)->PixelRow = 0;
        }
        placcessor_GetCtx(self->accessor_)->PixelCol = 0;

        u8 spcount =
            hasValidSpData ? placcessor_GetCtx(self->accessor_)->SpCount : 0;
        self->ctx_.ToDrawSpsFrontSz = spcount;
        // the max size of self->ctx_.ToDrawSpsFront <= NH_MAX_VISIBLE_SP_NUM,
        // so no risk of unsigned interger wrapping around
        // static_assert(
        //     NH_MAX_VISIBLE_SP_NUM >= 1 &&
        //         NH_MAX_VISIBLE_SP_NUM - 1 <=
        //             std::numeric_limits<
        //                 decltype(self->ctx_.ToDrawSpsFront)::value_type>::max(),
        //     "Risk of unsigned interger wrapping around");
        for (u8 i = 0; i < spcount; ++i) {
            self->ctx_.ToDrawSpsFront[i] = i;
        }
        self->ctx_.ToDrawSpsBackSz = 0;
    }

    /* Get active sprites */
    self->ctx_.ActiveSpsSz = 0;
    if (self->ctx_.ToDrawSpsFrontSz) {
        // curX is in range [0, 256)
        u8 curX = (u8)(col - 2);
        for (int i = 0; i < self->ctx_.ToDrawSpsFrontSz; ++i) {
            const u8 spIdx = self->ctx_.ToDrawSpsFront[i];
            u8 spX = placcessor_GetCtx(self->accessor_)->SpPosX[spIdx];
            if (spX <= curX && (spX >= 256 - 8 || curX < spX + 8)) {
                u8 fineX = curX - spX;
                // sprite with lower index gets pushed first, which ensures
                // correct priority among sprites.
                self->ctx_.ActiveSps[self->ctx_.ActiveSpsSz++] =
                    ToByte2(spIdx, fineX);

                self->ctx_.ToDrawSpsBack[self->ctx_.ToDrawSpsBackSz++] = spIdx;
            } else if (spX < 256 - 8 && curX >= spX + 8) {
                // drop this sprite
            } else {
                self->ctx_.ToDrawSpsBack[self->ctx_.ToDrawSpsBackSz++] = spIdx;
            }
        }
        // cleanup useless slots
        {
            u8 *tmp = self->ctx_.ToDrawSpsFront;
            self->ctx_.ToDrawSpsFront = self->ctx_.ToDrawSpsBack;
            self->ctx_.ToDrawSpsBack = tmp;
        }
        self->ctx_.ToDrawSpsBackSz = 0;
    }

    /* rendering */
    {
        // @TODO: Background palette hack
        // https://www.nesdev.org/wiki/PPU_palettes#The_background_palette_hack

        outcolor_s bgClr = bgRender(self->accessor_);
        if (!placcessor_BgEnabled(self->accessor_) ||
            (!(placcessor_GetCtx(self->accessor_)->PixelCol & ~0x07) &&
             !(placcessor_GetReg(self->accessor_, PR_PPUMASK) & 0x02))) {
            bgClr.Color = getBackdropClr(self->accessor_);
            bgClr.Pattern = 0;
        }

        outcolor_s spClr = spRender(self->accessor_, &self->ctx_);
        bool renderSp = placcessor_SpEnabled(self->accessor_) && hasValidSpData;
        if (!renderSp ||
            (!(placcessor_GetCtx(self->accessor_)->PixelCol & ~0x07) &&
             !(placcessor_GetReg(self->accessor_, PR_PPUMASK) & 0x04))) {
            spClr.Color = getBackdropClr(self->accessor_);
            spClr.Pattern = 0;
        }

        muxer(self->accessor_, bgClr, spClr);

        // Mark dirty after rendering to the last dot
        if (col == 257 &&
            239 == placcessor_GetCtx(self->accessor_)->ScanlineNo) {
            placcessor_FinishFrame(self->accessor_);
        }
    }

    /* pixel coordinate advance */
    if (placcessor_GetCtx(self->accessor_)->PixelCol + 1 >= NH_NES_WIDTH) {
        placcessor_GetCtx(self->accessor_)->PixelCol = 0;
        if (placcessor_GetCtx(self->accessor_)->PixelRow + 1 >= NH_NES_HEIGHT) {
            if (placcessor_GetCtx(self->accessor_)->PixelRow + 1 >
                NH_NES_HEIGHT) {
                ASSERT_FATAL(placcessor_GetLogger(self->accessor_),
                             "Pixel rendering row out of bound %d",
                             placcessor_GetCtx(self->accessor_)->PixelRow);
            }

            placcessor_GetCtx(self->accessor_)->PixelRow = 0;
        } else {
            ++placcessor_GetCtx(self->accessor_)->PixelRow;
        }
    } else {
        ++placcessor_GetCtx(self->accessor_)->PixelCol;
    }
}

outcolor_s
bgRender(placcessor_s *accessor)
{
    /* 1. Bit selection mask by finx X scroll */
    const u8 x = *placcessor_GetX(accessor);
    if (x > 7) {
        ASSERT_FATAL(placcessor_GetLogger(accessor),
                     "Invalid background X value: " U8FMTX, x);
    }
    u16 bitShiftAndMask = 0x8000 >> x;

    /* 2. Get palette index */
    plctx_s *ctx = placcessor_GetCtx(accessor);
    bool palIdxLowerBit = ctx->SfBgPalIdxLower & bitShiftAndMask;
    bool palIdxUpperBit = ctx->SfBgPalIdxUpper & bitShiftAndMask;
    // 2-bit
    u8 palIdx = ((u8)(palIdxUpperBit) << 1) | (u8)(palIdxLowerBit);

    /* 3. Get pattern data (i.e. index into palette) */
    bool patDataLowBit = ctx->SfBgPatLower & bitShiftAndMask;
    bool patDataUpperBit = ctx->SfBgPatUpper & bitShiftAndMask;
    // 2-bit
    u8 patData = ((u8)(patDataUpperBit) << 1) | (u8)(patDataLowBit);

    /* 4. get palette index color */
    // @TODO: Background palette hack
    const int palSp = false;
    int colorIdx = patData ? (palSp << 4) | (palIdx << 2) | patData
                           : NH_PALETTE_BACKDROP_IDX;
    u8 idxClrByte = placcessor_GetColorByte(accessor, colorIdx);

    /* 5. conversion from index color to RGB color */
    color_s pixel =
        placcessor_GetPal(accessor)->ToRgb((palettecolor_s){idxClrByte});

    return outcolor_Bg(pixel, patData);
}

outcolor_s
spRender(placcessor_s *accessor, renderctx_s *slctx)
{
    // Although the COLOR of "ColorEmpty" may be a valid value for sprite
    // 0, but the pattern member being a transparent value ensures that it won't
    // trigger sprite 0 hit. So we are safe to use this, when no sprites are
    // rendered at this pixel.
    outcolor_s color = ColorEmpty;

    // search for the first non-transparent pixel among activated sprites.
    for (int idx = 0; idx < slctx->ActiveSpsSz; ++idx) {
        u8 i;
        u8 fineX;
        FromByte2(slctx->ActiveSps[idx], &i, &fineX);

        plctx_s *ctx = placcessor_GetCtx(accessor);
        /* 1. Bit selection mask by finx X scroll */
        // fineX must have been in range [0, 8)
        u8 bitShiftAndMask = 0x80 >> fineX;

        /* 2. Get palette index */
        // 2-bit
        u8 palIdx = ctx->SpAttr[i] & 0x03;

        /* 3. Get pattern data (i.e. index into palette) */
        // Flipping of both X and Y was done in the fetch stage already.
        bool patDataLowBit = ctx->SfSpPatLower[i] & bitShiftAndMask;
        bool patDataUpperBit = ctx->SfSpPatUpper[i] & bitShiftAndMask;
        // 2-bit
        u8 patData = ((u8)(patDataUpperBit) << 1) | (u8)(patDataLowBit);

        /* Skip transparent pixel */
        if (!patData) {
            continue;
        }

        /* 4. get palette index color */
        // @TODO: Background palette hack
        const int palSp = true;
        int colorIdx = patData ? (palSp << 4) | (palIdx << 2) | patData
                               : NH_PALETTE_BACKDROP_IDX;
        u8 idxClrByte = placcessor_GetColorByte(accessor, colorIdx);

        /* 5. conversion from index color to RGB color */
        color_s pixel =
            placcessor_GetPal(accessor)->ToRgb((palettecolor_s){idxClrByte});

        /* 6. stuff in priority, and return */
        // static_assert(NH_MAX_VISIBLE_SP_NUM >= 1 &&
        //                   std::numeric_limits<u8>::max() >=
        //                       NH_MAX_VISIBLE_SP_NUM - 1,
        //               "Invalid range for sprite index");
        // If this line includes sprite 0, it must be at index 0.
        color = outcolor_Sp(pixel, patData, (ctx->SpAttr[i] & 0x20) != 0,
                            i == 0 && ctx->WithSp0);
        // Got non-transparent pixel
        break;
    }

    return color;
}

void
muxer(placcessor_s *accessor, const outcolor_s bgclr, const outcolor_s spclr)
{
    /* Priority decision */
    // Check the decision table for details
    // https://www.nesdev.org/wiki/PPU_rendering#Preface
    color_s outClr =
        !spclr.Pattern
            ? bgclr.Color
            : ((bgclr.Pattern && spclr.Priority) ? bgclr.Color : spclr.Color);

    /* Sprite 0 hit */
    // https://www.nesdev.org/wiki/PPU_OAM#Sprite_zero_hits
    // 1. If background or sprite rendering is disabled in PPUMASK
    // ($2001)
    // 2. At x=0 to x=7 if the left-side clipping window is enabled (if
    // bit 2 or bit 1 of PPUMASK is 0)
    // 3. At x=255, for an obscure reason related to the pixel pipeline
    // 4. At any pixel where the background or sprite pixel is
    // transparent
    // 5. If sprite 0 hit has already occurred this frame
    if ((!placcessor_BgEnabled(accessor) || !placcessor_SpEnabled(accessor)) ||
        (((placcessor_GetReg(accessor, PR_PPUMASK) & 0x06) != 0x06) &&
         !(placcessor_GetCtx(accessor)->PixelCol & ~0x07)) ||
        placcessor_GetCtx(accessor)->PixelCol == 255 ||
        (!bgclr.Pattern || !spclr.Pattern)
        /* || (placcessor_GetReg(accessor, PR_PPUSTATUS) & 0x40) */) {
    } else {
        if (spclr.Sp0 && (bgclr.Pattern && spclr.Pattern)) {
            // if (!(placcessor_GetReg(accessor, PR_PPUSTATUS) & 0x40))
            {
                *placcessor_RegOf(accessor, PR_PPUSTATUS) |= 0x40;
            }
        }
    }

    // Actually pixel ouput is delayed by 2 cycles, but since we trigger
    // the callback on a per-frame basis, write to frame buffer immediately will
    // be functionally the same.
    // https://www.nesdev.org/wiki/PPU_rendering#Cycles_1-256
    frmbuf_Write(placcessor_GetFrmbuf(accessor),
                 placcessor_GetCtx(accessor)->PixelRow,
                 placcessor_GetCtx(accessor)->PixelCol, outClr);
}
