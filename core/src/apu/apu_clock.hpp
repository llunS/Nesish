#pragma once

#include "nhbase/klass.hpp"

#include "types.hpp"

namespace nh {

/// @brief A struct to sync get/put cycle for APU and DMA(OAM/DMC)
struct APUClock {
  public:
    APUClock();
    ~APUClock() = default;
    NB_KLZ_DELETE_COPY_MOVE(APUClock);

  public:
    void
    tick();

    void
    power_up();
    void
    reset();

    bool
    get() const;
    bool
    put() const;

    bool
    even() const;
    bool
    odd() const;

  private:
    // This may wrap around back to 0, which is fine, since current
    // implementation doesn't assume infinite range.
    Cycle m_cycle;
};

} // namespace nh
