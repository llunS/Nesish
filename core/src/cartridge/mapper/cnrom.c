#include "cnrom.h"

#include "memory/mmem.h"
#include "memory/vmem.h"
#include "cartridge/inesromaccessor.h"

#define NH_128_PRG_RAM_SIZE 16 * 1024
#define NH_256_PRG_RAM_SIZE 32 * 1024

void
cnrom_Init(cnrom_s *self, const inesromaccessor_s *accessor)
{
    mapperbase_Init(&self->base_, accessor);
}

void
cnrom_Deinit(void *self)
{
    (void)(self);
}

NHErr
cnrom_Validate(const void *me)
{
    const cnrom_s *self = (const cnrom_s *)(me);

    usize prgRomSize;
    inesromaccessor_GetPrgRom(self->base_.romaccessor, NULL, &prgRomSize);
    if (!(prgRomSize == NH_128_PRG_RAM_SIZE ||
          prgRomSize == NH_256_PRG_RAM_SIZE))
    {
        return NH_ERR_CORRUPTED;
    }
    // @TEST: Right not to support CHR RAM?
    if (inesromaccessor_UseChrRam(self->base_.romaccessor))
    {
        return NH_ERR_CORRUPTED;
    }

    return NH_ERR_OK;
}

void
cnrom_Powerup(void *me)
{
    cnrom_s *self = (cnrom_s *)(me);

    self->chrbnk_ = 0;
}

void
cnrom_Reset(void *self)
{
    cnrom_Powerup(self);
}

static NHErr
prgRomDecode(const mementry_s *entry, addr_t addr, u8 **ptr)
{
    cnrom_s *self = (cnrom_s *)entry->Opaque;

    // address mirroring
    addr_t reladdr = (addr - entry->Begin);
    if (self->prgromctx_.Size == NH_128_PRG_RAM_SIZE)
    {
        // @TEST: If this behaves like NROM of 16KB?
        // 16K mask, to map second 16KB to the first.
        reladdr &= 0x3FFF;
    }

    *ptr = self->prgromctx_.Base + reladdr;
    return NH_ERR_OK;
}

static NHErr
chrRomGet(const mementry_s *entry, addr_t addr, u8 *val)
{
    cnrom_s *self = (cnrom_s *)entry->Opaque;

    u8 bank = self->chrbnk_;
    // 8KB window
    addr_t prgRomStart = bank * 8 * 1024;
    addr_t addrbase = entry->Begin;
    addr_t memidx = prgRomStart + (addr - addrbase);

    // Handle upper unused bank bits
    // Asummeing "self->chrromctx_.Size" not 0 and "self->chrromctx_.Base" not
    // NULL.
    {
        memidx = memidx % self->chrromctx_.Size;
    }

    *val = *(self->chrromctx_.Base + memidx);
    return NH_ERR_OK;
}

static NHErr
chrRomSet(const mementry_s *entry, addr_t addr, u8 val)
{
    (void)(addr);
    cnrom_s *self = (cnrom_s *)entry->Opaque;

    self->chrbnk_ = val;
    return NH_ERR_OK;
}

void
cnrom_MapMemory(void *me, mmem_s *mmem, vmem_s *vmem)
{
    cnrom_s *self = (cnrom_s *)(me);

    // PRG ROM
    {
        inesromaccessor_GetPrgRom(self->base_.romaccessor,
                                  &self->prgromctx_.Base,
                                  &self->prgromctx_.Size);
        mementry_s entry;
        mementry_Init(&entry, 0x8000, 0xFFFF, true, prgRomDecode, self);
        membase_SetMapping(&mmem->Base, MMP_PRGROM, entry);
    }

    // CHR ROM
    {
        inesromaccessor_GetChrRom(self->base_.romaccessor,
                                  &self->chrromctx_.Base,
                                  &self->chrromctx_.Size);
        mementry_s entry;
        mementry_InitExt(&entry, NH_PATTERN_ADDR_HEAD, NH_PATTERN_ADDR_TAIL,
                         false, chrRomGet, chrRomSet, self);
        membase_SetMapping(&vmem->Base, VMP_PAT, entry);
    }

    // mirroring
    mapperbase_setFixedVhMirror(&self->base_, vmem);
}

void
cnrom_UnmapMemory(void *me, mmem_s *mmem, vmem_s *vmem)
{
    cnrom_s *self = (cnrom_s *)(me);

    membase_UnsetMapping(&mmem->Base, MMP_PRGROM);

    membase_UnsetMapping(&vmem->Base, VMP_PAT);
    mapperbase_unsetFixedVhMirror(&self->base_, vmem);
}
