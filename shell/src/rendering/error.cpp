#include "error.hpp"

#include "glad/glad.h"

#include <cstdio>

namespace sh {

bool
checkGLError()
{
    GLenum errorCode;
    if ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::fprintf(stderr, "GL Error: %u\n", errorCode);
        return true;
    }
    return false;
}

} // namespace sh
