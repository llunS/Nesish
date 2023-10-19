#include "linctr.h"

/* Basically like length counter, but with a higher resolution (quarter frame
 * instead of half frame). */
// https://www.nesdev.org/wiki/APU_Triangle

void
linctr_Init(linctr_s *self)
{
    self->counter_ = 0;
    self->control_ = false;
    self->reload_ = false;
    self->reloadval_ = 0;
}

u8
linctr_Val(const linctr_s *self)
{
    return self->counter_;
}

void
linctr_Tick(linctr_s *self)
{
    if (self->reload_)
    {
        self->counter_ = self->reloadval_;
    }
    else
    {
        if (self->counter_ > 0)
        {
            --self->counter_;
        }
    }

    if (!self->control_)
    {
        self->reload_ = false;
    }
}

void
linctr_SetCtrl(linctr_s *self, bool set)
{
    self->control_ = set;
}

void
linctr_SetReload(linctr_s *self)
{
    self->reload_ = true;
}

void
linctr_SetReloadVal(linctr_s *self, u8 reload)
{
    self->reloadval_ = reload;
}
