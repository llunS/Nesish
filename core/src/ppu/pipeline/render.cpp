#include "render.hpp"

#include "spec.hpp"
#include "ppu/pipeline_accessor.hpp"
#include "assert.hpp"
#include "types.hpp"

#include <limits>
#include <cstring>
#include <numeric>

namespace nh {

struct OutputColor {
  public:
    constexpr OutputColor(const Color &i_clr, Byte i_pattern)
        : OutputColor(i_clr, i_pattern, false, false)
    {
    }
    constexpr OutputColor(const Color &i_clr, Byte i_pattern, bool i_priority,
                          bool i_sp_0)
        : color(i_clr)
        , pattern(i_pattern)
        , priority(i_priority)
        , sp_0(i_sp_0)
    {
    }

    Color color;
    Byte pattern;  // 2-bit;
    bool priority; // sprite only. true: behind background
    bool sp_0;     // sprite only. if this is sprite 0
};

static constexpr OutputColor ColorEmpty = {{0x00, 0x00, 0x00}, 0x00};

static OutputColor
pv_bg_render(PipelineAccessor *io_accessor);
static OutputColor
pv_sp_render(PipelineAccessor *io_accessor, Render::Context *io_ctx);
static void
pv_muxer(PipelineAccessor *io_accessor, const OutputColor &i_bg_clr,
         const OutputColor &i_sp_clr);

Render::Render(PipelineAccessor *io_accessor)
    : Tickable(NH_SCANLINE_CYCLES)
    , m_accessor(io_accessor)
{
    m_ctx.to_draw_sps_f.reserve(NH_MAX_VISIBLE_SP_NUM);
    m_ctx.to_draw_sps_b.reserve(NH_MAX_VISIBLE_SP_NUM);
    m_ctx.active_sps.reserve(NH_MAX_VISIBLE_SP_NUM);
}

Cycle
Render::on_tick(Cycle i_curr, Cycle i_total)
{
    (void)(i_total);

    if (2 <= i_curr && i_curr <= 257)
    {
        /* reset some states */
        if (2 == i_curr)
        {
            // reset pixel coordinate
            if (0 == m_accessor->get_context().scanline_no)
            {
                m_accessor->get_context().pixel_row = 0;
            }
            m_accessor->get_context().pixel_col = 0;

            m_ctx.to_draw_sps_f.clear();
            if (m_accessor->get_context().sp_count)
            {
                m_ctx.to_draw_sps_f.resize(m_accessor->get_context().sp_count);
                std::iota(m_ctx.to_draw_sps_f.begin(),
                          m_ctx.to_draw_sps_f.end(), 0);
            }
            m_ctx.to_draw_sps_b.clear();
        }

        /* Get active sprites */
        m_ctx.active_sps.clear();
        if (!m_ctx.to_draw_sps_f.empty())
        {
            // cur_x is in range [0, 256)
            Byte cur_x = Byte(i_curr - 2);
            for (const auto &sp_idx : m_ctx.to_draw_sps_f)
            {
                Byte sp_x = m_accessor->get_context().sp_pos_x[sp_idx];
                if (sp_x <= cur_x && (sp_x >= 256 - 8 || cur_x < sp_x + 8))
                {
                    Byte fine_x = cur_x - sp_x;
                    // sprite with lower index gets pushed first, which ensures
                    // correct priority among sprites.
                    m_ctx.active_sps.push_back({sp_idx, fine_x});

                    m_ctx.to_draw_sps_b.push_back(sp_idx);
                }
                else if (sp_x < 256 - 8 && cur_x >= sp_x + 8)
                {
                    // drop this sprite
                }
                else
                {
                    m_ctx.to_draw_sps_b.push_back(sp_idx);
                }
            }
            // cleanup useless slots
            m_ctx.to_draw_sps_b.swap(m_ctx.to_draw_sps_f);
            m_ctx.to_draw_sps_b.clear();
        }

        /* rendering */
        {
            // @TODO: Background palette hack
            // https://www.nesdev.org/wiki/PPU_palettes#The_background_palette_hack

            auto get_backdrop_clr = [](PipelineAccessor *io_accessor) -> Color {
                Byte backdrop_byte = 0;
                auto error = io_accessor->get_color_byte(
                    NH_PALETTE_ADDR_BG_BACKDROP, backdrop_byte);
                if (NH_FAILED(error))
                {
                    backdrop_byte = 0x30; // set it to white to be apparent.
                }
                Color backdrop =
                    io_accessor->get_palette().to_rgb(backdrop_byte);
                return backdrop;
            };

            OutputColor bg_clr = pv_bg_render(m_accessor);
            if (!m_accessor->bg_enabled() ||
                (!(m_accessor->get_context().pixel_col & ~0x07) &&
                 !(m_accessor->get_register(PPU::PPUMASK) & 0x02)))
            {
                bg_clr.color = get_backdrop_clr(m_accessor);
                bg_clr.pattern = 0;
            }

            OutputColor sp_clr = pv_sp_render(m_accessor, &m_ctx);
            // Don't draw sprites on the first visible scanline,
            // since sprite evaluation doesn't occur on pre-render scanline
            // and thus no valid data is available for rendering.
            bool render_sp = m_accessor->sp_enabled() &&
                             0 != m_accessor->get_context().scanline_no;
            if (!render_sp ||
                (!(m_accessor->get_context().pixel_col & ~0x07) &&
                 !(m_accessor->get_register(PPU::PPUMASK) & 0x04)))
            {
                sp_clr.color = get_backdrop_clr(m_accessor);
                sp_clr.pattern = 0;
            }

            pv_muxer(m_accessor, bg_clr, sp_clr);

            // Mark dirty after rendering to the last dot
            if (i_curr == 257 && 239 == m_accessor->get_context().scanline_no)
            {
                m_accessor->finish_frame();
            }
        }

        /* pixel coordinate advance */
        if (m_accessor->get_context().pixel_col + 1 >= NH_NES_WIDTH)
        {
            m_accessor->get_context().pixel_col = 0;
            if (m_accessor->get_context().pixel_row + 1 >= NH_NES_HEIGHT)
            {
                if (m_accessor->get_context().pixel_row + 1 > NH_NES_HEIGHT)
                {
                    NH_ASSERT_FATAL(m_accessor->get_logger(),
                                    "Pixel rendering row out of bound {}",
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

    return 1;
}

OutputColor
pv_bg_render(PipelineAccessor *io_accessor)
{
    /* 1. Bit selection mask by finx X scroll */
    if (io_accessor->get_x() > 7)
    {
        NH_ASSERT_FATAL(io_accessor->get_logger(),
                        "Invalid background X value: {}", io_accessor->get_x());
    }
    Byte2 bit_shift_and_mask = 0x8000 >> io_accessor->get_x();

    /* 2. Get palette index */
    auto &ctx = io_accessor->get_context();
    bool palette_idx_lower_bit =
        ctx.sf_bg_palette_idx_lower & bit_shift_and_mask;
    bool palette_idx_upper_bit =
        ctx.sf_bg_palette_idx_upper & bit_shift_and_mask;
    // 2-bit
    Byte palette_idx =
        (Byte(palette_idx_upper_bit) << 1) | Byte(palette_idx_lower_bit);

    /* 3. Get pattern data (i.e. index into palette) */
    bool pattern_data_lower_bit = ctx.sf_bg_pattern_lower & bit_shift_and_mask;
    bool pattern_data_upper_bit = ctx.sf_bg_pattern_upper & bit_shift_and_mask;
    // 2-bit
    Byte pattern_data =
        (Byte(pattern_data_upper_bit) << 1) | Byte(pattern_data_lower_bit);

    /* 4. get palette index color */
    Address idx_color_addr =
        NH_PALETTE_ADDR_BG_OR_MASK | (palette_idx << 2) | pattern_data;
    // @TODO: Background palette hack
    if ((idx_color_addr & NH_PALETTE_ADDR_BACKDROP_MASK) ==
        NH_PALETTE_ADDR_BG_BACKDROP)
    {
        idx_color_addr = NH_PALETTE_ADDR_BG_BACKDROP;
    }
    Byte idx_color_byte;
    auto error = io_accessor->get_color_byte(idx_color_addr, idx_color_byte);
    if (NH_FAILED(error))
    {
        NH_ASSERT_FATAL(io_accessor->get_logger(),
                        "Failed to fetch palette color byte for bg: {}",
                        idx_color_addr);
        idx_color_byte = 0x30; // set it to white to be apparent.
    }

    /* 5. conversion from index color to RGB color */
    Color pixel = io_accessor->get_palette().to_rgb(idx_color_byte);

    return {pixel, pattern_data};
}

OutputColor
pv_sp_render(PipelineAccessor *io_accessor, Render::Context *io_ctx)
{
    // Although the COLOR of "ColorEmpty" may be a valid value for sprite
    // 0, but the pattern member being a transparent value ensures that it won't
    // trigger sprite 0 hit. So we are safe to use this, when no sprites are
    // rendered at this pixel.
    OutputColor color = ColorEmpty;

    // search for the first non-transparent pixel among activated sprites.
    for (decltype(io_ctx->active_sps.size()) idx = 0;
         idx < io_ctx->active_sps.size(); ++idx)
    {
        auto i = io_ctx->active_sps[idx].first; // sprite index of [0, 7)
        auto fine_x = io_ctx->active_sps[idx].second;

        auto &ctx = io_accessor->get_context();
        /* 1. Bit selection mask by finx X scroll */
        // fine_x must have been in range [0, 8)
        Byte bit_shift_and_mask = 0x80 >> fine_x;

        /* 2. Get palette index */
        // 2-bit
        Byte palette_idx = ctx.sp_attr[i] & 0x03;

        /* 3. Get pattern data (i.e. index into palette) */
        // @NOTE: Flipping of both X and Y was done in the fetch stage already.
        bool pattern_data_lower_bit =
            ctx.sf_sp_pattern_lower[i] & bit_shift_and_mask;
        bool pattern_data_upper_bit =
            ctx.sf_sp_pattern_upper[i] & bit_shift_and_mask;
        // 2-bit
        Byte pattern_data =
            (Byte(pattern_data_upper_bit) << 1) | Byte(pattern_data_lower_bit);

        /* Skip transparent pixel */
        if (!pattern_data)
        {
            continue;
        }

        /* 4. get palette index color */
        Address idx_color_addr =
            NH_PALETTE_ADDR_SP_OR_MASK | (palette_idx << 2) | pattern_data;
        // @TODO: Background palette hack
        if ((idx_color_addr & NH_PALETTE_ADDR_BACKDROP_MASK) ==
            NH_PALETTE_ADDR_BG_BACKDROP)
        {
            idx_color_addr = NH_PALETTE_ADDR_BG_BACKDROP;
        }
        Byte idx_color_byte;
        auto error =
            io_accessor->get_color_byte(idx_color_addr, idx_color_byte);
        if (NH_FAILED(error))
        {
            NH_ASSERT_FATAL(io_accessor->get_logger(),
                            "Failed to fetch palette color byte for sp: {}",
                            idx_color_addr);
            idx_color_byte = 0x30; // set it to white to be apparent.
        }

        /* 5. conversion from index color to RGB color */
        Color pixel = io_accessor->get_palette().to_rgb(idx_color_byte);

        /* 6. stuff in priority, and return */
        static_assert(NH_MAX_VISIBLE_SP_NUM >= 1 &&
                          std::numeric_limits<Byte>::max() >=
                              NH_MAX_VISIBLE_SP_NUM - 1,
                      "Invalid range for sprite index");

        // If this line includes sprite 0, it must be at index 0.
        color = {pixel, pattern_data, (ctx.sp_attr[i] & 0x20) != 0,
                 i == 0 && io_accessor->get_context().with_sp0};
        // Got non-transparent pixel
        break;
    }

    return color;
}

void
pv_muxer(PipelineAccessor *io_accessor, const OutputColor &i_bg_clr,
         const OutputColor &i_sp_clr)
{
    /* Priority decision */
    // Check the decision table for details
    // https://www.nesdev.org/wiki/PPU_rendering#Preface
    Color output_clr =
        !i_sp_clr.pattern
            ? i_bg_clr.color
            : ((i_bg_clr.pattern && i_sp_clr.priority) ? i_bg_clr.color
                                                       : i_sp_clr.color);

    /* Sprite 0 hit */
    // https://www.nesdev.org/wiki/PPU_OAM#Sprite_zero_hits
    // 1. If background or sprite rendering is disabled in PPUMASK
    // ($2001)
    // 2. At x=0 to x=7 if the left-side clipping window is enabled (if
    // bit 2 or bit 1 of PPUMASK is 0)
    // 3. At x=255, for an obscure reason related to the pixel pipeline
    // 4. At any pixel where the background or sprite pixel is
    // transparent
    // 5. If sprite 0 hit has already occurred this frame
    if ((!io_accessor->bg_enabled() || !io_accessor->sp_enabled()) ||
                (((io_accessor->get_register(PPU::PPUMASK) & 0x06) != 0x06) &&
                 !(io_accessor->get_context().pixel_col & ~0x07)) ||
                io_accessor->get_context().pixel_col == 255 ||
                (!i_bg_clr.pattern || !i_sp_clr.pattern) /*||
                (io_accessor->get_register(PPU::PPUSTATUS) & 0x40)*/)
    {
    }
    else
    {
        if (i_sp_clr.sp_0 && (i_bg_clr.pattern && i_sp_clr.pattern))
        {
            // if (!(io_accessor->get_register(PPU::PPUSTATUS) & 0x40))
            {
                io_accessor->get_register(PPU::PPUSTATUS) |= 0x40;
            }
        }
    }

    // Actually pixel ouput is delayed by 2 cycles, but since we trigger
    // the callback on a per-frame basis, write to frame buffer immediately will
    // be functionally the same.
    // https://www.nesdev.org/wiki/PPU_rendering#Cycles_1-256
    io_accessor->get_frame_buf().write(io_accessor->get_context().pixel_row,
                                       io_accessor->get_context().pixel_col,
                                       output_clr);
}

} // namespace nh
