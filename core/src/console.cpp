#include "console.hpp"

#include <type_traits>

#include "nhbase/filesystem.hpp"
#include "cartridge/cartridge_loader.hpp"
#include "spec.hpp"
#include "log.hpp"
#include "assert.hpp"

namespace nh {

constexpr int Console::CTRL_SIZE;

Console::Console(NHLogger *i_logger)
    : m_cpu(&m_memory, &m_ppu, &m_apu, i_logger)
    , m_memory(i_logger)
    , m_ppu(&m_video_memory, &m_cpu, m_debug_flags, i_logger)
    , m_oam_dma(m_apu_clock, m_memory, m_ppu)
    , m_video_memory(i_logger)
    , m_apu(m_apu_clock, m_dmc_dma, i_logger)
    , m_dmc_dma(m_apu_clock, m_memory, m_apu)
    , m_cart(nullptr)
    , m_ctrl_regs{}
    , m_ctrls{}
    , m_logger(i_logger)
    , m_debug_flags(NHD_DBG_OFF)
    , m_time(0)
{
    hard_wire();
}

Console::~Console()
{
    if (m_cart)
    {
        m_cart->unmap_memory(&m_memory, &m_video_memory);
        delete m_cart;
    }
    m_cart = nullptr;
}

void
Console::hard_wire()
{
    /* PPU registers */
    {
        auto get = [](const MappingEntry *i_entry, Address i_addr,
                      Byte &o_val) -> NHErr {
            PPU *ppu = (PPU *)i_entry->opaque;

            Address addr =
                (i_addr & NH_PPU_REG_ADDR_MASK) | NH_PPU_REG_ADDR_HEAD;
            PPU::Register reg =
                PPU::Register(addr - i_entry->begin + PPU::PPUCTRL);
            o_val = ppu->read_register(reg);
            return NH_ERR_OK;
        };
        auto set = [](const MappingEntry *i_entry, Address i_addr,
                      Byte i_val) -> NHErr {
            PPU *ppu = (PPU *)i_entry->opaque;

            Address addr =
                (i_addr & NH_PPU_REG_ADDR_MASK) | NH_PPU_REG_ADDR_HEAD;
            PPU::Register reg =
                PPU::Register(addr - i_entry->begin + PPU::PPUCTRL);
            ppu->write_register(reg, i_val);
            return NH_ERR_OK;
        };
        m_memory.set_mapping(MemoryMappingPoint::PPU,
                             {NH_PPU_REG_ADDR_HEAD, NH_PPU_REG_ADDR_TAIL, false,
                              get, set, &m_ppu});
    }
    /* APU registers, OAM DMA register, Controller register */
    {
        auto get = [](const MappingEntry *i_entry, Address i_addr,
                      Byte &o_val) -> NHErr {
            Console *thiz = (Console *)i_entry->opaque;

            // OAM DMA
            if (NH_OAMDMA_ADDR == i_addr)
            {
                return NH_ERR_WRITE_ONLY;
            }
            // Controller registers.
            else if (NH_CTRL1_REG_ADDR == i_addr || NH_CTRL2_REG_ADDR == i_addr)
            {
                Byte val = thiz->read_ctrl_reg(NH_CTRL1_REG_ADDR == i_addr
                                                   ? CtrlReg::REG_4016
                                                   : CtrlReg::REG_4017);
                // Partial open bus
                // https://www.nesdev.org/wiki/Open_bus_behavior#CPU_open_bus
                o_val = (val & 0x1F) | (thiz->m_memory.get_latch() & ~0x1F);
                return NH_ERR_OK;
            }
            // Left with APU registers
            else
            {
                APU::Register reg = APU::addr_to_regsiter(i_addr);
                if (APU::Register::SIZE == reg)
                {
                    o_val = 0xFF;
                    return NH_ERR_PROGRAMMING;
                }
                // APU Status doesn't require partial open bus as it
                // seems, according to
                // cpu_exec_space/test_cpu_exec_space_apu.nes
                Byte val;
                auto err = thiz->m_apu.read_register(reg, val);
                if (NH_FAILED(err))
                {
                    return err;
                }
                o_val = val;
                return NH_ERR_OK;
            }
        };
        auto set = [](const MappingEntry *i_entry, Address i_addr,
                      Byte i_val) -> NHErr {
            Console *thiz = (Console *)i_entry->opaque;

            if (NH_OAMDMA_ADDR == i_addr)
            {
                // OAM DMA high address (this port is located on the CPU)
                thiz->m_oam_dma.initiate(i_val);
                return NH_ERR_OK;
            }
            else if (NH_CTRL1_REG_ADDR == i_addr)
            {
                thiz->write_ctrl_reg(CtrlReg::REG_4016, i_val);
                return NH_ERR_OK;
            }
            else
            {
                APU::Register reg = APU::addr_to_regsiter(i_addr);
                if (APU::Register::SIZE == reg)
                {
                    return NH_ERR_PROGRAMMING;
                }
                thiz->m_apu.write_register(reg, i_val);
                return NH_ERR_OK;
            }
        };
        m_memory.set_mapping(MemoryMappingPoint::APU_OAMDMA_CTRL,
                             {NH_APU_REG_ADDR_HEAD, NH_APU_REG_ADDR_TAIL, false,
                              get, set, this});
    }
}

Byte
Console::read_ctrl_reg(CtrlReg i_reg)
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
                NH_ASSERT_FATAL(
                    m_logger,
                    "Implementation error for controller register: {}", index);
            }
            else
            {
                auto ctrl = m_ctrls[index];
                // @TODO: Other bits
                bool primaryBit{0};
                if (!ctrl)
                {
                    // Report 0 for unconnected controller.
                    // https://www.nesdev.org/wiki/Standard_controller#Output_($4016/$4017_read)
                    primaryBit = 0;
                }
                else
                {
                    primaryBit = (bool)ctrl->report(ctrl->user);
                }
                // @NOTE: Other bits are 0 as initialized.
                val = (val & 0xFE) | Byte(primaryBit);
            }
        }
        break;

        default:
            break;
    }

    return val;
}

void
Console::write_ctrl_reg(CtrlReg i_reg, Byte i_val)
{
    m_ctrl_regs[i_reg] = i_val;

    switch (i_reg)
    {
        case CtrlReg::REG_4016:
        {
            bool strobeOn = i_val & 0x01;
            for (std::underlying_type<NHCtrlSlot>::type i = 0; i < CTRL_SIZE;
                 ++i)
            {
                auto ctrl = m_ctrls[i];
                if (!ctrl)
                {
                    continue;
                }

                ctrl->strobe(strobeOn, ctrl->user);
            }
        }
        break;

        default:
            break;
    }
}

void
Console::plug_controller(NHCtrlSlot i_slot, NHController *i_controller)
{
    m_ctrls[i_slot] = i_controller;
}

void
Console::unplug_controller(NHCtrlSlot i_slot)
{
    m_ctrls[i_slot] = nullptr;
}

NHErr
Console::insert_cartridge(const std::string &i_rom_path)
{
    if (!nb::file_exists(i_rom_path))
    {
        NH_LOG_ERROR(m_logger, "Invalid cartridge path: \"{}\"", i_rom_path);
        return NH_ERR_INVALID_ARGUMENT;
    }

    NHErr err = NH_ERR_OK;

    // 1. load cartridge
    NH_LOG_INFO(m_logger, "Loading cartridge...");
    Cartridge *cart = nullptr;
    err = CartridgeLoader::load_cartridge(i_rom_path, CartridgeType::INES,
                                          &cart, m_logger);
    if (NH_FAILED(err))
    {
        goto l_cleanup;
    }
    err = cart->validate();
    if (NH_FAILED(err))
    {
        goto l_cleanup;
    }

    // 2. map to address space
    NH_LOG_INFO(m_logger, "Mapping cartridge...");
    cart->map_memory(&m_memory, &m_video_memory);

l_cleanup:
    if (NH_FAILED(err))
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
Console::power_up()
{
    if (!m_cart)
    {
        NH_LOG_ERROR(m_logger, "Power up without cartridge inserted");
        return;
    }

    // @NOTE: Setup memory first, since other components depends on their
    // states.
    // Setup internal RAM first in case mapper changes its content
    // afterwards (if there is any). Set to a consistent RAM startup state.
    m_memory.set_bulk(NH_INTERNAL_RAM_ADDR_HEAD, NH_INTERNAL_RAM_ADDR_TAIL,
                      0xFF);
    m_cart->power_up();

    m_cpu.power_up();
    m_ppu.power_up();
    m_apu.power_up();
    m_oam_dma.power_up();
    m_dmc_dma.power_up();
    m_apu_clock.power_up();

    m_time = 0;

    reset_trivial();
}

void
Console::reset()
{
    if (!m_cart)
    {
        NH_LOG_ERROR(m_logger, "Reset without cartridge inserted");
        return;
    }

    // The internal memory was unchanged,
    // i.e. "m_memory" is not changed
    m_cart->reset();

    m_cpu.reset();
    m_ppu.reset();
    m_apu.reset();
    m_oam_dma.reset();
    m_dmc_dma.reset();
    m_apu_clock.reset();

    reset_trivial();
}

void
Console::reset_trivial()
{
    for (std::underlying_type<CtrlReg>::type i = 0; i < CtrlReg::SIZE; ++i)
    {
        m_ctrl_regs[i] = 0;
    }
    for (std::underlying_type<NHCtrlSlot>::type i = 0; i < CTRL_SIZE; ++i)
    {
        if (m_ctrls[i])
        {
            m_ctrls[i]->reset(m_ctrls[i]->user);
        }
    }
}

Cycle
Console::advance(double i_delta)
{
    double next = m_time + i_delta;
    Cycle cpu_ticks = Cycle(next * NH_CPU_HZ) - Cycle(m_time * NH_CPU_HZ);
    m_time = next;
    return cpu_ticks;
}

bool
Console::tick(bool *o_cpu_instr)
{
    // @NOTE: Tick DMA before CPU, since CPU may be halted by them
    // @NOTE: the RDY disable implementation depends on this order.
    bool dma_halt = m_cpu.dma_halt();
    bool dmc_dma_get = m_dmc_dma.tick(dma_halt);
    m_oam_dma.tick(dma_halt, dmc_dma_get);

    // NTSC version ticks PPU 3 times per CPU tick

    // @NOTE: The tick order between CPU and PPU has to do with VBL timing.
    // Order: P->C(pre)->P->C(post)->P, plus one special case of Reading $2002
    // one PPU clock before VBL is set.
    // Test rom: vbl_nmi_timing/2.vbl_timing.nes, etc.
    // https://www.nesdev.org/wiki/PPU_frame_timing#VBL_Flag_Timing
    /* Reading $2002 within a few PPU clocks of when VBL is set results in
     * special-case behavior. Reading one PPU clock before reads it as clear and
     * never sets the flag or generates NMI for that frame. Reading on the same
     * PPU clock or one later reads it as set, clears it, and suppresses the NMI
     * for that frame. Reading two or more PPU clocks before/after it's set
     * behaves normally (reads flag's value, clears it, and doesn't affect NMI
     * operation). This suppression behavior is due to the $2002 read pulling
     * the NMI line back up too quickly after it drops (NMI is active low) for
     * the CPU to see it. (CPU inputs like NMI are sampled each clock.) */
    m_ppu.tick();

    bool read_2002 = false;
    bool instr_done =
        m_cpu.pre_tick(m_dmc_dma.rdy() || m_oam_dma.rdy(), read_2002);
    if (o_cpu_instr)
    {
        *o_cpu_instr = instr_done;
    }

    m_ppu.tick(read_2002);
    m_cpu.post_tick();
    m_ppu.tick();

    // @NOTE: Tick after CPU pre_tick(), frame counter reset relys on this.
    // blargg_apu_2005.07.30/04.clock_jitter.nes
    // @NOTE: Tick after CPU pre_tick(), length counter halt delay relys on
    // this. blargg_apu_2005.07.30/10.len_halt_timing.nes
    // @NOTE: Tick after CPU pre_tick(), length counter reload during ticking
    // relys on this. blargg_apu_2005.07.30/11.len_reload_timing.nes
    // @NOTE: Tick after CPU post_tick(), according to
    // blargg_apu_2005.07.30/08.irq_timing.nes
    m_apu.tick();

    // Tick the clock last
    m_apu_clock.tick();

    // APU generates a sample every CPU cycle.
    return true;
}

double
Console::elapsed(Cycle i_ticks)
{
    return double(i_ticks) / NH_CPU_HZ; // s
}

const FrameBuffer &
Console::get_frame() const
{
    return m_ppu.get_frame();
}

float
Console::get_sample_rate() const
{
    // APU generates a sample every CPU cycle.
    return NH_CPU_HZ;
}

float
Console::get_sample() const
{
    return m_apu.amplitude();
}

void
Console::set_debug_on(NHDFlags i_flag)
{
    nhd::debug_on(m_debug_flags, i_flag);
}

void
Console::set_debug_off(NHDFlags i_flag)
{
    nhd::debug_off(m_debug_flags, i_flag);
}

const nhd::Palette &
Console::dbg_get_palette() const
{
    return m_ppu.dbg_get_palette();
}

const nhd::OAM &
Console::dbg_get_oam() const
{
    return m_ppu.dbg_get_oam();
}

const nhd::PatternTable &
Console::dbg_get_ptn_tbl(bool i_right) const
{
    return m_ppu.dbg_get_ptn_tbl(i_right);
}

void
Console::dbg_set_ptn_tbl_palette(NHDPaletteSet i_palette)
{
    m_ppu.dbg_set_ptn_tbl_palette((unsigned char)i_palette);
}

CPU *
Console::test_get_cpu()
{
    return &m_cpu;
}

} // namespace nh
