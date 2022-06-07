#include "filesystem.hpp"

#include <cstdio>

namespace ln {

bool
file_rename(const std::string &i_old, const std::string &i_new, bool i_force)
{
    if (i_force && file_exists(i_new))
    {
        auto err = std::remove(i_new.c_str());
        if (err)
        {
            return false;
        }
    }

    auto err = std::rename(i_old.c_str(), i_new.c_str());
    return !err;
}

} // namespace ln
