#include "pipeline.hpp"

#include "ppu/pipeline_accessor.hpp"
#include "spec.hpp"

#define POSTRENDER_SL_IDX 261
#define POSTRENDER_SL -1

namespace nh {

constexpr int Pipeline::SCANLINE_COUNT;

Pipeline::Pipeline(PipelineAccessor *io_accessor)
    : Tickable(1)
    , m_accessor(io_accessor)
    , m_pre_render_scanline(io_accessor)
    , m_visible_scanline(io_accessor)
    , m_post_render_scanline(NH_SCANLINE_CYCLES)
    , m_vblank_scanline(io_accessor)
    , m_scanline_sequence{}
    // Start from pre-render scanline
    // Or else ppu_vbl_nmi/ppu_vbl_nmi.nes fails,
    // even if each individual test passes. Don't know why.
    , m_curr_scanline_idx(POSTRENDER_SL_IDX)
{
    /* populate scanline sequence */
    {
        for (int i = 0; i <= 239; ++i)
        {
            m_scanline_sequence[i] = &m_visible_scanline;
        }
        m_scanline_sequence[240] = &m_post_render_scanline;
        for (int i = 241; i <= 260; ++i)
        {
            m_scanline_sequence[i] = &m_vblank_scanline;
        }
        m_scanline_sequence[261] = &m_pre_render_scanline;
    }

    reset_context();
}

void
Pipeline::reset()
{
    Tickable::reset();

    m_pre_render_scanline.reset();
    m_visible_scanline.reset();
    m_post_render_scanline.reset();
    m_vblank_scanline.reset();

    m_curr_scanline_idx = POSTRENDER_SL_IDX;

    reset_context();
}

void
Pipeline::reset_context()
{
    auto &ctx = m_accessor->get_context();
    ctx.odd_frame = false;
    ctx.skip_cycle = false;
    ctx.scanline_no = POSTRENDER_SL;
    ctx.pixel_row = ctx.pixel_col = 0;
}

Cycle
Pipeline::on_tick(Cycle i_curr, Cycle i_total)
{
    (void)(i_curr);
    (void)(i_total);

    auto &ctx = m_accessor->get_context();

    Tickable *curr = m_scanline_sequence[m_curr_scanline_idx];
    curr->tick();
    if (curr->done())
    {
        // Update index to next scanline
        if (m_curr_scanline_idx + 1 >= SCANLINE_COUNT)
        {
            m_curr_scanline_idx = 0;
        }
        else
        {
            ++m_curr_scanline_idx;
        }
        Tickable *next = m_scanline_sequence[m_curr_scanline_idx];

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

        next->reset();
    }

    return 0; // unstoppable
}

} // namespace nh
