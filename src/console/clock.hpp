#pragma once

#include <cstddef>

#include "common/klass.hpp"
#include "common/time.hpp"

namespace ln {

typedef std::size_t Cycle;
typedef double Hz_t;

struct Clock {
  public:
    Clock(Hz_t i_frequency);
    LN_KLZ_DELETE_COPY_MOVE(Clock);

    Cycle
    advance(Time_t i_ms);

  private:
    Time_t m_cycle;
    Time_t m_time;
};

} // namespace ln
