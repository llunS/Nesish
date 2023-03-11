#include "nhbase/filesystem.hpp"

#include <cstdio>

namespace nb {

bool
file_rename(const std::string &path_old, const std::string &path_new,
            bool force)
{
    if (force && file_exists(path_new))
    {
        auto err = std::remove(path_new.c_str());
        if (err)
        {
            return false;
        }
    }

    auto err = std::rename(path_old.c_str(), path_new.c_str());
    return !err;
}

} // namespace nb
