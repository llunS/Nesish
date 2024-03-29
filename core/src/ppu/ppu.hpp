#pragma once

#include "types.hpp"
#include "nhbase/klass.hpp"
#include "ppu/frame_buffer.hpp"
#include "ppu/palette_default.hpp"
#include "memory/video_memory.hpp"
#include "spec.hpp"

#include "debug/debug_flags.hpp"
#include "debug/palette.hpp"
#include "debug/oam.hpp"
#include "debug/pattern_table.hpp"

struct NHLogger;

namespace nh {

struct PipelineAccessor;
struct Pipeline;

struct PPU {
  public:
    PPU(VideoMemory *i_memory, const NHDFlag &i_debug_flags,
        NHLogger *i_logger);
    ~PPU();
    NB_KLZ_DELETE_COPY_MOVE(PPU);

  public:
    void
    power_up();
    void
    reset();

    void
    tick(bool i_no_nmi = false);

    bool
    nmi() const;

  public:
    // https://wiki.nesdev.org/w/index.php?title=PPU_registers
    // Values must be valid array index, see "m_regs".
    enum Register {
        /* clang-format off */
        PPUCTRL = 0, // VPHB SINN / NMI enable (V), PPU master/slave (P), sprite height (H), background tile select (B), sprite tile select (S), increment mode (I), nametable select (NN)
        PPUMASK, // BGRs bMmG / color emphasis (BGR), sprite enable (s), background enable (b), sprite left column enable (M), background left column enable (m), greyscale (G)
        PPUSTATUS, // VSO- ---- / vblank (V), sprite 0 hit (S), sprite overflow (O); read resets write pair for $2005/$2006
        OAMADDR, // OAM read/write address
        OAMDATA, // OAM data read/write
        PPUSCROLL, // fine scroll position (two writes: X scroll, Y scroll)
        PPUADDR, // PPU read/write address (two writes: most significant byte, least significant byte)
        PPUDATA, // PPU data read/write

        SIZE,
        /* clang-format on */
    };

    Byte
    read_register(Register i_reg);
    void
    write_register(Register i_reg, Byte i_val);

  private:
    bool
    reg_read_only(Register i_reg);
    bool
    reg_wrtie_only(Register i_reg);

  private:
    friend struct Console;
    const FrameBuffer &
    get_frame() const;

  private:
    /* debug */

    friend struct Console;
    const nhd::Palette &
    dbg_get_palette() const;
    friend struct Console;
    const nhd::OAM &
    dbg_get_oam() const;
    friend struct Console;
    const nhd::PatternTable &
    dbg_get_ptn_tbl(bool i_right) const;
    friend struct Console;
    /// @param i_idx [0, 7]
    void
    dbg_set_ptn_tbl_palette(unsigned char i_idx);

  private:
    Byte &
    get_register(PPU::Register i_reg);
    const Byte &
    get_register(PPU::Register i_reg) const;

    void
    inc_vram_addr();

    void
    check_gen_nmi();

  private:
    void
    reset_trivial();

  private:
    Byte m_regs[Register::SIZE];
    Byte m_ppudata_buf;

    /* OAM */
    Byte m_oam[NH_OAM_SIZE];

    // ---- External components references
    VideoMemory *m_memory;

    friend struct PipelineAccessor;
    PipelineAccessor *m_pipeline_accessor;

    /* internal registers */
    // https://www.nesdev.org/wiki/PPU_scrolling#PPU_internal_registers
    Byte2 v;
    Byte2 t;
    Byte x;
    Byte w;

    struct PipelineContext {
        // ------ Shared pipeline states
        bool odd_frame;
        bool skip_cycle; // skip last cycle at pre-render scanline
        int scanline_no; // [-1, 260], i.e. 261 == -1.
        int pixel_row;
        int pixel_col;

        // ------ Background
        // fetch
        Byte bg_nt_byte;          // intermediate to get pattern
        Byte bg_attr_palette_idx; // 2-bit
        Byte bg_lower_sliver;
        Byte bg_upper_sliver;

        // rendering
        /* internal shift registers (and the latches) */
        // https://www.nesdev.org/wiki/PPU_rendering#Preface
        Byte2 sf_bg_pattern_lower;
        Byte2 sf_bg_pattern_upper;
        // the latch need only 1 bit each, but we expand it to 8-bit for
        // convenience.
        Byte2 sf_bg_palette_idx_lower;
        Byte2 sf_bg_palette_idx_upper;

        // ------ Sprite
        // pre-fetch
        Byte sec_oam[NH_SEC_OAM_SIZE];
        // fetch and register reload
        Byte sf_sp_pattern_lower[NH_MAX_VISIBLE_SP_NUM];
        Byte sf_sp_pattern_upper[NH_MAX_VISIBLE_SP_NUM];
        Byte sp_attr[NH_MAX_VISIBLE_SP_NUM];
        Byte sp_pos_x[NH_MAX_VISIBLE_SP_NUM];
        // metadata passed on to rendering stage on next scanline
        Byte sp_count; // how many sprites are in range
        bool with_sp0; // sprites to render include sprite 0
    } m_pipeline_ctx;
    Pipeline *m_pipeline;

    FrameBuffer m_back_buf;
    FrameBuffer m_front_buf;

    PaletteDefault m_palette;

    // The data bus used to communicate with CPU, to implement open bus
    // behavior
    // https://www.nesdev.org/wiki/PPU_registers#Ports
    // https://www.nesdev.org/wiki/Open_bus_behavior#PPU_open_bus
    Byte m_io_db;

    // ---- temporaries for one tick
    bool m_no_nmi;

  private:
    /* debug */

    const NHDFlag &m_debug_flags;
    nhd::Palette m_palette_snap;
    nhd::OAM m_oam_snap;
    nhd::PatternTable m_ptn_tbl_l_snap;
    nhd::PatternTable m_ptn_tbl_r_snap;
    unsigned char m_ptn_tbl_palette_idx;

  private:
    NHLogger *m_logger;
};

} // namespace nh
