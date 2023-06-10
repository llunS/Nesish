#include "nhbase/path.hpp"

#include <Windows.h>

#include "path_private.hpp"

namespace nb {

char
path_delimiter()
{
    return '\\';
}

std::string
get_exec_dir()
{
    // add 1, not sure "MAX_PATH" include "NULL" or not.
    char path[MAX_PATH + 1];
    DWORD ret = GetModuleFileNameA(NULL, path, sizeof(path));
    if (!ret || ret == sizeof(path))
    {
        return "";
    }
    return dirname(path);
}

} // namespace nb
