#include "video_memory.hpp"

#define BASE MappableMemory<VideoMemoryMappingPoint, NH_ADDRESSABLE_SIZE>

namespace nh {

VideoMemory::VideoMemory(NHLogger *i_logger)
    : BASE(i_logger)
    , m_ram{}
    , m_palette{}
{
    // Nametable mirror
    {
        auto decode = [](const MappingEntry *i_entry, Address i_addr,
                         Byte *&o_addr) -> NHErr {
            VideoMemory *thiz = (VideoMemory *)i_entry->opaque;

            return thiz->decode_addr(i_addr & NH_NT_MIRROR_ADDR_MASK, o_addr);
        };
        set_mapping(VideoMemoryMappingPoint::NAMETABLE_MIRROR,
                    {NH_NT_MIRROR_ADDR_HEAD, NH_NT_MIRROR_ADDR_TAIL, false,
                     decode, this});
    }
    // Palette
    {
        auto decode = [](const MappingEntry *i_entry, Address i_addr,
                         Byte *&o_addr) -> NHErr {
            VideoMemory *thiz = (VideoMemory *)i_entry->opaque;

            Address addr = (i_addr & NH_PALETTE_ADDR_MASK) | i_entry->begin;
            // Addresses $3F10/$3F14/$3F18/$3F1C are mirrors of
            // $3F00/$3F04/$3F08/$3F0C.
            if ((addr & NH_PALETTE_ADDR_MIRROR_MASK) == 0)
            {
                addr &= NH_PALETTE_ADDR_BG_MASK;
            }

            Address index = addr - i_entry->begin;
            o_addr = &thiz->m_palette[index];
            return NH_ERR_OK;
        };
        set_mapping(
            VideoMemoryMappingPoint::PALETTE,
            {NH_PALETTE_ADDR_HEAD, NH_PALETTE_ADDR_TAIL, false, decode, this});
    }
    // Invalid addresses
    {
        auto decode = [](const MappingEntry *i_entry, Address i_addr,
                         Byte *&o_addr) -> NHErr {
            VideoMemory *thiz = (VideoMemory *)i_entry->opaque;

            // Valid addresses are $0000-$3FFF; higher addresses will be
            // mirrored down.
            return thiz->decode_addr(i_addr & NH_PPU_INVALID_ADDR_MASK, o_addr);
        };
        set_mapping(VideoMemoryMappingPoint::INVALID_REST,
                    {NH_PPU_INVALID_ADDR_HEAD, NH_PPU_INVALID_ADDR_TAIL, false,
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
                              Byte *&o_addr) -> NHErr {
        VideoMemory *thiz = (VideoMemory *)i_entry->opaque;

        static_assert(sizeof(thiz->m_ram) == sizeof(Byte) * 2 * NH_NT_ONE_SIZE,
                      "Wrong internal ram size");

        MirrorMode mode = i_mode_cb();
        switch (mode)
        {
            case MI_H:
            {
                // 0x2400 -> 0x2000
                // 0x2C00 -> 0x2800
                Address addr = i_addr & NH_NT_H_MIRROR_ADDR_MASK;
                Address index =
                    addr - i_entry->begin -
                    (addr >= NH_NT_2_ADDR_HEAD ? NH_NT_ONE_SIZE : 0);
                o_addr = &thiz->m_ram[index];
                return NH_ERR_OK;
            }
            break;

            case MI_V:
            {
                // 0x2800 -> 0x2000
                // 0x2C00 -> 0x2400
                Address addr = i_addr & NH_NT_V_MIRROR_ADDR_MASK;
                Address index = addr - i_entry->begin;
                o_addr = &thiz->m_ram[index];
                return NH_ERR_OK;
            }
            break;

            case MI_1_LOW:
            {
                // 0x2400 -> 0x2000
                // 0x2800 -> 0x2000
                // 0x2C00 -> 0x2000
                Address addr = i_addr & NH_NT_1_MIRROR_ADDR_MASK;
                Address index = addr - i_entry->begin;
                o_addr = &thiz->m_ram[index];
                return NH_ERR_OK;
            }
            break;

            case MI_1_HIGH:
            {
                // 0x2400 -> 0x2000
                // 0x2800 -> 0x2000
                // 0x2C00 -> 0x2000
                Address addr = i_addr & NH_NT_1_MIRROR_ADDR_MASK;
                Address index = addr - i_entry->begin + NH_NT_ONE_SIZE;
                o_addr = &thiz->m_ram[index];
                return NH_ERR_OK;
            }
            break;

            default:
            {
                // Impossible
                o_addr = &thiz->m_ram[0];
                return NH_ERR_OK;
            }
            break;
        }
    };
    set_mapping(VideoMemoryMappingPoint::NAMETABLE,
                {NH_NT_ADDR_HEAD, NH_NT_ADDR_TAIL, false, decode, this});
}

void
VideoMemory::unset_mirror()
{
    unset_mapping(VideoMemoryMappingPoint::NAMETABLE);
}

NHErr
VideoMemory::get_byte(Address i_addr, Byte &o_val) const
{
    // https://www.nesdev.org/wiki/Open_bus_behavior#PPU_open_bus
    auto err = BASE::get_byte(i_addr, o_val);
    if (err == NH_ERR_UNAVAILABLE)
    {
        o_val = i_addr & 0x00FF;
        err = NH_ERR_OK;
    }
    return err;
}

Byte
VideoMemory::get_palette_byte(int i_idx)
{
    return m_palette[i_idx];
}

} // namespace nh
