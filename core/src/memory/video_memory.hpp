#pragma once

#include "memory/mappable_memory.hpp"
#include "nhbase/klass.hpp"
#include "spec.hpp"
#include "types.hpp"

#include <functional>

namespace nh {

/* @NOTE: Enumerators must be unique and are valid array index */
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
    : public MappableMemory<VideoMemoryMappingPoint, NH_ADDRESSABLE_SIZE> {
  public:
    VideoMemory(NHLogger *i_logger);
    NB_KLZ_DELETE_COPY_MOVE(VideoMemory);

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

  public:
    NHErr
    get_byte(Address i_addr, Byte &o_val) const;

    Byte
    get_palette_byte(int i_idx);

  private:
    // https://www.nesdev.org/wiki/PPU_memory_map
    Byte m_ram[NH_PPU_INTERNAL_RAM_SIZE];
    Byte m_palette[NH_PALETTE_SIZE];
};

} // namespace nh
