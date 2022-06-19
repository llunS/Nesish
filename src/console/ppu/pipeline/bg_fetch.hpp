#pragma once

#include "console/ppu/pipeline/ticker.hpp"
#include "common/klass.hpp"
#include "console/ppu/pipeline/lambda_ticker.hpp"

namespace ln {

struct PipelineAccessor;

struct BgFetch : public Ticker {
  public:
    BgFetch(PipelineAccessor *io_accessor);
    LN_KLZ_DELETE_COPY_MOVE(BgFetch);

    void
    reset() override;
    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    PipelineAccessor *m_accessor;

    LambdaTicker m_bg_tile_fetch;
};

} // namespace ln
