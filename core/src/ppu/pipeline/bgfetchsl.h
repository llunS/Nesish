#pragma once

#include "types.h"

typedef struct placcessor_s placcessor_s;

typedef struct bgfetchsl_s {
    placcessor_s *accessor_;
} bgfetchsl_s;

void
bgfetchsl_Init(bgfetchsl_s *self, placcessor_s *accessor);

void
bgfetchsl_Tick(bgfetchsl_s *self, cycle_t col);
