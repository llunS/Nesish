#include "divider.h"

void
divider_Init(divider_s *self, u16 reload)
{
    self->reload_ = reload;
    self->ctr_ = reload;
}

u16
divider_GetReload(const divider_s *self)
{
    return self->reload_;
}

void
divider_SetReload(divider_s *self, u16 reload)
{
    self->reload_ = reload;
}

void
divider_Reload(divider_s *self)
{
    self->ctr_ = self->reload_;
}

bool
divider_Tick(divider_s *self)
{
    if (!self->ctr_) {
        divider_Reload(self);
        return true;
    } else {
        --self->ctr_;
        return false;
    }
}
