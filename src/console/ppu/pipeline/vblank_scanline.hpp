#pragma once

#include "console/ppu/pipeline/tickable.hpp"
#include "common/klass.hpp"

namespace ln {

struct PipelineAccessor;

struct VBlankScanline : public Tickable {
  public:
    VBlankScanline(PipelineAccessor *io_accessor);
    LN_KLZ_DELETE_COPY_MOVE(VBlankScanline);

    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    PipelineAccessor *m_accessor;
};

} // namespace ln
