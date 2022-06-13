#include "visible_scanline.hpp"

#include "console/spec.hpp"
#include "console/ppu/pipeline_accessor.hpp"

namespace ln {

VisibleScanline::VisibleScanline(PipelineAccessor *io_accessor)
    : Ticker(LN_SCANLINE_CYCLES)
    , m_accessor(io_accessor)
    , m_bg(io_accessor, true)
{
}

void
VisibleScanline::reset()
{
    Ticker::reset();

    m_bg.reset();
}

Cycle
VisibleScanline::on_tick(Cycle i_curr, Cycle i_total)
{
    (void)(i_total);

    Cycle steps = 1;

    /* Skip 1 cycle on first scanline, if current frame is odd and rendering is
     * enabled */
    // @TESTME: Need test rom to verify this.
    if (i_curr == 0 && 0 == m_accessor->get_context().scanline_no)
    {
        bool skip = m_accessor->get_context().is_odd_frame &&
                    m_accessor->rendering_enabled();
        if (skip)
        {
            ++i_curr;
            ++steps;

            m_bg.tick();
        }
    }

    {
        m_bg.tick();
    }

    return steps;
}

} // namespace ln
