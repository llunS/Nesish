#include "norm.hpp"

#include "console/assert.hpp"

#define LN_NORM_128_PRG_RAM_SIZE 16 * 1024
#define LN_NORM_256_PRG_RAM_SIZE 32 * 1024

namespace ln {

NORM::NORM(const INES::RomAccessor *i_accessor)
    : Mapper{i_accessor}
    , m_prg_ram{}
    , m_chr_rom{}
{
}

Error
NORM::validate() const
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
NORM::map_memory(const INES *i_nes, Memory *i_memory, PPUMemory *i_ppu_memory)
{
    // PRG ROM
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            auto thiz = (NORM *)i_entry->opaque;
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
            auto thiz = (NORM *)i_entry->opaque;

            return thiz->m_prg_ram + (i_addr - i_entry->begin);
        };
        i_memory->set_mapping(MemoryMappingPoint::PRG_RAM,
                              {0x6000, 0x7FFF, false, decode, this});
    }

    // CHR ROM
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            auto thiz = (NORM *)i_entry->opaque;

            return thiz->m_chr_rom + (i_addr - i_entry->begin);
        };
        i_ppu_memory->set_mapping(
            PPUMemoryMappingPoint::PATTERN,
            {LN_PATTERN_ADDR_HEAD, LN_PATTERN_ADDR_TAIL, true, decode, this});
    }
    // mirroring
    i_ppu_memory->configure_mirror(i_nes->h_mirror());
}

void
NORM::unmap_memory(const INES *i_nes, Memory *i_memory,
                   PPUMemory *i_ppu_memory) const
{
    (void)(i_nes);

    i_memory->unset_mapping(MemoryMappingPoint::PRG_ROM);
    i_memory->unset_mapping(MemoryMappingPoint::PRG_RAM);

    i_ppu_memory->unset_mapping(PPUMemoryMappingPoint::PATTERN);
    i_ppu_memory->unset_mapping(PPUMemoryMappingPoint::NAMETABLE);
}

} // namespace ln
