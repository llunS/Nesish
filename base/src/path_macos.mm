#include "nhbase/path.hpp"

#include <stdlib.h>

#include <mach-o/dyld.h>
#include <sys/syslimits.h>

#include "path_private.hpp"

namespace nb {

char
path_delimiter()
{
    return '/';
}

std::string
get_exec_path()
{
    char path[PATH_MAX + 1];
    uint32_t size = sizeof path;
    if (_NSGetExecutablePath(path, &size) == 0)
    {
        // The realpath() function resolves all symbolic links, extra ``/''
        // charac-ters, characters, and references to /./ and /../ in file_name
        char abs_path[PATH_MAX + 1];
        if (realpath(path, abs_path))
        {
            return abs_path;
        }
    }

    // failure
    return "";
}

} // namespace nb
