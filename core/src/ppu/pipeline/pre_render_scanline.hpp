#pragma once

#include "ppu/pipeline/tickable.hpp"
#include "nhbase/klass.hpp"
#include "ppu/pipeline/bg_fetch.hpp"
#include "ppu/pipeline/sp_eval_fetch.hpp"

namespace nh {

struct PipelineAccessor;

struct PreRenderScanline : public Tickable {
  public:
    PreRenderScanline(PipelineAccessor *io_accessor);
    NB_KLZ_DELETE_COPY_MOVE(PreRenderScanline);

    void
    reset() override;
    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    PipelineAccessor *m_accessor;

    BgFetch m_bg;
    SpEvalFetch m_sp;
};

} // namespace nh
