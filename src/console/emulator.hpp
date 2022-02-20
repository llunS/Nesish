#ifndef LN_CONSOLE_EMULATOR_HPP
#define LN_CONSOLE_EMULATOR_HPP

#include "common/error.hpp"
#include "console/cpu/cpu.hpp"
#include "console/memory.hpp"
#include "common/klass.hpp"

#include <string>

namespace ln {

struct Emulator {
  public:
    Emulator();
    LN_KLZ_DELETE_COPY_MOVE(Emulator);

    Error
    insert_cartridge(const std::string &i_rom_path);
    Error
    power_up();
    Error
    reset();

  private:
    CPU m_cpu;
    Memory m_memory;
};

} // namespace ln

#endif // LN_CONSOLE_EMULATOR_HPP
