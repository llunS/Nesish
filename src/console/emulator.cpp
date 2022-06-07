#include "emulator.hpp"

#include <type_traits>

#include "common/filesystem.hpp"
#include "common/logger.hpp"
#include "console/cartridge/cartridge_loader.hpp"

namespace ln {

Emulator::Emulator()
    : m_cpu(&m_memory)
    , m_cart(nullptr)
    , m_controllers{}
{
    hard_wire();
}

Emulator::~Emulator()
{
    for (std::underlying_type<ControllerSlot>::type i = 0;
         i < ControllerSlot::SIZE; ++i)
    {
        delete m_controllers[i];
        m_controllers[i] = nullptr;
    }

    if (m_cart)
    {
        m_cart->unmap_memory(&m_memory, &m_ppu_memory);
        delete m_cart;
    }
    m_cart = nullptr;
}

void
Emulator::hard_wire()
{
    // PPU register memory mapping.
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            PPU *ppu = (PPU *)i_entry->opaque;

            PPU::Register reg = PPU::Register(i_addr - i_entry->begin);
            return &ppu->get_register(reg);
        };
        m_memory.set_mapping(
            MemoryMappingPoint::PPU_REGISTER,
            {LN_PPUCTRL_ADDR, LN_PPUDATA_ADDR, false, decode, &m_ppu});
    }
    {
        auto decode = [](const MappingEntry *i_entry,
                         Address i_addr) -> Byte * {
            (void)(i_addr);
            PPU *ppu = (PPU *)i_entry->opaque;

            return &ppu->get_register(PPU::OAMDMA);
        };
        m_memory.set_mapping(
            MemoryMappingPoint::OAMDMA,
            {LN_OAMDMA_ADDR, LN_OAMDMA_ADDR, false, decode, &m_ppu});
    }
}

void
Emulator::plug_controller(ControllerSlot i_slot, Controller *i_controller)
{
    m_controllers[i_slot] = i_controller;
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
    cart->map_memory(&m_memory, &m_ppu_memory);

l_cleanup:
    if (LN_FAILED(err))
    {
        delete cart;
        cart = nullptr;
    }
    else
    {
        if (m_cart)
        {
            m_cart->unmap_memory(&m_memory, &m_ppu_memory);
            delete m_cart;
        }
        m_cart = cart;
    }

    return err;
}

void
Emulator::power_up()
{
    m_cpu.power_up();
    m_ppu.power_up();
}

void
Emulator::reset()
{
    m_cpu.reset();
    m_ppu.reset();
}

const CPU &
Emulator::get_cpu() const
{
    return m_cpu;
}

void
Emulator::run_test(Address i_entry, TestExitFunc i_exit_func, void *i_context)
{
    m_cpu.set_entry(i_entry);

    std::size_t idx_exec_instrs = 0;
    while (i_exit_func && !i_exit_func(&m_cpu, idx_exec_instrs, i_context))
    {
        if (m_cpu.step())
        {
            ++idx_exec_instrs;
        }
    }
}

void
Emulator::tick_cpu_test()
{
    m_cpu.tick();
}

} // namespace ln
