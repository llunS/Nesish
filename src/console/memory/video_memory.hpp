#pragma once

#include "console/memory/mappable_memory.hpp"
#include "common/klass.hpp"
#include "console/spec.hpp"
#include "console/types.hpp"
#include <functional>

namespace ln {

/* @IMPL: Valid enumerators must be unique and can be used as array index */
enum class VideoMemoryMappingPoint : unsigned char {
    INVALID = 0,

    // https://www.nesdev.org/wiki/PPU_memory_map
    PATTERN,
    NAMETABLE,
    NAMETABLE_MIRROR,
    PALETTE,

    INVALID_REST,

    SIZE,
};

struct VideoMemory
    : public MappableMemory<VideoMemoryMappingPoint, LN_ADDRESSABLE_SIZE> {
  public:
    VideoMemory();
    LN_KLZ_DELETE_COPY_MOVE(VideoMemory);

  public:
    enum MirrorMode {
        MI_H,
        MI_V,
        MI_1_LOW,
        MI_1_HIGH,
    };
    void
    set_mirror_fixed(MirrorMode i_mode);
    typedef std::function<MirrorMode()> ModeCallback;
    void
    set_mirror_dyn(ModeCallback i_mode_cb);
    void
    unset_mirror();

  private:
    // https://www.nesdev.org/wiki/PPU_memory_map
    Byte m_ram[LN_PPU_INTERNAL_RAM_SIZE];
    Byte m_palette[LN_PALETTE_SIZE];
};

} // namespace ln
