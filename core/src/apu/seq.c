#include "seq.h"

#include "nhassert.h"

// https://www.nesdev.org/wiki/APU_Pulse

void
seq_Init(seq_s *self, NHLogger *logger)
{
    self->dutyidx_ = 0;
    self->seqidx_ = 0;
    self->logger_ = logger;
}

bool
seq_Val(const seq_s *self)
{
    static const int seqs[4][8] = {{0, 1, 0, 0, 0, 0, 0, 0},
                                   {0, 1, 1, 0, 0, 0, 0, 0},
                                   {0, 1, 1, 1, 1, 0, 0, 0},
                                   {1, 0, 0, 1, 1, 1, 1, 1}};
    return !!seqs[self->dutyidx_][self->seqidx_];
}

void
seq_Tick(seq_s *self)
{
    if (self->seqidx_ >= 7) {
        self->seqidx_ = 0;
    } else {
        ++self->seqidx_;
    }
}

void
seq_SetDuty(seq_s *self, int index)
{
    if (index > 3 || index < 0) {
        // This is a development-time error.
        ASSERT_FATAL(self->logger_, "Duty index invalid: %d", index);
        return;
    }
    self->dutyidx_ = index;
}

void
seq_Reset(seq_s *self)
{
    self->seqidx_ = 0;
}
