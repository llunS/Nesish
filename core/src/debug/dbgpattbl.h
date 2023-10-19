#pragma once

#include "ppu/color.h"
#include "types.h"

#define DBGPATTBL_TILES_W 16
#define DBGPATTBL_TILES_H 16
#define DBGPATTBL_TILE_W 8
#define DBGPATTBL_TILE_H 8

#define DBGPATTBL_TILES (DBGPATTBL_TILES_W * DBGPATTBL_TILES_H)

typedef struct dbgpattbl_s {
    // 16x16 tiles each with 8x8 pixels
    color_s pixels_[(DBGPATTBL_TILES_W * DBGPATTBL_TILES_H) *
                    (DBGPATTBL_TILE_W * DBGPATTBL_TILE_H)];
} dbgpattbl_s;

void
dbgpattble_Init(dbgpattbl_s *self);

inline int
dbgpattble_GetWidth(const dbgpattbl_s *self)
{
    (void)(self);
    return DBGPATTBL_TILES_W * DBGPATTBL_TILE_W;
}
inline int
dbgpattble_GetHeight(const dbgpattbl_s *self)
{
    (void)(self);
    return DBGPATTBL_TILES_H * DBGPATTBL_TILE_H;
}
const u8 *
dbgpattble_GetData(const dbgpattbl_s *self);

void
dbgpattble_SetPixel(dbgpattbl_s *self, int tileidx, int finey, int finex,
                    color_s color);
