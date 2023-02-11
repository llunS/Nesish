#include "mapper.hpp"

namespace ln {

Mapper::Mapper(const INES::RomAccessor *i_accessor)
    : m_rom_accessor(i_accessor)
{
}

void
Mapper::set_fixed_vh_mirror(VideoMemory *o_video_memory)
{
    o_video_memory->set_mirror_fixed(
        m_rom_accessor->h_mirror() ? VideoMemory::MI_H : VideoMemory::MI_V);
}

void
Mapper::unset_fixed_vh_mirror(VideoMemory *o_video_memory)
{
    o_video_memory->unset_mirror();
}

} // namespace ln
