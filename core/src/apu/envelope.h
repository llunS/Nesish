#pragma once

#include "types.h"
#include "apu/divider.h"

typedef struct envelope_s {
    bool start_; // start flag
    divider_s divider_;
    u8 decaylv_; // [0, 15]

    bool loop_;
    bool const_;
    u8 constvol_;
} envelope_s;

void
envelope_Init(envelope_s *self);

/// @return Volume in range of [0, 15]
u8
envelope_Volume(const envelope_s *self);

void
envelope_Tick(envelope_s *self);

void
envelope_SetDividerReload(envelope_s *self, u16 reload);
void
envelope_SetLoop(envelope_s *self, bool set);
void
envelope_SetConst(envelope_s *self, bool set);
void
envelope_SetConstVol(envelope_s *self, u8 vol);
void
envelope_Restart(envelope_s *self);
