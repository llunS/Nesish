#include "frmctr.h"

#include "apu/pulse.h"
#include "apu/tri.h"
#include "apu/noise.h"

// https://www.nesdev.org/wiki/APU_Frame_Counter

static void
tickLengthCounterAndSweep(frmctr_s *self);
static void
tickEnvelopeAndLinearCounter(frmctr_s *self);

void
frmctr_Init(frmctr_s *self, pulse_s *pulse1, pulse_s *pulse2, tri_s *triangle,
            noise_s *noise)
{
    self->pulse1_ = pulse1;
    self->pulse2_ = pulse2;
    self->triangle_ = triangle;
    self->noise_ = noise;
}

/// @brief Reset some states required in our implementation rather than in
/// actual hardware
void
frmctr_Powerup(frmctr_s *self)
{
    self->timer_ = 0; // set again later due to register write powerup
    self->irq_ = false;

    self->mode_ = false;       // set again later due to register write powerup
    self->irqInhibit_ = false; // set again later due to register write powerup

    self->firstloop_ = true;
    self->resetCtr_ = 0;
    // self->modeTmp_: doesn't matter
}

static void
checkSetIrq(frmctr_s *self)
{
    if (!self->mode_ && !self->irqInhibit_)
    {
        self->irq_ = true;
    }
};

void
frmctr_Tick(frmctr_s *self)
{
    /* delay reset logic */
    if (self->resetCtr_)
    {
        --self->resetCtr_;
        if (!self->resetCtr_)
        {
            self->mode_ = self->modeTmp_;
            self->timer_ = 0;
            self->firstloop_ = true;
        }
    }

    // Check details of the following timing in blargg_apu_2005.07.30/readme.txt
    switch (self->timer_)
    {
        case 7458:
        {
            tickEnvelopeAndLinearCounter(self);
        }
        break;

        case 14914:
        {
            tickEnvelopeAndLinearCounter(self);
            tickLengthCounterAndSweep(self);
        }
        break;

        case 22372:
        {
            tickEnvelopeAndLinearCounter(self);
        }
        break;

        case 29829:
        {
            checkSetIrq(self);
        }
        break;
        case 0:
        {
            if (!self->mode_)
            {
                if (!self->firstloop_)
                {
                    tickEnvelopeAndLinearCounter(self);
                    tickLengthCounterAndSweep(self);

                    checkSetIrq(self);
                }
            }
            else
            {
                tickEnvelopeAndLinearCounter(self);
                tickLengthCounterAndSweep(self);
            }
        }
        break;
        case 1:
        {
            if (!self->firstloop_)
            {
                checkSetIrq(self);
            }
        }
        break;

        default:
            break;
    }

    // Update timer to next value
    ++self->timer_;
    if (!self->mode_)
    {
        if (self->timer_ >= 29830)
        {
            self->timer_ = 0;
            self->firstloop_ = false;
        }
    }
    else
    {
        if (self->timer_ >= 37282)
        {
            self->timer_ = 0;
            self->firstloop_ = false;
        }
    }
}

bool
frmctr_Irq(const frmctr_s *self)
{
    return self->irq_;
}

void
frmctr_ResetTimer(frmctr_s *self)
{
    self->timer_ = 0;
}

void
frmctr_SetIrqInhibit(frmctr_s *self, bool set)
{
    self->irqInhibit_ = set;
    if (set)
    {
        self->irq_ = false;
    }
}

void
frmctr_DelaySetMode(frmctr_s *self, bool mode, bool delay)
{
    // call this while one is pending would just replace the pending one

    static const unsigned char tickorder = 1; // ticked after CPU
    static const unsigned char preDec = 1;    // check tick()
    unsigned char delayval = delay;
    self->modeTmp_ = mode;
    self->resetCtr_ = tickorder + preDec + delayval;
}

void
frmctr_ClearIrq(frmctr_s *self)
{
    self->irq_ = false;
}

void
tickLengthCounterAndSweep(frmctr_s *self)
{
    pulse_TickLenCtr(self->pulse1_);
    pulse_TickSweep(self->pulse1_);

    pulse_TickLenCtr(self->pulse2_);
    pulse_TickSweep(self->pulse2_);

    tri_TickLenCtr(self->triangle_);

    noise_TickLenCtr(self->noise_);
}

void
tickEnvelopeAndLinearCounter(frmctr_s *self)
{
    pulse_TickEnvelope(self->pulse1_);

    pulse_TickEnvelope(self->pulse2_);

    tri_TickLinCtr(self->triangle_);

    noise_TickEnvelope(self->noise_);
}
