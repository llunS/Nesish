#pragma once

#include "console/ppu/pipeline/tickable.hpp"
#include "common/klass.hpp"
#include "console/ppu/pipeline/functor_tickable.hpp"

namespace nh {

struct PipelineAccessor;

struct BgFetch : public Tickable {
  public:
    BgFetch(PipelineAccessor *io_accessor);
    LN_KLZ_DELETE_COPY_MOVE(BgFetch);

    void
    reset() override;
    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    PipelineAccessor *m_accessor;

    FunctorTickable m_bg_tile_fetch;
};

} // namespace nh
