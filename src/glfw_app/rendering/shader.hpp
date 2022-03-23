
#ifndef LN_GLFWAPP_RENDERING_SHADER_HPP
#define LN_GLFWAPP_RENDERING_SHADER_HPP

#include "common/klass.hpp"
#include "common/error.hpp"

#include "glfw_app/glad/glad.h"

namespace ln_app {

struct Shader {
  public:
    Shader();
    ~Shader();
    LN_KLZ_DELETE_COPY_MOVE(Shader);

    ln::Error
    compile(const char *i_vert, const char *i_frag);

    GLuint
    program() const;

  private:
    GLuint m_id;
};

} // namespace ln_app

#endif // LN_GLFWAPP_RENDERING_SHADER_HPP
