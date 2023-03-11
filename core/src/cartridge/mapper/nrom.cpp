#include "nrom.hpp"

#define NH_128_PRG_RAM_SIZE 16 * 1024
#define NH_256_PRG_RAM_SIZE 32 * 1024

namespace nh {

NROM::NROM(const INES::RomAccessor *i_accessor)
    : Mapper{i_accessor}
    , m_prg_ram{}
    , m_chr_ram{}
{
}

NHErr
NROM::validate() const
{
    std::size_t prg_rom_size;
    m_rom_accessor->get_prg_rom(nullptr, &prg_rom_size);
    if (!(prg_rom_size == NH_128_PRG_RAM_SIZE ||
          prg_rom_size == NH_256_PRG_RAM_SIZE))
    {
        return NH_ERR_CORRUPTED;
    }

    return NH_ERR_OK;
}

void
NROM::power_up()
{
}

void
NROM::reset()
{
}

void
NROM::map_memory(Memory *o_memory, VideoMemory *o_video_memory)
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
                // 16K mask, to map second 16KB to the first.
                rel_address &= 0x3FFF;
            }

            o_addr = mem_base + rel_address;
            return NH_ERR_OK;
        };
        o_memory->set_mapping(MemoryMappingPoint::PRG_ROM,
                              {0x8000, 0xFFFF, true, decode, this});
    }

    // PRG RAM
    {
        auto decode = [](const MappingEntry *i_entry, Address i_addr,
                         Byte *&o_addr) -> NHErr {
            auto thiz = (NROM *)i_entry->opaque;

            // No way to tell if it's 2KB or 4KB, and then mirror them to
            // fill the 8KB area.
            // Because PRG RAM size is specified in 8KB units in iNES format.

            o_addr = thiz->m_prg_ram + (i_addr - i_entry->begin);
            return NH_ERR_OK;
        };
        o_memory->set_mapping(MemoryMappingPoint::PRG_RAM,
                              {0x6000, 0x7FFF, false, decode, this});
    }

    // CHR ROM/RAM
    {
        Byte *mem_base;
        if (!m_rom_accessor->use_chr_ram())
        {
            m_rom_accessor->get_chr_rom(&mem_base, nullptr);
        }
        // CHR RAM
        else
        {
            mem_base = m_chr_ram;
        }

        auto decode = [mem_base](const MappingEntry *i_entry, Address i_addr,
                                 Byte *&o_addr) -> NHErr {
            o_addr = mem_base + (i_addr - i_entry->begin);
            return NH_ERR_OK;
        };
        o_video_memory->set_mapping(VideoMemoryMappingPoint::PATTERN,
                                    {NH_PATTERN_ADDR_HEAD, NH_PATTERN_ADDR_TAIL,
                                     !m_rom_accessor->use_chr_ram(), decode,
                                     this});
    }

    // mirroring
    set_fixed_vh_mirror(o_video_memory);
}

void
NROM::unmap_memory(Memory *o_memory, VideoMemory *o_video_memory)
{
    o_memory->unset_mapping(MemoryMappingPoint::PRG_ROM);
    o_memory->unset_mapping(MemoryMappingPoint::PRG_RAM);

    o_video_memory->unset_mapping(VideoMemoryMappingPoint::PATTERN);
    unset_fixed_vh_mirror(o_video_memory);
}

} // namespace nh
