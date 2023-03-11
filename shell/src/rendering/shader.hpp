
#pragma once

#include "nhbase/klass.hpp"

#include "glad/glad.h"

namespace sh {

struct Shader {
  public:
    Shader();
    ~Shader();
    NB_KLZ_DELETE_COPY_MOVE(Shader);

    int
    compile(const char *i_vert, const char *i_frag);

    GLuint
    program() const;

  private:
    GLuint m_id;
};

} // namespace sh
