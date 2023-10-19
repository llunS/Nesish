#pragma once

#include "types.h"

typedef struct spevalfetchctx_s {
    /* eval */
    /* some states might be used afterwards at fetch stage */
    u8 SpEvalBus; // temporary bus for eval stage
    u8 SecOamWriteIdx;
    int CopyCtr;
    u8 InitOamAddr;
    int N, M;
    bool NOverflow;
    u8 SpGot;
    bool SpOverflow; // in one scanline
    bool SecOamWritten;
    bool Sp0InRange;

    /* fetch */
    u8 SecOamReadIdx;
    u8 SpTileByte;
    u8 SpAttrByte;
    u8 SpPosY;
    u8 SpIdxReload;
} spevalfetchctx_s;

typedef struct placcessor_s placcessor_s;

typedef struct spevalfetchsl_s {
    placcessor_s *accessor_;

    spevalfetchctx_s ctx_;
} spevalfetchsl_s;

void
spevalfetchsl_Init(spevalfetchsl_s *self, placcessor_s *accessor);

void
spevalfetchsl_Tick(spevalfetchsl_s *self, cycle_t col);
