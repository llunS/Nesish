#include "emulator.hpp"

#include <type_traits>

#include "common/filesystem.hpp"
#include "common/logger.hpp"
#include "console/cartridge/cartridge_loader.hpp"
#include "console/spec.hpp"

namespace ln {

Emulator::Emulator()
    : m_cpu(&m_memory, &m_ppu)
    , m_ppu(&m_video_memory, &m_cpu)
    , m_cart(nullptr)
    , m_controllers{}
    , m_cpu_clock(LN_CPU_HZ)
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
        m_cart->unmap_memory(&m_memory, &m_video_memory);
        delete m_cart;
    }
    m_cart = nullptr;
}

void
Emulator::hard_wire()
{
    // PPU registers memory mapping.
    {
        auto get = [](const MappingEntry *i_entry, Address i_addr,
                      Byte &o_val) -> Error {
            PPU *ppu = (PPU *)i_entry->opaque;

            PPU::Register reg = PPU::Register(i_addr - i_entry->begin);
            o_val = ppu->read_register(reg);
            return Error::OK;
        };
        auto set = [](const MappingEntry *i_entry, Address i_addr,
                      Byte i_val) -> Error {
            PPU *ppu = (PPU *)i_entry->opaque;

            PPU::Register reg = PPU::Register(i_addr - i_entry->begin);
            ppu->write_register(reg, i_val);
            return Error::OK;
        };
        m_memory.set_mapping(
            MemoryMappingPoint::PPU_REGISTER,
            {LN_PPUCTRL_ADDR, LN_PPUDATA_ADDR, false, get, set, &m_ppu});
    }
    {
        auto get = [](const MappingEntry *i_entry, Address i_addr,
                      Byte &o_val) -> Error {
            (void)(i_addr);
            PPU *ppu = (PPU *)i_entry->opaque;

            o_val = ppu->read_register(PPU::OAMDMA);
            return Error::OK;
        };
        auto set = [](const MappingEntry *i_entry, Address i_addr,
                      Byte i_val) -> Error {
            (void)(i_addr);
            PPU *ppu = (PPU *)i_entry->opaque;

            ppu->write_register(PPU::OAMDMA, i_val);
            return Error::OK;
        };
        m_memory.set_mapping(
            MemoryMappingPoint::OAMDMA,
            {LN_OAMDMA_ADDR, LN_OAMDMA_ADDR, false, get, set, &m_ppu});
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
        LN_LOG_ERROR(ln::get_logger(), "Invalid cartridge path: \"{}\"",
                     i_rom_path);
        return Error::INVALID_ARGUMENT;
    }

    Error err = Error::OK;

    // 1. load cartridge
    LN_LOG_INFO(ln::get_logger(), "Loading cartridge...");
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
    LN_LOG_INFO(ln::get_logger(), "Mapping cartridge...");
    cart->map_memory(&m_memory, &m_video_memory);

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
            m_cart->unmap_memory(&m_memory, &m_video_memory);
            delete m_cart;
        }
        m_cart = cart;
    }

    return err;
}

void
Emulator::power_up()
{
    if (!m_cart)
    {
        LN_LOG_ERROR(ln::get_logger(), "Power up without cartridge inserted");
        return;
    }

    m_cpu.power_up();
    m_ppu.power_up();
}

void
Emulator::reset()
{
    m_cpu.reset();
    m_ppu.reset();
}

void
Emulator::advance(Time_t i_ms)
{
    Cycle cpu_cycles = m_cpu_clock.advance(i_ms);
    for (decltype(cpu_cycles) i = 0; i < cpu_cycles; ++i)
    {
        auto instr_cycles = m_cpu.tick();
        // one instruction is complete or we just need to let other components
        // move forward.
        if (instr_cycles)
        {
            constexpr int TICK_CPU_TO_PPU = 3; // NTSC version
            for (decltype(TICK_CPU_TO_PPU * instr_cycles) j = 0;
                 j < TICK_CPU_TO_PPU * instr_cycles; ++j)
            {
                m_ppu.tick();
            }

            /* Check interrupts at the end of each instruction */
            // @IMPL: Do this after we are done emulating other components, at
            // least include PPU because it can generate NMI interrupt.
            m_cpu.poll_interrupt();
        }
    }
}

FrameBuffer *
Emulator::frame_dirty()
{
    return m_ppu.frame_dirty();
}

const CPU &
Emulator::get_cpu() const
{
    return m_cpu;
}

void
Emulator::run_test(Address i_entry, TestExitFunc i_exit_func, void *i_context)
{
    m_cpu.set_entry_test(i_entry);

    std::size_t idx_exec_instrs = 0;
    while (i_exit_func && !i_exit_func(&m_cpu, idx_exec_instrs, i_context))
    {
        if (m_cpu.step_test())
        {
            ++idx_exec_instrs;
        }
    }
}

Cycle
Emulator::tick_cpu_test()
{
    return m_cpu.tick();
}

} // namespace ln
