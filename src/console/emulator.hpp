#ifndef LN_CONSOLE_EMULATOR_HPP
#define LN_CONSOLE_EMULATOR_HPP

#include "common/error.hpp"
#include "console/cpu/cpu.hpp"
#include "console/mmu.hpp"
#include "common/klass.hpp"
#include "console/dllexport.h"
#include "console/peripheral/controller.hpp"

#include <string>
#include <cstddef>
#include <memory>

namespace ln {

struct LN_CONSOLE_API Emulator {
  public:
    Emulator();
    LN_KLZ_DELETE_COPY_MOVE(Emulator);

    enum ControllerSlot {
        P1,
        P2,
        SIZE,
    };

    void
    plug_controller(ControllerSlot i_slot, Controller *i_controller);

    Error
    insert_cartridge(const std::string &i_rom_path);
    void
    power_up();
    void
    reset();

    typedef void (*TestInitFunc)(ln::MMU *i_mmu, void *i_context);
    typedef bool (*TestExitFunc)(const ln::CPU *i_cpu, const ln::MMU *i_mmu,
                                 std::size_t i_instr, void *i_context);
    void
    run_test(Address i_entry, TestInitFunc i_init_func,
             TestExitFunc i_exit_func, void *i_context);

  private:
    CPU m_cpu;
    MMU m_mmu;

    std::unique_ptr<Controller> m_controllers[ControllerSlot::SIZE];
};

} // namespace ln

#endif // LN_CONSOLE_EMULATOR_HPP
