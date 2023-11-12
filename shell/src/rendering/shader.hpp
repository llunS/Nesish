
#pragma once

#include "nhbase/klass.hpp"
#include "misc/glfunc.hpp"

namespace sh
{

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
