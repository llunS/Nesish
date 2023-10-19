#include "dbgspr.h"

#include <string.h>

void
dbgspr_Init(dbgspr_s *self)
{
    self->is8x16_ = false;
    memset(self->pixels_, 0, sizeof(self->pixels_));
    self->Y = 255;
    self->Tile = 255;
    self->Attr = 255;
    self->X = 255;
}

int
dbgspr_GetWidth(const dbgspr_s *self)
{
    (void)(self);
    return DBGSPR_WIDTH_;
}

int
dbgspr_GetHeight(const dbgspr_s *self)
{
    return self->is8x16_ ? 16 : 8;
}

const u8 *
dbgspr_GetData(const dbgspr_s *self)
{
    // @NOTE: color_s must be of 3 consecutive u8(s)
    return (u8 *)(&self->pixels_[0]);
}

int
dbgspr_PalSet(const dbgspr_s *self)
{
    return self->Attr & 0x03;
}

bool
dbgspr_Bg(const dbgspr_s *self)
{
    return self->Attr & 0x20;
}

bool
dbgspr_FlipX(const dbgspr_s *self)
{
    return self->Attr & 0x40;
}

bool
dbgspr_FlipY(const dbgspr_s *self)
{
    return self->Attr & 0x80;
}

void
dbgspr_SetMode(dbgspr_s *self, bool is8x16)
{
    self->is8x16_ = is8x16;
}

void
dbgspr_SetPixel(dbgspr_s *self, int row, int col, color_s color)
{
    // Save the bound check, leave it to the user.
    self->pixels_[row * DBGSPR_WIDTH_ + col] = color;
}

void
dbgspr_SetRaw(dbgspr_s *self, u8 y, u8 tile, u8 attr, u8 x)
{
    self->Y = y;
    self->Tile = tile;
    self->Attr = attr;
    self->X = x;
}
