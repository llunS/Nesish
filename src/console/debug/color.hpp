#pragma once

#include "common/klass.hpp"

namespace nhd {

struct Color {
  public:
    Color();
    ~Color() = default;
    LN_KLZ_DEFAULT_COPY(Color);

  public:
    unsigned char index;

    unsigned char r;
    unsigned char g;
    unsigned char b;
};

} // namespace nhd
