#pragma once

#include "nhbase/klass.hpp"
#include "ppu/pipeline/bg_fetch.hpp"
#include "ppu/pipeline/sp_eval_fetch.hpp"
#include "types.hpp"

namespace nh {

struct PipelineAccessor;

struct PreRenderScanline {
  public:
    PreRenderScanline(PipelineAccessor *io_accessor);
    NB_KLZ_DELETE_COPY_MOVE(PreRenderScanline);

    void
    tick(Cycle i_col);

  private:
    PipelineAccessor *m_accessor;

    BgFetch m_bg;
    SpEvalFetch m_sp;
};

} // namespace nh
