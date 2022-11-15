#include "emulator.hpp"

#include <type_traits>

#include "common/filesystem.hpp"
#include "common/logger.hpp"
#include "console/cartridge/cartridge_loader.hpp"
#include "console/spec.hpp"
#include "console/assert.hpp"

namespace ln {

Emulator::Emulator()
    : m_cpu(&m_memory, &m_ppu)
    , m_ppu(&m_video_memory, &m_cpu, m_debug_flags)
    , m_cart(nullptr)
    , m_ctrl_regs{}
    , m_ctrls{}
    , m_cpu_clock(LN_CPU_HZ)
    , m_debug_flags(lnd::DBG_OFF)
{
    hard_wire();
}

Emulator::~Emulator()
{
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

            Address addr =
                (i_addr & LN_PPU_REG_ADDR_MASK) | LN_PPU_REG_ADDR_HEAD;
            PPU::Register reg = PPU::Register(addr - i_entry->begin);
            o_val = ppu->read_register(reg);
            return Error::OK;
        };
        auto set = [](const MappingEntry *i_entry, Address i_addr,
                      Byte i_val) -> Error {
            PPU *ppu = (PPU *)i_entry->opaque;

            Address addr =
                (i_addr & LN_PPU_REG_ADDR_MASK) | LN_PPU_REG_ADDR_HEAD;
            PPU::Register reg = PPU::Register(addr - i_entry->begin);
            ppu->write_register(reg, i_val);
            return Error::OK;
        };
        m_memory.set_mapping(MemoryMappingPoint::PPU_REG,
                             {LN_PPU_REG_ADDR_HEAD, LN_PPU_REG_ADDR_TAIL, false,
                              get, set, &m_ppu});
    }
    // OAM DMA
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
    // Controller registers (along with the one for APU)
    {
        auto get = [](const MappingEntry *i_entry, Address i_addr,
                      Byte &o_val) -> Error {
            Emulator *thiz = (Emulator *)i_entry->opaque;

            static_assert(CtrlReg::REG_4016 == 0,
                          "CtrlReg must be consecutive and starts from 0");
            CtrlReg reg = CtrlReg(i_addr - i_entry->begin);
            o_val = thiz->read_ctrl_reg(reg);
            return Error::OK;
        };
        auto set = [](const MappingEntry *i_entry, Address i_addr,
                      Byte i_val) -> Error {
            Emulator *thiz = (Emulator *)i_entry->opaque;

            static_assert(CtrlReg::REG_4016 == 0,
                          "CtrlReg must be consecutive and starts from 0");
            CtrlReg reg = CtrlReg(i_addr - i_entry->begin);
            thiz->write_ctrl_reg(reg, i_val);
            return Error::OK;
        };
        m_memory.set_mapping(MemoryMappingPoint::APU_CTRL_REG,
                             {LN_CTRL_REG_ADDR_HEAD, LN_CTRL_REG_ADDR_TAIL,
                              false, get, set, this});
    }
}

Byte
Emulator::read_ctrl_reg(CtrlReg i_reg)
{
    auto val = m_ctrl_regs[i_reg];

    switch (i_reg)
    {
        case CtrlReg::REG_4016:
        case CtrlReg::REG_4017:
        {
            int index = i_reg - CtrlReg::REG_4016;
            if (index < 0 || index >= CTRL_SIZE)
            {
                LN_ASSERT_FATAL(
                    "Implementation error for controller register: {}", index);
            }
            else
            {
                auto ctrl = m_ctrls[index];
                /* @NOTE: We don't support parsing other bits yet */
                Byte primaryBit{0};
                if (!ctrl)
                {
                    // Repot 0 for unconnected controller.
                    // https://www.nesdev.org/wiki/Standard_controller#Output_($4016/$4017_read)
                    primaryBit = 0;
                }
                else
                {
                    primaryBit = ctrl->report();
                }
                val = (val & 0xFE) | primaryBit;
            }
        }
        break;

        default:
            break;
    }

    return val;
}

void
Emulator::write_ctrl_reg(CtrlReg i_reg, Byte i_val)
{
    m_ctrl_regs[i_reg] = i_val;

    switch (i_reg)
    {
        case CtrlReg::REG_4016:
        {
            bool strobeOn = i_val & 0x01;
            for (std::underlying_type<CtrlSlot>::type i = 0; i < CTRL_SIZE; ++i)
            {
                auto ctrl = m_ctrls[i];
                if (!ctrl)
                {
                    continue;
                }

                ctrl->strobe(strobeOn);
            }
        }
        break;

        default:
            break;
    }
}

void
Emulator::plug_controller(CtrlSlot i_slot, Controller *i_controller)
{
    m_ctrls[i_slot] = i_controller;
}

void
Emulator::unplug_controller(CtrlSlot i_slot)
{
    m_ctrls[i_slot] = nullptr;
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

const FrameBuffer &
Emulator::get_frame() const
{
    return m_ppu.get_frame();
}

void
Emulator::set_debug_on(lnd::DebugFlags i_flag)
{
    lnd::debug_on(m_debug_flags, i_flag);
}

void
Emulator::set_debug_off(lnd::DebugFlags i_flag)
{
    lnd::debug_off(m_debug_flags, i_flag);
}

Color
Emulator::get_palette_color(int i_idx) const
{
    Address color_addr = LN_PALETTE_ADDR_HEAD + i_idx;
    Byte color_byte = 0;
    // @NOTE: The address is in VRAM.
    (void)m_video_memory.get_byte(color_addr, color_byte);
    Color clr = m_ppu.get_palette().to_rgb(color_byte);
    return clr;
}

const lnd::OAM &
Emulator::get_oam() const
{
    return m_ppu.get_oam();
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
