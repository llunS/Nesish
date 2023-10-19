#pragma once

#include "types.h"

typedef struct pulse_s pulse_s;
typedef struct tri_s tri_s;
typedef struct noise_s noise_s;

typedef struct frmctr_s {
    pulse_s *pulse1_;
    pulse_s *pulse2_;
    tri_s *triangle_;
    noise_s *noise_;

    unsigned int timer_;
    bool irq_;

    bool mode_;
    bool irqInhibit_;

    bool firstloop_;
    unsigned char resetCtr_;
    bool modeTmp_;
} frmctr_s;

void
frmctr_Init(frmctr_s *self, pulse_s *pulse1, pulse_s *pulse2, tri_s *triangle,
            noise_s *noise);

void
frmctr_Powerup(frmctr_s *self);

/// @brief Tick this every CPU cycle
void
frmctr_Tick(frmctr_s *self);

bool
frmctr_Irq(const frmctr_s *self);

void
frmctr_ResetTimer(frmctr_s *self);

void
frmctr_SetIrqInhibit(frmctr_s *self, bool set);
void
frmctr_DelaySetMode(frmctr_s *self, bool mode, bool delay);
void
frmctr_ClearIrq(frmctr_s *self);
