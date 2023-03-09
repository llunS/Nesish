#pragma once

#include "console/ppu/pipeline/tickable.hpp"
#include "common/klass.hpp"
#include "console/ppu/pipeline/bg_fetch.hpp"
#include "console/ppu/pipeline/sp_eval_fetch.hpp"
#include "console/ppu/pipeline/render.hpp"

namespace nh {

struct PipelineAccessor;

struct VisibleScanline : public Tickable {
  public:
    VisibleScanline(PipelineAccessor *io_accessor);
    LN_KLZ_DELETE_COPY_MOVE(VisibleScanline);

    void
    reset() override;
    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    PipelineAccessor *m_accessor;

    Render m_render;
    BgFetch m_bg;
    SpEvalFetch m_sp;
};

} // namespace nh
