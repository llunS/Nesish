#include "nrom.hpp"

#include "console/assert.hpp"

#define LN_NORM_128_PRG_RAM_SIZE 16 * 1024
#define LN_NORM_256_PRG_RAM_SIZE 32 * 1024

namespace ln {

NROM::NROM(const INES::RomAccessor *i_accessor)
    : Mapper{i_accessor}
    , m_prg_ram{}
    , m_chr_ram{}
{
}

Error
NROM::validate() const
{
    std::size_t prg_rom_size;
    m_rom_accessor->get_prg_rom(nullptr, &prg_rom_size);
    if (!(prg_rom_size == LN_NORM_128_PRG_RAM_SIZE ||
          prg_rom_size == LN_NORM_256_PRG_RAM_SIZE))
    {
        return Error::CORRUPTED;
    }

    return Error::OK;
}

void
NROM::map_memory(const INES *i_nes, Memory *i_memory,
                 VideoMemory *i_video_memory)
{
    // PRG ROM
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            auto thiz = (NROM *)i_entry->opaque;
            auto accessor = thiz->m_rom_accessor;

            std::size_t rom_size;
            Byte *rom_base;
            accessor->get_prg_rom(&rom_base, &rom_size);

            // address mirroring
            Address rel_address = (i_addr - i_entry->begin);
            if (rom_size == LN_NORM_128_PRG_RAM_SIZE)
            {
                // 16K mask, to map second 16KB to the first.
                rel_address &= 0x3FFF;
            }

            return rom_base + rel_address;
        };
        i_memory->set_mapping(MemoryMappingPoint::PRG_ROM,
                              {0x8000, 0xFFFF, true, decode, this});
    }

    // PRG RAM
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            auto thiz = (NROM *)i_entry->opaque;

            // @TODO: mirror if "prg_ram_size" less than 8k,
            // wrap "prg_ram_size" in a function to deal with 2.0 format

            return thiz->m_prg_ram + (i_addr - i_entry->begin);
        };
        i_memory->set_mapping(MemoryMappingPoint::PRG_RAM,
                              {0x6000, 0x7FFF, false, decode, this});
    }

    // CHR ROM
    if (!m_rom_accessor->use_chr_ram())
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            auto thiz = (NROM *)i_entry->opaque;
            auto accessor = thiz->m_rom_accessor;

            Byte *rom_base;
            accessor->get_chr_rom(&rom_base, nullptr);

            return rom_base + (i_addr - i_entry->begin);
        };
        i_video_memory->set_mapping(
            VideoMemoryMappingPoint::PATTERN,
            {LN_PATTERN_ADDR_HEAD, LN_PATTERN_ADDR_TAIL, true, decode, this});
    }
    // CHR RAM
    else
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            auto thiz = (NROM *)i_entry->opaque;

            return thiz->m_chr_ram + (i_addr - i_entry->begin);
        };
        i_video_memory->set_mapping(
            VideoMemoryMappingPoint::PATTERN,
            {LN_PATTERN_ADDR_HEAD, LN_PATTERN_ADDR_TAIL, false, decode, this});
    }

    // mirroring
    i_video_memory->configure_mirror(i_nes->h_mirror());
}

void
NROM::unmap_memory(const INES *i_nes, Memory *i_memory,
                   VideoMemory *i_video_memory) const
{
    (void)(i_nes);

    i_memory->unset_mapping(MemoryMappingPoint::PRG_ROM);
    i_memory->unset_mapping(MemoryMappingPoint::PRG_RAM);

    i_video_memory->unset_mapping(VideoMemoryMappingPoint::PATTERN);
    i_video_memory->unset_mapping(VideoMemoryMappingPoint::NAMETABLE);
}

} // namespace ln
