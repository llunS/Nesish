
#pragma once

#include "nhbase/klass.hpp"
#include "nesish/nesish.h"
#include "glad/glad.h"

namespace sh {

/// @brief Wrap an OpenGL texture resource
struct Texture {
  public:
    Texture();
    ~Texture();
    NB_KLZ_DELETE_COPY_MOVE(Texture);

  public:
    void
    cleanup();

  public:
    bool
    from_frame(NHFrame i_frame);
    bool
    from_ptn_tbl(NHDPatternTable i_tbl);
    bool
    from_sprite(NHDSprite i_sprite);

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
