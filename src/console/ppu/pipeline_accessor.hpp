#pragma once

#include "common/klass.hpp"
#include "console/types.hpp"
#include "console/ppu/ppu.hpp"

namespace ln {

struct PipelineAccessor {
  public:
    LN_KLZ_DELETE_COPY_MOVE(PipelineAccessor);

  public:
    PPU::PipelineContext &
    get_context();

    Byte &
    get_register(PPU::Register i_reg);
    Byte2 &
    get_v();
    Byte2 &
    get_t();
    Byte &
    get_x();

    VideoMemory *
    get_memory();

    const Palette &
    get_palette();
    FrameBuffer &
    get_frame_buf();

    bool
    bg_enabled();
    bool
    sprite_enabled();
    bool
    rendering_enabled();

    void
    check_gen_nmi();

    void
    finish_frame();

  private:
    PipelineAccessor(PPU *i_ppu);
    friend struct PPU; // allow constructor access

  private:
    PPU *m_ppu;
};

} // namespace ln
