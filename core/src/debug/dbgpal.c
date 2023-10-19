#include "dbgpal.h"

#include <string.h>

void
dbgpal_Init(dbgpal_s *self)
{
    memset(self->colors_, 0, sizeof(self->colors_));
}

NHDColor
dbgpal_GetColor(const dbgpal_s *self, int idx)
{
    return self->colors_[idx];
}

void
dbgpal_SetColor(dbgpal_s *self, int idx, u8 index, u8 r, u8 g, u8 b)
{
    self->colors_[idx].index = index;
    self->colors_[idx].r = r;
    self->colors_[idx].g = g;
    self->colors_[idx].b = b;
}
