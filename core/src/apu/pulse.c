#include "pulse.h"

// https://www.nesdev.org/wiki/APU_Pulse

void
pulse_Init(pulse_s *self, bool mode1, NHLogger *logger)
{
    envelope_Init(&self->envel_);
    sweep_Init(&self->sweep_, &self->timer_, mode1);
    divider_Init(&self->timer_, 0);
    seq_Init(&self->seq_, logger);
    lenctr_Init(&self->len_, logger);
}

u8
pulse_Amp(const pulse_s *self)
{
    if (!seq_Val(&self->seq_))
    {
        return 0;
    }
    if (sweep_Muted(&self->sweep_, NULL))
    {
        return 0;
    }
    if (!lenctr_Val(&self->len_))
    {
        return 0;
    }
    return envelope_Volume(&self->envel_);
}

void
pulse_TickTimer(pulse_s *self)
{
    if (divider_Tick(&self->timer_))
    {
        seq_Tick(&self->seq_);
    }
}

void
pulse_TickEnvelope(pulse_s *self)
{
    envelope_Tick(&self->envel_);
}

void
pulse_TickSweep(pulse_s *self)
{
    sweep_Tick(&self->sweep_);
}

void
pulse_TickLenCtr(pulse_s *self)
{
    lenctr_Tick(&self->len_);
}

envelope_s *
pulse_Envelope(pulse_s *self)
{
    return &self->envel_;
}

divider_s *
pulse_Timer(pulse_s *self)
{
    return &self->timer_;
}

sweep_s *
pulse_Sweep(pulse_s *self)
{
    return &self->sweep_;
}

seq_s *
pulse_Seq(pulse_s *self)
{
    return &self->seq_;
}

lenctr_s *
pulse_LenCtr(pulse_s *self)
{
    return &self->len_;
}
