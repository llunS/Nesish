#pragma once

#include "apu/envelope.h"
#include "apu/sweep.h"
#include "apu/divider.h"
#include "apu/seq.h"
#include "apu/lenctr.h"

#include "types.h"

typedef struct NHLogger NHLogger;

typedef struct pulse_s {
    envelope_s envel_;
    sweep_s sweep_;
    divider_s timer_;
    seq_s seq_;
    lenctr_s len_;
} pulse_s;

void
pulse_Init(pulse_s *self, bool mode1, NHLogger *logger);

/// @return Amplitude in range of [0, 15]
u8
pulse_Amp(const pulse_s *self);

void
pulse_TickTimer(pulse_s *self);
void
pulse_TickEnvelope(pulse_s *self);
void
pulse_TickSweep(pulse_s *self);
void
pulse_TickLenCtr(pulse_s *self);

envelope_s *
pulse_Envelope(pulse_s *self);
sweep_s *
pulse_Sweep(pulse_s *self);
divider_s *
pulse_Timer(pulse_s *self);
seq_s *
pulse_Seq(pulse_s *self);
lenctr_s *
pulse_LenCtr(pulse_s *self);
