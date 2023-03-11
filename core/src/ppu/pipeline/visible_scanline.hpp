#pragma once

#include "ppu/pipeline/tickable.hpp"
#include "nhbase/klass.hpp"
#include "ppu/pipeline/bg_fetch.hpp"
#include "ppu/pipeline/sp_eval_fetch.hpp"
#include "ppu/pipeline/render.hpp"

namespace nh {

struct PipelineAccessor;

struct VisibleScanline : public Tickable {
  public:
    VisibleScanline(PipelineAccessor *io_accessor);
    NB_KLZ_DELETE_COPY_MOVE(VisibleScanline);

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
