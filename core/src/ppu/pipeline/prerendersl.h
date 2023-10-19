#pragma once

#include "ppu/pipeline/bgfetchsl.h"
#include "ppu/pipeline/spevalfetchsl.h"
#include "types.h"

typedef struct placcessor_s placcessor_s;

typedef struct prerendersl_s {
    placcessor_s *accessor_;

    bgfetchsl_s bg_;
    spevalfetchsl_s sp_;
} prerendersl_s;

void
prerendersl_Init(prerendersl_s *self, placcessor_s *accessor);

void
prerendersl_Tick(prerendersl_s *self, cycle_t col);
