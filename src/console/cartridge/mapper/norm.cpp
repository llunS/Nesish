#include "norm.hpp"

#include "console/assert.hpp"

#define LN_NORM_128_PRG_RAM_SIZE 16 * 1024
#define LN_NORM_256_PRG_RAM_SIZE 32 * 1024

namespace ln {

NORM::NORM(const INES::RomAccessor *i_accessor)
    : Mapper{i_accessor}
    , m_prg_ram{}
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
NORM::map_memory(Memory *i_memory)
{
    // PRG ROM
    auto prg_rom_decode = [](const MappingEntry *i_entry,
                             Address i_addr) -> Byte * {
        auto this_ = (NORM *)i_entry->opaque;
        auto accessor = this_->m_rom_accessor;

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

        Byte *byte_ptr = rom_base + rel_address;
        return byte_ptr;
    };
    i_memory->set_mapping(MemoryMappingPoint::PRG_ROM,
                          {0x8000, 0xFFFF, true, prg_rom_decode, (void *)this});

    // PRG RAM
    auto prg_ram_decode = [](const MappingEntry *i_entry,
                             Address i_addr) -> Byte * {
        auto this_ = (NORM *)i_entry->opaque;

        Byte *byte_ptr = this_->m_prg_ram + (i_addr - i_entry->begin);
        return byte_ptr;
    };
    i_memory->set_mapping(
        MemoryMappingPoint::PRG_RAM,
        {0x6000, 0x7FFF, false, prg_ram_decode, (void *)this});

    // @TODO: CHR
}

void
NORM::unmap_memory(Memory *i_memory) const
{
    i_memory->unset_mapping(MemoryMappingPoint::PRG_ROM);
    i_memory->unset_mapping(MemoryMappingPoint::PRG_RAM);

    // @TODO: CHR
}

} // namespace ln
