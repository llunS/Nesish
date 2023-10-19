#pragma once

#include "ppu/pipeline/prerendersl.h"
#include "ppu/pipeline/visiblesl.h"

typedef struct placcessor_s placcessor_s;

typedef struct pl_s {
    placcessor_s *accessor_;

    prerendersl_s prerendersl_;
    visiblesl_s visiblesl_;

    int currslIdx_;
    int currslCol_;
} pl_s;

void
pl_Init(pl_s *self, placcessor_s *accessor);

void
pl_Reset(pl_s *self);

void
pl_Tick(pl_s *self);
