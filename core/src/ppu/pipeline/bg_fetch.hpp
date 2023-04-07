#pragma once

#include "nhbase/klass.hpp"
#include "types.hpp"

namespace nh {

struct PipelineAccessor;

struct BgFetch {
  public:
    BgFetch(PipelineAccessor *io_accessor);
    NB_KLZ_DELETE_COPY_MOVE(BgFetch);

    void
    tick(Cycle i_col);

  private:
    PipelineAccessor *m_accessor;
};

} // namespace nh
