#pragma once

#include "nhbase/klass.hpp"
#include "ppu/pipeline/bg_fetch.hpp"
#include "ppu/pipeline/sp_eval_fetch.hpp"
#include "ppu/pipeline/render.hpp"
#include "types.hpp"

namespace nh {

struct PipelineAccessor;

struct VisibleScanline {
  public:
    VisibleScanline(PipelineAccessor *io_accessor);
    NB_KLZ_DELETE_COPY_MOVE(VisibleScanline);

    void
    tick(Cycle i_col);

  private:
    Render m_render;
    BgFetch m_bg;
    SpEvalFetch m_sp;
};

} // namespace nh
