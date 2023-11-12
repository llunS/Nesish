#include "noise.h"

#include "nhassert.h"

// https://www.nesdev.org/wiki/APU_Noise

void
noise_Init(noise_s *self, NHLogger *logger)
{
    envelope_Init(&self->envel_);
    divider_Init(&self->timer_, 0);
    self->shift_ = 0;
    lenctr_Init(&self->len_, logger);

    self->mode_ = false;

    self->logger_ = logger;
}

u8
noise_Amp(const noise_s *self)
{
    if (self->shift_ & 0x0001) {
        return 0;
    }
    if (!lenctr_Val(&self->len_)) {
        return 0;
    }
    return envelope_Volume(&self->envel_);
}

void
noise_TickTimer(noise_s *self)
{
    if (divider_Tick(&self->timer_)) {
        bool otherbit =
            self->mode_ ? self->shift_ & 0x0040 : self->shift_ & 0x0002;
        u16 feedback = (self->shift_ ^ (u16)otherbit) & 0x0001;
        self->shift_ >>= 1;
        self->shift_ = (self->shift_ & ~0x4000) | (feedback << 14);
    }
}

void
noise_TickEnvelope(noise_s *self)
{
    envelope_Tick(&self->envel_);
}

void
noise_TickLenCtr(noise_s *self)
{
    lenctr_Tick(&self->len_);
}

void
noise_SetMode(noise_s *self, bool set)
{
    self->mode_ = set;
}

void
noise_SetTimerReload(noise_s *self, u8 index)
{
    static const u16 lookup[0x10] = {4,   8,   16,  32,  64,  96,   128,  160,
                                     202, 254, 380, 508, 762, 1016, 2034, 4068};
    // static_assert(lookup[0x0F], "Missing elements");

    if (index >= 0x10) {
        ASSERT_FATAL(self->logger_, "Invalid noise timer reload index: " U8FMT,
                     index);
        return;
    }
    divider_SetReload(&self->timer_, lookup[index]);
}

void
noise_ResetLfsr(noise_s *self)
{
    // On power-up, the LFSR is loaded with the value 1.
    // So it will shift in a 1 the first time
    self->shift_ = 1;
}

envelope_s *
noise_Envelope(noise_s *self)
{
    return &self->envel_;
}

lenctr_s *
noise_LenCtr(noise_s *self)
{
    return &self->len_;
}
