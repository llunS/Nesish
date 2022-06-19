#include "video_memory.hpp"

namespace ln {

VideoMemory::VideoMemory()
    : m_ram{}
    , m_palette{}
{
    // Nametable mirror
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            VideoMemory *thiz = (VideoMemory *)i_entry->opaque;

            return thiz->decode_addr_raw(i_addr & LN_NT_MIRROR_ADDR_MASK);
        };
        set_mapping(VideoMemoryMappingPoint::NAMETABLE_MIRROR,
                    {LN_NT_MIRROR_ADDR_HEAD, LN_NT_MIRROR_ADDR_TAIL, false,
                     decode, this});
    }
    // Palette
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            VideoMemory *thiz = (VideoMemory *)i_entry->opaque;

            Address addr = (i_addr & LN_PALETTE_ADDR_MASK) | i_entry->begin;
            // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of
            // $3F00/$3F04/$3F08/$3F0C.
            if ((addr & LN_PALETTE_ADDR_BACKDROP_MASK) == 0)
            {
                addr &= LN_PALETTE_ADDR_BKG_MASK;
            }

            Address index = addr - i_entry->begin;
            return &thiz->m_palette[index];
        };
        set_mapping(
            VideoMemoryMappingPoint::PALETTE,
            {LN_PALETTE_ADDR_HEAD, LN_PALETTE_ADDR_TAIL, false, decode, this});
    }
    // Invalid addresses
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            VideoMemory *thiz = (VideoMemory *)i_entry->opaque;

            // Valid addresses are $0000-$3FFF; higher addresses will be
            // mirrored down.
            return thiz->decode_addr_raw(i_addr & LN_PPU_INVALID_ADDR_MASK);
        };
        set_mapping(VideoMemoryMappingPoint::INVALID_REST,
                    {LN_PPU_INVALID_ADDR_HEAD, LN_PPU_INVALID_ADDR_TAIL, false,
                     decode, this});
    }
}

void
VideoMemory::configure_mirror(bool i_horizontal)
{
    auto decode = [i_horizontal](const MappingEntry *i_entry,
                                 Address i_addr) -> Byte * {
        VideoMemory *thiz = (VideoMemory *)i_entry->opaque;

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
    set_mapping(VideoMemoryMappingPoint::NAMETABLE,
                {LN_NT_ADDR_HEAD, LN_NT_ADDR_TAIL, false, decode, this});
}

} // namespace ln
