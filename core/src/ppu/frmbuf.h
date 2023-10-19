#pragma once

#include "ppu/color.h"
#include "spec.h"
#include "nesish/nesish.h"

#define FRMBUF_WIDTH NH_NES_WIDTH
#define FRMBUF_HEIGHT NH_NES_HEIGHT

typedef struct frmbuf_s {
    color_s *buf_;
} frmbuf_s;

bool
frmbuf_Init(frmbuf_s *self);
void
frmbuf_Deinit(frmbuf_s *self);

void
frmbuf_Write(frmbuf_s *self, int row, int col, color_s c);

void
frmbuf_Swap(frmbuf_s *self, frmbuf_s *other);

const u8 *
frmbuf_Data(const frmbuf_s *self);
