#include "mmc1.h"

/* Variants exists in these forms:
 * 1. The use of the bit4(and 3) of the PRG bank register: MMC1/MMC1A/...
 * 2. The use of CHR bank registerS when CHR is used in RAM mode:
 *    SNORM/SOROM/...
 */

#include "memory/mmem.h"
#include "memory/vmem.h"
#include "cartridge/inesromaccessor.h"

#include <string.h>

static void
clearShift(mmc1_s *self);
static void
resetPrgBnkMode(mmc1_s *self);
static u8 *
regOfAddr(mmc1_s *self, addr_t addr);
static bool
prgRamEnabled(const mmc1_s *self);

void
mmc1_Init(mmc1_s *self, const inesromaccessor_s *accessor, mmc1var_e var)
{
    mapperbase_Init(&self->base_, accessor);

    self->var_ = var;
    memset(self->prgram_, 0, sizeof(self->prgram_));
    memset(self->chrram_, 0, sizeof(self->chrram_));
    self->noprgbanking32K_ = false;

    usize prgRomSize;
    inesromaccessor_GetPrgRom(self->base_.romaccessor, NULL, &prgRomSize);
    if (prgRomSize == 32 * 1024)
    {
        usize ramsize = inesromaccessor_GetPrgRamSize(self->base_.romaccessor);
        // NES 2.0 format may be required to detect this, because
        // "ramsize" may very well be 0 for iNES format.
        if (ramsize == 0)
        {
            // Don't forbid banking for this.
            // 0 for iNES format.
        }
        // SIROM
        else if (ramsize == 8 * 1024)
        {
        }
        // Other no PRG banking 32KB boards
        // SEROM, SHROM, and SH1ROM
        else
        {
            self->noprgbanking32K_ = true;
        }
    }
}

void
mmc1_Deinit(void *self)
{
    (void)(self);
}

void
clearShift(mmc1_s *self)
{
    self->shift_ = 0x10;
}

void
resetPrgBnkMode(mmc1_s *self)
{
    self->ctrl_ |= 0x0C;
}

u8 *
regOfAddr(mmc1_s *self, addr_t addr)
{
    if (0x8000 <= addr && addr <= 0x9FFF)
    {
        return &self->ctrl_;
    }
    else if (0xA000 <= addr && addr <= 0xBFFF)
    {
        return &self->ch0bnk_;
    }
    else if (0xC000 <= addr && addr <= 0xDFFF)
    {
        return &self->ch1bnk_;
    }
    else if (0xE000 <= addr && addr <= 0xFFFF)
    {
        return &self->prgbnk_;
    }
    else
    {
        // Invalid branch
        return &self->ctrl_;
    }
}

bool
prgRamEnabled(const mmc1_s *self)
{
    switch (self->var_)
    {
        case MMC1B:
        {
            return !(self->prgbnk_ & 0x10);
        }
        break;

        default:
            return true;
            break;
    }
}

NHErr
mmc1_Validate(const void *me)
{
    const mmc1_s *self = (const mmc1_s *)(me);

    if (self->var_ != MMC1B)
    {
        return NH_ERR_UNIMPLEMENTED;
    }

    // @TODO: Support other board variants,
    // We can't detect them now without NES 2.0 format support.

    return NH_ERR_OK;
}

void
mmc1_Powerup(void *me)
{
    mmc1_s *self = (mmc1_s *)(me);

    clearShift(self);

    self->ctrl_ = 0;
    resetPrgBnkMode(self);

    self->ch0bnk_ = 0;
    self->ch1bnk_ = 0;

    switch (self->var_)
    {
        case MMC1B:
        {
            // PRG RAM is enabled by default
            self->prgbnk_ &= 0xEF;
        }
        break;

        default:
        {
            self->prgbnk_ = 0;
        }
        break;
    }
}

void
mmc1_Reset(void *self)
{
    mmc1_Powerup(self);
}

static NHErr
prgRomGet(const mementry_s *entry, addr_t addr, u8 *val)
{
    mmc1_s *self = (mmc1_s *)entry->Opaque;

    addr_t memidx = 0;
    u8 prgRomBankMode = (self->ctrl_ >> 2) & 0x03;
    switch (prgRomBankMode)
    {
        // 32KB
        case 0:
        case 1:
        {
            u8 bank = (self->prgbnk_ >> 1) & 0x07;
            // 32KB window
            addr_t prgRomStart = bank * 32 * 1024;
            addr_t addrbase = entry->Begin;
            memidx = prgRomStart + (addr - addrbase);
        }
        break;

        // fix first bank at $8000 and switch 16 KB bank at $C000
        case 2:
        // fix last bank at $C000 and switch 16 KB bank at $8000
        case 3:
        {
            if (self->noprgbanking32K_)
            {
                addr_t prgRomStart = 0;
                addr_t addrbase = entry->Begin;
                memidx = prgRomStart + (addr - addrbase);
            }
            else
            {
                u8 bank = 0;
                addr_t addrbase = entry->Begin;

                bool lowerPrgRomArea = (addr & 0xC000) == 0x8000;
                addr_t fixedCpuAddrStart = (addr_t)(prgRomBankMode) << 14;
                // 0x4000 half the size of the PRG ROM area
                // inclusive range
                addr_t fixedCpuAddrEnd = fixedCpuAddrStart + (0x4000 - 1);
                if (fixedCpuAddrStart <= addr && addr <= fixedCpuAddrEnd)
                {
                    // Max bank count is 512KB / 16KB = 32, which
                    // fits in a u8;
                    typedef u8 BankCount_t;
                    BankCount_t bankCnt =
                        (BankCount_t)(self->prgromctx_.Size / (16 * 1024));
                    if (bankCnt <= 0)
                    {
                        return NH_ERR_PROGRAMMING; // or corrupted rom
                    }
                    BankCount_t lastBank = bankCnt - 1;

                    bank = lowerPrgRomArea ? 0 : lastBank;
                    addrbase = fixedCpuAddrStart;
                }
                else
                {
                    bank = self->prgbnk_ & 0x0F;
                    addrbase = lowerPrgRomArea ? 0x8000 : 0xC000;
                }

                addr_t prgRomStart = bank * 16 * 1024;
                memidx = prgRomStart + (addr - addrbase);
            }
        }
        break;

        default:
            return NH_ERR_PROGRAMMING;
            break;
    }

    // Mirror as necessary in case things go wrong.
    // Asummeing "self->prgromctx_.Size" not 0 and "self->prgromctx_.Base" not
    // NULL.
    {
        memidx = memidx % self->prgromctx_.Size;
    }
    *val = *(self->prgromctx_.Base + memidx);
    return NH_ERR_OK;
}

static NHErr
prgRomSet(const mementry_s *entry, addr_t addr, u8 val)
{
    mmc1_s *self = (mmc1_s *)entry->Opaque;

    if (val & 0x80)
    {
        clearShift(self);
        resetPrgBnkMode(self);
    }
    else
    {
        // @TODO: When the serial port is written to on consecutive
        // cycles, it ignores every write after the first. In practice,
        // this only happens when the CPU executes read-modify-write
        // instructions, which first write the original value before
        // writing the modified one on the next cycle.[1] This
        // restriction only applies to the data being written on bit 0;
        // the bit 7 reset is never ignored. Bill & Ted's Excellent
        // Adventure does a reset by using INC on a ROM location
        // containing $FF and requires that the $00 write on the next
        // cycle is ignored. Shinsenden, however, uses illegal
        // instruction $7F (RRA abs,X) to set bit 7 on the second write
        // and will crash after selecting the みる (look) option if this
        // reset is ignored.[2] This write-ignore behavior appears to be
        // intentional and is believed to ignore all consecutive write
        // cycles after the first even if that first write does not
        // target the serial port.[3]

        bool lastWrite = self->shift_ & 0x01;
        self->shift_ >>= 1;
        self->shift_ = (self->shift_ & 0xEF) | ((val & 0x01) << 4);
        if (lastWrite)
        {
            *regOfAddr(self, addr) = self->shift_ & 0x1F;
            clearShift(self);
        }
    }

    return NH_ERR_OK;
}

static NHErr
prgRamDecode(const mementry_s *entry, addr_t addr, u8 **ptr)
{
    mmc1_s *self = (mmc1_s *)entry->Opaque;

    if (!prgRamEnabled(self))
    {
        return NH_ERR_UNAVAILABLE;
    }

    addr_t index = addr - entry->Begin;
    // Mirror as necessary in case things go wrong.
    {
        // @TODO: Support PRG RAM banking, i.e. other board variants.
        // Until then, assuming fixed at first bank (because >= 8KB
        // without banking capacity is effectively 8KB).
        index = index % (8 * 1024);
    }
    *ptr = self->prgram_ + index;
    return NH_ERR_OK;
}

static NHErr
chrDecode(const mementry_s *entry, addr_t addr, u8 **ptr)
{
    mmc1_s *self = (mmc1_s *)entry->Opaque;

    addr_t memidx = 0;
    switch ((self->ctrl_ >> 4) & 0x01)
    {
        // switch 8 KB at a time
        case 0:
        {
            u8 bank = (self->ch0bnk_ >> 1) & 0x0F;
            // CHR pattern area is of 8KB size
            addr_t addrbase = entry->Begin;
            addr_t prgRomStart = bank * 8 * 1024;
            memidx = prgRomStart + (addr - addrbase);
        }
        break;

        // switch two separate 4 KB banks
        case 1:
        {
            u8 bank;
            addr_t addrbase;
            if (addr >= NH_PATTERN_1_ADDR_HEAD)
            {
                bank = self->ch1bnk_ & 0x1F;
                addrbase = NH_PATTERN_1_ADDR_HEAD;
            }
            else
            {
                bank = self->ch0bnk_ & 0x1F;
                addrbase = NH_PATTERN_0_ADDR_HEAD;
            }
            addr_t prgRomStart = bank * 4 * 1024;
            memidx = prgRomStart + (addr - addrbase);
        }
        break;

        default:
            // Impossible
            break;
    }

    // Mirror as necessary in case things go wrong.
    // Asummeing size not 0 and "self->chrctx_.Base" not NULL.
    {
        memidx = memidx % self->chrctx_.Size;
    }
    *ptr = self->chrctx_.Base + memidx;
    return NH_ERR_OK;
}

static mirmode_e
dynMirror(void *opaque)
{
    mmc1_s *self = (mmc1_s *)opaque;

    switch (self->ctrl_ & 0x03)
    {
        case 0:
            return MM_1LOW;
            break;
        case 1:
            return MM_1HIGH;
            break;
        case 2:
            return MM_V;
            break;
        case 3:
            return MM_H;
            break;

        default:
            // Impossible branch
            return MM_1LOW;
            break;
    }
}

void
mmc1_MapMemory(void *me, mmem_s *mmem, vmem_s *vmem)
{
    mmc1_s *self = (mmc1_s *)(me);

    // PRG ROM
    {
        inesromaccessor_GetPrgRom(self->base_.romaccessor,
                                  &self->prgromctx_.Base,
                                  &self->prgromctx_.Size);
        // This is writable to use the serial port.
        mementry_s entry;
        mementry_InitExt(&entry, 0x8000, 0xFFFF, false, prgRomGet, prgRomSet,
                         self);
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
                                      &self->chrctx_.Base, &self->chrctx_.Size);
        }
        else
        {
            self->chrctx_.Base = self->chrram_;
            _Static_assert(sizeof(self->chrram_) % sizeof(u8) == 0,
                           "Incorrect CHR RAM size");
            self->chrctx_.Size = sizeof(self->chrram_) / sizeof(u8);
        }
        mementry_s entry;
        mementry_Init(&entry, NH_PATTERN_ADDR_HEAD, NH_PATTERN_ADDR_TAIL,
                      nousechrrom, chrDecode, self);
        membase_SetMapping(&vmem->Base, VMP_PAT, entry);
    }

    // mirroring
    vmem_SetMirrorDyn(vmem, dynMirror, self);
}

void
mmc1_UnmapMemory(void *self, mmem_s *mmem, vmem_s *vmem)
{
    (void)(self);

    membase_UnsetMapping(&mmem->Base, MMP_PRGROM);
    membase_UnsetMapping(&mmem->Base, MMP_PRGRAM);

    membase_UnsetMapping(&vmem->Base, VMP_PAT);
    vmem_UnsetMirror(vmem);
}
