#pragma once

#include "nesish/nesish.h"
#include "cartridge/ines.hpp"
#include "memory/memory.hpp"
#include "memory/video_memory.hpp"

namespace nh {

struct Mapper {
  public:
    Mapper(const INES::RomAccessor *i_accessor);
    virtual ~Mapper() = default;

    virtual NHErr
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

} // namespace nh
