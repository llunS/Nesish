#include "nhbase/path.hpp"

#include <unistd.h>
#include <limits.h>

#include "path_private.hpp"

namespace nb {

char
path_delimiter()
{
    return '/';
}

std::string
get_exec_dir()
{
    char path[PATH_MAX + 1];
    ssize_t size = readlink("/proc/self/exe", path, PATH_MAX);
    if (size > 0)
    {
        path[size] = 0;
        return dirname(path);
    }

    // failure
    return "";
}

} // namespace nb
