#pragma once

#include "ppu/color.h"
#include "types.h"

#define DBGSPR_WIDTH_ 8

typedef struct dbgspr_s {
    u8 Y;
    u8 Tile;
    u8 Attr;
    u8 X;

    bool is8x16_;
    color_s pixels_[DBGSPR_WIDTH_ * 16]; // up to 8x16
} dbgspr_s;

void
dbgspr_Init(dbgspr_s *self);

int
dbgspr_GetWidth(const dbgspr_s *self);
int
dbgspr_GetHeight(const dbgspr_s *self);
const u8 *
dbgspr_GetData(const dbgspr_s *self);

int
dbgspr_PalSet(const dbgspr_s *self);
bool
dbgspr_Bg(const dbgspr_s *self);
bool
dbgspr_FlipX(const dbgspr_s *self);
bool
dbgspr_FlipY(const dbgspr_s *self);

void
dbgspr_SetMode(dbgspr_s *self, bool is8x16);
void
dbgspr_SetPixel(dbgspr_s *self, int row, int col, color_s color);
void
dbgspr_SetRaw(dbgspr_s *self, u8 y, u8 tile, u8 attr, u8 x);
