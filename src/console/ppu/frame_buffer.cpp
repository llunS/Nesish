#include "frame_buffer.hpp"

#include <type_traits>

#include "console/assert.hpp"

namespace ln {

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
    if (i_row < 0 || i_row >= HEIGHT || i_col < 0 || i_col >= WIDTH)
    {
        LN_ASSERT_ERROR("Framebuffer write out of boundary: {}, {}", i_row,
                        i_col);
        return;
    }
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

} // namespace ln
