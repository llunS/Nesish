#include "bg_fetch_render.hpp"

#include <functional>

#include "console/types.hpp"
#include "console/ppu/pipeline_accessor.hpp"
#include "console/spec.hpp"
#include "console/assert.hpp"

namespace ln {

static void
pvt_nt_byte_fetch(PipelineAccessor *io_accessor);
static Cycle
pvt_tile_fetch(Cycle i_curr, Cycle i_total, PipelineAccessor *io_accessor);

static void
pvt_render(PipelineAccessor *io_accessor);
static void
pvt_shift_regs_shift(PipelineAccessor *io_accessor);
static void
pvt_shift_regs_reload(PipelineAccessor *io_accessor);

BgFetchRender::BgFetchRender(PipelineAccessor *io_accessor,
                             bool i_render_enabled)
    : Ticker(LN_SCANLINE_CYCLES)
    , m_accessor(io_accessor)
    , m_render_enabled(i_render_enabled)
    , m_bg_tile_fetch(8, std::bind(pvt_tile_fetch, std::placeholders::_1,
                                   std::placeholders::_2, io_accessor))
{
    m_bg_tile_fetch.set_done();
}

void
BgFetchRender::reset()
{
    Ticker::reset();

    m_bg_tile_fetch.set_done();
}

Cycle
BgFetchRender::on_tick(Cycle i_curr, Cycle i_total)
{
    (void)(i_total);

    /* @IMPL: shift registers shift should happen before shift registers reload
     */
    if (2 <= i_curr && i_curr <= 257)
    {
        /* @IMPL: render should happen before shift */
        // Render only for visible scanlines, but shift and reload happen even
        // for pre-render scanlines, because it prepares for data for the first
        // visible scanline.
        if (m_render_enabled)
        {
            // reset pixel coordinate
            if (2 == i_curr)
            {
                if (0 == m_accessor->get_context().scanline_no)
                {
                    m_accessor->get_context().pixel_row = 0;
                }
                m_accessor->get_context().pixel_col = 0;
            }
            // @TODO: backdrop color 0
            // https://www.nesdev.org/wiki/PPU_palettes#Backdrop_color_(palette_index_0)_uses
            // @TODO: left 8 pixels clipped off
            if (m_accessor->bg_enabled())
            {
                pvt_render(m_accessor);
            }
            if (i_curr == 257)
            {
                /* @IMPL: Mark dirty after rendering to the last dot */
                m_accessor->finish_frame();
            }

            // pixel coordinate advance
            if (m_accessor->get_context().pixel_col + 1 >= LN_NES_WIDTH)
            {
                ++m_accessor->get_context().pixel_col = 0;
                if (m_accessor->get_context().pixel_row + 1 >= LN_NES_HEIGHT)
                {
                    if (m_accessor->get_context().pixel_row + 1 > LN_NES_HEIGHT)
                    {
                        LN_ASSERT_FATAL("Pixel render row out of bound {}",
                                        m_accessor->get_context().pixel_row);
                    }

                    m_accessor->get_context().pixel_row = 0;
                }
                else
                {
                    ++m_accessor->get_context().pixel_row;
                }
            }
            else
            {
                ++m_accessor->get_context().pixel_col;
            }
        }

        /* shift */
        pvt_shift_regs_shift(m_accessor);
    }
    else if (322 <= i_curr && i_curr <= 337)
    {
        pvt_shift_regs_shift(m_accessor);
    }

    switch (i_curr)
    {
        case 256:
        {
            if (m_accessor->rendering_enabled())
            {
                /* increment Y component of v */
                Byte2 &v = m_accessor->get_v();
                if ((v & 0x7000) != 0x7000) // if fine Y < 7
                {
                    v += 0x1000; // increment fine Y
                }
                else
                {
                    v &= ~0x7000;                     // fine Y = 0
                    Byte y = Byte((v & 0x03E0) >> 5); // let y = coarse Y
                    if (y == 29)
                    {
                        y = 0;       // coarse Y = 0
                        v ^= 0x0800; // switch vertical nametable
                    }
                    else if (y == 31)
                    {
                        y = 0; // coarse Y = 0, nametable not switched
                    }
                    else
                    {
                        y += 1; // increment coarse Y
                    }
                    v = (v & ~0x03E0) |
                        ((Byte2)(y) << 5); // put coarse Y back into v
                }
            }
        }
        // fall through
        case 8:
        case 16:
        case 24:
        case 32:
        case 40:
        case 48:
        case 56:
        case 64:
        case 72:
        case 80:
        case 88:
        case 96:
        case 104:
        case 112:
        case 120:
        case 128:
        case 136:
        case 144:
        case 152:
        case 160:
        case 168:
        case 176:
        case 184:
        case 192:
        case 200:
        case 208:
        case 216:
        case 224:
        case 232:
        case 240:
        case 248:
        /* first two tiles on next scanline */
        case 328:
        case 336:
        {
            if (m_accessor->rendering_enabled())
            {
                /* increment X component of v */
                Byte2 &v = m_accessor->get_v();
                if ((v & 0x001F) == 31) // if coarse X == 31
                {
                    v &= ~0x001F; // coarse X = 0
                    v ^= 0x0400;  // switch horizontal nametable
                }
                else
                {
                    v += 1; // increment coarse X
                }
            }
        }
        break;

        case 257:
        {
            if (m_accessor->rendering_enabled())
            {
                /* copies all bits related to horizontal position from t to v */
                Byte2 &v = m_accessor->get_v();
                const Byte2 &t = m_accessor->get_t();
                v = (v & ~0x041F) | (t & 0x041F);
            }

            pvt_shift_regs_reload(m_accessor);
        }
        break;

        /* mysterious 2 nametable byte fetches */
        case 338:
        case 340:
        {
            pvt_nt_byte_fetch(m_accessor);
        }
        break;

        case 9:
        case 17:
        case 25:
        case 33:
        case 41:
        case 49:
        case 57:
        case 65:
        case 73:
        case 81:
        case 89:
        case 97:
        case 105:
        case 113:
        case 121:
        case 129:
        case 137:
        case 145:
        case 153:
        case 161:
        case 169:
        case 177:
        case 185:
        case 193:
        case 201:
        case 209:
        case 217:
        case 225:
        case 233:
        case 241:
        case 249:
        /* first two tiles on next scanline */
        case 329:
        case 337:
        {
            pvt_shift_regs_reload(m_accessor);
        }
        break;

        default:
            break;
    }

    /* @IMPL: shift registers reload should happen before tile fetch */
    switch (i_curr)
    {
        case 1:
        case 9:
        case 17:
        case 25:
        case 33:
        case 41:
        case 49:
        case 57:
        case 65:
        case 73:
        case 81:
        case 89:
        case 97:
        case 105:
        case 113:
        case 121:
        case 129:
        case 137:
        case 145:
        case 153:
        case 161:
        case 169:
        case 177:
        case 185:
        case 193:
        case 201:
        case 209:
        case 217:
        case 225:
        case 233:
        case 241:
        case 249:
        /* first two tiles on next scanline */
        case 321:
        case 329:
        {
            m_bg_tile_fetch.reset();
        }
        break;

        default:
            break;
    }
    m_bg_tile_fetch.tick();

    return 1;
}

void
pvt_nt_byte_fetch(PipelineAccessor *io_accessor)
{
    const Byte2 &v = io_accessor->get_v();
    Address tile_addr = 0x2000 | (v & 0x0FFF);

    Byte byte;
    auto error = io_accessor->get_memory()->get_byte(tile_addr, byte);
    if (LN_FAILED(error))
    {
        LN_ASSERT_FATAL("Failed to fetch nametable byte for bg: {}, {}", v,
                        tile_addr);
        byte = 0xFF; // set to a consistent value.
    }

    io_accessor->get_context().bg_nt_byte = byte;
}

Cycle
pvt_tile_fetch(Cycle i_curr, Cycle i_total, PipelineAccessor *io_accessor)
{
    (void)(i_total);

    /* fetch pattern table tile byte */
    auto get_pattern_sliver = [](PipelineAccessor *io_accessor,
                                 bool i_upper) -> Byte {
        bool tbl_right = io_accessor->get_register(PPU::PPUCTRL) & 0x10;
        Byte tile_idx = io_accessor->get_context().bg_nt_byte;
        Byte fine_y = Byte((io_accessor->get_v() >> 12) & 0x07);
        Address sliver_addr =
            fine_y | (i_upper << 3) | (tile_idx << 4) | (tbl_right << 12);

        Byte byte;
        auto error = io_accessor->get_memory()->get_byte(sliver_addr, byte);
        if (LN_FAILED(error))
        {
            LN_ASSERT_FATAL("Failed to fetch pattern byte for bg: {}, {}",
                            sliver_addr, i_upper);
            byte = 0xFF; // set to a consistent value.
        }

        return byte;
    };

    switch (i_curr)
    {
        case 1:
        {
            /* fetch nametable byte */
            pvt_nt_byte_fetch(io_accessor);
        }
        break;

        case 3:
        {
            const Byte2 &v = io_accessor->get_v();

            int coarse_x = v & 0x001F;
            int coarse_y = (v >> 5) & 0x001F;

            /* fetch attribute table byte */
            Address attr_addr = 0x23C0 | (v & 0x0C00) |
                                Address((coarse_y << 1) & 0x38) |
                                Address(coarse_x >> 2);
            Byte attr_byte;
            auto error =
                io_accessor->get_memory()->get_byte(attr_addr, attr_byte);
            if (LN_FAILED(error))
            {
                LN_ASSERT_FATAL("Failed to fetch attribute byte for bg: {}, {}",
                                v, attr_addr);
                attr_byte = 0xFF; // set to a consistent value.
            }

            // index of 2x2-tile block in 4x4-tile block.
            int col = coarse_x % 4 / 2;
            int row = coarse_y % 4 / 2;
            int shifts = (row * 2 + col) * 2;
            Byte attr_palette_idx = (attr_byte >> shifts) & 0x03;

            /* fetch attribute table index */
            io_accessor->get_context().bg_attr_palette_idx = attr_palette_idx;
        }
        break;

        case 5:
        {
            /* fetch pattern table tile low */
            io_accessor->get_context().bg_lower_sliver =
                get_pattern_sliver(io_accessor, false);
        }
        break;

        case 7:
        {
            /* fetch pattern table tile high */
            io_accessor->get_context().bg_upper_sliver =
                get_pattern_sliver(io_accessor, true);
        }
        break;

        default:
            break;
    }

    return 1;
}

void
pvt_render(PipelineAccessor *io_accessor)
{
    /* 1. Bit selection mask by finx X scroll */
    if (io_accessor->get_x() > 7)
    {
        LN_ASSERT_FATAL("Invalid X value: {}", io_accessor->get_x());
    }
    Byte2 bit_shift_and_mask = 0x8000 >> io_accessor->get_x();

    /* 2. Get palette idx */
    auto &ctx = io_accessor->get_context();
    bool palette_idx_lower_bit =
        ctx.shift_palette_idx_lower & bit_shift_and_mask;
    bool palette_idx_upper_bit =
        ctx.shift_palette_idx_upper & bit_shift_and_mask;
    // 2-bit
    Byte palette_idx =
        (Byte(palette_idx_upper_bit) << 1) | Byte(palette_idx_lower_bit);

    /* 3. Get pattern data (i.e. index into palette) */
    bool pattern_data_lower_bit = ctx.shift_pattern_lower & bit_shift_and_mask;
    bool pattern_data_upper_bit = ctx.shift_pattern_upper & bit_shift_and_mask;
    // 2-bit
    Byte pattern_data =
        (Byte(pattern_data_upper_bit) << 1) | Byte(pattern_data_lower_bit);

    /* 4. get palette index color */
    Address idx_color_addr =
        LN_PALETTE_ADDR_BKG_OR_MASK | (palette_idx << 2) | pattern_data;
    Byte idx_color_byte;
    auto error =
        io_accessor->get_memory()->get_byte(idx_color_addr, idx_color_byte);
    if (LN_FAILED(error))
    {
        LN_ASSERT_FATAL("Failed to fetch palette color byte for bg: {}",
                        idx_color_addr);
        idx_color_byte = 0x30; // set it to white to be apparent.
    }

    /* 5. conversion from index color to RGB color */
    Color pixel = io_accessor->get_palette().to_rgb(idx_color_byte);

    /* 6. ouput the pixel */
    // @IMPL: Actually pixel ouput is delayed by 2 cycles, but since we trigger
    // the callback on per-frame basis, write to frame buffer immediately will
    // be functionally the same.
    // https://www.nesdev.org/wiki/PPU_rendering#Cycles_1-256
    io_accessor->get_frame_buf().write(io_accessor->get_context().pixel_row,
                                       io_accessor->get_context().pixel_col,
                                       pixel);
}

void
pvt_shift_regs_shift(PipelineAccessor *io_accessor)
{
    auto &ctx = io_accessor->get_context();
    ctx.shift_pattern_lower <<= 1;
    ctx.shift_pattern_upper <<= 1;
    ctx.shift_palette_idx_lower <<= 1;
    ctx.shift_palette_idx_upper <<= 1;
}

void
pvt_shift_regs_reload(PipelineAccessor *io_accessor)
{
    auto &ctx = io_accessor->get_context();
    ctx.shift_pattern_lower =
        (ctx.shift_pattern_lower & 0xFF00) | ctx.bg_lower_sliver;
    ctx.shift_pattern_upper =
        (ctx.shift_pattern_upper & 0xFF00) | ctx.bg_upper_sliver;
    ctx.shift_palette_idx_lower =
        (ctx.shift_palette_idx_lower & 0xFF00) |
        (ctx.bg_attr_palette_idx & 0x01 ? 0xFF : 0x00);
    ctx.shift_palette_idx_upper =
        (ctx.shift_palette_idx_upper & 0xFF00) |
        (ctx.bg_attr_palette_idx & 0x02 ? 0xFF : 0x00);
}

} // namespace ln
