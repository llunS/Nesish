#include "tri.h"

// https://www.nesdev.org/wiki/APU_Triangle

#define SEQ_SIZE 32

void
tri_Init(tri_s *self, NHLogger *logger)
{
    divider_Init(&self->timer_, 0);
    linctr_Init(&self->lin_);
    lenctr_Init(&self->len_, logger);
}

u8
tri_Amp(const tri_s *self)
{
    return self->amp_;
}

void
tri_Powerup(tri_s *self)
{
    lenctr_Powerup(&self->len_);

    self->amp_ = 0;
    self->seqidx_ = 0;
}

void
tri_Reset(tri_s *self)
{
    self->seqidx_ = 0;
}

void
tri_TickTimer(tri_s *self)
{
    if (divider_Tick(&self->timer_)) {
        if (linctr_Val(&self->lin_) && lenctr_Val(&self->len_)) {
            static const u8 seqs[SEQ_SIZE] = {
                15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5,  4,  3,  2,  1,  0,
                0,  1,  2,  3,  4,  5,  6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
            // static_assert(seqs[SEQ_SIZE - 1], "Missing elements");
            self->amp_ = seqs[self->seqidx_];

            if (self->seqidx_ >= SEQ_SIZE - 1) {
                self->seqidx_ = 0;
            } else {
                ++self->seqidx_;
            }
        }
    }
}

void
tri_TickLinCtr(tri_s *self)
{
    linctr_Tick(&self->lin_);
}

void
tri_TickLenCtr(tri_s *self)
{
    lenctr_Tick(&self->len_);
}

divider_s *
tri_Timer(tri_s *self)
{
    return &self->timer_;
}

linctr_s *
tri_LinCtr(tri_s *self)
{
    return &self->lin_;
}

lenctr_s *
tri_LenCtr(tri_s *self)
{
    return &self->len_;
}
