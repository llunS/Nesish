#include "pcm_writer.hpp"

namespace ln_app {

PCMWriter::PCMWriter()
    : m_file(nullptr)
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

    FILE *fp = std::fopen(i_path.c_str(), "wb");
    if (!fp)
    {
        return 1;
    }
    m_file = fp;
    return 0;
}

int
PCMWriter::close()
{
    int err = 0;
    if (m_file)
    {
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

int
PCMWriter::write_s16le(short i_val)
{
    // @TODO: buffer small chunks

    unsigned char bytes[2] = {
        (unsigned char)(i_val),
        (unsigned char)(i_val >> 8),
    };
    // 2 can fit in int.
    return int(std::fwrite(bytes, 1, 2, m_file));
}

} // namespace ln_app