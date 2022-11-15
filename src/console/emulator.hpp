#pragma once

#include "common/error.hpp"
#include "console/dllexport.h"
#include "common/klass.hpp"
#include "console/cpu/cpu.hpp"
#include "console/memory/memory.hpp"
#include "console/ppu/ppu.hpp"
#include "console/memory/video_memory.hpp"
#include "console/cartridge/cartridge.hpp"
#include "console/peripheral/controller.hpp"
#include "console/clock.hpp"
#include "console/ppu/frame_buffer.hpp"
#include "console/spec.hpp"
#include "console/types.hpp"
#include "console/ppu/color.hpp"

#include "console/debug/debug_flags.hpp"

#include <string>
#include <cstddef>

namespace ln {

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

    LN_CONSOLE_API void
    advance(Time_t i_ms);

    LN_CONSOLE_API const FrameBuffer &
    get_frame() const;

  public:
    /* debug */

    LN_CONSOLE_API void
    set_debug_on(lnd::DebugFlags i_flag);
    LN_CONSOLE_API void
    set_debug_off(lnd::DebugFlags i_flag);

    LN_CONSOLE_API const lnd::Palette &
    get_palette_dbg() const;
    LN_CONSOLE_API const lnd::OAM &
    get_oam_dbg() const;

  public:
    /* test */

    LN_CONSOLE_API const CPU &
    get_cpu() const;

    typedef bool (*TestExitFunc)(const ln::CPU *i_cpu, std::size_t i_instr,
                                 void *i_context);
    LN_CONSOLE_API void
    run_test(Address i_entry, TestExitFunc i_exit_func, void *i_context);
    LN_CONSOLE_API Cycle
    tick_cpu_test();

  private:
    void
    hard_wire();

  private:
    // @IMPL: The enumerator must be consecutive and starts from 0 to be
    // implicitly converted from integer type.
    // e.g. See hard_wire() for details.
    enum CtrlReg {
        REG_4016 = 0,
        REG_4017,
    };

    Byte
    read_ctrl_reg(CtrlReg i_reg);
    void
    write_ctrl_reg(CtrlReg i_reg, Byte i_val);

  private:
    CPU m_cpu;
    Memory m_memory;
    PPU m_ppu;
    VideoMemory m_video_memory;

    Cartridge *m_cart;

    Byte m_ctrl_regs[LN_CTRL_REG_ADDR_TAIL - LN_CTRL_REG_ADDR_HEAD + 1];
    Controller *m_ctrls[CTRL_SIZE]; // References

    Clock m_cpu_clock;

  private:
    lnd::DebugFlags m_debug_flags;
};

} // namespace ln
