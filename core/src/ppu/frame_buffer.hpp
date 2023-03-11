#pragma once

#include "ppu/color.hpp"
#include "nhbase/klass.hpp"
#include "spec.hpp"

namespace nh {

struct FrameBuffer {
  public:
    FrameBuffer();
    ~FrameBuffer();
    NB_KLZ_DELETE_COPY_MOVE(FrameBuffer);

    constexpr static int WIDTH = NH_NES_WIDTH;
    constexpr static int HEIGHT = NH_NES_HEIGHT;

    void
    write(int i_row, int i_col, const Color &i_clr);

    void
    swap(FrameBuffer &i_other);

    const Byte *
    get_data() const;

  private:
    Color *m_buf;
};

} // namespace nh
