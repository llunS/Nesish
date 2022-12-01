#include "length_counter.hpp"

#include "console/assert.hpp"

// https://www.nesdev.org/wiki/APU_Length_Counter

namespace ln {

LengthCounter::LengthCounter()
    : m_counter(0)
    , m_halt(false)
    , m_enabled(false)
{
}

Byte
LengthCounter::value() const
{
    return m_counter;
}

void
LengthCounter::tick()
{
    if (m_halt)
    {
        return;
    }
    if (!m_counter)
    {
        return;
    }
    --m_counter;
}

void
LengthCounter::set_halt(bool i_set)
{
    m_halt = i_set;
}

void
LengthCounter::set_enabled(bool i_set)
{
    m_enabled = i_set;
    if (!i_set)
    {
        m_counter = 0;
    }
}

void
LengthCounter::check_load(Byte i_index)
{
    if (!m_enabled)
    {
        return;
    }

    static constexpr Byte length_table[32] = {
        10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
        12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30};
    static_assert(length_table[31], "Elements missing.");

    if (i_index >= 32)
    {
        // This is a development-time error.
        LN_ASSERT_FATAL("Length counter index value: {}", i_index);
        return;
    }
    m_counter = length_table[i_index];
}

} // namespace ln
