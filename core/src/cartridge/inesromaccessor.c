#include "inesromaccessor.h"

#include "cartridge/ines.h"

void
inesromaccessor_init_(inesromaccessor_s *self, const ines_s *ines)
{
    self->ines_ = ines;
}

bool
inesromaccessor_MirrorH(const inesromaccessor_s *self)
{
    return !self->ines_->header_.Mirror;
}

void
inesromaccessor_GetPrgRom(const inesromaccessor_s *self, u8 **addr, usize *size)
{
    if (addr)
        *addr = self->ines_->prgrom_;
    if (size)
        *size = self->ines_->prgromsize_;
}

usize
inesromaccessor_GetPrgRamSize(const inesromaccessor_s *self)
{
    return self->ines_->header_.PrgRamSize * 8 * 1024;
}

void
inesromaccessor_GetChrRom(const inesromaccessor_s *self, u8 **addr, usize *size)
{
    if (addr)
        *addr = self->ines_->chrrom_;
    if (size)
        *size = self->ines_->chrromsize_;
}

bool
inesromaccessor_UseChrRam(const inesromaccessor_s *self)
{
    return self->ines_->usechrram_;
}
