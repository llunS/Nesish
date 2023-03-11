#pragma once

#include "ppu/pipeline/tickable.hpp"
#include "nhbase/klass.hpp"

namespace nh {

struct IdleTicker : public Tickable {
  public:
    IdleTicker(Cycle i_total);
    NB_KLZ_DELETE_COPY_MOVE(IdleTicker);

    Cycle
    on_tick(Cycle i_curr, Cycle i_total) override;
};

} // namespace nh
