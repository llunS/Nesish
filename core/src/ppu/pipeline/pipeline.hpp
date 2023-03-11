#pragma once

#include "ppu/pipeline/tickable.hpp"
#include "nhbase/klass.hpp"
#include "ppu/pipeline/pre_render_scanline.hpp"
#include "ppu/pipeline/visible_scanline.hpp"
#include "ppu/pipeline/vblank_scanline.hpp"
#include "ppu/pipeline/idle_ticker.hpp"

namespace nh {

struct PipelineAccessor;

struct Pipeline : public Tickable {
  public:
    Pipeline(PipelineAccessor *io_accessor);
    NB_KLZ_DELETE_COPY_MOVE(Pipeline);

    void
    reset() override;
    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  private:
    void
    reset_context();

  private:
    PipelineAccessor *m_accessor;

    PreRenderScanline m_pre_render_scanline;
    VisibleScanline m_visible_scanline;
    IdleTicker m_post_render_scanline;
    VBlankScanline m_vblank_scanline;

    static constexpr int SCANLINE_COUNT = 262;
    Tickable *m_scanline_sequence[SCANLINE_COUNT];
    int m_curr_scanline_idx;
};

} // namespace nh
