#include "sweep.h"

// https://www.nesdev.org/wiki/APU_Sweep

static u16
target_reload(const sweep_s *self);

void
sweep_Init(sweep_s *self, divider_s *chtimer, bool mode1)
{
    divider_Init(&self->divider_, 0);
    self->reload_ = false;

    self->enabled_ = false;
    self->negate_ = false;
    self->shift_ = 0;

    self->chtimer_ = chtimer;
    self->mode1_ = mode1;
}

bool
sweep_Muted(const sweep_s *self, u16 *target)
{
    if (divider_GetReload(self->chtimer_) < 8)
    {
        return true;
    }
    u16 val = target ? *target : target_reload(self);
    if (val > 0x07FF)
    {
        return true;
    }
    return false;
}

void
sweep_Tick(sweep_s *self)
{
    // Tick as normal first, then check reload.
    // This means if the divider was 0 before the reload, the period adjustment
    // check applies as normal.
    // https://archive.nes.science/nesdev-forums/f3/t11083.xhtml
    // https://archive.nes.science/nesdev-forums/f2/t19285.xhtml

    if (divider_Tick(&self->divider_))
    {
        if (self->enabled_ && self->shift_)
        {
            u16 target = target_reload(self);
            if (!sweep_Muted(self, &target))
            {
                // update the channel's period.
                divider_SetReload(self->chtimer_, target);
            }
        }
    }

    if (self->reload_)
    {
        // Do this even if it may have been reloaded already in the above
        // tick(), for it doesn't matter.
        divider_Reload(&self->divider_);
    }
    self->reload_ = false; // clear the flag
}

void
sweep_SetEnabled(sweep_s *self, bool set)
{
    self->enabled_ = set;
}

void
sweep_SetDividerReload(sweep_s *self, u16 reload)
{
    divider_SetReload(&self->divider_, reload);
}

void
sweep_SetNegate(sweep_s *self, bool set)
{
    self->negate_ = set;
}

void
sweep_SetShiftCount(sweep_s *self, u8 count)
{
    self->shift_ = count;
}

void
sweep_Reload(sweep_s *self)
{
    self->reload_ = true;
}

u16
target_reload(const sweep_s *self)
{
    u16 curr = divider_GetReload(self->chtimer_);
    u16 delta = curr >> self->shift_;
    if (self->mode1_)
    {
        delta = -delta - 1;
    }
    else
    {
        delta = -delta;
    }
    u16 target = curr + delta;
    return target;
}
