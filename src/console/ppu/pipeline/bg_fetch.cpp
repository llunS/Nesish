#include "bg_fetch.hpp"

#include <functional>

#include "console/types.hpp"
#include "console/ppu/pipeline_accessor.hpp"
#include "console/spec.hpp"
#include "console/assert.hpp"

namespace nh {

static void
pv_nt_byte_fetch(PipelineAccessor *io_accessor);
static Cycle
pv_tile_fetch(Cycle i_curr, Cycle i_total, PipelineAccessor *io_accessor);

static void
pv_shift_regs_shift(PipelineAccessor *io_accessor);
static void
pv_shift_regs_reload(PipelineAccessor *io_accessor);

BgFetch::BgFetch(PipelineAccessor *io_accessor)
    : Tickable(LN_SCANLINE_CYCLES)
    , m_accessor(io_accessor)
    , m_bg_tile_fetch(8, std::bind(pv_tile_fetch, std::placeholders::_1,
                                   std::placeholders::_2, io_accessor))
{
    m_bg_tile_fetch.set_done();
}

void
BgFetch::reset()
{
    Tickable::reset();

    m_bg_tile_fetch.set_done();
}

Cycle
BgFetch::on_tick(Cycle i_curr, Cycle i_total)
{
    (void)(i_total);

    // @NOTE: shift registers shift should happen before shift registers reload
    if ((2 <= i_curr && i_curr <= 257) || (322 <= i_curr && i_curr <= 337))
    {
        /* shift */
        pv_shift_regs_shift(m_accessor);
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

            pv_shift_regs_reload(m_accessor);
        }
        break;

        /* mysterious 2 nametable byte fetches */
        case 338:
        case 340:
        {
            pv_nt_byte_fetch(m_accessor);
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
            pv_shift_regs_reload(m_accessor);
        }
        break;

        default:
            break;
    }

    // @NOTE: shift registers reload should happen before tile fetch
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
pv_nt_byte_fetch(PipelineAccessor *io_accessor)
{
    const Byte2 &v = io_accessor->get_v();
    Address tile_addr = 0x2000 | (v & 0x0FFF);

    Byte byte;
    auto error = io_accessor->get_memory()->get_byte(tile_addr, byte);
    if (LN_FAILED(error))
    {
        LN_ASSERT_FATAL("Failed to fetch nametable byte for bg: {}, {}", v,
                        tile_addr);
        byte = 0xFF; // set to apparent value.
    }

    io_accessor->get_context().bg_nt_byte = byte;
}

Cycle
pv_tile_fetch(Cycle i_curr, Cycle i_total, PipelineAccessor *io_accessor)
{
    (void)(i_total);

    /* fetch pattern table tile byte */
    auto get_pattern_sliver = [](PipelineAccessor *io_accessor,
                                 bool i_upper) -> Byte {
        bool tbl_right = io_accessor->get_register(PPU::PPUCTRL) & 0x10;
        Byte tile_idx = io_accessor->get_context().bg_nt_byte;
        Byte fine_y = Byte((io_accessor->get_v() >> 12) & 0x07);

        Address sliver_addr =
            io_accessor->get_sliver_addr(tbl_right, tile_idx, i_upper, fine_y);
        Byte byte;
        auto error = io_accessor->get_memory()->get_byte(sliver_addr, byte);
        if (LN_FAILED(error))
        {
            LN_ASSERT_FATAL("Failed to fetch pattern byte for bg: ${:04X}, {}",
                            sliver_addr, i_upper);
            byte = 0xFF; // set to apparent value.
        }

        return byte;
    };

    switch (i_curr)
    {
        case 1:
        {
            /* fetch nametable byte */
            pv_nt_byte_fetch(io_accessor);
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
                attr_byte = 0xFF; // set to apparent value.
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
pv_shift_regs_shift(PipelineAccessor *io_accessor)
{
    auto &ctx = io_accessor->get_context();
    ctx.sf_bg_pattern_lower <<= 1;
    ctx.sf_bg_pattern_upper <<= 1;
    ctx.sf_bg_palette_idx_lower <<= 1;
    ctx.sf_bg_palette_idx_upper <<= 1;
}

void
pv_shift_regs_reload(PipelineAccessor *io_accessor)
{
    auto &ctx = io_accessor->get_context();
    ctx.sf_bg_pattern_lower =
        (ctx.sf_bg_pattern_lower & 0xFF00) | ctx.bg_lower_sliver;
    ctx.sf_bg_pattern_upper =
        (ctx.sf_bg_pattern_upper & 0xFF00) | ctx.bg_upper_sliver;
    ctx.sf_bg_palette_idx_lower =
        (ctx.sf_bg_palette_idx_lower & 0xFF00) |
        (ctx.bg_attr_palette_idx & 0x01 ? 0xFF : 0x00);
    ctx.sf_bg_palette_idx_upper =
        (ctx.sf_bg_palette_idx_upper & 0xFF00) |
        (ctx.bg_attr_palette_idx & 0x02 ? 0xFF : 0x00);
}

} // namespace nh
