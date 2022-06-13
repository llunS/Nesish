#include "pipeline.hpp"

#include "console/ppu/pipeline_accessor.hpp"
#include "console/spec.hpp"

namespace ln {

constexpr int Pipeline::SCANLINE_COUNT;

Pipeline::Pipeline(PipelineAccessor *io_accessor)
    : Ticker(1)
    , m_accessor(io_accessor)
    , m_pre_render_scanline(io_accessor)
    , m_visible_scanline(io_accessor)
    , m_post_render_scanline(LN_SCANLINE_CYCLES)
    , m_vblank_scanline(io_accessor)
    , m_scanline_sequence{}
    , m_curr_scanline_idx(0)
{
    /* populate scanline sequence */
    {
        m_scanline_sequence[0] = &m_pre_render_scanline;
        for (int i = 1; i <= 240; ++i)
        {
            m_scanline_sequence[i] = &m_visible_scanline;
        }
        m_scanline_sequence[241] = &m_post_render_scanline;
        for (int i = 242; i <= 261; ++i)
        {
            m_scanline_sequence[i] = &m_vblank_scanline;
        }
    }

    reset_context();
}

void
Pipeline::reset()
{
    Ticker::reset();

    m_pre_render_scanline.reset();
    m_visible_scanline.reset();
    m_post_render_scanline.reset();
    m_vblank_scanline.reset();

    m_curr_scanline_idx = 0;

    reset_context();
}

void
Pipeline::reset_context()
{
    auto &ctx = m_accessor->get_context();
    ctx.is_odd_frame = false;
    ctx.scanline_no = -1;
}

Cycle
Pipeline::on_tick(Cycle i_curr, Cycle i_total)
{
    (void)(i_curr);
    (void)(i_total);

    auto &ctx = m_accessor->get_context();

    Ticker *curr = m_scanline_sequence[m_curr_scanline_idx];
    curr->tick();
    if (curr->done())
    {
        // one frame
        if (m_curr_scanline_idx + 1 >= SCANLINE_COUNT)
        {
            m_curr_scanline_idx = 0;

            ctx.is_odd_frame = !ctx.is_odd_frame;
        }
        else
        {
            ++m_curr_scanline_idx;
        }
        ctx.scanline_no = m_curr_scanline_idx + 1 >= SCANLINE_COUNT
                              ? -1
                              : m_curr_scanline_idx - 1;

        Ticker *next = m_scanline_sequence[m_curr_scanline_idx];
        next->reset();
    }

    return 0; // unstoppable
}

} // namespace ln
