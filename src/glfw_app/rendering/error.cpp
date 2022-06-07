#include "error.hpp"

#include "glfw_app/glad/glad.h"

#include <cstdio>

namespace ln_app {

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

} // namespace ln_app
