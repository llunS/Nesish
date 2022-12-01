#pragma once

#include "common/klass.hpp"

#include <string>
#include <cstdio>

namespace ln_app {

struct PCMWriter {
  public:
    PCMWriter();
    ~PCMWriter();
    LN_KLZ_DELETE_COPY_MOVE(PCMWriter);

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

} // namespace ln_app
