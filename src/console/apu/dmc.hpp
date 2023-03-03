#pragma once

#include "common/klass.hpp"

#include "console/apu/divider.hpp"

#include "console/types.hpp"

namespace ln {

struct DMCDMA;

struct DMC {
  public:
    DMC(DMCDMA &o_dmc_dma);
    ~DMC() = default;
    LN_KLZ_DELETE_COPY_MOVE(DMC);

  public:
    /// @return Amplitude in range of [0, 127]
    Byte
    amplitude() const;

    bool
    interrupt() const;

    void
    put_sample(Address i_sample_addr, Byte i_sample);

    /// @brief Tick this every APU cycle (2 CPU cycles)
    void
    tick_timer();

  public:
    void
    set_interrupt_enabled(bool i_set);
    void
    set_loop(bool i_set);
    void
    set_timer_reload(Byte i_index);
    void
    load(Byte i_val);
    void
    set_sample_addr(Byte i_sample_addr);
    void
    set_sample_length(Byte i_sample_length);
    void
    set_enabled(bool i_set);
    void
    clear_interrupt();
    bool
    bytes_remained() const;

  private:
    void
    restart_playback();

  private:
    DMCDMA &m_dmc_dma;

    bool m_irq_enabled;
    bool m_loop;
    Address m_sample_addr;
    Address m_sample_length;

    Divider m_timer;
    Byte m_shift;
    Byte m_bits_remaining;
    Byte m_level;
    bool m_silence;

    Byte m_sample_buffer;
    bool m_sample_buffer_empty;

    Address m_sample_curr;
    Address m_sample_bytes_left;

    bool m_irq;
};

} // namespace ln
