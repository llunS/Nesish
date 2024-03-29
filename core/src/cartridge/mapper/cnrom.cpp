#include "cnrom.hpp"

#define NH_128_PRG_RAM_SIZE 16 * 1024
#define NH_256_PRG_RAM_SIZE 32 * 1024

namespace nh {

CNROM::CNROM(const INES::RomAccessor *i_accessor)
    : Mapper{i_accessor}
{
}

NHErr
CNROM::validate() const
{
    std::size_t prg_rom_size;
    m_rom_accessor->get_prg_rom(nullptr, &prg_rom_size);
    if (!(prg_rom_size == NH_128_PRG_RAM_SIZE ||
          prg_rom_size == NH_256_PRG_RAM_SIZE))
    {
        return NH_ERR_CORRUPTED;
    }
    // @TEST: Right not to support CHR RAM?
    if (m_rom_accessor->use_chr_ram())
    {
        return NH_ERR_CORRUPTED;
    }

    return NH_ERR_OK;
}

void
CNROM::power_up()
{
    m_chr_bnk = 0;
}

void
CNROM::reset()
{
    power_up();
}

void
CNROM::map_memory(Memory *o_memory, VideoMemory *o_video_memory)
{
    // PRG ROM
    {
        Byte *mem_base;
        std::size_t mem_size;
        m_rom_accessor->get_prg_rom(&mem_base, &mem_size);
        auto decode = [mem_base, mem_size](const MappingEntry *i_entry,
                                           Address i_addr,
                                           Byte *&o_addr) -> NHErr {
            // address mirroring
            Address rel_address = (i_addr - i_entry->begin);
            if (mem_size == NH_128_PRG_RAM_SIZE)
            {
                // @TEST: If this behaves like NROM of 16KB?
                // 16K mask, to map second 16KB to the first.
                rel_address &= 0x3FFF;
            }

            o_addr = mem_base + rel_address;
            return NH_ERR_OK;
        };
        o_memory->set_mapping(MemoryMappingPoint::PRG_ROM,
                              {0x8000, 0xFFFF, true, decode, this});
    }

    // CHR ROM
    {
        Byte *mem_base;
        std::size_t mem_size;
        m_rom_accessor->get_chr_rom(&mem_base, &mem_size);
        auto get = [mem_base, mem_size](const MappingEntry *i_entry,
                                        Address i_addr, Byte &o_val) -> NHErr {
            auto thiz = (CNROM *)i_entry->opaque;

            Byte bank = thiz->m_chr_bnk;
            // 8KB window
            Address prg_rom_start = bank * 8 * 1024;
            Address addr_base = i_entry->begin;
            Address mem_idx = prg_rom_start + (i_addr - addr_base);

            // Handle upper unused bank bits
            // Asummeing "mem_size" not 0 and "mem_base" not nullptr.
            {
                mem_idx = mem_idx % mem_size;
            }

            o_val = *(mem_base + mem_idx);
            return NH_ERR_OK;
        };
        auto set = [](const MappingEntry *i_entry, Address i_addr,
                      Byte i_val) -> NHErr {
            (void)(i_addr);
            auto thiz = (CNROM *)i_entry->opaque;

            thiz->m_chr_bnk = i_val;
            return NH_ERR_OK;
        };

        o_video_memory->set_mapping(VideoMemoryMappingPoint::PATTERN,
                                    {NH_PATTERN_ADDR_HEAD, NH_PATTERN_ADDR_TAIL,
                                     false, get, set, this});
    }

    // mirroring
    set_fixed_vh_mirror(o_video_memory);
}

void
CNROM::unmap_memory(Memory *o_memory, VideoMemory *o_video_memory)
{
    o_memory->unset_mapping(MemoryMappingPoint::PRG_ROM);

    o_video_memory->unset_mapping(VideoMemoryMappingPoint::PATTERN);
    unset_fixed_vh_mirror(o_video_memory);
}

} // namespace nh
