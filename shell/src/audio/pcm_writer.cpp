#include "pcm_writer.hpp"

#include "nhbase/vc_intrinsics.h"

namespace sh {

PCMWriter::PCMWriter()
    : m_file(nullptr)
    , m_buf_pos(0)
{
}

PCMWriter::~PCMWriter()
{
    (void)close();
}

int
PCMWriter::open(const std::string &i_path)
{
    if (i_path.empty())
    {
        return 1;
    }
    if (is_open())
    {
        return 1;
    }

    NB_VC_WARNING_PUSH
    NB_VC_WARNING_DISABLE(4996)
    FILE *fp = std::fopen(i_path.c_str(), "wb");
    NB_VC_WARNING_POP
    if (!fp)
    {
        return 1;
    }
    m_file = fp;
    m_buf_pos = 0;
    return 0;
}

int
PCMWriter::close()
{
    int err = 0;
    if (m_file)
    {
        // flush remaining if any
        if (m_buf_pos)
        {
            (void)std::fwrite(m_buf, 1, m_buf_pos, m_file);
        }

        err = std::fclose(m_file);
        m_file = nullptr;
    }
    else
    {
        err = 1;
    }
    return err;
}

bool
PCMWriter::is_open() const
{
    return m_file;
}

bool
PCMWriter::write_byte(unsigned char i_val)
{
    if (m_buf_pos >= BUF_SIZE)
    {
        // 256 can fit in int
        int written = (int)std::fwrite(m_buf, 1, BUF_SIZE, m_file);
        m_buf_pos -= written;
    }
    if (m_buf_pos >= BUF_SIZE)
    {
        return false;
    }

    m_buf[m_buf_pos++] = i_val;
    if (m_buf_pos >= BUF_SIZE)
    {
        // 256 can fit in int
        int written = (int)std::fwrite(m_buf, 1, BUF_SIZE, m_file);
        m_buf_pos -= written;
    }
    return true;
}

int
PCMWriter::write_s16le(short i_val)
{
    if (!write_byte((unsigned char)(i_val)))
    {
        return 0;
    }
    if (!write_byte((unsigned char)(i_val >> 8)))
    {
        return 1;
    }
    return 2;
}

} // namespace sh
