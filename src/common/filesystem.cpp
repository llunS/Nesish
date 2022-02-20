#include "filesystem.hpp"

#include <cstdio>

namespace ln {

bool
file_rename(const std::string &i_old, const std::string &i_new)
{
    return !std::rename(i_old.c_str(), i_new.c_str());
}

} // namespace ln
