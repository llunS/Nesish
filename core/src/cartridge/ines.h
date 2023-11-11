#pragma once

#include "nesish/nesish.h"
#include "types.h"
#include "cartridge/cart.h"
#include "cartridge/inesromaccessor.h"
#include "cartridge/mapper/mapper.h"

typedef struct inesheader_s {
    // https://wiki.nesdev.org/w/index.php/INES
    u8 Magic[4];
    u8 PrgRomSize; // in 16KB units.
    u8 ChrRomSize; // in 8KB units, 0 implies CHR RAM.

    // flag 6
    ubit Mirror : 1; // 0: horizontal, 1: vertical.
    ubit PersistentMem : 1;
    ubit Trainer : 1; // ignored, unsupported.
    ubit FourScrVram : 1;
    ubit MapperLow : 4;

    // flag 7
    ubit VsUnisystem : 1;  // ignored, unsupported.
    ubit Playchoice10 : 1; // ignored, unsupported.
    ubit Ines2 : 2;        // 2: NES 2.0 format.
    ubit MapperHigh : 4;

    // This was a later extension to the iNES format and not widely used.
    u8 PrgRamSize; // in 8KB units, 0 infers 8KB for compatibility.
} inesheader_s;

typedef struct ines_s {
    inesheader_s header_;

    u8 mappernum_;
    mapper_s mapper_;

    u8 *prgrom_;
    usize prgromsize_;
    u8 *chrrom_;
    usize chrromsize_;
    bool usechrram_;

    inesromaccessor_s romaccessor_;

    NHLogger *logger_;
} ines_s;

void
ines_Deinit(void *self);

NHErr
ines_Validate(const void *self);

void
ines_Powerup(void *self);
void
ines_Reset(void *self);

void
ines_MapMemory(void *self, mmem_s *mmem, vmem_s *vmem);
void
ines_UnmapMemory(void *self, mmem_s *mmem, vmem_s *vmem);

void
ines_init_(ines_s *self, NHLogger *logger);

NHErr
ines_resolve_(ines_s *self);
