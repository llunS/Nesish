#include "envelope.h"

// https://www.nesdev.org/wiki/APU_Envelope

void
envelope_Init(envelope_s *self)
{
    self->start_ = false;
    divider_Init(&self->divider_, 0);
    self->decaylv_ = 0;
    self->loop_ = false;
    self->const_ = false;
    self->constvol_ = 0;
}

u8
envelope_Volume(const envelope_s *self)
{
    if (self->const_) {
        return self->constvol_;
    } else {
        return self->decaylv_;
    }
}

void
envelope_Tick(envelope_s *self)
{
    if (!self->start_) {
        if (divider_Tick(&self->divider_)) {
            if (self->decaylv_) {
                --self->decaylv_;
            } else {
                if (self->loop_) {
                    self->decaylv_ = 15;
                } else {
                    // stay at 0.
                }
            }
        }
    } else {
        self->start_ = false;
        self->decaylv_ = 15;
        divider_Reload(&self->divider_);
    }
}

void
envelope_SetDividerReload(envelope_s *self, u16 reload)
{
    divider_SetReload(&self->divider_, reload);
}

void
envelope_SetLoop(envelope_s *self, bool set)
{
    self->loop_ = set;
}

void
envelope_SetConst(envelope_s *self, bool set)
{
    self->const_ = set;
}

void
envelope_SetConstVol(envelope_s *self, u8 vol)
{
    self->constvol_ = vol;
}

void
envelope_Restart(envelope_s *self)
{
    self->start_ = true;
}
