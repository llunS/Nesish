#pragma once

#include "types.h"

typedef struct NHLogger NHLogger;

/// @brief Sequencer for pulse channel
typedef struct seq_s {
    int dutyidx_;
    int seqidx_;

    NHLogger *logger_;
} seq_s;

void
seq_Init(seq_s *self, NHLogger *logger);

bool
seq_Val(const seq_s *self);

void
seq_Tick(seq_s *self);

void
seq_SetDuty(seq_s *self, int index);
void
seq_Reset(seq_s *self);
