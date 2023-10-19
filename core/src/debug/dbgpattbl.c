#include "dbgpattbl.h"

#include <string.h>

void
dbgpattble_Init(dbgpattbl_s *self)
{
    memset(self->pixels_, 0, sizeof(self->pixels_));
}

const u8 *
dbgpattble_GetData(const dbgpattbl_s *self)
{
    // @NOTE: color_s must be of 3 consecutive u8(s)
    return (u8 *)(&self->pixels_[0]);
}

void
dbgpattble_SetPixel(dbgpattbl_s *self, int tileidx, int finey, int finex,
                    color_s color)
{
    // Save the bound check, leave it to the user.
    int tiley = tileidx / DBGPATTBL_TILES_W;
    int tilex = tileidx % DBGPATTBL_TILES_W;
    int row = tiley * DBGPATTBL_TILE_H + finey;
    int col = tilex * DBGPATTBL_TILE_W + finex;
    self->pixels_[row * dbgpattble_GetWidth(self) + col] = color;
}
