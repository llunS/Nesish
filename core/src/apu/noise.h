#pragma once

#include "apu/envelope.h"
#include "apu/divider.h"
#include "apu/lenctr.h"

#include "types.h"

typedef struct NHLogger NHLogger;

typedef struct noise_s {
    envelope_s envel_;
    divider_s timer_;
    u16 shift_;
    lenctr_s len_;

    bool mode_;

    NHLogger *logger_;
} noise_s;

void
noise_Init(noise_s *self, NHLogger *logger);

/// @return Amplitude in range of [0, 15]
u8
noise_Amp(const noise_s *self);

void
noise_TickTimer(noise_s *self);
void
noise_TickEnvelope(noise_s *self);
void
noise_TickLenCtr(noise_s *self);

void
noise_ResetLfsr(noise_s *self);
void
noise_SetMode(noise_s *self, bool set);
void
noise_SetTimerReload(noise_s *self, u8 index);

envelope_s *
noise_Envelope(noise_s *self);
lenctr_s *
noise_LenCtr(noise_s *self);
