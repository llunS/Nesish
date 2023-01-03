#include "noise.hpp"

#include "console/assert.hpp"

// https://www.nesdev.org/wiki/APU_Noise

namespace ln {

Noise::Noise()
    : m_shift(0)
    , m_mode(false)
{
}

Byte
Noise::amplitude() const
{
    if (m_shift & 0x0001)
    {
        return 0;
    }
    if (!m_length.value())
    {
        return 0;
    }
    return m_envel.volume();
}

void
Noise::tick_timer()
{
    if (m_timer.tick())
    {
        bool other_bit = m_mode ? (m_shift & 0x0040) : (m_shift & 0x0002);
        bool feedback = (m_shift ^ decltype(m_shift)(other_bit)) & 0x0001;
        m_shift >>= 1;
        m_shift = (m_shift & ~0x4000) | (feedback << 14);
    }
}

void
Noise::tick_envelope()
{
    m_envel.tick();
}

void
Noise::tick_length_counter()
{
    m_length.tick();
}

void
Noise::set_mode(bool i_set)
{
    m_mode = i_set;
}

void
Noise::set_timer_reload(Byte i_index)
{
    static constexpr Byte2 lookup[0x10] = {4,   8,    16,   32,  64,  96,
                                           128, 160,  202,  254, 380, 508,
                                           762, 1016, 2034, 4068};
    static_assert(lookup[0x0F], "Missing elements");

    if (i_index >= 0x10)
    {
        LN_ASSERT_FATAL("Invalid noise timer reload index: {}", i_index);
        return;
    }
    m_timer.set_reload(lookup[i_index]);
}

void
Noise::reset_lfsr()
{
    m_shift = 0;
}

Envelope &
Noise::envelope()
{
    return m_envel;
}

LengthCounter &
Noise::length_counter()
{
    return m_length;
}

} // namespace ln
