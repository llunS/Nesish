#pragma once

#include "types.h"

typedef struct divider_s {
    u16 reload_;
    u16 ctr_;
} divider_s;

void
divider_Init(divider_s *self, u16 reload);

u16
divider_GetReload(const divider_s *self);
void
divider_SetReload(divider_s *self, u16 reload);
void
divider_Reload(divider_s *self);

bool
divider_Tick(divider_s *self);
