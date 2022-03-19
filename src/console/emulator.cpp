#include "emulator.hpp"

#include "common/filesystem.hpp"
#include "common/logger.hpp"
#include "console/cartridge/cartridge_loader.hpp"

namespace ln {

Emulator::Emulator()
    : m_cpu(&m_memory)
    , m_controllers{}
{
}

void
Emulator::plug_controller(ControllerSlot i_slot, Controller *i_controller)
{
    m_controllers[i_slot].reset(i_controller);
}

Error
Emulator::insert_cartridge(const std::string &i_rom_path)
{
    if (!file_exists(i_rom_path))
    {
        get_logger()->error("Invalid cartridge path: {}", i_rom_path);
        return Error::INVALID_ARGUMENT;
    }

    Error err = Error::OK;

    // 1. load cartridge
    get_logger()->info("Loading cartridge...");
    Cartridge *cart = nullptr;
    err =
        CartridgeLoader::load_cartridge(i_rom_path, CartridgeType::INES, &cart);
    if (LN_FAILED(err))
    {
        goto l_cleanup;
    }
    err = cart->validate();
    if (LN_FAILED(err))
    {
        goto l_cleanup;
    }

    // 2. map to address space
    get_logger()->info("Mapping cartridge...");
    cart->map_memory(&m_memory);

l_cleanup:
    if (LN_FAILED(err))
    {
        delete cart;
        cart = nullptr;
    }

    return err;
}

void
Emulator::power_up()
{
    m_cpu.power_up();
    // @TODO: Other components
}

void
Emulator::reset()
{
    m_cpu.reset();
    // @TODO: Other components
}

void
Emulator::run_test(Address i_entry, TestInitFunc i_init_func,
                   TestExitFunc i_exit_func, void *i_context)
{
    m_cpu.set_entry(i_entry);

    if (i_init_func)
        i_init_func(&m_memory, i_context);

    std::size_t idx_exec_instrs = 0;
    while (!i_exit_func(&m_cpu, &m_memory, idx_exec_instrs, i_context))
    {
        if (m_cpu.step())
        {
            ++idx_exec_instrs;
        }
    }
}

} // namespace ln
