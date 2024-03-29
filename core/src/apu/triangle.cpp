#include "triangle.hpp"

// https://www.nesdev.org/wiki/APU_Triangle

#define SEQ_SIZE 32

namespace nh {

Triangle::Triangle(NHLogger *i_logger)
    : m_length(i_logger)
{
}

Byte
Triangle::amplitude() const
{
    return m_amp;
}

void
Triangle::power_up()
{
    m_length.power_up();

    m_amp = 0;
    m_seq_idx = 0;
}

void
Triangle::reset()
{
    m_seq_idx = 0;
}

void
Triangle::tick_timer()
{
    if (m_timer.tick())
    {
        if (m_linear.value() && m_length.value())
        {
            static constexpr Byte sequences[SEQ_SIZE] = {
                15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5,  4,  3,  2,  1,  0,
                0,  1,  2,  3,  4,  5,  6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
            static_assert(sequences[SEQ_SIZE - 1], "Missing elements");
            m_amp = sequences[m_seq_idx];

            if (m_seq_idx >= SEQ_SIZE - 1)
            {
                m_seq_idx = 0;
            }
            else
            {
                ++m_seq_idx;
            }
        }
    }
}

void
Triangle::tick_linear_counter()
{
    m_linear.tick();
}

void
Triangle::tick_length_counter()
{
    m_length.tick();
}

Divider &
Triangle::timer()
{
    return m_timer;
}

LinearCounter &
Triangle::linear_counter()
{
    return m_linear;
}

LengthCounter &
Triangle::length_counter()
{
    return m_length;
}

} // namespace nh
