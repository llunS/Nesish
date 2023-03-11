#pragma once

#include "ppu/pipeline/tickable.hpp"
#include "nhbase/klass.hpp"

namespace nh {

struct PipelineAccessor;

struct VBlankScanline : public Tickable {
  public:
    VBlankScanline(PipelineAccessor *io_accessor);
    NB_KLZ_DELETE_COPY_MOVE(VBlankScanline);

    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    PipelineAccessor *m_accessor;
};

} // namespace nh
