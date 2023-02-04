#include "mmc1.hpp"

/* Variants exists in these forms:
 * 1. The use of the bit4(and 3) of the PRG bank register: MMC1/MMC1A/...
 * 2. The use of CHR bank registerS when CHR is used in RAM mode:
 *    SNORM/SOROM/...
 */

namespace ln {

MMC1::MMC1(const INES::RomAccessor *i_accessor, Variant i_var)
    : Mapper{i_accessor}
    , m_variant(i_var)
    , m_shift(0)
    , m_ctrl(0)
    , m_chr0_bnk(0)
    , m_chr1_bnk(0)
    , m_prg_bnk(0)
    , m_prg_ram{}
    , m_chr_ram{}
    , m_no_prg_banking_32K(false)
{
    clear_shift();
    reset_prg_bank_mode();

    switch (m_variant)
    {
        case V_B:
        {
            // PRG RAM is enabled by default
            m_prg_bnk &= 0xEF;
        }
        break;

        default:
            break;
    }

    std::size_t prg_rom_size;
    m_rom_accessor->get_prg_rom(nullptr, &prg_rom_size);
    if (prg_rom_size == 32 * 1024)
    {
        auto ram_size = m_rom_accessor->get_prg_ram_size();
        // @NOTE: NES 2.0 format may be required to detect this,
        // Because "ram_size" may very well be 0 for iNES format.
        if (ram_size == 0)
        {
            // Don't forbid banking for this.
            // 0 for iNES format.
        }
        // SIROM
        else if (ram_size == 8 * 1024)
        {
        }
        // Other no PRG banking 32KB boards
        // SEROM, SHROM, and SH1ROM
        else
        {
            m_no_prg_banking_32K = true;
        }
    }
}

void
MMC1::clear_shift()
{
    m_shift = 0x10;
}

void
MMC1::reset_prg_bank_mode()
{
    m_ctrl |= 0x0C;
}

Byte &
MMC1::regsiter_of_addr(Address i_addr)
{
    if (0x8000 <= i_addr && i_addr <= 0x9FFF)
    {
        return m_ctrl;
    }
    else if (0xA000 <= i_addr && i_addr <= 0xBFFF)
    {
        return m_chr0_bnk;
    }
    else if (0xC000 <= i_addr && i_addr <= 0xDFFF)
    {
        return m_chr1_bnk;
    }
    else if (0xE000 <= i_addr && i_addr <= 0xFFFF)
    {
        return m_prg_bnk;
    }
    else
    {
        // Invalid branch
        return m_ctrl;
    }
}

bool
MMC1::prg_ram_enabled() const
{
    switch (m_variant)
    {
        case V_B:
        {
            return !(m_prg_bnk & 0x10);
        }
        break;

        default:
            return true;
            break;
    }
}

Error
MMC1::validate() const
{
    if (m_variant != V_B)
    {
        return Error::UNIMPLEMENTED;
    }

    // @TODO: Support other board variants,
    // We can't detect them now without NES 2.0 format support.

    return Error::OK;
}

void
MMC1::map_memory(Memory *o_memory, VideoMemory *o_video_memory)
{
    // PRG ROM
    {
        auto get = [](const MappingEntry *i_entry, Address i_addr,
                      Byte &o_val) -> Error {
            auto thiz = (MMC1 *)i_entry->opaque;
            auto accessor = thiz->m_rom_accessor;

            Byte *rom_base;
            std::size_t rom_size;
            accessor->get_prg_rom(&rom_base, &rom_size);

            Address mem_idx = 0;
            Byte prg_rom_bank_mode = (thiz->m_ctrl >> 2) & 0x03;
            switch (prg_rom_bank_mode)
            {
                // 32KB
                case 0:
                case 1:
                {
                    Byte bank = (thiz->m_prg_bnk >> 1) & 0x07;
                    Address prg_rom_start = bank * 32 * 1024;
                    // PRG ROM area is of 32KB size
                    Address addr_base = i_entry->begin;
                    mem_idx = prg_rom_start + (i_addr - addr_base);
                }
                break;

                // fix first bank at $8000 and switch 16 KB bank at $C000
                case 2:
                // fix last bank at $C000 and switch 16 KB bank at $8000
                case 3:
                {
                    if (thiz->m_no_prg_banking_32K)
                    {
                        Address prg_rom_start = 0;
                        Address addr_base = i_entry->begin;
                        mem_idx = prg_rom_start + (i_addr - addr_base);
                    }
                    else
                    {
                        Byte bank = 0;
                        Address addr_base = i_entry->begin;

                        bool lower_prg_rom_area = (i_addr & 0xC000) == 0x8000;
                        Address fixed_cpu_addr_start =
                            Address(prg_rom_bank_mode) << 14;
                        // 0x4000 half the size of the PRG ROM area
                        // inclusive range
                        Address fixed_cpu_addr_end =
                            fixed_cpu_addr_start + (0x4000 - 1);
                        if (fixed_cpu_addr_start <= i_addr &&
                            i_addr <= fixed_cpu_addr_end)
                        {
                            // @NOTE: Max bank count is 512KB / 16KB = 32, which
                            // fits in a Byte;
                            typedef Byte BankCount_t;
                            BankCount_t bank_cnt = static_cast<BankCount_t>(
                                rom_size / (16 * 1024));
                            if (bank_cnt <= 0)
                            {
                                return Error::PROGRAMMING; // or corrupted rom
                            }
                            BankCount_t last_bank = bank_cnt - 1;

                            bank = lower_prg_rom_area ? 0 : last_bank;
                            addr_base = fixed_cpu_addr_start;
                        }
                        else
                        {
                            bank = thiz->m_prg_bnk & 0x0F;
                            addr_base = lower_prg_rom_area ? 0x8000 : 0xC000;
                        }

                        Address prg_rom_start = bank * 16 * 1024;
                        mem_idx = prg_rom_start + (i_addr - addr_base);
                    }
                }
                break;

                default:
                    return Error::PROGRAMMING;
                    break;
            }

            // @IMPL: Mirror as necessary in case things go wrong.
            // Asummeing "rom_size" not 0 and "rom_base" not nullptr.
            {
                mem_idx = mem_idx % rom_size;
            }
            o_val = *(rom_base + mem_idx);
            return Error::OK;
        };
        auto set = [](const MappingEntry *i_entry, Address i_addr,
                      Byte i_val) -> Error {
            auto thiz = (MMC1 *)i_entry->opaque;

            if (i_val & 0x80)
            {
                thiz->clear_shift();
                thiz->reset_prg_bank_mode();
            }
            else
            {
                // @TODO: Wait until CPU supports Consecutive-cycle writes, then
                // supports ingoring second write bit 0.
                // https://www.nesdev.org/wiki/MMC1#Consecutive-cycle_writes

                bool last_write = thiz->m_shift & 0x01;
                thiz->m_shift >>= 1;
                thiz->m_shift = (thiz->m_shift & 0xEF) | ((i_val & 0x01) << 4);
                if (last_write)
                {
                    thiz->regsiter_of_addr(i_addr) = thiz->m_shift & 0x1F;
                    thiz->clear_shift();
                }
            }

            return Error::OK;
        };
        // This is writable to use the serial port.
        o_memory->set_mapping(MemoryMappingPoint::PRG_ROM,
                              {0x8000, 0xFFFF, false, get, set, this});
    }

    // PRG RAM
    {
        auto decode = [](const MappingEntry *i_entry, Address i_addr,
                         Byte *&o_addr) -> ln::Error {
            auto thiz = (MMC1 *)i_entry->opaque;

            if (!thiz->prg_ram_enabled())
            {
                return Error::UNAVAILABLE;
            }

            Address index = i_addr - i_entry->begin;
            // @IMPL: Mirror as necessary in case things go wrong.
            {
                // @TODO: Support PRG RAM banking, i.e. other board variants.
                // Until then, assuming fixed at first bank (because >= 8KB
                // without banking capacity is effectively 8KB).
                index = index % (8 * 1024);
            }
            o_addr = thiz->m_prg_ram + index;
            return Error::OK;
        };
        o_memory->set_mapping(MemoryMappingPoint::PRG_RAM,
                              {0x6000, 0x7FFF, false, decode, this});
    }

    // CHR ROM/RAM
    {
        Byte *mem_base;
        std::size_t mem_size;
        if (!m_rom_accessor->use_chr_ram())
        {
            m_rom_accessor->get_chr_rom(&mem_base, &mem_size);
        }
        else
        {
            mem_base = m_chr_ram;
            static_assert(sizeof(m_chr_ram) % sizeof(Byte) == 0,
                          "Incorrect CHR RAM size");
            mem_size = sizeof(m_chr_ram) / sizeof(Byte);
        }

        auto decode = [mem_base, mem_size](const MappingEntry *i_entry,
                                           Address i_addr,
                                           Byte *&o_addr) -> ln::Error {
            auto thiz = (MMC1 *)i_entry->opaque;

            Address mem_idx = 0;
            switch ((thiz->m_ctrl >> 4) & 0x01)
            {
                // switch 8 KB at a time
                case 0:
                {
                    Byte bank = (thiz->m_chr0_bnk >> 1) & 0x0F;
                    // CHR pattern area is of 8KB size
                    Address addr_base = i_entry->begin;
                    Address prg_rom_start = bank * 8 * 1024;
                    mem_idx = prg_rom_start + (i_addr - addr_base);
                }
                break;

                // switch two separate 4 KB banks
                case 1:
                {
                    Byte bank;
                    Address addr_base;
                    if (i_addr >= LN_PATTERN_1_ADDR_HEAD)
                    {
                        bank = thiz->m_chr1_bnk & 0x1F;
                        addr_base = LN_PATTERN_1_ADDR_HEAD;
                    }
                    else
                    {
                        bank = thiz->m_chr0_bnk & 0x1F;
                        addr_base = LN_PATTERN_0_ADDR_HEAD;
                    }
                    Address prg_rom_start = bank * 4 * 1024;
                    mem_idx = prg_rom_start + (i_addr - addr_base);
                }
                break;

                default:
                    // Impossible
                    break;
            }

            // @IMPL: Mirror as necessary in case things go wrong.
            // Asummeing size not 0 and "mem_base" not nullptr.
            {
                mem_idx = mem_idx % mem_size;
            }
            o_addr = mem_base + mem_idx;
            return Error::OK;
        };
        o_video_memory->set_mapping(VideoMemoryMappingPoint::PATTERN,
                                    {LN_PATTERN_ADDR_HEAD, LN_PATTERN_ADDR_TAIL,
                                     !m_rom_accessor->use_chr_ram(), decode,
                                     this});
    }

    // mirroring
    o_video_memory->set_mirror_dyn([this]() -> VideoMemory::MirrorMode {
        switch (this->m_ctrl & 0x03)
        {
            case 0:
                return VideoMemory::MI_1_LOW;
                break;
            case 1:
                return VideoMemory::MI_1_HIGH;
                break;
            case 2:
                return VideoMemory::MI_V;
                break;
            case 3:
                return VideoMemory::MI_H;
                break;

            default:
                // Impossible branch
                return VideoMemory::MI_1_LOW;
                break;
        }
    });
}

void
MMC1::unmap_memory(Memory *o_memory, VideoMemory *o_video_memory)
{
    o_memory->unset_mapping(MemoryMappingPoint::PRG_ROM);
    o_memory->unset_mapping(MemoryMappingPoint::PRG_RAM);

    o_video_memory->unset_mapping(VideoMemoryMappingPoint::PATTERN);
    o_video_memory->unset_mirror();
}

} // namespace ln
