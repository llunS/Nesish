#include "nhbase/filesystem.hpp"

#include <sys/stat.h>

namespace nb
{

bool
file_exists(const std::string &path)
{
    struct stat st;
    return (stat(path.c_str(), &st) == 0) && !(st.st_mode & S_IFDIR);
}

} // namespace nb
