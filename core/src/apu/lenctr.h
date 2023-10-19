#pragma once

#include "types.h"

typedef struct NHLogger NHLogger;

typedef struct lenctr_s {
    u8 ctr_;
    bool halt_;
    bool enabled_;

    bool toSetHalt_;
    bool haltval_;
    bool toload_;
    u8 loadval_;

    NHLogger *logger_;
} lenctr_s;

void
lenctr_Init(lenctr_s *self, NHLogger *logger);

u8
lenctr_Val(const lenctr_s *self);

void
lenctr_Tick(lenctr_s *self);

void
lenctr_Powerup(lenctr_s *self);

void
lenctr_PostSetHalt(lenctr_s *self, bool set);
void
lenctr_FlushHaltSet(lenctr_s *self);
void
lenctr_SetEnabled(lenctr_s *self, bool set);
void
lenctr_PostSetLoad(lenctr_s *self, u8 index);
void
lenctr_FlushLoadSet(lenctr_s *self);
