#pragma once

#include "nhbase/klass.hpp"

#include <atomic>

namespace sh {

template <typename T, unsigned int N> struct Channel {
  public:
    Channel();
    ~Channel() = default;
    NB_KLZ_DELETE_COPY_MOVE(Channel);

  public:
    typedef T value_t;

  public:
    bool
    try_send(const value_t &i_val);

    bool
    try_receive(value_t &o_val);

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

} // namespace sh

#include "channel.inl"
