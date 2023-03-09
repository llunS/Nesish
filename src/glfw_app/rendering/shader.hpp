
#pragma once

#include "common/klass.hpp"
#include "common/error.hpp"

#include "glfw_app/glad/glad.h"

namespace sh {

struct Shader {
  public:
    Shader();
    ~Shader();
    LN_KLZ_DELETE_COPY_MOVE(Shader);

    nh::Error
    compile(const char *i_vert, const char *i_frag);

    GLuint
    program() const;

  private:
    GLuint m_id;
};

} // namespace sh
