#include "emulator.hpp"

namespace ln {

Emulator::Emulator()
    : m_cpu(&m_memory)
{
}

Error
Emulator::insert_cartridge(const std::string &i_rom_path)
{
    (void)(i_rom_path);
    return Error::OK;
}

Error
Emulator::power_up()
{
    return Error::OK;
}

} // namespace ln
