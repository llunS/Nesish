#include "frame_buffer.hpp"

#include <type_traits>

namespace nh {

FrameBuffer::FrameBuffer()
{
    m_buf = new Color[WIDTH * HEIGHT]();
}

FrameBuffer::~FrameBuffer()
{
    delete[] m_buf;
}

void
FrameBuffer::write(int i_row, int i_col, const Color &i_clr)
{
    m_buf[i_row * WIDTH + i_col] = i_clr;
}

void
FrameBuffer::swap(FrameBuffer &i_other)
{
    auto tmp = this->m_buf;
    this->m_buf = i_other.m_buf;
    i_other.m_buf = tmp;
}

const Byte *
FrameBuffer::get_data() const
{
    static_assert(std::is_pod<Color>::value, "Incorrect pointer position");
    return (Byte *)(&m_buf[0]);
}

} // namespace nh
