
#pragma once

#include "common/klass.hpp"
#include "common/error.hpp"

#include "glfw_app/rendering/shader.hpp"
#include "glfw_app/glad/glad.h"
#include "glfw_app/rendering/texture.hpp"

namespace ln {
struct FrameBuffer;
} // namespace ln

namespace ln_app {

/// @brief A fullscreen rect renderer for emulator frame buffer
struct Renderer {
  public:
    Renderer();
    ~Renderer();
    LN_KLZ_DELETE_COPY_MOVE(Renderer);

    ln::Error
    setup();

  public:
    void
    render(const ln::FrameBuffer &i_frame_buf);

  private:
    void
    cleanup();

  private:
    GLuint m_vbo;
    GLuint m_vao;
    Shader m_shader;

    Texture m_tex;
};

} // namespace ln_app
