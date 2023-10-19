#pragma once

#include "types.h"
#include "spec.h"

typedef struct renderctx_s {
    u8 *ToDrawSpsFront;
    int ToDrawSpsFrontSz;
    u8 *ToDrawSpsBack;
    int ToDrawSpsBackSz;

    u8 ToDrawSps1[NH_MAX_VISIBLE_SP_NUM];
    u8 ToDrawSps2[NH_MAX_VISIBLE_SP_NUM];

    u16 ActiveSps[NH_MAX_VISIBLE_SP_NUM];
    int ActiveSpsSz;
} renderctx_s;

typedef struct placcessor_s placcessor_s;

typedef struct rendersl_s {
    placcessor_s *accessor_;
    renderctx_s ctx_;
} rendersl_s;

void
rendersl_Init(rendersl_s *self, placcessor_s *accessor);

void
rendersl_Tick(rendersl_s *self, cycle_t col);
