#pragma once

#include "nhbase/klass.hpp"
#include "types.hpp"
#include "ppu/ppu.hpp"

namespace nhd {
struct Sprite;
} // namespace nhd

namespace nh {

struct PipelineAccessor final {
  public:
    NB_KLZ_DELETE_COPY_MOVE(PipelineAccessor);

  public:
    PPU::PipelineContext &
    get_context();

    NHLogger *
    get_logger() const;

    Byte &
    get_register(PPU::Register i_reg);
    const Byte &
    get_register(PPU::Register i_reg) const;

    Byte2 &
    get_v();
    Byte2 &
    get_t();
    Byte &
    get_x();

    Byte
    get_oam_byte(Byte i_addr);
    Byte *
    get_oam_ptr(Byte i_addr);

    VideoMemory *
    get_memory();

    Byte
    get_color_byte(int i_idx);
    const Palette &
    get_palette();

    FrameBuffer &
    get_frame_buf();

    bool
    bg_enabled() const;
    bool
    sp_enabled() const;
    bool
    rendering_enabled() const;
    bool
    is_8x16_sp() const;

    bool
    no_nmi() const;

    void
    finish_frame();

  public:
    static Address
    get_sliver_addr(bool i_tbl_right, Byte i_tile_idx, bool i_upper,
                    Byte i_fine_y);
    static NHErr
    get_ptn_sliver(bool i_tbl_right, Byte i_tile_idx, bool i_upper,
                   Byte i_fine_y, const VideoMemory *i_vram, Byte &o_val);

    static void
    resolve_sp_ptn_tbl(Byte i_tile_byte, bool i_8x16, bool i_ptn_tbl_bit,
                       bool &o_high_ptn_tbl);
    static void
    resolve_sp_tile(Byte i_tile_byte, bool i_8x16, bool i_flip_y,
                    Byte i_fine_y_sp, Byte &o_tile_idx);

  private:
    /* debug */

    bool
    capture_palette_on();
    void
    capture_palette();

    bool
    capture_oam_on();
    void
    capture_oam();
    void
    update_oam_sprite(nhd::Sprite &o_sprite, int i_idx);

    bool
    capture_ptn_tbls_on();
    void
    capture_ptn_tbls();

  private:
    PipelineAccessor(PPU *i_ppu);
    friend struct PPU; // allow constructor access

  private:
    PPU *m_ppu;
};

} // namespace nh
