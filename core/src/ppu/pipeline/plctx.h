#pragma once

#include "spec.h"
#include "types.h"

typedef struct plctx_s {
    // ------ Shared pipeline states
    bool OddFrame;
    bool SkipCycle; // skip last cycle at pre-render scanline
    int ScanlineNo; // [-1, 260], i.e. 261 == -1.
    int PixelRow;
    int PixelCol;

    // ------ Background
    // fetch
    u8 BgNtByte;     // intermediate to get pattern
    u8 BgAttrPalIdx; // 2-bit
    u8 BgLowerSliver;
    u8 BgUpperSliver;

    // rendering
    /* internal shift registers (and the latches) */
    // https://www.nesdev.org/wiki/PPU_rendering#Preface
    u16 SfBgPatLower;
    u16 SfBgPatUpper;
    // the latch need only 1 bit each, but we expand it to 8-bit for
    // convenience.
    u16 SfBgPalIdxLower;
    u16 SfBgPalIdxUpper;

    // ------ Sprite
    // pre-fetch
    u8 SecOam[NH_SEC_OAM_SIZE];
    // fetch and register reload
    u8 SfSpPatLower[NH_MAX_VISIBLE_SP_NUM];
    u8 SfSpPatUpper[NH_MAX_VISIBLE_SP_NUM];
    u8 SpAttr[NH_MAX_VISIBLE_SP_NUM];
    u8 SpPosX[NH_MAX_VISIBLE_SP_NUM];
    // metadata passed on to rendering stage on next scanline
    u8 SpCount;   // how many sprites are in range
    bool WithSp0; // sprites to render include sprite 0
} plctx_s;
