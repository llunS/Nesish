#pragma once

#include "nhbase/klass.hpp"
#include "types.hpp"

#include <vector>
#include <utility>

namespace nh {

struct PipelineAccessor;

struct Render {
  public:
    Render(PipelineAccessor *io_accessor);
    NB_KLZ_DELETE_COPY_MOVE(Render);

    void
    tick(Cycle i_col);

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
