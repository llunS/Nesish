#include "dmc.h"

#include "nhassert.h"

#include "apu/dmcdma.h"

// https://www.nesdev.org/wiki/APU_DMC

static void
restartPlayback(dmc_s *self);

void
dmc_Init(dmc_s *self, dmcdma_s *dmcdma, NHLogger *logger)
{
    self->dmcdma_ = dmcdma;

    self->irqEnabled_ = false;
    self->loop_ = false;
    self->sampleaddr_ = 0;
    self->samplelen_ = 0;

    divider_Init(&self->timer_, 0);
    self->shift_ = 0;
    self->bitsleft_ = 8;
    self->level_ = 0;
    self->silence_ = true;

    self->samplebuf_ = 0;
    self->samplebufEmpty_ = true;

    self->samplecur_ = 0;
    self->sampleBytesLeft_ = 0;

    self->irq_ = false;

    self->logger_ = logger;
}

u8
dmc_Amp(const dmc_s *self)
{
    return self->level_;
}

bool
dmc_Irq(const dmc_s *self)
{
    return self->irq_;
}

void
dmc_TickTimer(dmc_s *self)
{
    /* Sample fetch */
    // The sample fetch is done by Memory Reader (DMA), which runs independently
    // of the Output Unit.

    // @TEST: Not certain about this
    // When sample fetch done by DMA and silence check below occur at the same
    // cycle, Ticking DMA first plays the fetched sample sooner by one output
    // cycle than it would do otherwise.

    /* Output Unit */
    // i.e. playback of samples
    if (divider_Tick(&self->timer_)) {
        if (!self->silence_) {
            if (self->shift_ & 0x01) {
                if (self->level_ + 2 <= 127) {
                    self->level_ += 2;
                }
            } else {
                if (self->level_ >= 2) {
                    self->level_ -= 2;
                }
            }
        }

        self->shift_ >>= 1;

        if (self->bitsleft_ <= 0) {
            ASSERT_FATAL(self->logger_, "Invalid bits remaining value " U8FMT,
                         self->bitsleft_);
        }
        // let it overflow to make the bug apparent.
        --self->bitsleft_;
        /* new output cycle */
        if (!self->bitsleft_) {
            self->bitsleft_ = 8;

            if (self->samplebufEmpty_) {
                self->silence_ = true;
            } else {
                self->silence_ = false;

                self->shift_ = self->samplebuf_;
                self->samplebufEmpty_ = true;

                // Check to do reload DMA
                // No need to check "self->samplebufEmpty_" in this block
                if (/*self->samplebufEmpty_ &&*/ self->sampleBytesLeft_ > 0) {
                    dmcdma_Initiate(self->dmcdma_, self->samplecur_, true);
                }
            }
        }
    }
}

void
dmc_SetIrqEnabled(dmc_s *self, bool set)
{
    self->irqEnabled_ = set;
    if (!set) {
        self->irq_ = false;
    }
}

void
dmc_SetLoop(dmc_s *self, bool set)
{
    self->loop_ = set;
}

void
dmc_SetTimerReload(dmc_s *self, u8 index)
{
    static const u16 periods[0x10] = {
        428 / 2, 380 / 2, 340 / 2, 320 / 2, 286 / 2, 254 / 2, 226 / 2, 214 / 2,
        190 / 2, 160 / 2, 142 / 2, 128 / 2, 106 / 2, 84 / 2,  72 / 2,  54 / 2};
    // static_assert(periods[0x0F], "Missing elements");

    if (index >= 0x10) {
        ASSERT_FATAL(self->logger_, "Invalid dmc timer reload index: " U8FMT,
                     index);
        return;
    }
    u16 reload = periods[index] - 1;
    divider_SetReload(&self->timer_, reload);
}

void
dmc_Load(dmc_s *self, u8 val)
{
    self->level_ = val;
}

void
dmc_SetSampleAddr(dmc_s *self, u8 sampleaddr)
{
    self->sampleaddr_ = 0xC000 + (sampleaddr << 6);
}

void
dmc_SetSampleLen(dmc_s *self, u8 samplelen)
{
    self->samplelen_ = (samplelen << 4) + 1;
}

void
dmc_SetEnabled(dmc_s *self, bool set)
{
    if (!set) {
        self->sampleBytesLeft_ = 0;
    } else {
        // Restart only if there's no bytes left to load
        if (!self->sampleBytesLeft_) {
            restartPlayback(self);

            // Load DMA only if we restarts, it looks right since
            // remaining bytes could be left to reload DMA.
            if (self->samplebufEmpty_ && self->sampleBytesLeft_ > 0) {
                dmcdma_Initiate(self->dmcdma_, self->samplecur_, false);
            }
        }
    }
}

void
dmc_ClearIrq(dmc_s *self)
{
    self->irq_ = false;
}

bool
dmc_BytesRemained(const dmc_s *self)
{
    return self->sampleBytesLeft_;
}

void
dmc_PutSample(dmc_s *self, addr_t sampleaddr, u8 sample)
{
    // Due to DMA costing time, the DMC may be disabled (i.e.
    // self->sampleBytesLeft_ == 0) after DMA has been initiated.
    // Check address as well in case it's stale request (IF there exists this
    // kind).
    if (sampleaddr == self->samplecur_ && self->sampleBytesLeft_ > 0) {
        self->samplebuf_ = sample;
        self->samplebufEmpty_ = false;

        if (self->samplecur_ >= 0xFFFF) {
            self->samplecur_ = 0x8000;
        } else {
            ++self->samplecur_;
        }

        --self->sampleBytesLeft_;
        if (!self->sampleBytesLeft_) {
            if (self->loop_) {
                restartPlayback(self);
            } else {
                if (self->irqEnabled_) {
                    self->irq_ = true;
                }
            }
        }
    } else {
        // Failed due to address mismatch (if any)
        // if (self->sampleBytesLeft_ <= 0)
        {
            // Discard previously initiated DMA result
        }

        // else if (sampleaddr != self->samplecur_)
        {
            // In current implementation, no way sampleaddr > self->samplecur_,
            // so it must be that self->samplecur_ < self->samplecur_. Then it's
            // a stale DMA we are happy to ignore (if this exists). The bytes
            // left counter must have been updated already, i.e. we are free of
            // possible stuck due to counter not being updated.
        }
    }
}

void
restartPlayback(dmc_s *self)
{
    self->samplecur_ = self->sampleaddr_;
    self->sampleBytesLeft_ = self->samplelen_;
}
