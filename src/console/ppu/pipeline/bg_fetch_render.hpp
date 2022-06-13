#pragma once

#include "console/ppu/pipeline/ticker.hpp"
#include "common/klass.hpp"
#include "console/ppu/pipeline/lambda_ticker.hpp"

namespace ln {

struct PipelineAccessor;

struct BgFetchRender : public Ticker {
  public:
    BgFetchRender(PipelineAccessor *io_accessor, bool i_render_enabled);
    LN_KLZ_DELETE_COPY_MOVE(BgFetchRender);

    void
    reset() override;
    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    PipelineAccessor *m_accessor;
    const bool m_render_enabled;

    LambdaTicker m_bg_tile_fetch;
};

} // namespace ln
