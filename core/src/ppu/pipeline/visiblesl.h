#pragma once

#include "ppu/pipeline/bgfetchsl.h"
#include "ppu/pipeline/spevalfetchsl.h"
#include "ppu/pipeline/rendersl.h"
#include "types.h"

typedef struct placcessor_s placcessor_s;

typedef struct visiblesl_s {
    rendersl_s render_;
    bgfetchsl_s bg_;
    spevalfetchsl_s sp_;
} visiblesl_s;

void
visiblesl_Init(visiblesl_s *self, placcessor_s *accessor);

void
visiblesl_Tick(visiblesl_s *self, cycle_t col);
