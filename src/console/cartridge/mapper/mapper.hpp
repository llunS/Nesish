#pragma once

#include "common/error.hpp"
#include "console/cartridge/ines.hpp"
#include "console/memory/memory.hpp"
#include "console/memory/video_memory.hpp"

namespace ln {

struct Mapper {
  public:
    Mapper(const INES::RomAccessor *i_accessor);
    virtual ~Mapper() = default;

    virtual Error
    validate() const = 0;

    virtual void
    power_up() = 0;
    virtual void
    reset() = 0;

    virtual void
    map_memory(Memory *o_memory, VideoMemory *o_video_memory) = 0;
    virtual void
    unmap_memory(Memory *o_memory, VideoMemory *o_video_memory) = 0;

  protected:
    void
    set_fixed_vh_mirror(VideoMemory *o_video_memory);
    void
    unset_fixed_vh_mirror(VideoMemory *o_video_memory);

  protected:
    const INES::RomAccessor *m_rom_accessor;
};

} // namespace ln
