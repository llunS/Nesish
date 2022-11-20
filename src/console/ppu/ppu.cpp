#include "ppu.hpp"

#include "console/ppu/pipeline/pipeline.hpp"
#include "console/ppu/pipeline_accessor.hpp"
#include "console/spec.hpp"
#include "console/assert.hpp"
#include "console/cpu/cpu.hpp"

namespace ln {

PPU::PPU(VideoMemory *i_memory, CPU *i_cpu,
         const lnd::DebugFlags &i_debug_flags)
    : m_ppudata_buf(0xFF)
    , m_memory(i_memory)
    , m_cpu(i_cpu)
    , m_pipeline_accessor(nullptr)
    , m_pipeline(nullptr)
    , m_debug_flags(i_debug_flags)
{
    m_pipeline_accessor = new PipelineAccessor(this);
    m_pipeline = new Pipeline(m_pipeline_accessor);

    /* init pipieline context */
    {
        m_pipeline_ctx.is_odd_frame = 0;
    }
}

PPU::~PPU()
{
    delete m_pipeline;
    m_pipeline = nullptr;
    delete m_pipeline_accessor;
    m_pipeline_accessor = nullptr;
}

void
PPU::power_up()
{
    // https://www.nesdev.org/wiki/PPU_power_up_state
    get_register(PPUCTRL) = 0x00;
    get_register(PPUMASK) = 0x00;
    get_register(PPUSTATUS) = 0xA0; // +0+x xxxx
    get_register(OAMADDR) = 0x00;
    // @NOTE: latches should be cleared as well
    get_register(PPUSCROLL) = 0x00;
    get_register(PPUADDR) = 0x00;

    // @NOTE: The palette content is unspecified according to the link above,
    // but the Nintendulator debugger shows black for them.

    m_ppudata_buf = 0x00;
}

void
PPU::reset()
{
    get_register(PPUCTRL) = 0x00;
    get_register(PPUMASK) = 0x00;
    // @NOTE: latches should be cleared as well
    get_register(PPUSCROLL) = 0x00;

    m_ppudata_buf = 0x00;
    m_pipeline_ctx.is_odd_frame = 0;
}

void
PPU::tick()
{
    // @TODO: impl warm-up period

    m_pipeline->tick();
}

Byte
PPU::read_register(Register i_reg)
{
    if (reg_wrtie_only(i_reg))
    {
        // @NOTE: We should return what's left in the bus, but we didn't emulate
        // that, so forget about it.
        return 0xFF;
    }

    auto val = m_regs[i_reg];

    switch (i_reg)
    {
        case PPUSTATUS:
        {
            m_regs[i_reg] &= 0x7F; // clear VBLANK flag
            this->w = 0;
            // @TODO/@TOTEST:
            // https://www.nesdev.org/wiki/PPU_frame_timing#VBL_Flag_Timing
        }
        break;

        case OAMDATA:
        {
            val = m_oam[m_regs[OAMADDR]];
        }
        break;

        case PPUDATA:
        {
            Address vram_addr = this->v & LN_PPU_ADDR_MASK;
            Byte tmp_val;
            auto err = m_memory->get_byte(vram_addr, tmp_val);
            if (LN_FAILED(err))
            {
                LN_ASSERT_FATAL("Failed to read PPUDATA: {}", vram_addr);
                tmp_val = 0xFF;
            }
            if (0 <= vram_addr && vram_addr <= LN_NT_MIRROR_ADDR_TAIL)
            {
                val = this->m_ppudata_buf;
                this->m_ppudata_buf = tmp_val;
            }
            else
            {
                Byte tmp_nt_val;
                Address nt_addr = vram_addr & LN_NT_MIRROR_ADDR_MASK;
                auto error = m_memory->get_byte(nt_addr, tmp_nt_val);
                if (LN_FAILED(error))
                {
                    LN_ASSERT_FATAL("Failed to read PPUDATA nametable data: {}",
                                    nt_addr);
                    tmp_nt_val = 0xFF;
                }
                val = tmp_val;
                this->m_ppudata_buf = tmp_nt_val;
            }

            inc_vram_addr();

            // @TODO: $2007 reads
            // https://www.nesdev.org/wiki/PPU_scrolling#$2007_reads_and_writes

            // @TODO: DPCM conflict
            // https://www.nesdev.org/wiki/PPU_registers#Read_conflict_with_DPCM_samples
        }
        break;

        default:
            break;
    }

    return val;
}

void
PPU::write_register(Register i_reg, Byte i_val)
{
    if (reg_read_only(i_reg))
    {
        return;
    }

    m_regs[i_reg] = i_val;

    switch (i_reg)
    {
        case PPUCTRL:
        {
            this->t = (this->t & ~0x0C00) | ((Byte2)(i_val & 0x03) << 10);

            check_gen_nmi();
        }
        break;

        case OAMDATA:
        {
            m_oam[m_regs[OAMADDR]] = i_val;
            ++m_regs[OAMADDR];

            // @QUIRK: glitchy OAMADDR update
            // We don't emulate this.
            // https://www.nesdev.org/wiki/PPU_registers#OAM_data_($2004)_%3C%3E_read/write
            // Writes to OAMDATA during rendering (on the pre-render line and
            // the visible lines 0-239, provided either sprite or background
            // rendering is enabled) do not modify values in OAM, but do perform
            // a glitchy increment of OAMADDR, bumping only the high 6 bits
            // (i.e., it bumps the [n] value in PPU sprite evaluation - it's
            // plausible that it could bump the low bits instead depending on
            // the current status of sprite evaluation). This extends to DMA
            // transfers via OAMDMA, since that uses writes to $2004. For
            // emulation purposes, it is probably best to completely ignore
            // writes during rendering.

            // @QUIRK: Un implemented bits
            // https://www.nesdev.org/wiki/PPU_OAM#Byte_2
            // We don't emulate this.
            // Not sure if we can know the byte is a attribute byte (i.e. byte
            // 2).
        }
        break;

        case PPUSCROLL:
        {
            if (!this->w)
            {
                this->t = (this->t & ~0x001F) | (i_val >> 3);
                this->x = i_val & 0x07;
            }
            else
            {
                this->t = (this->t & ~0x73E0) | ((Byte2)(i_val & 0x07) << 12) |
                          ((Byte2)(i_val & 0xF8) << 2);
            }
            this->w = !this->w;
        }
        break;

        case PPUADDR:
        {
            if (!this->w)
            {
                this->t = (this->t & ~0xFF00) | ((Byte2)(i_val & 0x3F) << 8);
            }
            else
            {
                this->t = (this->t & ~0x00FF) | (Byte2)(i_val);
                this->v = this->t;
            }
            this->w = !this->w;
        }
        break;

        case PPUMASK:
        {
            // @TODO: grayscale, emphasis, 8-column clipping
        }
        break;

        case PPUDATA:
        {
            Address vram_addr = this->v & LN_PPU_ADDR_MASK;
            auto err = m_memory->set_byte(vram_addr, i_val);
            if (LN_FAILED(err))
            {
                LN_ASSERT_FATAL("Failed to write PPUDATA: {}, {}", vram_addr,
                                i_val);
            }

            inc_vram_addr();

            // @TODO: $2007 writes
            // https://www.nesdev.org/wiki/PPU_scrolling#$2007_reads_and_writes
        }
        break;

        case OAMDMA:
        {
            m_cpu->init_oam_dma(i_val);
        }

        default:
            break;
    }
}

bool
PPU::reg_read_only(Register i_reg)
{
    switch (i_reg)
    {
        case PPUSTATUS:
            return true;
            break;

        default:
            break;
    }
    return false;
}

bool
PPU::reg_wrtie_only(Register i_reg)
{
    switch (i_reg)
    {
        case PPUCTRL:
        case PPUMASK:
        case OAMADDR:
        case PPUSCROLL:
        case PPUADDR:
        case OAMDMA:
            return true;
            break;

        default:
            break;
    }
    return false;
}

const FrameBuffer &
PPU::get_frame() const
{
    return m_front_buf;
}

const lnd::Palette &
PPU::get_palette_dbg() const
{
    return m_palette_snapshot;
}

const lnd::OAM &
PPU::get_oam_dbg() const
{
    return m_oam_snapshot;
}

Byte &
PPU::get_register(PPU::Register i_reg)
{
    return m_regs[i_reg];
}

void
PPU::inc_vram_addr()
{
    Byte offset = (get_register(PPUCTRL) & 0x04) ? 32 : 1;
    this->v += offset;
}

void
PPU::check_gen_nmi()
{
    if ((get_register(PPUCTRL) & 0x80) && (get_register(PPUSTATUS) & 0x80))
    {
        m_cpu->set_nmi(true);
    }
}

} // namespace ln
