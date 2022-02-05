#ifndef LN_CONSOLE_EMULATOR_HPP
#define LN_CONSOLE_EMULATOR_HPP

#include "common/error.hpp"
#include "console/cpu.hpp"
#include "console/memory.hpp"

#include <string>

namespace ln {

struct Emulator {
  public:
    Emulator();

    Error
    insert_cartridge(const std::string &i_rom_path);
    Error
    power_up();

  private:
    CPU m_cpu;
    Memory m_memory;
};

} // namespace ln

#endif // LN_CONSOLE_EMULATOR_HPP
