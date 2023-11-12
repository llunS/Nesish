#include "lenctr.h"

#include "nhassert.h"

// https://www.nesdev.org/wiki/APU_Length_Counter

static void
checkLoad(lenctr_s *self, u8 index);

void
lenctr_Init(lenctr_s *self, NHLogger *logger)
{
    self->ctr_ = 0;
    self->halt_ = false;
    self->enabled_ = false;
    self->logger_ = logger;
}

u8
lenctr_Val(const lenctr_s *self)
{
    return self->ctr_;
}

void
lenctr_Tick(lenctr_s *self)
{
    if (self->halt_) {
        return;
    }
    if (self->ctr_ > 0) {
        --self->ctr_;
        // Length reload is completely ignored if written during length
        // clocking and length counter is non-zero before clocking
        self->toload_ = false;
    }
}

/// @brief Reset some states required in our implementation rather than in
/// actual hardware
void
lenctr_Powerup(lenctr_s *self)
{
    self->toSetHalt_ = false;
    self->toload_ = false;
}

void
lenctr_PostSetHalt(lenctr_s *self, bool set)
{
    self->toSetHalt_ = true;
    self->haltval_ = set;
}

void
lenctr_FlushHaltSet(lenctr_s *self)
{
    if (self->toSetHalt_) {
        self->halt_ = self->haltval_;
        self->toSetHalt_ = false;
    }
}

void
lenctr_SetEnabled(lenctr_s *self, bool set)
{
    self->enabled_ = set;
    if (!set) {
        self->ctr_ = 0;
    }
}

void
lenctr_PostSetLoad(lenctr_s *self, u8 index)
{
    self->toload_ = true;
    self->loadval_ = index;
}

void
lenctr_FlushLoadSet(lenctr_s *self)
{
    if (self->toload_) {
        checkLoad(self, self->loadval_);
        self->toload_ = false;
    }
}

void
checkLoad(lenctr_s *self, u8 index)
{
    if (!self->enabled_) {
        return;
    }

    static const u8 lentable[32] = {
        10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
        12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30};
    // static_assert(lentable[31], "Elements missing");

    if (index >= 32) {
        // This is a development-time error.
        ASSERT_FATAL(self->logger_, "Length counter index value: " U8FMT,
                     index);
        return;
    }
    self->ctr_ = lentable[index];
}
