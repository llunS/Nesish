#pragma once

#include "console/ppu/color.hpp"
#include "common/klass.hpp"
#include "console/spec.hpp"
#include "console/dllexport.h"

namespace nh {

struct FrameBuffer {
  public:
    FrameBuffer();
    ~FrameBuffer();
    LN_KLZ_DELETE_COPY_MOVE(FrameBuffer);

    constexpr static int WIDTH = LN_NES_WIDTH;
    constexpr static int HEIGHT = LN_NES_HEIGHT;

    void
    write(int i_row, int i_col, const Color &i_clr);

    void
    swap(FrameBuffer &i_other);

    LN_CONSOLE_API const Byte *
    get_data() const;

  private:
    Color *m_buf;
};

} // namespace nh
