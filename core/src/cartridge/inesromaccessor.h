#pragma once

#include "types.h"

typedef struct ines_s ines_s;

typedef struct inesromaccessor_s {
    const ines_s *ines_;
} inesromaccessor_s;

bool
inesromaccessor_MirrorH(const inesromaccessor_s *self);

void
inesromaccessor_GetPrgRom(const inesromaccessor_s *self, u8 **addr,
                          usize *size);
usize
inesromaccessor_GetPrgRamSize(const inesromaccessor_s *self);

void
inesromaccessor_GetChrRom(const inesromaccessor_s *self, u8 **addr,
                          usize *size);
bool
inesromaccessor_UseChrRam(const inesromaccessor_s *self);

void
inesromaccessor_init_(inesromaccessor_s *self, const ines_s *ines);
