#pragma once

#include "common/error.hpp"
#include "console/dllexport.h"
#include "common/klass.hpp"
#include "common/time.hpp"

#include "console/cpu/cpu.hpp"
#include "console/memory/memory.hpp"

#include "console/ppu/ppu.hpp"
#include "console/ppu/oam_dma.hpp"
#include "console/memory/video_memory.hpp"
#include "console/ppu/frame_buffer.hpp"
#include "console/ppu/color.hpp"

#include "console/cartridge/cartridge.hpp"
#include "console/peripheral/controller.hpp"

#include "console/apu/apu_clock.hpp"
#include "console/apu/apu.hpp"
#include "console/apu/dmc_dma.hpp"

#include "console/spec.hpp"
#include "console/types.hpp"
#include "console/debug/debug_flags.hpp"

#include <string>
#include <cstddef>

namespace nh {

enum CtrlSlot {
    CTRL_P1,
    CTRL_P2,
    CTRL_SIZE,
};

struct Emulator {
  public:
    LN_CONSOLE_API
    Emulator();
    LN_CONSOLE_API ~Emulator();
    LN_KLZ_DELETE_COPY_MOVE(Emulator);

    LN_CONSOLE_API void
    plug_controller(CtrlSlot i_slot, Controller *i_controller);
    LN_CONSOLE_API void
    unplug_controller(CtrlSlot i_slot);

    LN_CONSOLE_API Error
    insert_cartridge(const std::string &i_rom_path);
    LN_CONSOLE_API void
    power_up();
    LN_CONSOLE_API void
    reset();

    /// @return How many seconds have passed for input ticks
    LN_CONSOLE_API Time_t
    elapsed(Cycle i_ticks);
    /// @param i_duration In seconds
    /// @return How many ticks are generated for input duration
    LN_CONSOLE_API Cycle
    ticks(Time_t i_duration);
    /// @brief Advance 1 CPU tick
    /// @param o_cpu_instr If a CPU instruction has completed
    /// @return If a new audio sample is available
    LN_CONSOLE_API bool
    tick(bool *o_cpu_instr = nullptr);

    LN_CONSOLE_API const FrameBuffer &
    get_frame() const;

    LN_CONSOLE_API float
    get_sample_rate() const;
    /// @return Amplitude in range [0, 1]
    LN_CONSOLE_API float
    get_sample() const;

  public:
    /* debug */

    LN_CONSOLE_API void
    set_debug_on(nhd::DebugFlags i_flag);
    LN_CONSOLE_API void
    set_debug_off(nhd::DebugFlags i_flag);

    LN_CONSOLE_API const nhd::Palette &
    get_palette_dbg() const;
    LN_CONSOLE_API const nhd::OAM &
    get_oam_dbg() const;
    LN_CONSOLE_API const nhd::PatternTable &
    get_ptn_tbl_dbg(bool i_right) const;
    enum PaletteSet : unsigned char {
        BG0 = 0,
        BG1 = 1,
        BG2 = 2,
        BG3 = 3,
        SP0 = 4,
        SP1 = 5,
        SP2 = 6,
        SP3 = 7,
    };
    LN_CONSOLE_API void
    set_ptn_tbl_palette_dbg(PaletteSet i_palette);

  public:
    /* test */

    LN_CONSOLE_API const CPU &
    get_cpu_test() const;

    typedef void (*TestInitFunc)(nh::CPU *io_cpu, void *i_context);
    LN_CONSOLE_API void
    init_test(TestInitFunc i_init_func, void *i_context);

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
    Controller *m_ctrls[CTRL_SIZE]; // References

  private:
    nhd::DebugFlags m_debug_flags;
};

} // namespace nh
