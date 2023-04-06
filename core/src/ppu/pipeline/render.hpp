#pragma once

#include "ppu/pipeline/tickable.hpp"
#include "nhbase/klass.hpp"

#include <vector>
#include <utility>

namespace nh {

struct PipelineAccessor;

struct Render : public Tickable {
  public:
    Render(PipelineAccessor *io_accessor);
    NB_KLZ_DELETE_COPY_MOVE(Render);

    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;

  public:
    struct Context {
        std::vector<Byte> to_draw_sps_f;
        std::vector<Byte> to_draw_sps_b;
        std::vector<std::pair<Byte, Byte>> active_sps;
    };

  private:
    PipelineAccessor *m_accessor;
    Context m_ctx;
};

} // namespace nh
