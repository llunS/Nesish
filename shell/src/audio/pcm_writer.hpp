#pragma once

#include "nhbase/klass.hpp"

#include <string>
#include <cstdio>

namespace sh {

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
    std::FILE *m_file;
};

} // namespace sh
