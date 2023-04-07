#pragma once

#include "nhbase/klass.hpp"
#include "ppu/pipeline/pre_render_scanline.hpp"
#include "ppu/pipeline/visible_scanline.hpp"

namespace nh {

struct PipelineAccessor;

struct Pipeline {
  public:
    Pipeline(PipelineAccessor *io_accessor);
    NB_KLZ_DELETE_COPY_MOVE(Pipeline);

    void
    reset();

    void
    tick();

  private:
    void
    advance_counter();

  private:
    PipelineAccessor *m_accessor;

    PreRenderScanline m_pre_render_scanline;
    VisibleScanline m_visible_scanline;

    int m_curr_scanline_idx;
    int m_curr_scanline_col;
};

} // namespace nh
