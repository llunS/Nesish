#pragma once

#include "ppu/pipeline/tickable.hpp"
#include "nhbase/klass.hpp"

namespace nh {

struct PipelineAccessor;

struct Render : public Tickable {
  public:
    Render(PipelineAccessor *io_accessor);
    NB_KLZ_DELETE_COPY_MOVE(Render);

    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    PipelineAccessor *m_accessor;
};

} // namespace nh
