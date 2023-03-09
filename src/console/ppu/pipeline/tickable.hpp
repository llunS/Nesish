#pragma once

#include "console/types.hpp"

namespace nh {

struct Tickable {
  public:
    Tickable(Cycle i_total);
    virtual ~Tickable() = default;

    virtual void
    reset();

    bool
    done();
    void
    set_done();

    void
    tick();

    virtual Cycle
    on_tick(Cycle i_curr, Cycle i_total) = 0;

  private:
    Cycle m_curr;
    Cycle m_total;
};

} // namespace nh
