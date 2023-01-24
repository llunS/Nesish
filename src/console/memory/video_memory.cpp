#include "video_memory.hpp"

namespace ln {

VideoMemory::VideoMemory()
    : m_ram{}
    , m_palette{}
{
    // Nametable mirror
    {
        auto decode = [](const MappingEntry *i_entry, Address i_addr,
                         Byte *&o_addr) -> ln::Error {
            VideoMemory *thiz = (VideoMemory *)i_entry->opaque;

            return thiz->decode_addr_raw(i_addr & LN_NT_MIRROR_ADDR_MASK,
                                         o_addr);
        };
        set_mapping(VideoMemoryMappingPoint::NAMETABLE_MIRROR,
                    {LN_NT_MIRROR_ADDR_HEAD, LN_NT_MIRROR_ADDR_TAIL, false,
                     decode, this});
    }
    // Palette
    {
        auto decode = [](const MappingEntry *i_entry, Address i_addr,
                         Byte *&o_addr) -> ln::Error {
            VideoMemory *thiz = (VideoMemory *)i_entry->opaque;

            Address addr = (i_addr & LN_PALETTE_ADDR_MASK) | i_entry->begin;
            // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of
            // $3F00/$3F04/$3F08/$3F0C.
            if ((addr & LN_PALETTE_ADDR_MIRROR_MASK) == 0)
            {
                addr &= LN_PALETTE_ADDR_BG_MASK;
            }

            Address index = addr - i_entry->begin;
            o_addr = &thiz->m_palette[index];
            return Error::OK;
        };
        set_mapping(
            VideoMemoryMappingPoint::PALETTE,
            {LN_PALETTE_ADDR_HEAD, LN_PALETTE_ADDR_TAIL, false, decode, this});
    }
    // Invalid addresses
    {
        auto decode = [](const MappingEntry *i_entry, Address i_addr,
                         Byte *&o_addr) -> ln::Error {
            VideoMemory *thiz = (VideoMemory *)i_entry->opaque;

            // Valid addresses are $0000-$3FFF; higher addresses will be
            // mirrored down.
            return thiz->decode_addr_raw(i_addr & LN_PPU_INVALID_ADDR_MASK,
                                         o_addr);
        };
        set_mapping(VideoMemoryMappingPoint::INVALID_REST,
                    {LN_PPU_INVALID_ADDR_HEAD, LN_PPU_INVALID_ADDR_TAIL, false,
                     decode, this});
    }
}

void
VideoMemory::set_mirror_fixed(MirrorMode i_mode)
{
    set_mirror_dyn([i_mode]() { return i_mode; });
}

void
VideoMemory::set_mirror_dyn(ModeCallback i_mode_cb)
{
    auto decode = [i_mode_cb](const MappingEntry *i_entry, Address i_addr,
                              Byte *&o_addr) -> ln::Error {
        VideoMemory *thiz = (VideoMemory *)i_entry->opaque;

        static_assert(sizeof(thiz->m_ram) == sizeof(Byte) * 2 * LN_NT_ONE_SIZE,
                      "Wrong internal ram size.");

        MirrorMode mode = i_mode_cb();
        switch (mode)
        {
            case MI_H:
            {
                // 0x2400 -> 0x2000
                // 0x2C00 -> 0x2800
                Address addr = i_addr & LN_NT_H_MIRROR_ADDR_MASK;
                Address index =
                    addr - i_entry->begin -
                    (addr >= LN_NT_2_ADDR_HEAD ? LN_NT_ONE_SIZE : 0);
                o_addr = &thiz->m_ram[index];
                return Error::OK;
            }
            break;

            case MI_V:
            {
                // 0x2800 -> 0x2000
                // 0x2C00 -> 0x2400
                Address addr = i_addr & LN_NT_V_MIRROR_ADDR_MASK;
                Address index = addr - i_entry->begin;
                o_addr = &thiz->m_ram[index];
                return Error::OK;
            }
            break;

            case MI_1_LOW:
            {
                // 0x2400 -> 0x2000
                // 0x2800 -> 0x2000
                // 0x2C00 -> 0x2000
                Address addr = i_addr & LN_NT_1_MIRROR_ADDR_MASK;
                Address index = addr - i_entry->begin;
                o_addr = &thiz->m_ram[index];
                return Error::OK;
            }
            break;

            case MI_1_HIGH:
            {
                // 0x2400 -> 0x2000
                // 0x2800 -> 0x2000
                // 0x2C00 -> 0x2000
                Address addr = i_addr & LN_NT_1_MIRROR_ADDR_MASK;
                Address index = addr - i_entry->begin + LN_NT_ONE_SIZE;
                o_addr = &thiz->m_ram[index];
                return Error::OK;
            }
            break;

            default:
            {
                // Impossible
                o_addr = &thiz->m_ram[0];
                return Error::OK;
            }
            break;
        }
    };
    set_mapping(VideoMemoryMappingPoint::NAMETABLE,
                {LN_NT_ADDR_HEAD, LN_NT_ADDR_TAIL, false, decode, this});
}

void
VideoMemory::unset_mirror()
{
    unset_mapping(VideoMemoryMappingPoint::NAMETABLE);
}

} // namespace ln
