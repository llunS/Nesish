#include "nrom.h"

#include "memory/mmem.h"
#include "memory/vmem.h"
#include "cartridge/inesromaccessor.h"

#include <string.h>

#define NH_128_PRG_RAM_SIZE 16 * 1024
#define NH_256_PRG_RAM_SIZE 32 * 1024

void
nrom_Init(nrom_s *self, const inesromaccessor_s *accessor)
{
    mapperbase_Init(&self->base_, accessor);

    memset(self->prgram_, 0, sizeof(self->prgram_));
    memset(self->chrram_, 0, sizeof(self->chrram_));
}

NHErr
nrom_Validate(const nrom_s *self)
{
    usize prgRomSize;
    inesromaccessor_GetPrgRom(self->base_.romaccessor, NULL, &prgRomSize);
    if (!(prgRomSize == NH_128_PRG_RAM_SIZE ||
          prgRomSize == NH_256_PRG_RAM_SIZE))
    {
        return NH_ERR_CORRUPTED;
    }

    return NH_ERR_OK;
}

void
nrom_Powerup(nrom_s *self)
{
    (void)(self);
}

void
nrom_Reset(nrom_s *self)
{
    (void)(self);
}

static NHErr
prgRomDecode(const mementry_s *entry, addr_t addr, u8 **ptr)
{
    nrom_s *self = (nrom_s *)entry->Opaque;

    // address mirroring
    addr_t reladdr = (addr - entry->Begin);
    if (self->prgromctx_.Size == NH_128_PRG_RAM_SIZE)
    {
        // 16K mask, to map second 16KB to the first.
        reladdr &= 0x3FFF;
    }

    *ptr = self->prgromctx_.Base + reladdr;
    return NH_ERR_OK;
}

static NHErr
prgRamDecode(const mementry_s *entry, addr_t addr, u8 **ptr)
{
    nrom_s *self = (nrom_s *)entry->Opaque;

    // No way to tell if it's 2KB or 4KB, and then mirror them to
    // fill the 8KB area.
    // Because PRG RAM size is specified in 8KB units in iNES format.

    *ptr = self->prgram_ + (addr - entry->Begin);
    return NH_ERR_OK;
}

static NHErr
chrDecode(const mementry_s *entry, addr_t addr, u8 **ptr)
{
    nrom_s *self = (nrom_s *)entry->Opaque;

    *ptr = self->chrctx_.Base + (addr - entry->Begin);
    return NH_ERR_OK;
}

void
nrom_MapMemory(nrom_s *self, mmem_s *mmem, vmem_s *vmem)
{
    // PRG ROM
    {
        inesromaccessor_GetPrgRom(self->base_.romaccessor,
                                  &self->prgromctx_.Base,
                                  &self->prgromctx_.Size);
        mementry_s entry;
        mementry_Init(&entry, 0x8000, 0xFFFF, true, prgRomDecode, self);
        membase_SetMapping(&mmem->Base, MMP_PRGROM, entry);
    }

    // PRG RAM
    {
        mementry_s entry;
        mementry_Init(&entry, 0x6000, 0x7FFF, false, prgRamDecode, self);
        membase_SetMapping(&mmem->Base, MMP_PRGRAM, entry);
    }

    // CHR ROM/RAM
    {
        bool nousechrrom = !inesromaccessor_UseChrRam(self->base_.romaccessor);
        if (nousechrrom)
        {
            inesromaccessor_GetChrRom(self->base_.romaccessor,
                                      &self->chrctx_.Base, NULL);
        }
        // CHR RAM
        else
        {
            self->chrctx_.Base = self->chrram_;
        }
        mementry_s entry;
        mementry_Init(&entry, NH_PATTERN_ADDR_HEAD, NH_PATTERN_ADDR_TAIL,
                      nousechrrom, chrDecode, self);
        membase_SetMapping(&vmem->Base, VMP_PAT, entry);
    }

    // mirroring
    mapperbase_setFixedVhMirror(&self->base_, vmem);
}

void
nrom_UnmapMemory(nrom_s *self, mmem_s *mmem, vmem_s *vmem)
{
    membase_UnsetMapping(&mmem->Base, MMP_PRGROM);
    membase_UnsetMapping(&mmem->Base, MMP_PRGRAM);

    membase_UnsetMapping(&vmem->Base, VMP_PAT);
    mapperbase_unsetFixedVhMirror(&self->base_, vmem);
}
