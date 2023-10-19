#pragma once

#include "types.h"

/// @brief A struct to sync get/put cycle for APU and DMA(OAM/DMC)
typedef struct apuclock_s {
    // This may wrap around back to 0, which is fine, since current
    // implementation doesn't assume infinite range.
    cycle_t cycle_;
} apuclock_s;

void
apuclock_Init(apuclock_s *self);

void
apuclock_Tick(apuclock_s *self);

void
apuclock_Powerup(apuclock_s *self);
void
apuclock_Reset(apuclock_s *self);

bool
apuclock_Get(const apuclock_s *self);
bool
apuclock_Put(const apuclock_s *self);

bool
apuclock_Even(const apuclock_s *self);
bool
apuclock_Odd(const apuclock_s *self);
