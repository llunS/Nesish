#include "nhbase/path.hpp"

#include "nhbase/config.hpp"

#include "path_private.hpp"

namespace nb
{

char
path_delimiter()
{
    // Emscripten behaves like a variant of Unix
    return '/';
}

std::string
get_exec_dir()
{
    // A special directory located at root
    return NB_WEB_USER_DIR;
}

} // namespace nb
