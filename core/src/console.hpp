#pragma once

#include "nhbase/klass.hpp"
#include "nesish/nesish.h"

#include "cpu/cpu.hpp"
#include "memory/memory.hpp"

#include "ppu/ppu.hpp"
#include "ppu/oam_dma.hpp"
#include "memory/video_memory.hpp"
#include "ppu/frame_buffer.hpp"

#include "cartridge/cartridge.hpp"

#include "apu/apu_clock.hpp"
#include "apu/apu.hpp"
#include "apu/dmc_dma.hpp"

#include "spec.hpp"
#include "types.hpp"
#include "debug/debug_flags.hpp"

#include <string>

namespace nh {

struct Console {
  public:
    Console(NHLogger *i_logger);
    ~Console();
    NB_KLZ_DELETE_COPY_MOVE(Console);

    void
    plug_controller(NHCtrlSlot i_slot, NHController *i_controller);
    void
    unplug_controller(NHCtrlSlot i_slot);

    NHErr
    insert_cartridge(const std::string &i_rom_path);
    void
    power_up();
    void
    reset();

    /// @return How many seconds have passed for input ticks
    double
    elapsed(Cycle i_ticks);
    /// @param i_duration In seconds
    /// @return How many ticks are generated for input duration
    Cycle
    ticks(double i_duration);
    /// @brief Advance 1 CPU tick
    /// @param o_cpu_instr If a CPU instruction has completed
    /// @return If a new audio sample is available
    bool
    tick(bool *o_cpu_instr = nullptr);

    const FrameBuffer &
    get_frame() const;

    float
    get_sample_rate() const;
    /// @return Amplitude in range [0, 1]
    float
    get_sample() const;

  public:
    /* debug */

    void
    set_debug_on(NHDFlags i_flag);
    void
    set_debug_off(NHDFlags i_flag);

    const nhd::Palette &
    dbg_get_palette() const;
    const nhd::OAM &
    dbg_get_oam() const;
    const nhd::PatternTable &
    dbg_get_ptn_tbl(bool i_right) const;
    void
    dbg_set_ptn_tbl_palette(NHDPaletteSet i_palette);

  public:
    /* test */

    CPU *
    test_get_cpu();

  private:
    void
    hard_wire();

  private:
    // @NOTE: Values must be valid array index, see "m_ctrl_regs".
    enum CtrlReg {
        REG_4016 = 0,
        REG_4017,
        SIZE,
    };

    Byte
    read_ctrl_reg(CtrlReg i_reg);
    void
    write_ctrl_reg(CtrlReg i_reg, Byte i_val);

  private:
    void
    reset_internal();

  private:
    CPU m_cpu;
    Memory m_memory;
    PPU m_ppu;
    OAMDMA m_oam_dma;
    VideoMemory m_video_memory;
    APUClock m_apu_clock;
    APU m_apu;
    DMCDMA m_dmc_dma;

    Cartridge *m_cart;

    Byte m_ctrl_regs[CtrlReg::SIZE];
    static constexpr int CTRL_SIZE = 2;
    static_assert(CTRL_SIZE == CtrlReg::SIZE, "?");
    NHController *m_ctrls[CTRL_SIZE]; // References

  private:
    NHLogger *m_logger;

  private:
    NHDFlags m_debug_flags;
};

} // namespace nh
