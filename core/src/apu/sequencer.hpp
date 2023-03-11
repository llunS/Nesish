#pragma once

#include "nhbase/klass.hpp"

struct NHLogger;

namespace nh {

/// @brief Sequencer for pulse channel
struct Sequencer {
  public:
    Sequencer(NHLogger *i_logger);
    ~Sequencer() = default;
    NB_KLZ_DELETE_COPY_MOVE(Sequencer);

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

  private:
    NHLogger *m_logger;
};

} // namespace nh
