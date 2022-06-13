#pragma once

#include "console/ppu/pipeline/ticker.hpp"
#include "common/klass.hpp"
#include "console/ppu/pipeline/bg_fetch_render.hpp"

namespace ln {

struct PipelineAccessor;

struct VisibleScanline : public Ticker {
  public:
    VisibleScanline(PipelineAccessor *io_accessor);
    LN_KLZ_DELETE_COPY_MOVE(VisibleScanline);

    void
    reset() override;
    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    PipelineAccessor *m_accessor;

    BgFetchRender m_bg;
};

} // namespace ln
