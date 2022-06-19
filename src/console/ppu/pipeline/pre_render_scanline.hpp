#pragma once

#include "console/ppu/pipeline/ticker.hpp"
#include "common/klass.hpp"
#include "console/ppu/pipeline/bg_fetch.hpp"
#include "console/ppu/pipeline/sp_eval_fetch.hpp"

namespace ln {

struct PipelineAccessor;

struct PreRenderScanline : public Ticker {
  public:
    PreRenderScanline(PipelineAccessor *io_accessor);
    LN_KLZ_DELETE_COPY_MOVE(PreRenderScanline);

    void
    reset() override;
    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    PipelineAccessor *m_accessor;

    BgFetch m_bg;
    SpEvalFetch m_sp;
};

} // namespace ln
