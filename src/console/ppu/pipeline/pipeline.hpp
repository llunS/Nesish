#pragma once

#include "console/ppu/pipeline/tickable.hpp"
#include "common/klass.hpp"
#include "console/ppu/pipeline/pre_render_scanline.hpp"
#include "console/ppu/pipeline/visible_scanline.hpp"
#include "console/ppu/pipeline/vblank_scanline.hpp"
#include "console/ppu/pipeline/idle_ticker.hpp"

namespace ln {

struct PipelineAccessor;

struct Pipeline : public Tickable {
  public:
    Pipeline(PipelineAccessor *io_accessor);
    LN_KLZ_DELETE_COPY_MOVE(Pipeline);

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

} // namespace ln
