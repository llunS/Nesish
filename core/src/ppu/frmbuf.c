#include "frmbuf.h"

#include <stdlib.h>

bool
frmbuf_Init(frmbuf_s *self)
{
    self->buf_ = calloc(FRMBUF_WIDTH * FRMBUF_HEIGHT, sizeof(color_s));
    if (!self->buf_)
    {
        return false;
    }
    return true;
}

void
frmbuf_Deinit(frmbuf_s *self)
{
    if (self->buf_)
    {
        free(self->buf_);
        self->buf_ = NULL;
    }
}

void
frmbuf_Write(frmbuf_s *self, int row, int col, color_s c)
{
    self->buf_[row * FRMBUF_WIDTH + col] = c;
}

void
frmbuf_Swap(frmbuf_s *self, frmbuf_s *other)
{
    color_s *tmp = self->buf_;
    self->buf_ = other->buf_;
    other->buf_ = tmp;
}

const u8 *
frmbuf_Data(const frmbuf_s *self)
{
    // @NOTE: color_s must be of 3 consecutive u8(s)
    return (u8 *)&self->buf_[0];
}
