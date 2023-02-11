#pragma once

#include "common/error.hpp"
#include "console/memory/memory.hpp"
#include "console/memory/video_memory.hpp"

namespace ln {

struct Cartridge {
  public:
    virtual ~Cartridge() = default;

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
};

} // namespace ln
