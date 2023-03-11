#pragma once

#include "nesish/nesish.h"
#include "memory/memory.hpp"
#include "memory/video_memory.hpp"

namespace nh {

struct Cartridge {
  public:
    virtual ~Cartridge() = default;

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
};

} // namespace nh
