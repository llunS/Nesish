#include "path.hpp"

#include <Windows.h>

#include "common/path_private.hpp"

namespace ln {

char
path_delimiter()
{
    return '\\';
}

std::string
get_exec_path()
{
    // add 1, not sure "MAX_PATH" include "NULL" or not.
    char path[MAX_PATH + 1];
    DWORD ret = GetModuleFileNameA(NULL, path, sizeof(path));
    if (!ret || ret == sizeof(path))
    {
        return "";
    }
    return path;
}

} // namespace ln
