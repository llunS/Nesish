#pragma once

#include "common/klass.hpp"

#include <atomic>

namespace ln_app {

template <unsigned int N> struct Channel {
  public:
    Channel();
    ~Channel() = default;
    LN_KLZ_DELETE_COPY_MOVE(Channel);

  public:
    typedef float value_t;

  public:
    unsigned int
    p_size() const;
    void
    p_send(const value_t &i_val);

    bool
    c_try_receive(value_t &o_val);

  private:
    constexpr static unsigned int
    array_size()
    {
        return N + 1;
    }

  private:
    value_t m_buffer[array_size()];
    std::atomic<value_t *> m_begin;
    std::atomic<value_t *> m_end;
};

} // namespace ln_app

#include "channel.inl"
