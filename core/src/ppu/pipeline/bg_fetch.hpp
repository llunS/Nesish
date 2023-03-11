#pragma once

#include "ppu/pipeline/tickable.hpp"
#include "nhbase/klass.hpp"
#include "ppu/pipeline/functor_tickable.hpp"

namespace nh {

struct PipelineAccessor;

struct BgFetch : public Tickable {
  public:
    BgFetch(PipelineAccessor *io_accessor);
    NB_KLZ_DELETE_COPY_MOVE(BgFetch);

    void
    reset() override;
    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    PipelineAccessor *m_accessor;

    FunctorTickable m_bg_tile_fetch;
};

} // namespace nh
