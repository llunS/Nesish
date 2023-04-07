#include "pipeline.hpp"

#include "ppu/pipeline_accessor.hpp"
#include "spec.hpp"

#define POSTRENDER_SL_IDX 261
#define POSTRENDER_SL -1

namespace nh {

static constexpr int SCANLINE_COUNT = 262;

Pipeline::Pipeline(PipelineAccessor *io_accessor)
    : m_accessor(io_accessor)
    , m_pre_render_scanline(io_accessor)
    , m_visible_scanline(io_accessor)
{
    reset();
}

void
Pipeline::reset()
{
    // Start from pre-render scanline
    // Or else ppu_vbl_nmi/ppu_vbl_nmi.nes fails,
    // even if each individual test passes. Don't know why.
    m_curr_scanline_idx = POSTRENDER_SL_IDX;
    m_curr_scanline_col = 0;

    auto &ctx = m_accessor->get_context();
    ctx.odd_frame = false;
    ctx.skip_cycle = false;
    ctx.scanline_no = POSTRENDER_SL;
    ctx.pixel_row = ctx.pixel_col = 0;
}

void
Pipeline::tick()
{
    if (0 <= m_curr_scanline_idx && m_curr_scanline_idx <= 239)
    {
        /* Skip 1 cycle on first scanline, if current frame is odd and rendering
         * is enabled */
        if (0 == m_curr_scanline_idx && 0 == m_curr_scanline_col)
        {
            if (m_accessor->get_context().skip_cycle)
            {
                advance_counter();
            }
        }

        m_visible_scanline.tick(m_curr_scanline_col);
    }
    else if (241 == m_curr_scanline_idx)
    {
        if (1 == m_curr_scanline_col)
        {
            if (!m_accessor->no_nmi())
            {
                /* Set NMI_occurred in PPU to true */
                m_accessor->get_register(PPU::PPUSTATUS) |= 0x80;
            }
        }
    }
    else if (261 == m_curr_scanline_idx)
    {
        m_pre_render_scanline.tick(m_curr_scanline_col);
    }

    advance_counter();
}

void
Pipeline::advance_counter()
{
    // Update scaline column
    if (m_curr_scanline_col + 1 >= NH_SCANLINE_CYCLES)
    {
        m_curr_scanline_col = 0;

        // Update scanline row
        if (m_curr_scanline_idx + 1 >= SCANLINE_COUNT)
        {
            m_curr_scanline_idx = 0;
        }
        else
        {
            ++m_curr_scanline_idx;
        }

        auto &ctx = m_accessor->get_context();
        // Set scanline number based on index
        ctx.scanline_no = m_curr_scanline_idx + 1 >= SCANLINE_COUNT
                              ? -1
                              : m_curr_scanline_idx;
        // New frame
        if (POSTRENDER_SL == ctx.scanline_no)
        {
            ctx.odd_frame = !ctx.odd_frame;
            ctx.skip_cycle = false;
        }
    }
    else
    {
        ++m_curr_scanline_col;
    }
}

} // namespace nh
