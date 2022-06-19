#pragma once

#include "console/ppu/pipeline/ticker.hpp"
#include "common/klass.hpp"

namespace ln {

struct PipelineAccessor;

struct Render : public Ticker {
  public:
    Render(PipelineAccessor *io_accessor);
    LN_KLZ_DELETE_COPY_MOVE(Render);

    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    PipelineAccessor *m_accessor;
};

} // namespace ln