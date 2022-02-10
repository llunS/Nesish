#include "filesystem.hpp"

#include <cstdio>

namespace ln {

bool
file_exists(const std::string &i_path)
{
    if (i_path.empty())
    {
        return false;
    }

    if (auto file = std::fopen(i_path.c_str(), "rb"))
    {
        std::fclose(file);
        return true;
    }
    else
    {
        return false;
    }
}

bool
file_rename(const std::string &i_old, const std::string &i_new)
{
    return !std::rename(i_old.c_str(), i_new.c_str());
}

} // namespace ln
