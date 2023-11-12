#pragma once

#include "nhbase/klass.hpp"

#include <string>
#include <cstdio>

namespace sh
{

struct PCMWriter {
  public:
    PCMWriter();
    ~PCMWriter();
    NB_KLZ_DELETE_COPY_MOVE(PCMWriter);

  public:
    int
    open(const std::string &i_path);
    int
    close();

    bool
    is_open() const;

    int
    write_s16le(short i_val);

  private:
    bool
    write_byte(unsigned char i_val);

  private:
    std::FILE *m_file;

    constexpr static int BUF_SIZE = 256;
    unsigned char m_buf[BUF_SIZE];
    int m_buf_pos;
};

} // namespace sh
