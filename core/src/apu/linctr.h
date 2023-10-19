#pragma once

#include "types.h"

typedef struct linctr_s {
    u8 counter_;
    bool control_;
    bool reload_;
    u8 reloadval_;
} linctr_s;

void
linctr_Init(linctr_s *self);

u8
linctr_Val(const linctr_s *self);

void
linctr_Tick(linctr_s *self);

void
linctr_SetCtrl(linctr_s *self, bool set);
void
linctr_SetReload(linctr_s *self);
void
linctr_SetReloadVal(linctr_s *self, u8 reload);
