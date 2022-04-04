#include "ppu_memory.hpp"

namespace ln {

PPUMemory::PPUMemory()
    : m_ram{}
    , m_palette{}
    , m_oam{}
{
    // Nametable mirror
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            PPUMemory *thiz = (PPUMemory *)i_entry->opaque;

            return thiz->decode_addr(i_addr & LN_NT_MIRROR_ADDR_MASK, -1);
        };
        set_mapping(PPUMemoryMappingPoint::NAMETABLE_MIRROR,
                    {LN_NT_MIRROR_ADDR_HEAD, LN_NT_MIRROR_ADDR_TAIL, false,
                     decode, this});
    }
    // Palette
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            PPUMemory *thiz = (PPUMemory *)i_entry->opaque;

            Address addr = (i_addr & LN_PALETTE_ADDR_MASK) | i_entry->begin;
            // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of
            // $3F00/$3F04/$3F08/$3F0C.
            if ((addr & LN_PALETTE_ADDR_COLOR_MASK) == 0)
            {
                addr &= LN_PALETTE_ADDR_BKG_MASK;
            }

            Address index = addr - i_entry->begin;
            return &thiz->m_palette[index];
        };
        set_mapping(
            PPUMemoryMappingPoint::PALETTE,
            {LN_PALETTE_ADDR_HEAD, LN_PALETTE_ADDR_TAIL, false, decode, this});
    }
}

void
PPUMemory::configure_mirror(bool i_horizontal)
{
    auto decode = [i_horizontal](const MappingEntry *i_entry,
                                 Address i_addr) -> Byte * {
        PPUMemory *thiz = (PPUMemory *)i_entry->opaque;

        if (i_horizontal)
        {
            Address addr = i_addr & LN_NT_H_MIRROR_ADDR_MASK;
            Address index = addr - i_entry->begin -
                            (addr >= LN_NT_2_ADDR_HEAD ? LN_NT_SIZE : 0);
            return &thiz->m_ram[index];
        }
        else
        {
            Address addr = i_addr & LN_NT_V_MIRROR_ADDR_MASK;
            Address index = addr - i_entry->begin;
            return &thiz->m_ram[index];
        }
    };
    set_mapping(PPUMemoryMappingPoint::NAMETABLE,
                {LN_NT_ADDR_HEAD, LN_NT_ADDR_TAIL, false, decode, this});
}

} // namespace ln
