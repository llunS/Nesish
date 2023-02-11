#include "emulator.hpp"

#include <type_traits>

#include "common/filesystem.hpp"
#include "common/logger.hpp"
#include "console/cartridge/cartridge_loader.hpp"
#include "console/spec.hpp"
#include "console/assert.hpp"

namespace ln {

Emulator::Emulator()
    : m_cpu(&m_memory, &m_ppu, &m_apu)
    , m_ppu(&m_video_memory, &m_cpu, m_debug_flags)
    , m_cart(nullptr)
    , m_ctrl_regs{}
    , m_ctrls{}
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
    /* PPU registers */
    {
        auto get = [](const MappingEntry *i_entry, Address i_addr,
                      Byte &o_val) -> Error {
            PPU *ppu = (PPU *)i_entry->opaque;

            Address addr =
                (i_addr & LN_PPU_REG_ADDR_MASK) | LN_PPU_REG_ADDR_HEAD;
            PPU::Register reg =
                PPU::Register(addr - i_entry->begin + PPU::PPUCTRL);
            o_val = ppu->read_register(reg);
            return Error::OK;
        };
        auto set = [](const MappingEntry *i_entry, Address i_addr,
                      Byte i_val) -> Error {
            PPU *ppu = (PPU *)i_entry->opaque;

            Address addr =
                (i_addr & LN_PPU_REG_ADDR_MASK) | LN_PPU_REG_ADDR_HEAD;
            PPU::Register reg =
                PPU::Register(addr - i_entry->begin + PPU::PPUCTRL);
            ppu->write_register(reg, i_val);
            return Error::OK;
        };
        m_memory.set_mapping(MemoryMappingPoint::PPU,
                             {LN_PPU_REG_ADDR_HEAD, LN_PPU_REG_ADDR_TAIL, false,
                              get, set, &m_ppu});
    }
    /* APU registers, OAM DMA register, Controller register */
    {
        auto get = [](const MappingEntry *i_entry, Address i_addr,
                      Byte &o_val) -> Error {
            Emulator *thiz = (Emulator *)i_entry->opaque;

            // OAM DMA
            if (LN_OAMDMA_ADDR == i_addr)
            {
                return Error::WRITE_ONLY;
            }
            // Controller registers.
            else if (LN_CTRL1_REG_ADDR == i_addr || LN_CTRL2_REG_ADDR == i_addr)
            {
                Byte val = thiz->read_ctrl_reg(LN_CTRL1_REG_ADDR == i_addr
                                                   ? CtrlReg::REG_4016
                                                   : CtrlReg::REG_4017);
                // @IMPL: Partial open bus
                // https://www.nesdev.org/wiki/Open_bus_behavior#CPU_open_bus
                o_val = (val & 0x1F) | (thiz->m_memory.get_latch() & ~0x1F);
                return Error::OK;
            }
            // Left with APU registers
            else
            {
                APU::Register reg = APU::addr_to_regsiter(i_addr);
                if (APU::Register::SIZE == reg)
                {
                    o_val = 0xFF;
                    return Error::PROGRAMMING;
                }
                // @IMPL: APU Status doesn't require partial open bus as it
                // seems according to cpu_exec_space/test_cpu_exec_space_apu.nes
                Byte val;
                auto err = thiz->m_apu.read_register(reg, val);
                if (LN_FAILED(err))
                {
                    return err;
                }
                o_val = val;
                return Error::OK;
            }
        };
        auto set = [](const MappingEntry *i_entry, Address i_addr,
                      Byte i_val) -> Error {
            Emulator *thiz = (Emulator *)i_entry->opaque;

            if (LN_OAMDMA_ADDR == i_addr)
            {
                // OAM DMA high address (this port is located on the CPU)
                thiz->m_cpu.init_oam_dma(i_val);
                return Error::OK;
            }
            else if (LN_CTRL1_REG_ADDR == i_addr)
            {
                thiz->write_ctrl_reg(CtrlReg::REG_4016, i_val);
                return Error::OK;
            }
            else
            {
                APU::Register reg = APU::addr_to_regsiter(i_addr);
                if (APU::Register::SIZE == reg)
                {
                    return Error::PROGRAMMING;
                }
                thiz->m_apu.write_register(reg, i_val);
                return Error::OK;
            }
        };
        m_memory.set_mapping(MemoryMappingPoint::APU_OAMDMA_CTRL,
                             {LN_APU_REG_ADDR_HEAD, LN_APU_REG_ADDR_TAIL, false,
                              get, set, this});
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
                // @IMPL: Other bits are 0 as initialized.
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
    // consistent RAM startup state
    m_memory.set_bulk(LN_INTERNAL_RAM_ADDR_HEAD, LN_INTERNAL_RAM_ADDR_TAIL,
                      0xFF);
    m_ppu.power_up();
    m_apu.power_up();
    m_cart->power_up();

    reset_internal();
}

void
Emulator::reset()
{
    if (!m_cart)
    {
        LN_LOG_ERROR(ln::get_logger(), "Reset without cartridge inserted");
        return;
    }

    m_cpu.reset();
    // The internal memory was unchanged
    // i.e. "m_memory" is not changed
    m_ppu.reset();
    m_apu.reset();
    m_cart->reset();

    reset_internal();
}

void
Emulator::reset_internal()
{
    for (std::underlying_type<CtrlReg>::type i = 0; i < CtrlReg::SIZE; ++i)
    {
        m_ctrl_regs[i] = 0;
    }
    for (std::underlying_type<CtrlSlot>::type i = 0; i < CTRL_SIZE; ++i)
    {
        if (m_ctrls[i])
        {
            m_ctrls[i]->reset();
        }
    }
}

Time_t
Emulator::elapsed(Cycle i_ticks)
{
    return Time_t(i_ticks) / LN_CPU_HZ; // s
}

Cycle
Emulator::ticks(Time_t i_duration)
{
    return Cycle(LN_CPU_HZ * i_duration);
}

bool
Emulator::tick(bool *o_cpu_instr)
{
    constexpr int cpu_ticks = 1;
    bool instr_done = false;
    if (m_apu.fetching())
    {
        // Stall CPU.
    }
    else
    {
        instr_done = m_cpu.tick();
    }
    if (o_cpu_instr)
    {
        *o_cpu_instr = instr_done;
    }

    // @NOTE: Emulate PPU after CPU.
    constexpr int TICK_CPU_TO_PPU = 3; // NTSC version
    for (decltype(TICK_CPU_TO_PPU * cpu_ticks) j = 0;
         j < TICK_CPU_TO_PPU * cpu_ticks; ++j)
    {
        m_ppu.tick();
    }

    m_apu.tick(m_memory);
    // APU generates a sample every CPU cycle.
    return true;
}

const FrameBuffer &
Emulator::get_frame() const
{
    return m_ppu.get_frame();
}

float
Emulator::get_sample_rate() const
{
    // APU generates a sample every CPU cycle.
    return LN_CPU_HZ;
}

float
Emulator::get_sample() const
{
    return m_apu.amplitude();
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

const lnd::Palette &
Emulator::get_palette_dbg() const
{
    return m_ppu.get_palette_dbg();
}

const lnd::OAM &
Emulator::get_oam_dbg() const
{
    return m_ppu.get_oam_dbg();
}

const lnd::PatternTable &
Emulator::get_ptn_tbl_dbg(bool i_right) const
{
    return m_ppu.get_ptn_tbl_dbg(i_right);
}

void
Emulator::set_ptn_tbl_palette_dbg(PaletteSet i_palette)
{
    m_ppu.set_ptn_tbl_palette_dbg((unsigned char)i_palette);
}

const CPU &
Emulator::get_cpu_test() const
{
    return m_cpu;
}

void
Emulator::init_test(TestInitFunc i_init_func, void *i_context)
{
    i_init_func(&m_cpu, i_context);
}

} // namespace ln
