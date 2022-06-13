#pragma once

#include "console/types.hpp"
#include "common/klass.hpp"
#include "console/clock.hpp"
#include "console/ppu/frame_buffer.hpp"
#include "console/ppu/palette_default.hpp"
#include "console/memory/video_memory.hpp"

namespace ln {

struct PipelineAccessor;
struct Pipeline;
struct CPU;

struct PPU {
  public:
    PPU(VideoMemory *i_memory, CPU *i_cpu);
    ~PPU();
    LN_KLZ_DELETE_COPY_MOVE(PPU);

    void
    power_up();
    void
    reset();

    void
    tick();

    // @NOTE: The value must can be used as array index, see "m_regs".
    // https://wiki.nesdev.org/w/index.php?title=PPU_registers
    enum Register {
        PPUCTRL = 0, // NMI enable (V), PPU master/slave (P), sprite height (H),
                     // background tile select (B), sprite tile select (S),
                     // increment mode (I), nametable select (NN)
        PPUMASK,   // color emphasis (BGR), sprite enable (s), background enable
                   // (b), sprite left column enable (M), background left column
                   // enable (m), greyscale (G)
        PPUSTATUS, // vblank (V), sprite 0 hit (S), sprite overflow (O); read
                   // resets write pair for $2005/$2006
        OAMADDR,   // OAM read/write address
        OAMDATA,   // OAM data read/write
        PPUSCROLL, // fine scroll position (two writes: X scroll, Y scroll)
        PPUADDR,   // PPU read/write address (two writes: most significant byte,
                   // least significant byte)
        PPUDATA,   // PPU data read/write

        OAMDMA, // OAM DMA high address

        SIZE,
    };

    Byte
    read_register(Register i_reg);
    void
    write_register(Register i_reg, Byte i_val);

  private:
    /* allow access from other internal components */

    friend struct Emulator;
    FrameBuffer *
    frame_dirty();

  private:
    Byte &
    get_register(PPU::Register i_reg);

    void
    inc_vram_addr();

    void
    check_gen_nmi();

  private:
    Byte m_regs[Register::SIZE];
    Byte m_ppudata_buf;

    // ---- External components references
    VideoMemory *m_memory;
    CPU *m_cpu;

    friend struct PipelineAccessor;
    PipelineAccessor *m_pipeline_accessor;

    /* internal registers */
    // https://www.nesdev.org/wiki/PPU_scrolling#PPU_internal_registers
    Byte2 v;
    Byte2 t;
    Byte x;
    Byte w;

    struct PipelineContext {
        Byte bg_nt_byte;          // intermediate to get pattern
        Byte bg_attr_palette_idx; // 2-bit
        Byte bg_lower_sliver;
        Byte bg_upper_sliver;

        /* internal shift registers (and the latches) */
        // https://www.nesdev.org/wiki/PPU_rendering#Preface
        Byte2 shift_pattern_lower;
        Byte2 shift_pattern_upper;
        // the latch need only 1 bit each, but we expand it to 8-bit for
        // convenience.
        Byte2 shift_palette_idx_lower;
        Byte2 shift_palette_idx_upper;

        bool is_odd_frame;
        int scanline_no;
        int pixel_row;
        int pixel_col;
    } m_pipeline_ctx;
    Pipeline *m_pipeline;

    FrameBuffer m_frame_buf;
    bool m_frame_buf_dirty;

    PaletteDefault m_palette;
};

} // namespace ln
