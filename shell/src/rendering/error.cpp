#include "error.hpp"

#include "misc/glfunc.hpp"

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
