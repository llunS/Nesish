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
    if (m_counter > 0)
    {
        --m_counter;
        // Length reload is completely ignored if written during length
        // clocking and length counter is non-zero before clocking
        m_to_load = false;
    }
}

/// @brief Reset some states only required in implementation rather than in
/// hardware
void
LengthCounter::power_up()
{
    m_to_set_halt = false;
    m_to_load = false;
}

void
LengthCounter::post_set_halt(bool i_set)
{
    m_to_set_halt = true;
    m_halt_val = i_set;
}

void
LengthCounter::flush_halt_set()
{
    if (m_to_set_halt)
    {
        m_halt = m_halt_val;
        m_to_set_halt = false;
    }
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
LengthCounter::post_set_load(Byte i_index)
{
    m_to_load = true;
    m_load_val = i_index;
}

void
LengthCounter::flush_load_set()
{
    if (m_to_load)
    {
        check_load(m_load_val);
        m_to_load = false;
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
