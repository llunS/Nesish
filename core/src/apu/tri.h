#pragma once

#include "apu/divider.h"
#include "apu/linctr.h"
#include "apu/lenctr.h"

#include "types.h"

typedef struct NHLogger NHLogger;

/// @brief Triangle channel
typedef struct tri_s {
    divider_s timer_;
    linctr_s lin_;
    lenctr_s len_;

    u8 amp_;
    // Don't bother to make a sequencer class
    unsigned int seqidx_;
} tri_s;

void
tri_Init(tri_s *self, NHLogger *logger);

/// @return Amplitude in range of [0, 15]
u8
tri_Amp(const tri_s *self);

void
tri_Powerup(tri_s *self);
void
tri_Reset(tri_s *self);

void
tri_TickTimer(tri_s *self);
void
tri_TickLinCtr(tri_s *self);
void
tri_TickLenCtr(tri_s *self);

divider_s *
tri_Timer(tri_s *self);
linctr_s *
tri_LinCtr(tri_s *self);
lenctr_s *
tri_LenCtr(tri_s *self);
