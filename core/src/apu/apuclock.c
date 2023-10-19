#include "apuclock.h"

void
apuclock_Init(apuclock_s *self)
{
    self->cycle_ = 0;
}

void
apuclock_Tick(apuclock_s *self)
{
    ++self->cycle_;
}

void
apuclock_Powerup(apuclock_s *self)
{
    self->cycle_ = 0;
}

void
apuclock_Reset(apuclock_s *self)
{
    (void)(self);
    // do nothing
}

bool
apuclock_Get(const apuclock_s *self)
{
    return apuclock_Even(self);
}

bool
apuclock_Put(const apuclock_s *self)
{
    return apuclock_Odd(self);
}

bool
apuclock_Even(const apuclock_s *self)
{
    return self->cycle_ % 2 == 0;
}

bool
apuclock_Odd(const apuclock_s *self)
{
    return !apuclock_Even(self);
}
