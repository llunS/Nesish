#include "nhbase/filesystem.hpp"

#include <sys/stat.h>
#include <copyfile.h>

namespace nb {

bool
file_exists(const std::string &path)
{
    struct stat st;
    return (stat(path.c_str(), &st) == 0) && !(st.st_mode & S_IFDIR);
}

bool
file_copy(const std::string &from, const std::string &to)
{
    return !copyfile(from.c_str(), to.c_str(), NULL,
                     COPYFILE_ALL | COPYFILE_EXCL | COPYFILE_NOFOLLOW);
}

} // namespace nb
