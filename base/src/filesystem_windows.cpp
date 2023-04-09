#include "nhbase/filesystem.hpp"

#include <Windows.h>

namespace nb {

bool
file_exists(const std::string &path)
{
    DWORD attr = GetFileAttributesA(path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES &&
           !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

bool
file_copy(const std::string &from, const std::string &to)
{
    return CopyFileA(from.c_str(), to.c_str(), TRUE);
}

} // namespace nb
