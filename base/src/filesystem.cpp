#include "nhbase/filesystem.hpp"

#include <cstdio>

namespace nb
{

bool
file_rename(const std::string &from, const std::string &to, bool force)
{
    if (force && file_exists(to)) {
        auto err = std::remove(to.c_str());
        if (err) {
            return false;
        }
    }

    auto err = std::rename(from.c_str(), to.c_str());
    return !err;
}

} // namespace nb
