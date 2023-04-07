#include "visible_scanline.hpp"

#include "spec.hpp"
#include "ppu/pipeline_accessor.hpp"

namespace nh {

VisibleScanline::VisibleScanline(PipelineAccessor *io_accessor)
    : m_render(io_accessor)
    , m_bg(io_accessor)
    , m_sp(io_accessor)
{
}

void
VisibleScanline::tick(Cycle i_col)
{
    // Rendering happens before other data priming workload
    if (2 <= i_col && i_col <= 257)
    {
        m_render.tick(i_col);
    }
    if ((1 <= i_col && i_col <= 257) || (321 <= i_col && i_col <= 340))
    {
        m_bg.tick(i_col);
    }
    if (1 <= i_col && i_col <= 320)
    {
        m_sp.tick(i_col);
    }
}

} // namespace nh
