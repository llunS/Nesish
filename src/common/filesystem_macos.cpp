#include "filesystem.hpp"

#include <sys/stat.h>

namespace nh {

bool
file_exists(const std::string &i_path)
{
    struct stat st;
    return (stat(i_path.c_str(), &st) == 0) && !(st.st_mode & S_IFDIR);
}

} // namespace nh
