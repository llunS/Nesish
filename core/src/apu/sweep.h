#pragma once

#include "apu/divider.h"
#include "types.h"

typedef struct sweep_s {
    divider_s divider_;
    bool reload_;

    bool enabled_;
    bool negate_;
    u8 shift_;

    divider_s *chtimer_; // the channel timer.
    /* clang-format off */
    // Pulse 1 adds the ones' complement (-c - 1). Making 20 negative produces a change amount of -21.
    // Pulse 2 adds the two's complement (-c). Making 20 negative produces a change amount of -20.
    /* clang-format on */
    bool mode1_;
} sweep_s;

void
sweep_Init(sweep_s *self, divider_s *chtimer, bool mode1);

bool
sweep_Muted(const sweep_s *self, u16 *target);

void
sweep_Tick(sweep_s *self);

void
sweep_SetEnabled(sweep_s *self, bool set);
void
sweep_SetDividerReload(sweep_s *self, u16 reload);
void
sweep_SetNegate(sweep_s *self, bool set);
void
sweep_SetShiftCount(sweep_s *self, u8 count);
void
sweep_Reload(sweep_s *self);
