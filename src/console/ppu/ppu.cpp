#include "ppu.hpp"

#include "console/ppu/pipeline/pipeline.hpp"
#include "console/ppu/pipeline_accessor.hpp"
#include "console/spec.hpp"
#include "console/assert.hpp"
#include "console/cpu/cpu.hpp"

namespace ln {

PPU::PPU(VideoMemory *i_memory, CPU *i_cpu,
         const lnd::DebugFlags &i_debug_flags)
    : m_regs{}
    , m_oam{}
    , m_memory(i_memory)
    , m_cpu(i_cpu)
    , m_io_db(0)
    , m_debug_flags(i_debug_flags)
    , m_ptn_tbl_palette_idx(0)
{
    m_pipeline_accessor = new PipelineAccessor(this);
    m_pipeline = new Pipeline(m_pipeline_accessor);
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
    // @IMPL: Excluding side effects via get_register(), not sure if it's right.
    get_register(PPUCTRL) = 0x00;
    get_register(PPUMASK) = 0x00;
    get_register(PPUSTATUS) = 0xA0; // +0+x xxxx
    get_register(OAMADDR) = 0x00;
    // @NOTE: latche is cleared as well
    w = 0;
    get_register(PPUSCROLL) = 0x00;
    get_register(PPUADDR) = 0x00;
    m_ppudata_buf = 0x00;
    m_pipeline_ctx.odd_frame = false;

    // ---- Palette
    static constexpr Byte color_indices[LN_PALETTE_SIZE] = {
        /* clang-format off */
            0x09, 0x01, 0x00, 0x01, 0x00, 0x02, 0x02, 0x0D,
            0x08, 0x10, 0x08, 0x24, 0x00, 0x00, 0x04, 0x2C,
            0x09, 0x01, 0x34, 0x03, 0x00, 0x04, 0x00, 0x14,
            0x08, 0x3A, 0x00, 0x02, 0x00, 0x20, 0x2C, 0x08
        /* clang-format on */
    };
    static_assert(color_indices[LN_PALETTE_SIZE - 1], "Elements missing.");
    for (decltype(LN_PALETTE_SIZE) i = 0; i < LN_PALETTE_SIZE; ++i)
    {
        (void)m_memory->set_byte(Address(LN_PALETTE_ADDR_HEAD + i),
                                 color_indices[i]);
    }

    /* Unspecified:
     * OAM
     * Nametable RAM
     * CHR RAM
     */

    reset_internal();
}

void
PPU::reset()
{
    get_register(PPUCTRL) = 0x00;
    get_register(PPUMASK) = 0x00;
    // @NOTE: latche is cleared as well
    w = 0;
    get_register(PPUSCROLL) = 0x00;
    m_ppudata_buf = 0x00;
    m_pipeline_ctx.odd_frame = false;

    /* Unchanged:
     * OAM
     */
    /* Unchanged:
     * Palette
     * Nametable RAM
     * CHR RAM
     */

    reset_internal();
}

void
PPU::reset_internal()
{
    v = t = x = 0;

    m_pipeline->reset();

    m_io_db = 0;
}

void
PPU::tick(bool i_no_nmi)
{
    // @TODO: impl warm-up period

    m_no_nmi = i_no_nmi;

    m_pipeline->tick();

    m_no_nmi = false;
}

bool
PPU::nmi() const
{
    return (get_register(PPUCTRL) & 0x80) && (get_register(PPUSTATUS) & 0x80);
}

Byte
PPU::read_register(Register i_reg)
{
    if (reg_wrtie_only(i_reg))
    {
        // @IMPL: Open bus
        return m_io_db;
    }

    auto val = m_regs[i_reg];

    switch (i_reg)
    {
        case PPUSTATUS:
        {
            // Other 5 bits open bus behavior.
            val = (val & 0xE0) | (m_io_db & ~0xE0);

            m_regs[i_reg] &= 0x7F; // clear VBLANK flag
            this->w = 0;
            // @TODO @TEST:
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

    // fill the latch
    m_io_db = val;
    return val;
}

void
PPU::write_register(Register i_reg, Byte i_val)
{
    // fill the latch
    m_io_db = i_val;

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
        }
        break;

        case OAMDATA:
        {
            // @QUIRK: "The three unimplemented bits of each sprite's byte 2 do
            // not exist in the PPU and always read back as 0 on PPU revisions
            // that allow reading PPU OAM through OAMDATA ($2004). This can be
            // emulated by ANDing byte 2 with $E3 either when writing to or when
            // reading from OAM"
            // https://www.nesdev.org/wiki/PPU_OAM#Byte_2
            Byte oam_addr = m_regs[OAMADDR];
            if ((oam_addr & 0x03) == 0x02)
            {
                i_val &= 0xE3;
            }
            m_oam[oam_addr] = i_val;
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
                LN_ASSERT_FATAL("Failed to write PPUDATA: ${:04X}, 0x{:02X}",
                                vram_addr, i_val);
            }

            inc_vram_addr();

            // @TODO: $2007 writes
            // https://www.nesdev.org/wiki/PPU_scrolling#$2007_reads_and_writes
        }
        break;

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
    return m_palette_snap;
}

const lnd::OAM &
PPU::get_oam_dbg() const
{
    return m_oam_snap;
}

const lnd::PatternTable &
PPU::get_ptn_tbl_dbg(bool i_right) const
{
    return i_right ? m_ptn_tbl_r_snap : m_ptn_tbl_l_snap;
}

void
PPU::set_ptn_tbl_palette_dbg(unsigned char i_idx)
{
    m_ptn_tbl_palette_idx = i_idx;
}

Byte &
PPU::get_register(PPU::Register i_reg)
{
    return m_regs[i_reg];
}

const Byte &
PPU::get_register(PPU::Register i_reg) const
{
    return m_regs[i_reg];
}

void
PPU::inc_vram_addr()
{
    Byte offset = (get_register(PPUCTRL) & 0x04) ? 32 : 1;
    this->v += offset;
}

} // namespace ln
