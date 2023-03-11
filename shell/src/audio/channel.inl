// @FIXME: Lesser constraints on stores/loads.

namespace sh {

template <unsigned int N>
Channel<N>::Channel()
    : m_buffer{}
    , m_begin(m_buffer)
    , m_end(m_buffer)
{
    static_assert(N > 0, "Invalid size.");

    // synchronize any stores performed in the constructor.
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

template <unsigned int N>
unsigned int
Channel<N>::p_size() const
{
    const value_t *b = m_begin.load(std::memory_order_seq_cst);
    const value_t *e = m_end.load(std::memory_order_seq_cst);
    return (unsigned int)(e >= b ? e - b : array_size() - (b - e));
}

template <unsigned int N>
void
Channel<N>::p_send(const value_t &i_val)
{
    value_t *end = m_end.load(std::memory_order_seq_cst);
    value_t *next = end + 1 >= m_buffer + array_size() ? m_buffer : end + 1;
    while (next == m_begin.load(std::memory_order_seq_cst))
    {
    }
    *end = i_val;
    m_end.store(next, std::memory_order_seq_cst);
}

template <unsigned int N>
bool
Channel<N>::c_try_receive(value_t &o_val)
{
    value_t *begin = m_begin.load(std::memory_order_seq_cst);
    if (begin == m_end.load(std::memory_order_seq_cst))
    {
        return false;
    }

    // @FIXME: Supposed to be the latest-written value, confirm it
    auto val = *begin;
    m_begin.store(begin + 1 >= m_buffer + array_size() ? m_buffer : begin + 1,
                  std::memory_order_seq_cst);
    o_val = val;
    return true;
}

} // namespace sh
