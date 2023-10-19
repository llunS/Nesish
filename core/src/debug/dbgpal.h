#pragma once

#include "nesish/nesish.h"
#include "types.h"

typedef struct dbgpal_s {
    NHDColor colors_[NHD_PALETTE_COLORS];
} dbgpal_s;

void
dbgpal_Init(dbgpal_s *self);

NHDColor
dbgpal_GetColor(const dbgpal_s *self, int idx);

void
dbgpal_SetColor(dbgpal_s *self, int idx, u8 index, u8 r, u8 g, u8 b);
