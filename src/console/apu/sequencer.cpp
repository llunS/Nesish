#include "sequencer.hpp"

#include "console/assert.hpp"

// https://www.nesdev.org/wiki/APU_Pulse

namespace nh {

Sequencer::Sequencer()
    : m_duty_idx(0)
    , m_seq_idx(0)
{
}

bool
Sequencer::value() const
{
    static constexpr int sequences[4][8] = {{0, 1, 0, 0, 0, 0, 0, 0},
                                            {0, 1, 1, 0, 0, 0, 0, 0},
                                            {0, 1, 1, 1, 1, 0, 0, 0},
                                            {1, 0, 0, 1, 1, 1, 1, 1}};
    return !!sequences[m_duty_idx][m_seq_idx];
}

void
Sequencer::tick()
{
    if (m_seq_idx >= 7)
    {
        m_seq_idx = 0;
    }
    else
    {
        ++m_seq_idx;
    }
}

void
Sequencer::set_duty(int i_index)
{
    if (i_index > 3 || i_index < 0)
    {
        // This is a development-time error.
        LN_ASSERT_FATAL("Duty index invalid: {}", i_index);
        return;
    }
    m_duty_idx = i_index;
}

void
Sequencer::reset()
{
    m_seq_idx = 0;
}

} // namespace nh
