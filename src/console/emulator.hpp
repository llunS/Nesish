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

#include <string>
#include <cstddef>

namespace ln {

struct Emulator {
  public:
    LN_CONSOLE_API
    Emulator();
    LN_CONSOLE_API ~Emulator();
    LN_KLZ_DELETE_COPY_MOVE(Emulator);

    enum ControllerSlot {
        P1,
        P2,
        SIZE,
    };

    LN_CONSOLE_API void
    plug_controller(ControllerSlot i_slot, Controller *i_controller);

    LN_CONSOLE_API Error
    insert_cartridge(const std::string &i_rom_path);
    LN_CONSOLE_API void
    power_up();
    LN_CONSOLE_API void
    reset();

    LN_CONSOLE_API void
    advance(Time_t i_ms);

    LN_CONSOLE_API FrameBuffer *
    frame_dirty();

    /* query */

    LN_CONSOLE_API const CPU &
    get_cpu() const;

    /* test */

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
    CPU m_cpu;
    Memory m_memory;
    PPU m_ppu;
    VideoMemory m_video_memory;

    Cartridge *m_cart;

    Controller *m_controllers[ControllerSlot::SIZE];

    Clock m_cpu_clock;
};

} // namespace ln
