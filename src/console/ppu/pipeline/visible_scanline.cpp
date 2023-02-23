#include "visible_scanline.hpp"

#include "console/spec.hpp"
#include "console/ppu/pipeline_accessor.hpp"

namespace ln {

VisibleScanline::VisibleScanline(PipelineAccessor *io_accessor)
    : Tickable(LN_SCANLINE_CYCLES)
    , m_accessor(io_accessor)
    , m_render(io_accessor)
    , m_bg(io_accessor)
    , m_sp(io_accessor, false)
{
}

void
VisibleScanline::reset()
{
    Tickable::reset();

    m_render.reset();
    m_bg.reset();
    m_sp.reset();
}

Cycle
VisibleScanline::on_tick(Cycle i_curr, Cycle i_total)
{
    (void)(i_total);

    auto tick_logic = [](VisibleScanline *thiz) -> void {
        /* @IMPL: rendering happens before other data priming workload */
        thiz->m_render.tick();
        thiz->m_bg.tick();
        thiz->m_sp.tick();
    };

    Cycle steps = 1;

    /* Skip 1 cycle on first scanline, if current frame is odd and rendering is
     * enabled */
    if (i_curr == 0 && 0 == m_accessor->get_context().scanline_no)
    {
        if (m_accessor->get_context().skip_cycle)
        {
            // Advance to counter by skip count
            ++steps;
            tick_logic(this);
        }
    }

    {
        tick_logic(this);
    }

    return steps;
}

} // namespace ln
