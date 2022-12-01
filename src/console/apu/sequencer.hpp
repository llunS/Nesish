#pragma once

#include "common/klass.hpp"

namespace ln {

struct Sequencer {
  public:
    Sequencer();
    ~Sequencer() = default;
    LN_KLZ_DELETE_COPY_MOVE(Sequencer);

  public:
    bool
    value() const;

    void
    tick();

    void
    set_duty(int i_index);
    void
    reset();

  private:
    int m_duty_idx;
    int m_seq_idx;
};

} // namespace ln
