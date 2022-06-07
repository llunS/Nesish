#include "filesystem.hpp"

#include <Windows.h>

namespace ln {

bool
file_exists(const std::string &i_path)
{
    DWORD attr = GetFileAttributesA(i_path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES &&
           !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

} // namespace ln
