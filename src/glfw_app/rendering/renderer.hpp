
#pragma once

#include "common/klass.hpp"
#include "common/error.hpp"
#include "glfw_app/rendering/shader.hpp"

#include "glfw_app/glad/glad.h"

namespace ln_app {

struct Renderer {
  public:
    Renderer();
    ~Renderer();
    LN_KLZ_DELETE_COPY_MOVE(Renderer);

    ln::Error
    setup();

    static int
    get_width();
    static int
    get_height();

    void
    draw();

    GLuint
    texture() const;

  private:
    void
    cleanup();

  private:
    GLuint m_vbo;
    GLuint m_vao;
    Shader m_shader;
    GLuint m_fbo;
    GLuint m_texture;
};

} // namespace ln_app
