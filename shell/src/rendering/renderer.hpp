
#pragma once

#include "nesish/nesish.h"
#include "nhbase/klass.hpp"

#include "rendering/shader.hpp"
#include "glad/glad.h"
#include "rendering/texture.hpp"

namespace sh {

/// @brief A fullscreen rect renderer for emulator frame buffer
struct Renderer {
  public:
    Renderer();
    ~Renderer();
    NB_KLZ_DELETE_COPY_MOVE(Renderer);

    int
    setup();

  public:
    void
    render(NHFrame i_frame_buf);

  private:
    void
    cleanup();

  private:
    GLuint m_vbo;
    GLuint m_vao;
    Shader m_shader;

    Texture m_tex;
};

} // namespace sh
