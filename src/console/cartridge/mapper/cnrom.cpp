#include "cnrom.hpp"

#define LN_128_PRG_RAM_SIZE 16 * 1024
#define LN_256_PRG_RAM_SIZE 32 * 1024

namespace ln {

CNROM::CNROM(const INES::RomAccessor *i_accessor)
    : Mapper{i_accessor}
    , m_chr_bnk(0)
{
}

Error
CNROM::validate() const
{
    std::size_t prg_rom_size;
    m_rom_accessor->get_prg_rom(nullptr, &prg_rom_size);
    if (!(prg_rom_size == LN_128_PRG_RAM_SIZE ||
          prg_rom_size == LN_256_PRG_RAM_SIZE))
    {
        return Error::CORRUPTED;
    }
    // @TEST: Don't support CHR RAM
    if (m_rom_accessor->use_chr_ram())
    {
        return Error::CORRUPTED;
    }

    return Error::OK;
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
                                           Byte *&o_addr) -> ln::Error {
            // address mirroring
            Address rel_address = (i_addr - i_entry->begin);
            if (mem_size == LN_128_PRG_RAM_SIZE)
            {
                // @TEST: If this behaves like NROM for 16KB?
                // 16K mask, to map second 16KB to the first.
                rel_address &= 0x3FFF;
            }

            o_addr = mem_base + rel_address;
            return Error::OK;
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
                                        Address i_addr, Byte &o_val) -> Error {
            auto thiz = (CNROM *)i_entry->opaque;

            Byte bank = thiz->m_chr_bnk;
            // 8KB window
            Address prg_rom_start = bank * 8 * 1024;
            Address addr_base = i_entry->begin;
            Address mem_idx = prg_rom_start + (i_addr - addr_base);

            // @IMPL: Handle upper unused bank bits
            // Asummeing "mem_size" not 0 and "mem_base" not nullptr.
            {
                mem_idx = mem_idx % mem_size;
            }

            o_val = *(mem_base + mem_idx);
            return Error::OK;
        };
        auto set = [](const MappingEntry *i_entry, Address i_addr,
                      Byte i_val) -> Error {
            (void)(i_addr);
            auto thiz = (CNROM *)i_entry->opaque;

            thiz->m_chr_bnk = i_val;
            return Error::OK;
        };

        o_video_memory->set_mapping(VideoMemoryMappingPoint::PATTERN,
                                    {LN_PATTERN_ADDR_HEAD, LN_PATTERN_ADDR_TAIL,
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

} // namespace ln
