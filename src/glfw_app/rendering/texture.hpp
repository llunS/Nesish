
#pragma once

#include "common/klass.hpp"

#include "glfw_app/glad/glad.h"

namespace nh {
struct FrameBuffer;
} // namespace nh
namespace nhd {
struct PatternTable;
struct Sprite;
} // namespace nhd

namespace sh {

/// @brief Wrap an OpenGL texture resource
struct Texture {
  public:
    Texture();
    ~Texture();
    LN_KLZ_DELETE_COPY_MOVE(Texture);

  public:
    void
    cleanup();

  public:
    bool
    from_frame(const nh::FrameBuffer &i_frame_buf);
    bool
    from_ptn_tbl(const nhd::PatternTable &i_tbl);
    bool
    from_sprite(const nhd::Sprite &i_sprite);

    int
    get_width();
    int
    get_height();

    GLuint
    texture() const;

  private:
    bool
    genTexIf(int i_width, int i_height);

  private:
    GLuint m_tex;
    int m_width;
    int m_height;
};

} // namespace sh
