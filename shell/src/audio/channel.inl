namespace sh
{

template <typename T, unsigned int N>
Channel<T, N>::Channel()
    : m_buffer{}
    , m_begin(m_buffer)
    , m_end(m_buffer)
{
    static_assert(N > 0, "Invalid size");

    // synchronize any stores performed in the constructor.
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

template <typename T, unsigned int N>
bool
Channel<T, N>::try_send(const value_t &i_val)
{
    value_t *end = m_end.load(std::memory_order_relaxed);
    value_t *next = end + 1 >= m_buffer + array_size() ? m_buffer : end + 1;
    if (next == m_begin.load(std::memory_order_acquire)) {
        return false;
    }

    *end = i_val;
    m_end.store(next, std::memory_order_release);
    return true;
}

template <typename T, unsigned int N>
bool
Channel<T, N>::try_receive(value_t &o_val)
{
    value_t *begin = m_begin.load(std::memory_order_relaxed);
    if (begin == m_end.load(std::memory_order_acquire)) {
        return false;
    }

    auto val = *begin;
    m_begin.store(begin + 1 >= m_buffer + array_size() ? m_buffer : begin + 1,
                  std::memory_order_release);
    o_val = val;
    return true;
}

} // namespace sh
